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

// EnergyPlus::HVACDXSystem and VariableSpeedCoils Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Coils/CoilCoolingDX.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimAirServingZones.hh>
#include <EnergyPlus/UnitarySystem.hh>
#include <EnergyPlus/VariableSpeedCoils.hh>

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, VariableSpeedCoils_DOASDXCoilTest)
{
    // issue #6040
    std::string const idf_objects = delimited_string({

        "  Schedule:Compact,",
        "    FANANDCOILAVAILSCHED,    !- Name",
        "    FRACTION,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 19",
        "    For: Alldays,            !- Field 20",
        "    Until: 24:00,1.00;       !- Field 21",

        "  CoilSystem:Cooling:DX,",
        "    DX Cooling Coil System,  !- Name",
        "    FanAndCoilAvailSched,    !- Availability Schedule Name",
        "    DX Cooling Coil Air Inlet Node,  !- DX Cooling Coil System Inlet Node Name",
        "    Heating Coil Air Inlet Node,  !- DX Cooling Coil System Outlet Node Name",
        "    Heating Coil Air Inlet Node,  !- DX Cooling Coil System Sensor Node Name",
        "    Coil:Cooling:DX:VariableSpeed,!- Cooling Coil Object Type",
        "    VS DX Cooling Coil,      !- Cooling Coil Name",
        "    None,                    !- Dehumidification Control Type",
        "    yes,                     !- Run on Sensible Load",
        "    No,                      !- Run on Latent Load",
        "    Yes;                     !- Use Outdoor Air DX Cooling Coil",

        "  Coil:Cooling:DX:VariableSpeed,",
        "    VS DX Cooling Coil,              !- Name",
        "    ,                        !- Availability Schedule Name",
        "    DX Cooling Coil Air Inlet Node,  !- Air Inlet Node Name",
        "    Heating Coil Air Inlet Node,     !- Air Outlet Node Name",
        "    5,                       !- Number of Speeds {dimensionless}",
        "    5,                       !- Nominal Speed Level {dimensionless}",
        "    autosize,                !- Rated Total Cooling Capacity At Selected Nominal Speed Level {w}",
        "    autosize,                !- Rated Volumetric Air Flow Rate At Selected Nominal Speed Level {m3/s}",
        "    0,                       !- Nominal Time for Condensate to Begin Leaving the Coil {s}",
        "    0,                       !- Initial Moisture Evaporation Rate Divided by Steady-State AC Latent Capacity {dimensionless}",
        "    ,                        !- Maximum Cycling Rate",
        "    ,                        !- Latent Capacity Time Constant",
        "    ,                        !- Fan Delay Time",
        "    PLF Curve MultiComp 30% unloading, !- Energy Part Load Fraction Curve Name",
        "    ,                        !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "    ,                        !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10,                      !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    2,                       !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    33861.72,                !- Speed 1 Reference Unit Total Cooling Capacity At Rated Conditions {w}",
        "    0.70,                    !- Speed 1 Reference Unit Sensible Heat Ratio At Rated Conditions {dimensionless}",
        "    4.328973,                !- Speed 1 Reference Unit COP At Rated Conditions {dimensionless}",
        "    1.396964,                !- Speed 1 Reference Unit Air Flow Rate At Rated Conditions {m3/s}",
        "    773.3,                   !- Speed 1 2017 Rated Evaporator Fan Power Per Volume Flow Rate",
        "    934.4,                   !- Speed 1 2023 Rated Evaporator Fan Power Per Volume Flow Rate",
        "    ,                        !- Speed 1 Reference Unit Condenser Flow Rate at Rated Conditions {m3/s}",
        "    ,                        !- Speed 1 Reference Unit Pad Effectiveness of Evap Precooling at Rated Conditions {dimensionless}",
        "    5CapacityCurveIpakCS,    !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "    CAPFF Curve IpakCS,      !- Speed 1 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    5PowerCurveIpakCS,       !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFF Curve IpakCS,      !- Speed 1 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    35516.08,                !- Speed 2 Reference Unit Total Cooling Capacity At Rated Conditions {w}",
        "    0.78,                    !- Speed 2 Reference Unit Sensible Heat Ratio At Rated Conditions {dimensionless}",
        "    4.540061,                !- Speed 2 Reference Unit COP At Rated Conditions {dimensionless}",
        "    1.88779,                 !- Speed 2 Reference Unit Air Flow Rate At Rated Conditions {m3/s}",
        "    773.3,                   !- Speed 2 2017 Rated Evaporator Fan Power Per Volume Flow Rate",
        "    934.4,                   !- Speed 2 2023 Rated Evaporator Fan Power Per Volume Flow Rate",
        "    ,                        !- Speed 2 Reference Unit Condenser Flow Rate at Rated Conditions {m3/s}",
        "    ,                        !- Speed 2 Reference Unit Pad Effectiveness of Evap Precooling at Rated Conditions {dimensionless}",
        "    4CapacityCurve,          !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "    CAPFF Curve IpakCS,      !- Speed 2 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    4PowerCurveIpakCS,       !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFF Curve IpakCS,      !- Speed 2 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    65133.17,                !- Speed 3 Reference Unit Total Cooling Capacity At Rated Conditions {w}",
        "    0.70,                    !- Speed 3 Reference Unit Sensible Heat Ratio At Rated Conditions {dimensionless}",
        "    4.164418,                !- Speed 3 Reference Unit COP At Rated Conditions {dimensionless}",
        "    2.831685,                !- Speed 3 Reference Unit Air Flow Rate At Rated Conditions {m3/s}",
        "    773.3,                   !- Speed 3 2017 Rated Evaporator Fan Power Per Volume Flow Rate",
        "    934.4,                   !- Speed 3 2023 Rated Evaporator Fan Power Per Volume Flow Rate",
        "    ,                        !- Speed 3 Reference Unit Condenser Flow Rate at Rated Conditions {m3/s}",
        "    ,                        !- Speed 3 Reference Unit Pad Effectiveness of Evap Precooling at Rated Conditions {dimensionless}",
        "    3CapacityCurveIpakCS,    !- Speed 3 Total Cooling Capacity Function of Temperature Curve Name",
        "    CAPFF Curve IpakCS,      !- Speed 3 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    3PowerCurveIpakCS,       !- Speed 3 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFF Curve IpakCS,      !- Speed 3 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    119583.3,                !- Speed 4 Reference Unit Total Cooling Capacity At Rated Conditions {w}",
        "    0.62,                    !- Speed 4 Reference Unit Sensible Heat Ratio At Rated Conditions {dimensionless}",
        "    3.469661,                !- Speed 4 Reference Unit COP At Rated Conditions {dimensionless}",
        "    3.553764,                !- Speed 4 Reference Unit Air Flow Rate At Rated Conditions {m3/s}",
        "    773.3,                   !- Speed 4 2017 Rated Evaporator Fan Power Per Volume Flow Rate",
        "    934.4,                   !- Speed 4 2023 Rated Evaporator Fan Power Per Volume Flow Rate",
        "    ,                        !- Speed 4 Reference Unit Condenser Flow Rate at Rated Conditions {m3/s}",
        "    ,                        !- Speed 4 Reference Unit Pad Effectiveness of Evap Precooling at Rated Conditions {dimensionless}",
        "    2CapacityCurveIpakCS,    !- Speed 4 Total Cooling Capacity Function of Temperature Curve Name",
        "    CAPFF Curve IpakCS,      !- Speed 4 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    2PowerCurveIpakCS ,      !- Speed 4 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFF Curve IpakCS,      !- Speed 4 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    132769.7,                !- Speed 5 Reference Unit Total Cooling Capacity At Rated Conditions {w}",
        "    0.69,                    !- Speed 5 Reference Unit Sensible Heat Ratio At Rated Conditions {dimensionless}",
        "    3.822957,                !- Speed 5 Reference Unit COP At Rated Conditions {dimensionless}",
        "    5.66336932,              !- Speed 5 Reference Unit Air Flow Rate At Rated Conditions {m3/s}",
        "    773.3,                   !- Speed 5 2017 Rated Evaporator Fan Power Per Volume Flow Rate",
        "    934.4,                   !- Speed 5 2023 Rated Evaporator Fan Power Per Volume Flow Rate",
        "    ,                        !- Speed 5 Reference Unit Condenser Flow Rate at Rated Conditions {m3/s}",
        "    ,                        !- Speed 5 Reference Unit Pad Effectiveness of Evap Precooling at Rated Conditions {dimensionless}",
        "    1CapacityCurveIpakCS,    !- Speed 5 Total Cooling Capacity Function of Temperature Curve Name",
        "    CAPFF Curve IpakCS,      !- Speed 5 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    1PowerCurveIpakCS,       !- Speed 5 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFF Curve IpakCS;      !- Speed 5 Energy Input Ratio Function of Air Flow Fraction Curve Name",

        "Curve:Quadratic,",
        "    PLF Curve MultiComp 30% unloading,  !- Name",
        "    0.85,                    !- Coefficient1 Constant",
        "    0.8333,                  !- Coefficient2 x",
        "    0.0,                     !- Coefficient3 x**2",
        "    0.0,                     !- Minimum Value of x",
        "    0.3,                     !- Maximum Value of x",
        "    0.85,                    !- Minimum Curve Output",
        "    1.0,                     !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "Curve:Biquadratic,",
        "    4CapacityCurve,          !- Name",
        "    1.16,                    !- Coefficient1 Constant",
        "    -.0155,                  !- Coefficient2 x",
        "    0.00128,                 !- Coefficient3 x**2",
        "    -.00673,                 !- Coefficient4 y",
        "    0.0000797,               !- Coefficient5 y**2",
        "    -.000305,                !- Coefficient6 x*y",
        "    10,                      !- Minimum Value of x",
        "    23.88889,                !- Maximum Value of x",
        "    23.88889,                !- Minimum Value of y",
        "    40.55556,                !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless,           !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Curve:Cubic,",
        "    CAPFF Curve IpakCS,      !- Name",
        "    1,                       !- Coefficient1 Constant",
        "    0,                       !- Coefficient2 x",
        "    0,                       !- Coefficient3 x**2",
        "    0,                       !- Coefficient4 x**3",
        "    0,                       !- Minimum Value of x",
        "    1,                       !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "Curve:Cubic,",
        "    EIRFF Curve IpakCS,      !- Name",
        "    1,                       !- Coefficient1 Constant",
        "    0,                       !- Coefficient2 x",
        "    0,                       !- Coefficient3 x**",
        "    0,                       !- Coefficient4 x**3",
        "    0,                       !- Minimum Value of x",
        "    1,                       !- Maximum Value of x",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless;           !- Output Unit Type",

        "Curve:Biquadratic,",
        "    1CapacityCurveIpakCS,    !- Name",
        "    0.483,                   !- Coefficient1 Constant",
        "    0.0305,                  !- Coefficient2 x",
        "    0.0000458,               !- Coefficient3 x**2",
        "    0.00511,                 !- Coefficient4 y",
        "    -1.50E-04,               !- Coefficient5 y**2",
        "    -1.28E-04,               !- Coefficient6 x*y",
        "    8.88889,                 !- Minimum Value of x",
        "    21.6667,                 !- Maximum Value of x",
        "    12.7778,                 !- Minimum Value of y",
        "    51.6667,                 !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Curve:Biquadratic,",
        "    1PowerCurveIpakCS,       !- Name",
        "    1.33E+00,                !- Coefficient1 Constant",
        "    -3.40E-02,               !- Coefficient2 x",
        "    0.000939,                !- Coefficient3 x**2",
        "    -0.00858,                !- Coefficient4 y",
        "    0.000769,                !- Coefficient5 y**2",
        "    -0.000972,               !- Coefficient6 x*y",
        "    8.88889,                 !- Minimum Value of x",
        "    21.6667,                 !- Maximum Value of x",
        "    12.7778,                 !- Minimum Value of y",
        "    51.6667,                 !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Temperature,             !- Input Unit Type for X",
        "    Temperature,             !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Curve:Biquadratic,",
        "    2CapacityCurveIpakCS,    !- Name",
        "    0.515,                   !- Coefficient1 Constant",
        "    0.026,                   !- Coefficient2 x",
        "    0.000275,                !- Coefficient3 x**2",
        "    0.00377,                 !- Coefficient4 y",
        "    -0.000115,               !- Coefficient5 y**2",
        "    -0.00017,                !- Coefficient6 x*y",
        "    8.88889,                 !- Minimum Value of x",
        "    21.6667,                 !- Maximum Value of x",
        "    12.7778,                 !- Minimum Value of y",
        "    51.6667,                 !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless,           !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Curve:Biquadratic,",
        "    2PowerCurveIpakCS,       !- Name",
        "    1.41,                    !- Coefficient1 Constant",
        "    -0.0457,                 !- Coefficient2 x",
        "    0.00116,                 !- Coefficient3 x**2",
        "    -0.00625,                !- Coefficient4 y",
        "    0.000707,                !- Coefficient5 y**2",
        "    -0.000889,               !- Coefficient6 x*y",
        "    8.88889,                 !- Minimum Value of x",
        "    21.6667,                 !- Maximum Value of x",
        "    12.7778,                 !- Minimum Value of y",
        "    51.6667,                 !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless,           !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Curve:Biquadratic,",
        "    3CapacityCurveIpakCS,    !- Name",
        "    0.5,                     !- Coefficient1 Constant",
        "    0.0316,                  !- Coefficient2 x",
        "    0.00013,                 !- Coefficient3 x**2",
        "    0.00213,                 !- Coefficient4 y",
        "    -0.000107,               !- Coefficient5 y**2",
        "    -0.000157,               !- Coefficient6 x*y",
        "    8.88889,                 !- Minimum Value of x",
        "    21.6667,                 !- Maximum Value of x",
        "    12.7778,                 !- Minimum Value of y",
        "    51.6667,                 !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless,           !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Curve:Biquadratic,",
        "    3PowerCurveIpakCS,       !- Name",
        "    0.99,                    !- Coefficient1 Constant",
        "    -0.039,                  !- Coefficient2 x",
        "    0.00111,                 !- Coefficient3 x**2",
        "    0.0144,                  !- Coefficient4 y",
        "    0.000489,                !- Coefficient5 y**2",
        "    -0.00111,                !- Coefficient6 x*y",
        "    8.88889,                 !- Minimum Value of x",
        "    21.6667,                 !- Maximum Value of x",
        "    12.7778,                 !- Minimum Value of y",
        "    51.6667,                 !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless,           !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Curve:Biquadratic,",
        "    4CapacityCurveIpakCS,          !- Name",
        "    0.528,                   !- Coefficient1 Constant",
        "    0.0324,                  !- Coefficient2 x",
        "    -0.0000156,              !- Coefficient3 x**2",
        "    0.00265,                 !- Coefficient4 y",
        "    -0.000125,               !- Coefficient5 y**2",
        "    -0.000134,               !- Coefficient6 x*y",
        "    8.88889,                 !- Minimum Value of x",
        "    21.6667,                 !- Maximum Value of x",
        "    12.7778,                 !- Minimum Value of y",
        "    51.6667,                 !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless,           !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Curve:Biquadratic,",
        "    4PowerCurveIpakCS,       !- Name",
        "    0.972,                   !- Coefficient1 Constant",
        "    -0.0357,                 !- Coefficient2 x",
        "    0.00105,                 !- Coefficient3 x**2",
        "    0.0109,                  !- Coefficient4 y",
        "    0.000517,                !- Coefficient5 y**2",
        "    -0.00101,                !- Coefficient6 x*y",
        "    8.88889,                 !- Minimum Value of x",
        "    21.6667,                 !- Maximum Value of x",
        "    12.7778,                 !- Minimum Value of y",
        "    51.6667,                 !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless,           !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Curve:Biquadratic,",
        "    5CapacityCurveIpakCS,    !- Name",
        "    0.551,                   !- Coefficient1 Constant",
        "    0.0291,                  !- Coefficient2 x",
        "    0.000124,                !- Coefficient3 x**2",
        "    0.00196,                 !- Coefficient4 y",
        "    -0.00011,                !- Coefficient5 y**",
        "    -0.000144,               !- Coefficient6 x*y",
        "    8.88889,                 !- Minimum Value of x",
        "    21.6667,                 !- Maximum Value of x",
        "    12.7778,                 !- Minimum Value of y",
        "    51.6667,                 !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless,           !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type",

        "Curve:Biquadratic,",
        "    5PowerCurveIpakCS,       !- Name",
        "    0.916,                   !- Coefficient1 Constant",
        "    -0.0306,                 !- Coefficient2 x",
        "    0.000885,                !- Coefficient3 x**2",
        "    0.0126,                  !- Coefficient4 y",
        "    0.000495,                !- Coefficient5 y**2",
        "    -0.00103,                !- Coefficient6 x*y",
        "    8.88889,                 !- Minimum Value of x",
        "    21.6667,                 !- Maximum Value of x",
        "    12.7778,                 !- Minimum Value of y",
        "    51.6667,                 !- Maximum Value of y",
        "    ,                        !- Minimum Curve Output",
        "    ,                        !- Maximum Curve Output",
        "    Dimensionless,           !- Input Unit Type for X",
        "    Dimensionless,           !- Input Unit Type for Y",
        "    Dimensionless;           !- Output Unit Type    "

    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    state->dataZoneEquip->ZoneEquipInputsFilled = true;

    std::string compName = "DX COOLING COIL SYSTEM";
    bool zoneEquipment = false;
    UnitarySystems::UnitarySys::factory(*state, HVAC::UnitarySysType::Unitary_AnyCoilType, compName, zoneEquipment, 0);
    UnitarySystems::UnitarySys *thisSys = &state->dataUnitarySystems->unitarySys[0];

    EXPECT_EQ(thisSys->Name, "DX COOLING COIL SYSTEM");
    EXPECT_FALSE(thisSys->m_ISHundredPercentDOASDXCoil);
    EXPECT_EQ(thisSys->UnitType, "CoilSystem:Cooling:DX");
    EXPECT_ENUM_EQ(thisSys->m_coolCoilType, HVAC::CoilType::CoolingDXVariableSpeed);
}

TEST_F(EnergyPlusFixture, VariableSpeedCoils_RHControl)
{
    // issue #6920
    std::string const idf_objects = delimited_string({

        "  Schedule:Compact, AVAILSCHED, FRACTION, Through: 12/31, For: Alldays, Until: 24:00,1.00; ",

        "  CoilSystem:Cooling:DX,",
        "    DX Cooling Coil System,  !- Name",
        "    AvailSched,    !- Availability Schedule Name",
        "    DX Cooling Coil Air Inlet Node,  !- DX Cooling Coil System Inlet Node Name",
        "    Heating Coil Air Inlet Node,  !- DX Cooling Coil System Outlet Node Name",
        "    Heating Coil Air Inlet Node,  !- DX Cooling Coil System Sensor Node Name",
        "    Coil:Cooling:DX:VariableSpeed,!- Cooling Coil Object Type",
        "    VS DX Cooling Coil,      !- Cooling Coil Name",
        "    CoolReheat,              !- Dehumidification Control Type",
        "    Yes,                     !- Run on Sensible Load",
        "    Yes,                     !- Run on Latent Load",
        "    No;                      !- Use Outdoor Air DX Cooling Coil",

        "  Coil:Cooling:DX:VariableSpeed,",
        "    VS DX Cooling Coil,              !- Name",
        "    ,                        !- Availability Schedule Name",
        "    DX Cooling Coil Air Inlet Node,  !- Air Inlet Node Name",
        "    Heating Coil Air Inlet Node,     !- Air Outlet Node Name",
        "    5,                       !- Number of Speeds {dimensionless}",
        "    5,                       !- Nominal Speed Level {dimensionless}",
        "    132769.7,                !- Rated Total Cooling Capacity At Selected Nominal Speed Level {w}",
        "    5.66336932,              !- Rated Volumetric Air Flow Rate At Selected Nominal Speed Level {m3/s}",
        "    0,                       !- Nominal Time for Condensate to Begin Leaving the Coil {s}",
        "    0,                       !- Initial Moisture Evaporation Rate Divided by Steady-State AC Latent Capacity {dimensionless}",
        "    ,                        !- Maximum Cycling Rate",
        "    ,                        !- Latent Capacity Time Constant",
        "    ,                        !- Fan Delay Time",
        "    PLFCurve,                !- Energy Part Load Fraction Curve Name",
        "    ,                        !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "    ,                        !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10,                      !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    2,                       !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    33861.72,                !- Speed 1 Reference Unit Total Cooling Capacity At Rated Conditions {w}",
        "    0.78,                    !- Speed 1 Reference Unit Sensible Heat Ratio At Rated Conditions {dimensionless}",
        "    4.328973,                !- Speed 1 Reference Unit COP At Rated Conditions {dimensionless}",
        "    1.396964,                !- Speed 1 Reference Unit Air Flow Rate At Rated Conditions {m3/s}",
        "    773.3,                   !- Speed 1 2017 Rated Evaporator Fan Power Per Volume Flow Rate",
        "    934.4,                   !- Speed 1 2023 Rated Evaporator Fan Power Per Volume Flow Rate",
        "    ,                        !- Speed 1 Reference Unit Condenser Flow Rate at Rated Conditions {m3/s}",
        "    ,                        !- Speed 1 Reference Unit Pad Effectiveness of Evap Precooling at Rated Conditions {dimensionless}",
        "    1Cap,                    !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "    CAPFF,                   !- Speed 1 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    1Pow,                    !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFF,                   !- Speed 1 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    35516.08,                !- Speed 2 Reference Unit Total Cooling Capacity At Rated Conditions {w}",
        "    0.78,                    !- Speed 2 Reference Unit Sensible Heat Ratio At Rated Conditions {dimensionless}",
        "    4.540061,                !- Speed 2 Reference Unit COP At Rated Conditions {dimensionless}",
        "    1.88779,                 !- Speed 2 Reference Unit Air Flow Rate At Rated Conditions {m3/s}",
        "    773.3,                   !- Speed 2 2017 Rated Evaporator Fan Power Per Volume Flow Rate",
        "    934.4,                   !- Speed 2 2023 Rated Evaporator Fan Power Per Volume Flow Rate",
        "    ,                        !- Speed 2 Reference Unit Condenser Flow Rate at Rated Conditions {m3/s}",
        "    ,                        !- Speed 2 Reference Unit Pad Effectiveness of Evap Precooling at Rated Conditions {dimensionless}",
        "    1Cap,                    !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "    CAPFF,                   !- Speed 2 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    1Pow,                    !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFF,                   !- Speed 2 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    65133.17,                !- Speed 3 Reference Unit Total Cooling Capacity At Rated Conditions {w}",
        "    0.77,                    !- Speed 3 Reference Unit Sensible Heat Ratio At Rated Conditions {dimensionless}",
        "    4.164418,                !- Speed 3 Reference Unit COP At Rated Conditions {dimensionless}",
        "    2.831685,                !- Speed 3 Reference Unit Air Flow Rate At Rated Conditions {m3/s}",
        "    773.3,                   !- Speed 3 2017 Rated Evaporator Fan Power Per Volume Flow Rate",
        "    934.4,                   !- Speed 3 2023 Rated Evaporator Fan Power Per Volume Flow Rate",
        "    ,                        !- Speed 3 Reference Unit Condenser Flow Rate at Rated Conditions {m3/s}",
        "    ,                        !- Speed 3 Reference Unit Pad Effectiveness of Evap Precooling at Rated Conditions {dimensionless}",
        "    1Cap,                    !- Speed 3 Total Cooling Capacity Function of Temperature Curve Name",
        "    CAPFF,                   !- Speed 3 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    1Pow,                    !- Speed 3 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFF,                   !- Speed 3 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    119583.3,                !- Speed 4 Reference Unit Total Cooling Capacity At Rated Conditions {w}",
        "    0.76,                    !- Speed 4 Reference Unit Sensible Heat Ratio At Rated Conditions {dimensionless}",
        "    3.469661,                !- Speed 4 Reference Unit COP At Rated Conditions {dimensionless}",
        "    3.553764,                !- Speed 4 Reference Unit Air Flow Rate At Rated Conditions {m3/s}",
        "    773.3,                   !- Speed 4 2017 Rated Evaporator Fan Power Per Volume Flow Rate",
        "    934.4,                   !- Speed 4 2023 Rated Evaporator Fan Power Per Volume Flow Rate",
        "    ,                        !- Speed 4 Reference Unit Condenser Flow Rate at Rated Conditions {m3/s}",
        "    ,                        !- Speed 4 Reference Unit Pad Effectiveness of Evap Precooling at Rated Conditions {dimensionless}",
        "    1Cap,                    !- Speed 4 Total Cooling Capacity Function of Temperature Curve Name",
        "    CAPFF,                   !- Speed 4 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    1Pow,                    !- Speed 4 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFF,                   !- Speed 4 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    132769.7,                !- Speed 5 Reference Unit Total Cooling Capacity At Rated Conditions {w}",
        "    0.76,                    !- Speed 5 Reference Unit Sensible Heat Ratio At Rated Conditions {dimensionless}",
        "    3.822957,                !- Speed 5 Reference Unit COP At Rated Conditions {dimensionless}",
        "    5.66336932,              !- Speed 5 Reference Unit Air Flow Rate At Rated Conditions {m3/s}",
        "    773.3,                   !- Speed 5 2017 Rated Evaporator Fan Power Per Volume Flow Rate",
        "    934.4,                   !- Speed 5 2023 Rated Evaporator Fan Power Per Volume Flow Rate",
        "    ,                        !- Speed 5 Reference Unit Condenser Flow Rate at Rated Conditions {m3/s}",
        "    ,                        !- Speed 5 Reference Unit Pad Effectiveness of Evap Precooling at Rated Conditions {dimensionless}",
        "    1Cap,                    !- Speed 5 Total Cooling Capacity Function of Temperature Curve Name",
        "    CAPFF,                   !- Speed 5 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    1Pow,                    !- Speed 5 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFF;                   !- Speed 5 Energy Input Ratio Function of Air Flow Fraction Curve Name",

        "Curve:Quadratic, PLFCurve, 0.85, 0.83, 0.0, 0.0, 0.3, 0.85, 1.0, Dimensionless, Dimensionless; ",
        "Curve:Cubic, CAPFF, 1, 0, 0, 0, 0, 1, , , Dimensionless, Dimensionless; ",
        "Curve:Cubic, EIRFF, 1, 0, 0, 0, 0, 1, , , Dimensionless, Dimensionless; ",
        "Curve:Biquadratic, 1Cap, 0.483, 0.0305, 0.0000458, 0.00511, -1.50E-04, -1.28E-04, 8.89, 21.67, 12.78, 51.67, , , Temperature, Temperature, "
        "Dimensionless; ",
        "Curve:Biquadratic, 1Pow, 1.33, -0.034, 0.00094, -0.0086, 0.00077, -0.000972, 8.89, 21.7, 12.8, 51.7, , , Temperature, Temperature, "
        "Dimensionless; ",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    std::string compName = "DX COOLING COIL SYSTEM";
    state->dataZoneEquip->ZoneEquipInputsFilled = true;
    state->dataGlobal->NumOfZones = 1;
    state->dataZoneEquip->ZoneEquipConfig.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).IsControlled = true;

    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    state->dataAirSystemsData->PrimaryAirSystems.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).NumBranches = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).TotalComponents = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp(1).Name = compName;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp(1).CompType_Num = SimAirServingZones::CompType::DXSystem;
    OutputReportPredefined::SetPredefinedTables(*state);
    Sched::GetSchedule(*state, "AVAILSCHED")->currentVal = 1.0; // Enable schedule without calling schedule manager

    bool FirstHVACIteration = true;
    bool HXUnitOn = false;
    int InletNode = 1;
    int ControlNode = 2; // same as outlet node number
    int airLoopNum = 1;

    bool zoneEquipment = true;
    UnitarySystems::UnitarySys::factory(*state, HVAC::UnitarySysType::Unitary_AnyCoilType, compName, zoneEquipment, 0);
    UnitarySystems::UnitarySys *thisSys = &state->dataUnitarySystems->unitarySys[0];
    // call again to get the rest of the input when sysNum > -1
    UnitarySystems::UnitarySys::getUnitarySystemInput(*state, compName, false, 0);

    EXPECT_EQ(thisSys->Name, "DX COOLING COIL SYSTEM");
    EXPECT_FALSE(thisSys->m_ISHundredPercentDOASDXCoil);
    EXPECT_EQ(thisSys->UnitType, "CoilSystem:Cooling:DX");
    EXPECT_EQ(thisSys->m_coolCoilType, HVAC::CoilType::CoolingDXVariableSpeed);
    EXPECT_EQ(2, thisSys->CoolCtrlNode);

    // set up outdoor environment
    state->dataEnvrn->OutDryBulbTemp = 35.0;
    state->dataEnvrn->OutHumRat = 0.0196;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutWetBulbTemp = 27.0932;

    // set up inputs to test coil control
    thisSys->m_DesiredOutletTemp = 18.0;
    thisSys->m_DesiredOutletHumRat = 1.0;
    state->dataEnvrn->StdRhoAir = 1.2;
    state->dataLoopNodes->Node(InletNode).MassFlowRate = 5.66336932 * state->dataEnvrn->StdRhoAir;
    state->dataLoopNodes->Node(InletNode).Temp = 24.0;
    state->dataLoopNodes->Node(InletNode).HumRat = 0.012143698;
    state->dataLoopNodes->Node(InletNode).Enthalpy = 55029.3778; // conditions at 65 % RH
    state->dataLoopNodes->Node(ControlNode).TempSetPoint = thisSys->m_DesiredOutletTemp;
    Real64 RHControlHumRat = 0.01119276; // humrat at 24C, 60% RH
    state->dataLoopNodes->Node(ControlNode).HumRatMax = RHControlHumRat;

    // test sensible control
    HVAC::CompressorOp CompressorOn = HVAC::CompressorOp::On;
    thisSys->controlCoolingSystemToSP(*state, airLoopNum, FirstHVACIteration, HXUnitOn, CompressorOn);
    // system meets temperature set point
    EXPECT_NEAR(thisSys->m_DesiredOutletTemp, state->dataLoopNodes->Node(ControlNode).Temp, 0.001);
    // system was not told to meet humidity ratio set point (since DesiredOutletHumRat = 1.0)
    EXPECT_GT(state->dataLoopNodes->Node(ControlNode).HumRat, state->dataLoopNodes->Node(ControlNode).HumRatMax);
    // sensible load met by compressor speed 3
    EXPECT_EQ(3, thisSys->m_CoolingSpeedNum);

    // test latent control
    thisSys->m_DesiredOutletHumRat = RHControlHumRat;
    thisSys->controlCoolingSystemToSP(*state, airLoopNum, FirstHVACIteration, HXUnitOn, CompressorOn);

    // system over cools past temperature set point
    EXPECT_GT(thisSys->m_DesiredOutletTemp, state->dataLoopNodes->Node(ControlNode).Temp);
    // system does meet humidity ratio set point
    EXPECT_NEAR(state->dataLoopNodes->Node(ControlNode).HumRat, state->dataLoopNodes->Node(ControlNode).HumRatMax, 0.0000001);
    // latent load needed to increase compressor speed to speed 4
    EXPECT_EQ(4, thisSys->m_CoolingSpeedNum);

    // test for divide by zero error in controlCoolingSystemToSP
    thisSys->m_DesiredOutletTemp = 23.888888888888900;
    thisSys->m_DesiredOutletHumRat = 0.0092857142857142895;
    state->dataLoopNodes->Node(InletNode).MassFlowRate = 1.3840962084222401;
    state->dataLoopNodes->Node(InletNode).Temp = 16.566173051926114;
    state->dataLoopNodes->Node(InletNode).HumRat = 0.0092873376541228961;
    state->dataLoopNodes->Node(InletNode).HumRatMax = -999;

    has_err_output(true);
    EXPECT_NO_THROW(thisSys->controlCoolingSystemToSP(*state, airLoopNum, FirstHVACIteration, HXUnitOn, CompressorOn));
    EXPECT_EQ(thisSys->m_CoolingPartLoadFrac, 0);
    std::string const expected_error =
        "   ** Warning ** CoilSystem:Cooling:DX - sensible part-load ratio calculation failed: part-load ratio limits exceeded, for unit = DX "
        "COOLING COIL SYSTEM\n   **   ~~~   ** Estimated part-load ratio = 0.000\n   **   ~~~   ** The estimated part-load ratio will be used and "
        "the simulation continues. Occurrence info:\n   **   ~~~   **  Environment=, at Simulation time= 00:-15 - 00:00\n";
    compare_err_stream(expected_error, true);
}

TEST_F(EnergyPlusFixture, VariableSpeedCoils_LatentDegradation_Test)
{
    std::string const idf_objects = delimited_string({

        "  Schedule:Compact, AVAILSCHED, FRACTION, Through: 12/31, For: Alldays, Until: 24:00,1.00; ",

        "  CoilSystem:Cooling:DX,",
        "    DX Cooling Coil System,  !- Name",
        "    AvailSched,    !- Availability Schedule Name",
        "    DX Cooling Coil Air Inlet Node,  !- DX Cooling Coil System Inlet Node Name",
        "    Heating Coil Air Inlet Node,  !- DX Cooling Coil System Outlet Node Name",
        "    Heating Coil Air Inlet Node,  !- DX Cooling Coil System Sensor Node Name",
        "    Coil:Cooling:DX:VariableSpeed,!- Cooling Coil Object Type",
        "    VS DX Cooling Coil,      !- Cooling Coil Name",
        "    None,                    !- Dehumidification Control Type",
        "    Yes,                     !- Run on Sensible Load",
        "    No,                     !- Run on Latent Load",
        "    No;                      !- Use Outdoor Air DX Cooling Coil",

        "  Coil:Cooling:DX:VariableSpeed,",
        "    VS DX Cooling Coil,              !- Name",
        "    ,                        !- Availability Schedule Name",
        "    DX Cooling Coil Air Inlet Node,  !- Air Inlet Node Name",
        "    Heating Coil Air Inlet Node,     !- Air Outlet Node Name",
        "    5,                       !- Number of Speeds {dimensionless}",
        "    5,                       !- Nominal Speed Level {dimensionless}",
        "    132769.7,                !- Rated Total Cooling Capacity At Selected Nominal Speed Level {w}",
        "    5.66336932,              !- Rated Volumetric Air Flow Rate At Selected Nominal Speed Level {m3/s}",
        "    0,                       !- Nominal Time for Condensate to Begin Leaving the Coil {s}",
        "    0,                       !- Initial Moisture Evaporation Rate Divided by Steady-State AC Latent Capacity {dimensionless}",
        "    ,                        !- Maximum Cycling Rate",
        "    ,                        !- Latent Capacity Time Constant",
        "    ,                        !- Fan Delay Time",
        "    PLFCurve,                !- Energy Part Load Fraction Curve Name",
        "    ,                        !- Condenser Air Inlet Node Name",
        "    AirCooled,               !- Condenser Type",
        "    ,                        !- Evaporative Condenser Pump Rated Power Consumption {W}",
        "    ,                        !- Crankcase Heater Capacity {W}",
        "    ,                        !- Crankcase Heater Capacity Function of Temperature Curve Name",
        "    10,                      !- Maximum Outdoor Dry-Bulb Temperature for Crankcase Heater Operation {C}",
        "    ,                        !- Minimum Outdoor Dry-Bulb Temperature for Compressor Operation {C}",
        "    ,                        !- Supply Water Storage Tank Name",
        "    ,                        !- Condensate Collection Water Storage Tank Name",
        "    ,                        !- Basin Heater Capacity {W/K}",
        "    2,                       !- Basin Heater Setpoint Temperature {C}",
        "    ,                        !- Basin Heater Operating Schedule Name",
        "    33861.72,                !- Speed 1 Reference Unit Total Cooling Capacity At Rated Conditions {w}",
        "    0.78,                    !- Speed 1 Reference Unit Sensible Heat Ratio At Rated Conditions {dimensionless}",
        "    4.328973,                !- Speed 1 Reference Unit COP At Rated Conditions {dimensionless}",
        "    1.396964,                !- Speed 1 Reference Unit Air Flow Rate At Rated Conditions {m3/s}",
        "    773.3,                   !- Speed 1 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    934.4,                   !- Speed 1 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    ,                        !- Speed 1 Reference Unit Condenser Flow Rate at Rated Conditions {m3/s}",
        "    ,                        !- Speed 1 Reference Unit Pad Effectiveness of Evap Precooling at Rated Conditions {dimensionless}",
        "    1Cap,                    !- Speed 1 Total Cooling Capacity Function of Temperature Curve Name",
        "    CAPFF,                   !- Speed 1 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    1Pow,                    !- Speed 1 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFF,                   !- Speed 1 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    35516.08,                !- Speed 2 Reference Unit Total Cooling Capacity At Rated Conditions {w}",
        "    0.78,                    !- Speed 2 Reference Unit Sensible Heat Ratio At Rated Conditions {dimensionless}",
        "    4.540061,                !- Speed 2 Reference Unit COP At Rated Conditions {dimensionless}",
        "    1.88779,                 !- Speed 2 Reference Unit Air Flow Rate At Rated Conditions {m3/s}",
        "    773.3,                   !- Speed 2 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    934.4,                   !- Speed 2 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    ,                        !- Speed 2 Reference Unit Condenser Flow Rate at Rated Conditions {m3/s}",
        "    ,                        !- Speed 2 Reference Unit Pad Effectiveness of Evap Precooling at Rated Conditions {dimensionless}",
        "    1Cap,                    !- Speed 2 Total Cooling Capacity Function of Temperature Curve Name",
        "    CAPFF,                   !- Speed 2 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    1Pow,                    !- Speed 2 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFF,                   !- Speed 2 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    65133.17,                !- Speed 3 Reference Unit Total Cooling Capacity At Rated Conditions {w}",
        "    0.77,                    !- Speed 3 Reference Unit Sensible Heat Ratio At Rated Conditions {dimensionless}",
        "    4.164418,                !- Speed 3 Reference Unit COP At Rated Conditions {dimensionless}",
        "    2.831685,                !- Speed 3 Reference Unit Air Flow Rate At Rated Conditions {m3/s}",
        "    773.3,                   !- Speed 3 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    934.4,                   !- Speed 3 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    ,                        !- Speed 3 Reference Unit Condenser Flow Rate at Rated Conditions {m3/s}",
        "    ,                        !- Speed 3 Reference Unit Pad Effectiveness of Evap Precooling at Rated Conditions {dimensionless}",
        "    1Cap,                    !- Speed 3 Total Cooling Capacity Function of Temperature Curve Name",
        "    CAPFF,                   !- Speed 3 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    1Pow,                    !- Speed 3 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFF,                   !- Speed 3 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    119583.3,                !- Speed 4 Reference Unit Total Cooling Capacity At Rated Conditions {w}",
        "    0.76,                    !- Speed 4 Reference Unit Sensible Heat Ratio At Rated Conditions {dimensionless}",
        "    3.469661,                !- Speed 4 Reference Unit COP At Rated Conditions {dimensionless}",
        "    3.553764,                !- Speed 4 Reference Unit Air Flow Rate At Rated Conditions {m3/s}",
        "    773.3,                   !- Speed 4 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    934.4,                   !- Speed 4 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    ,                        !- Speed 4 Reference Unit Condenser Flow Rate at Rated Conditions {m3/s}",
        "    ,                        !- Speed 4 Reference Unit Pad Effectiveness of Evap Precooling at Rated Conditions {dimensionless}",
        "    1Cap,                    !- Speed 4 Total Cooling Capacity Function of Temperature Curve Name",
        "    CAPFF,                   !- Speed 4 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    1Pow,                    !- Speed 4 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFF,                   !- Speed 4 Energy Input Ratio Function of Air Flow Fraction Curve Name",
        "    132769.7,                !- Speed 5 Reference Unit Total Cooling Capacity At Rated Conditions {w}",
        "    0.76,                    !- Speed 5 Reference Unit Sensible Heat Ratio At Rated Conditions {dimensionless}",
        "    3.822957,                !- Speed 5 Reference Unit COP At Rated Conditions {dimensionless}",
        "    5.66336932,              !- Speed 5 Reference Unit Air Flow Rate At Rated Conditions {m3/s}",
        "    773.3,                   !- Speed 5 2017 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    934.4,                   !- Speed 5 2023 Rated Evaporator Fan Power Per Volume Flow Rate [W/(m3/s)]",
        "    ,                        !- Speed 5 Reference Unit Condenser Flow Rate at Rated Conditions {m3/s}",
        "    ,                        !- Speed 5 Reference Unit Pad Effectiveness of Evap Precooling at Rated Conditions {dimensionless}",
        "    1Cap,                    !- Speed 5 Total Cooling Capacity Function of Temperature Curve Name",
        "    CAPFF,                   !- Speed 5 Total Cooling Capacity Function of Air Flow Fraction Curve Name",
        "    1Pow,                    !- Speed 5 Energy Input Ratio Function of Temperature Curve Name",
        "    EIRFF;                   !- Speed 5 Energy Input Ratio Function of Air Flow Fraction Curve Name",

        "Curve:Quadratic, PLFCurve, 0.85, 0.83, 0.0, 0.0, 0.3, 0.85, 1.0, Dimensionless, Dimensionless; ",
        "Curve:Cubic, CAPFF, 1, 0, 0, 0, 0, 1, , , Dimensionless, Dimensionless; ",
        "Curve:Cubic, EIRFF, 1, 0, 0, 0, 0, 1, , , Dimensionless, Dimensionless; ",
        "Curve:Biquadratic, 1Cap, 0.483, 0.0305, 0.0000458, 0.00511, -1.50E-04, -1.28E-04, 8.89, 21.67, 12.78, 51.67, , , Temperature, Temperature, "
        "Dimensionless; ",
        "Curve:Biquadratic, 1Pow, 1.33, -0.034, 0.00094, -0.0086, 0.00077, -0.000972, 8.89, 21.7, 12.8, 51.7, , , Temperature, Temperature, "
        "Dimensionless; ",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    std::string compName = "DX COOLING COIL SYSTEM";
    state->dataZoneEquip->ZoneEquipInputsFilled = true;
    state->dataGlobal->NumOfZones = 1;
    state->dataZoneEquip->ZoneEquipConfig.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).IsControlled = true;

    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    state->dataAirSystemsData->PrimaryAirSystems.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).NumBranches = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).TotalComponents = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp(1).Name = compName;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp(1).CompType_Num = SimAirServingZones::CompType::DXSystem;
    OutputReportPredefined::SetPredefinedTables(*state);
    Sched::GetSchedule(*state, "AVAILSCHED")->currentVal = 1.0; // Enable schedule without calling schedule manager

    bool FirstHVACIteration = true;
    bool HXUnitOn = false;
    int InletNode = 1;
    int ControlNode = 2; // same as outlet node number
    int airLoopNum = 1;

    bool zoneEquipment = true;
    UnitarySystems::UnitarySys::factory(*state, HVAC::UnitarySysType::Unitary_AnyCoilType, compName, zoneEquipment, 0);
    UnitarySystems::UnitarySys *thisSys = &state->dataUnitarySystems->unitarySys[0];
    // call again to get the rest of the input when sysNum > -1
    UnitarySystems::UnitarySys::getUnitarySystemInput(*state, compName, false, 0);

    // set up outdoor environment
    state->dataEnvrn->OutDryBulbTemp = 35.0;
    state->dataEnvrn->OutHumRat = 0.0196;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutWetBulbTemp = 27.0932;

    // set up inputs to test coil control
    thisSys->m_DesiredOutletTemp = 22.0;
    state->dataEnvrn->StdRhoAir = 1.2;
    state->dataLoopNodes->Node(InletNode).MassFlowRate = 1.396964 * state->dataEnvrn->StdRhoAir;
    state->dataLoopNodes->Node(InletNode).Temp = 24.0;
    state->dataLoopNodes->Node(InletNode).HumRat = 0.014; // high zone RH, about 75%
    state->dataLoopNodes->Node(InletNode).Enthalpy = Psychrometrics::PsyHFnTdbW(
        state->dataLoopNodes->Node(InletNode).Temp, state->dataLoopNodes->Node(InletNode).HumRat); // 55029.3778; // conditions at 65 % RH
    state->dataLoopNodes->Node(ControlNode).TempSetPoint = thisSys->m_DesiredOutletTemp;

    // test sensible control
    HVAC::CompressorOp CompressorOn = HVAC::CompressorOp::On;
    thisSys->controlCoolingSystemToSP(*state, airLoopNum, FirstHVACIteration, HXUnitOn, CompressorOn);
    Real64 SHR = state->dataVariableSpeedCoils->VarSpeedCoil(1).QSensible / state->dataVariableSpeedCoils->VarSpeedCoil(1).QLoadTotal;
    EXPECT_NEAR(SHR, 0.49605, 0.0001);
    EXPECT_EQ(1, state->dataVariableSpeedCoils->VarSpeedCoil(1).SpeedNumReport);             // latent degradation only works at low speed
    EXPECT_NEAR(0.199, state->dataVariableSpeedCoils->VarSpeedCoil(1).PartLoadRatio, 0.001); // PLR is low

    // add latent degradation model
    state->dataVariableSpeedCoils->VarSpeedCoil(1).Twet_Rated = 1000.0;
    state->dataVariableSpeedCoils->VarSpeedCoil(1).Gamma_Rated = 1.5;
    thisSys->controlCoolingSystemToSP(*state, airLoopNum, FirstHVACIteration, HXUnitOn, CompressorOn);
    SHR = state->dataVariableSpeedCoils->VarSpeedCoil(1).QSensible / state->dataVariableSpeedCoils->VarSpeedCoil(1).QLoadTotal;
    EXPECT_NEAR(SHR, 1.0, 0.0001);                                                           // more sensible capacity so PLR should be lower
    EXPECT_EQ(1, state->dataVariableSpeedCoils->VarSpeedCoil(1).SpeedNumReport);             // latent degradation only works at low speed
    EXPECT_NEAR(0.099, state->dataVariableSpeedCoils->VarSpeedCoil(1).PartLoadRatio, 0.001); // PLR is lower, latent capacity is 0

    // test more reasonable zone RH,about 50%
    state->dataLoopNodes->Node(InletNode).HumRat = 0.0092994;
    state->dataLoopNodes->Node(InletNode).Enthalpy = Psychrometrics::PsyHFnTdbW(
        state->dataLoopNodes->Node(InletNode).Temp, state->dataLoopNodes->Node(InletNode).HumRat); // 55029.3778; // conditions at 65 % RH
    state->dataLoopNodes->Node(ControlNode).TempSetPoint = thisSys->m_DesiredOutletTemp;

    // remove latent degradation model
    state->dataVariableSpeedCoils->VarSpeedCoil(1).Twet_Rated = 0.0;
    state->dataVariableSpeedCoils->VarSpeedCoil(1).Gamma_Rated = 0.0;

    thisSys->controlCoolingSystemToSP(*state, airLoopNum, FirstHVACIteration, HXUnitOn, CompressorOn);
    SHR = state->dataVariableSpeedCoils->VarSpeedCoil(1).QSensible / state->dataVariableSpeedCoils->VarSpeedCoil(1).QLoadTotal;
    EXPECT_NEAR(SHR, 0.7626, 0.0001);
    EXPECT_EQ(1, state->dataVariableSpeedCoils->VarSpeedCoil(1).SpeedNumReport);             // latent degradation only works at low speed
    EXPECT_NEAR(0.143, state->dataVariableSpeedCoils->VarSpeedCoil(1).PartLoadRatio, 0.001); // PLR is low

    // add latent degradation model
    state->dataVariableSpeedCoils->VarSpeedCoil(1).Twet_Rated = 1000.0;
    state->dataVariableSpeedCoils->VarSpeedCoil(1).Gamma_Rated = 1.5;
    thisSys->controlCoolingSystemToSP(*state, airLoopNum, FirstHVACIteration, HXUnitOn, CompressorOn);
    SHR = state->dataVariableSpeedCoils->VarSpeedCoil(1).QSensible / state->dataVariableSpeedCoils->VarSpeedCoil(1).QLoadTotal;
    EXPECT_NEAR(SHR, 1.0, 0.0001);                                                           // more sensible capacity so PLR should be lower
    EXPECT_EQ(1, state->dataVariableSpeedCoils->VarSpeedCoil(1).SpeedNumReport);             // latent degradation only works at low speed
    EXPECT_NEAR(0.109, state->dataVariableSpeedCoils->VarSpeedCoil(1).PartLoadRatio, 0.001); // PLR is lower, latent capacity is 0
}

TEST_F(EnergyPlusFixture, NewDXCoilModel_RHControl)
{
    // issue #6920
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
        "    Yes,                     !- Apply Part Load Fraction to Speeds Greater than 1",
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
        "    DX Cool Cooling Coil Speed 2 Performance,  !- Name",
        "    1.0,                     !- Gross Total Cooling Capacity Fraction",
        "    1.0,                     !- Evaporator Air Flow Rate Fraction",
        "    1.0,                     !- Condenser Air Flow Rate Fraction",
        "    0.77,                    !- Gross Sensible Heat Ratio",
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

        "  Coil:Cooling:DX:CurveFit:OperatingMode,",
        "    DX Cool Cooling Coil Operating Mode2,  !- Name",
        "    15000,                   !- Rated Gross Total Cooling Capacity {W}",
        "    0.7,                     !- Rated Evaporator Air Flow Rate {m3/s}",
        "    ,                        !- Rated Condenser Air Flow Rate {m3/s}",
        "    0,                       !- Maximum Cycling Rate {cycles/hr}",
        "    0,                       !- Ratio of Initial Moisture Evaporation Rate and Steady State Latent Capacity {dimensionless}",
        "    0,                       !- Latent Capacity Time Constant {s}",
        "    0,                       !- Nominal Time for Condensate Removal to Begin {s}",
        "    Yes,                     !- Apply Part Load Fraction to Speeds Greater than 1",
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
        "Curve:Biquadratic, 1Cap, 0.483, 0.0305, 0.0000458, 0.00511, -1.50E-04, -1.28E-04, 8.89, 21.67, 12.78, 51.67, , , Temperature, Temperature, "
        "Dimensionless; ",
        "Curve:Biquadratic, 1Pow, 1.33, -0.034, 0.00094, -0.0086, 0.00077, -0.000972, 8.89, 21.7, 12.8, 51.7, , , Temperature, Temperature, "
        "Dimensionless; ",

    });

    ASSERT_TRUE(process_idf(idf_objects));
    state->dataGlobal->TimeStepsInHour = 1;
    state->dataGlobal->MinutesInTimeStep = 60;
    state->init_state(*state);

    std::string compName = "DX COOLING COIL SYSTEM";
    state->dataZoneEquip->ZoneEquipInputsFilled = true;
    state->dataGlobal->NumOfZones = 1;
    state->dataZoneEquip->ZoneEquipConfig.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).IsControlled = true;

    state->dataHVACGlobal->NumPrimaryAirSys = 1;
    state->dataAirSystemsData->PrimaryAirSystems.allocate(1);
    state->dataAirLoop->AirLoopControlInfo.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).NumBranches = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).TotalComponents = 1;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp.allocate(1);
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp(1).Name = compName;
    state->dataAirSystemsData->PrimaryAirSystems(1).Branch(1).Comp(1).CompType_Num = SimAirServingZones::CompType::DXSystem;
    OutputReportPredefined::SetPredefinedTables(*state);
    Sched::GetSchedule(*state, "AVAILSCHED")->currentVal = 1.0; // Enable schedule without calling schedule manager

    bool FirstHVACIteration = true;
    bool HXUnitOn = false;
    int InletNode = 1;
    int ControlNode = 2; // same as outlet node number
    int condenserNode = 3;
    int airLoopNum = 1;

    bool zoneEquipment = true;
    UnitarySystems::UnitarySys::factory(*state, HVAC::UnitarySysType::Unitary_AnyCoilType, compName, zoneEquipment, 0);
    UnitarySystems::UnitarySys *thisSys = &state->dataUnitarySystems->unitarySys[0];
    // call again to get the rest of the input when sysNum > -1
    UnitarySystems::UnitarySys::getUnitarySystemInput(*state, compName, false, 0);

    EXPECT_EQ(thisSys->Name, "DX COOLING COIL SYSTEM");
    EXPECT_FALSE(thisSys->m_ISHundredPercentDOASDXCoil);
    EXPECT_EQ(thisSys->UnitType, "CoilSystem:Cooling:DX");
    EXPECT_ENUM_EQ(thisSys->m_coolCoilType, HVAC::CoilType::CoolingDX);
    EXPECT_EQ(2, thisSys->CoolCtrlNode);

    // set up outdoor environment
    state->dataEnvrn->OutDryBulbTemp = 35.0;
    state->dataEnvrn->OutHumRat = 0.0196;
    state->dataEnvrn->OutBaroPress = 101325.0;
    state->dataEnvrn->OutWetBulbTemp = 27.0932;
    state->dataLoopNodes->Node(condenserNode).Temp = state->dataEnvrn->OutDryBulbTemp;
    state->dataLoopNodes->Node(condenserNode).HumRat = state->dataEnvrn->OutHumRat;

    // set up inputs to test coil control
    thisSys->m_DesiredOutletTemp = 20.0;
    thisSys->m_DesiredOutletHumRat = 1.0;
    state->dataEnvrn->StdRhoAir = 1.2;
    state->dataLoopNodes->Node(InletNode).MassFlowRate = 0.8 * state->dataEnvrn->StdRhoAir;
    state->dataLoopNodes->Node(InletNode).Press = state->dataEnvrn->OutBaroPress;
    state->dataLoopNodes->Node(InletNode).HumRat = 0.012143698;
    state->dataLoopNodes->Node(InletNode).Enthalpy = 55029.3778; // conditions at 65 % RH
    state->dataLoopNodes->Node(InletNode).Temp =
        Psychrometrics::PsyTdbFnHW(state->dataLoopNodes->Node(InletNode).Enthalpy, state->dataLoopNodes->Node(InletNode).HumRat);
    state->dataLoopNodes->Node(ControlNode).TempSetPoint = thisSys->m_DesiredOutletTemp;
    Real64 RHControlHumRat = 0.01; // humrat at 24C, ~60% RH
    state->dataLoopNodes->Node(ControlNode).HumRatMax = RHControlHumRat;

    // test sensible control
    state->dataGlobal->BeginEnvrnFlag = true;
    state->dataSize->CurSysNum = 1;
    state->dataSize->UnitarySysEqSizing.allocate(1);
    // run init to size system
    thisSys->initUnitarySystems(*state, 1, FirstHVACIteration, 0.0);
    HVAC::CompressorOp CompOn = HVAC::CompressorOp::On;
    thisSys->controlCoolingSystemToSP(*state, airLoopNum, FirstHVACIteration, HXUnitOn, CompOn);
    // system meets temperature set point
    Real64 outTemp1 = state->dataLoopNodes->Node(ControlNode).Temp;
    Real64 outHumRat1 = state->dataLoopNodes->Node(ControlNode).HumRat;
    EXPECT_NEAR(thisSys->m_DesiredOutletTemp, state->dataLoopNodes->Node(ControlNode).Temp, 0.001);
    // system was not told to meet humidity ratio set point (since DesiredOutletHumRat = 1.0)
    EXPECT_GT(state->dataLoopNodes->Node(ControlNode).HumRat, state->dataLoopNodes->Node(ControlNode).HumRatMax);
    // sensible load met by compressor speed 1
    EXPECT_EQ(1, thisSys->m_CoolingSpeedNum);

    // test cool reheat latent control
    thisSys->m_DesiredOutletHumRat = RHControlHumRat;
    thisSys->initUnitarySystems(*state, 1, FirstHVACIteration, 0.0);
    thisSys->controlCoolingSystemToSP(*state, airLoopNum, FirstHVACIteration, HXUnitOn, CompOn);

    Real64 outTemp2 = state->dataLoopNodes->Node(ControlNode).Temp;
    Real64 outHumRat2 = state->dataLoopNodes->Node(ControlNode).HumRat;
    // system over cools past temperature set point
    EXPECT_LT(outTemp2, thisSys->m_DesiredOutletTemp);
    EXPECT_LT(outTemp2, outTemp1);     // cool reheat overshoots Tsetpoint
    EXPECT_LT(outHumRat2, outHumRat1); // and provides more dehumidification

    // system does meet humidity ratio set point
    EXPECT_NEAR(outHumRat2, state->dataLoopNodes->Node(ControlNode).HumRatMax, 0.001);
    EXPECT_NEAR(outHumRat1, 0.01166, 0.0001);      // sensible control yields higher outlet humrat
    EXPECT_NEAR(outHumRat2, 0.01000, 0.0001);      // cool reheat control yields lower outlet humrat at set point
    EXPECT_NEAR(RHControlHumRat, 0.01000, 0.0001); // cool reheat controls to humrat set point

    // latent load needed to increase compressor speed to speed 2
    EXPECT_EQ(2, thisSys->m_CoolingSpeedNum);

    // test multimode latent control (change CoilSystem:Cooling:DX coolreheat to multimode)
    thisSys->m_DehumidControlType_Num = UnitarySystems::UnitarySys::DehumCtrlType::Multimode;
    thisSys->initUnitarySystems(*state, 1, FirstHVACIteration, 0.0);
    thisSys->controlCoolingSystemToSP(*state, airLoopNum, FirstHVACIteration, HXUnitOn, CompOn);

    Real64 outTemp3 = state->dataLoopNodes->Node(ControlNode).Temp;
    Real64 outHumRat3 = state->dataLoopNodes->Node(ControlNode).HumRat;
    // expect same sensible and better dehumidification using multimode since alternate mode coil specs have improved latent
    EXPECT_NEAR(outTemp3, outTemp1, 0.00001); // multimode controls to same Tsetpoint
    EXPECT_LT(outHumRat3, outHumRat1);        // lower outlet humrat with multimode's alternate operating mode
    EXPECT_NEAR(outHumRat1, 0.01166, 0.0001); // sensible control yields higher outlet humrat
    EXPECT_NEAR(outHumRat3, 0.01119, 0.0001); // multimode control yields lower outlet humrat

    // test for divide by zero error in controlCoolingSystemToSP
    thisSys->m_DesiredOutletTemp = 23.888888888888900;
    thisSys->m_DesiredOutletHumRat = 0.0092857142857142895;
    state->dataLoopNodes->Node(InletNode).MassFlowRate = 1.3840962084222401;
    state->dataLoopNodes->Node(InletNode).Temp = 16.566173051926114;
    state->dataLoopNodes->Node(InletNode).HumRat = 0.0092873376541228961;
    state->dataLoopNodes->Node(InletNode).HumRatMax = -999;

    has_err_output(true);
    EXPECT_NO_THROW(thisSys->controlCoolingSystemToSP(*state, airLoopNum, FirstHVACIteration, HXUnitOn, CompOn));
    EXPECT_EQ(thisSys->m_CoolingPartLoadFrac, 0);
    std::string const expected_error =
        "   ** Warning ** CoilSystem:Cooling:DX - sensible part-load ratio calculation failed: part-load ratio limits exceeded, for unit = DX "
        "COOLING COIL SYSTEM\n   **   ~~~   ** Estimated part-load ratio = 0.000\n   **   ~~~   ** The estimated part-load ratio will be used and "
        "the simulation continues. Occurrence info:\n   **   ~~~   **  Environment=, at Simulation time= 00:-15 - 00:00\n";
    compare_err_stream(expected_error, true);
}

} // namespace EnergyPlus
