# EnergyPlus, Copyright (c) 1996-present, The Board of Trustees of the
# University of Illinois, The Regents of the University of California, through
# Lawrence Berkeley National Laboratory (subject to receipt of any required
# approvals from the U.S. Dept. of Energy), Oak Ridge National Laboratory,
# managed by UT-Battelle, Alliance for Energy Innovation, LLC, and other
# contributors. All rights reserved.
#
# NOTICE: This Software was developed under funding from the U.S. Department of
# Energy and the U.S. Government consequently retains certain rights. As such,
# the U.S. Government has been granted for itself and others acting on its
# behalf a paid-up, nonexclusive, irrevocable, worldwide license in the
# Software to reproduce, distribute copies to the public, prepare derivative
# works, and perform publicly and display publicly, and to permit others to do
# so.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# (1) Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#
# (2) Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#
# (3) Neither the name of the University of California, Lawrence Berkeley
#     National Laboratory, the University of Illinois, U.S. Dept. of Energy nor
#     the names of its contributors may be used to endorse or promote products
#     derived from this software without specific prior written permission.
#
# (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in
#     stand-alone form without changes from the version obtained under this
#     License, or (ii) Licensee makes a reference solely to the software
#     portion of its product, Licensee must refer to the software as
#     "EnergyPlus version X" software, where "X" is the version number Licensee
#     obtained under this License and may not use a different name for the
#     software. Except as specifically required in this Section (4), Licensee
#     shall not use in a company name, a product name, in advertising,
#     publicity, or other promotional activities any name, trade name,
#     trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or
#     confusingly similar designation, without the U.S. Department of Energy's
#     prior written consent.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

from conftest import ExpandObjectsResult


def test_ideal_loads_without_exising_humidistat(prepare_and_run_expandobjects):
    """Test that Ideal Loads with a Humidistat are expanded correctly when no Humidistat exists in the original model."""
    ori_idf_text = """
Zone,
  Zone 1;                                 !- Name

HVACTemplate:Zone:IdealLoadsAirSystem,
  Zone 1,                                 !- Zone Name
  ,                                       !- Template Thermostat Name
  ,                                       !- System Availability Schedule Name
  ,                                       !- Maximum Heating Supply Air Temperature {C}
  ,                                       !- Minimum Cooling Supply Air Temperature {C}
  ,                                       !- Maximum Heating Supply Air Humidity Ratio {kgWater/kgDryAir}
  ,                                       !- Minimum Cooling Supply Air Humidity Ratio {kgWater/kgDryAir}
  ,                                       !- Heating Limit
  ,                                       !- Maximum Heating Air Flow Rate {m3/s}
  ,                                       !- Maximum Sensible Heating Capacity {W}
  ,                                       !- Cooling Limit
  ,                                       !- Maximum Cooling Air Flow Rate {m3/s}
  ,                                       !- Maximum Total Cooling Capacity {W}
  ,                                       !- Heating Availability Schedule Name
  ,                                       !- Cooling Availability Schedule Name
  Humidistat,                             !- Dehumidification Control Type
  ,                                       !- Cooling Sensible Heat Ratio {dimensionless}
  ,                                       !- Dehumidification Setpoint {percent}
  Humidistat,                             !- Humidification Control Type
  ,                                       !- Humidification Setpoint {percent}
  ,                                       !- Outdoor Air Method
  ,                                       !- Outdoor Air Flow Rate per Person {m3/s}
  ,                                       !- Outdoor Air Flow Rate per Zone Floor Area {m3/s-m2}
  ,                                       !- Outdoor Air Flow Rate per Zone {m3/s}
  ,                                       !- Design Specification Outdoor Air Object Name
  ;                                       !- Demand Controlled Ventilation Type
    """

    result: ExpandObjectsResult = prepare_and_run_expandobjects(ori_idf_text=ori_idf_text)
    assert result.err_text is None, f"ExpandObjects failed with error: {result.err_text}"
    assert (
        "! HVACTemplate:Zone:IdealLoadsAirSystem," in result.idf_text
    ), "Ideal Loads Air System should be preserved in the expanded IDF"
    expected_addition_text = """
ZoneControl:Humidistat,
  Zone 1 Humidistat,                                       !- Name
  Zone 1,                                                  !- Zone Name
  HVACTemplate-Always 30,                                  !- Humidifying Setpoint Schedule Name
  HVACTemplate-Always 60,                                  !- Dehumidifying Setpoint Schedule Name
  RelativeHumidity;                                        !- Control Variable

ScheduleTypeLimits,
  HVACTemplate Any Number;                                 !- Name

Schedule:Compact,
  HVACTemplate-Always 30,                                  !- Name
  HVACTemplate Any Number,                                 !- Schedule Type Limits Name
  Through: 12/31,                                          !- Field 1
  For: AllDays,                                            !- Field 2
  Until: 24:00,                                            !- Field 3
  30;                                                      !- Field 4

Schedule:Compact,
  HVACTemplate-Always 60,                                  !- Name
  HVACTemplate Any Number,                                 !- Schedule Type Limits Name
  Through: 12/31,                                          !- Field 1
  For: AllDays,                                            !- Field 2
  Until: 24:00,                                            !- Field 3
  60;                                                      !- Field 4

ZoneHVAC:EquipmentConnections,
  Zone 1,                                                  !- Zone Name
  Zone 1 Equipment,                                        !- Zone Conditioning Equipment List Name
  Zone 1 Ideal Loads Supply Inlet,                         !- Zone Air Inlet Node or NodeList Name
  ,                                                        !- Zone Air Exhaust Node or NodeList Name
  Zone 1 Zone Air Node,                                    !- Zone Air Node Name
  Zone 1 Return Outlet;                                    !- Zone Return Air Node Name

ZoneHVAC:EquipmentList,
  Zone 1 Equipment,                                        !- Name
  SequentialLoad,                                          !- Load Distribution Scheme
  ZoneHVAC:IdealLoadsAirSystem,                            !- Zone Equipment Object Type
  Zone 1 Ideal Loads Air System,                           !- Zone Equipment Name
  1,                                                       !- Zone Equipment Cooling Sequence
  1,                                                       !- Zone Equipment Heating or No-Load Sequence
  ,                                                        !- Zone Equipment Sequential Cooling Fraction Schedule Name
  ;                                                        !- Zone Equipment Sequential Heating Fraction Schedule Name

ZoneHVAC:IdealLoadsAirSystem,
  Zone 1 Ideal Loads Air System,                           !- Name
  ,                                                        !- Availability Schedule Name
  Zone 1 Ideal Loads Supply Inlet,                         !- Zone Supply Air Node Name
  ,                                                        !- Zone Exhaust Air Node Name
  ,                                                        !- System Inlet Air Node Name
  50,                                                      !- Maximum Heating Supply Air Temperature [C]
  13,                                                      !- Minimum Cooling Supply Air Temperature [C]
  0.0156,                                                  !- Maximum Heating Supply Air Humidity Ratio [kg-H20/kg-air]
  0.0077,                                                  !- Minimum Cooling Supply Air Humidity Ratio [kg-H20/kg-air]
  NoLimit,                                                 !- Heating Limit
  ,                                                        !- Maximum Heating Air Flow Rate {m3/s}
  ,                                                        !- Maximum Sensible Heating Capacity {m3/s}
  NoLimit,                                                 !- Cooling Limit
  ,                                                        !- Maximum Cooling Air Flow Rate {m3/s}
  ,                                                        !- Maximum Total Cooling Capacity {m3/s}
  ,                                                        !- Heating Availability Schedule Name
  ,                                                        !- Cooling Availability Schedule Name
  Humidistat,                                              !- Dehumidification Control Type
  0.7,                                                     !- Cooling Sensible Heat Ratio
  Humidistat,                                              !- Humidification Control Type
  ,                                                        !- Design Specification Outdoor Air Object Name
  ,                                                        !- Outdoor Air Inlet Node Name
  None,                                                    !- Demand Controlled Ventilation Type
  NoEconomizer,                                            !- Outdoor Air Economizer Type
  None,                                                    !- Heat Recovery Type
  0.7,                                                     !- Sensible Heat Recovery Effectiveness
  0.65;                                                    !- Latent Heat Recovery Effectiveness
"""

    assert expected_addition_text.strip() in result.new_section_text


def test_ideal_loads_with_exising_humidistat(prepare_and_run_expandobjects):

    ori_idf_text = """
Zone,
  Zone 1;                                 !- Name

HVACTemplate:Zone:IdealLoadsAirSystem,
  Zone 1,                                 !- Zone Name
  ,                                       !- Template Thermostat Name
  ,                                       !- System Availability Schedule Name
  ,                                       !- Maximum Heating Supply Air Temperature {C}
  ,                                       !- Minimum Cooling Supply Air Temperature {C}
  ,                                       !- Maximum Heating Supply Air Humidity Ratio {kgWater/kgDryAir}
  ,                                       !- Minimum Cooling Supply Air Humidity Ratio {kgWater/kgDryAir}
  ,                                       !- Heating Limit
  ,                                       !- Maximum Heating Air Flow Rate {m3/s}
  ,                                       !- Maximum Sensible Heating Capacity {W}
  ,                                       !- Cooling Limit
  ,                                       !- Maximum Cooling Air Flow Rate {m3/s}
  ,                                       !- Maximum Total Cooling Capacity {W}
  ,                                       !- Heating Availability Schedule Name
  ,                                       !- Cooling Availability Schedule Name
  Humidistat,                             !- Dehumidification Control Type
  ,                                       !- Cooling Sensible Heat Ratio {dimensionless}
  ,                                       !- Dehumidification Setpoint {percent}
  Humidistat,                             !- Humidification Control Type
  ,                                       !- Humidification Setpoint {percent}
  ,                                       !- Outdoor Air Method
  ,                                       !- Outdoor Air Flow Rate per Person {m3/s}
  ,                                       !- Outdoor Air Flow Rate per Zone Floor Area {m3/s-m2}
  ,                                       !- Outdoor Air Flow Rate per Zone {m3/s}
  ,                                       !- Design Specification Outdoor Air Object Name
  ;                                       !- Demand Controlled Ventilation Type

ZoneControl:Humidistat,
  Zone Control Humidistat 1,              !- Name
  Zone 1,                                 !- Zone Name
  Humidifying Setpoint Schedule,          !- Humidifying Setpoint Schedule Name
  Dehumidifying Setpoint Schedule;        !- Dehumidifying Setpoint Schedule Name

ScheduleTypeLimits,
  Any Number;                             !- Name

Schedule:Constant,
  Humidifying Setpoint Schedule,          !- Name
  Any Number,                             !- Schedule Type Limits Name
  30.0;                                   !- Hourly Value

Schedule:Constant,
  Dehumidifying Setpoint Schedule,        !- Name
  Any Number,                             !- Schedule Type Limits Name
  60.0;                                   !- Hourly Value
"""
    result: ExpandObjectsResult = prepare_and_run_expandobjects(ori_idf_text=ori_idf_text)
    assert result.err_text is not None
    assert (
        'ExpandObjects: Warning: In HVACTemplate:Zone:IdealLoadsAirSystem "Zone 1" a ZoneControl:Humidistat named '
        '"Zone Control Humidistat 1" was already defined for this zone, so the HVACTemplate-generated humidistat '
        "will not be created."
    ) in result.err_text
    assert (
        "! HVACTemplate:Zone:IdealLoadsAirSystem," in result.idf_text
    ), "Ideal Loads Air System should be preserved in the expanded IDF"

    assert "ZoneControl:Humidistat," not in result.new_section_text
    assert "HVACTemplate-Always 30," not in result.new_section_text
    assert "HVACTemplate-Always 60," not in result.new_section_text

    expected_addition_text = """
ZoneHVAC:EquipmentConnections,
  Zone 1,                                                  !- Zone Name
  Zone 1 Equipment,                                        !- Zone Conditioning Equipment List Name
  Zone 1 Ideal Loads Supply Inlet,                         !- Zone Air Inlet Node or NodeList Name
  ,                                                        !- Zone Air Exhaust Node or NodeList Name
  Zone 1 Zone Air Node,                                    !- Zone Air Node Name
  Zone 1 Return Outlet;                                    !- Zone Return Air Node Name

ZoneHVAC:EquipmentList,
  Zone 1 Equipment,                                        !- Name
  SequentialLoad,                                          !- Load Distribution Scheme
  ZoneHVAC:IdealLoadsAirSystem,                            !- Zone Equipment Object Type
  Zone 1 Ideal Loads Air System,                           !- Zone Equipment Name
  1,                                                       !- Zone Equipment Cooling Sequence
  1,                                                       !- Zone Equipment Heating or No-Load Sequence
  ,                                                        !- Zone Equipment Sequential Cooling Fraction Schedule Name
  ;                                                        !- Zone Equipment Sequential Heating Fraction Schedule Name

ZoneHVAC:IdealLoadsAirSystem,
  Zone 1 Ideal Loads Air System,                           !- Name
  ,                                                        !- Availability Schedule Name
  Zone 1 Ideal Loads Supply Inlet,                         !- Zone Supply Air Node Name
  ,                                                        !- Zone Exhaust Air Node Name
  ,                                                        !- System Inlet Air Node Name
  50,                                                      !- Maximum Heating Supply Air Temperature [C]
  13,                                                      !- Minimum Cooling Supply Air Temperature [C]
  0.0156,                                                  !- Maximum Heating Supply Air Humidity Ratio [kg-H20/kg-air]
  0.0077,                                                  !- Minimum Cooling Supply Air Humidity Ratio [kg-H20/kg-air]
  NoLimit,                                                 !- Heating Limit
  ,                                                        !- Maximum Heating Air Flow Rate {m3/s}
  ,                                                        !- Maximum Sensible Heating Capacity {m3/s}
  NoLimit,                                                 !- Cooling Limit
  ,                                                        !- Maximum Cooling Air Flow Rate {m3/s}
  ,                                                        !- Maximum Total Cooling Capacity {m3/s}
  ,                                                        !- Heating Availability Schedule Name
  ,                                                        !- Cooling Availability Schedule Name
  Humidistat,                                              !- Dehumidification Control Type
  0.7,                                                     !- Cooling Sensible Heat Ratio
  Humidistat,                                              !- Humidification Control Type
  ,                                                        !- Design Specification Outdoor Air Object Name
  ,                                                        !- Outdoor Air Inlet Node Name
  None,                                                    !- Demand Controlled Ventilation Type
  NoEconomizer,                                            !- Outdoor Air Economizer Type
  None,                                                    !- Heat Recovery Type
  0.7,                                                     !- Sensible Heat Recovery Effectiveness
  0.65;                                                    !- Latent Heat Recovery Effectiveness

Output:PreprocessorMessage,
  ExpandObjects,                                           !- Preprocessor Name
  Warning,                                                 !- Error Severity
  Warning: In HVACTemplate:Zone:IdealLoadsAirSystem "Zone 1" a,  !- message line
  ZoneControl:Humidistat named "Zone Control Humidistat 1" was already,  !- message line
  defined for this zone, so the HVACTemplate-generated humidistat will not be,  !- message line
  created.;                                                !- message line
"""

    assert expected_addition_text.strip() in result.new_section_text


def test_ideal_loads_with_exising_humidistat_but_no_dehum_schedule(prepare_and_run_expandobjects):
    """Test IdealLoadsAirSystem with Humidistat but existing ZoneControl:Humidistat has no dehum schedule.

    IdealLoadsAirSystem has Humidistat for both dehumidification and humidification control types.
    The existing ZoneControl:Humidistat is missing the Dehumidifying Schedule => hard error
    """

    ori_idf_text = """
Zone,
  Zone 1;                                 !- Name

HVACTemplate:Zone:IdealLoadsAirSystem,
  Zone 1,                                 !- Zone Name
  ,                                       !- Template Thermostat Name
  ,                                       !- System Availability Schedule Name
  ,                                       !- Maximum Heating Supply Air Temperature {C}
  ,                                       !- Minimum Cooling Supply Air Temperature {C}
  ,                                       !- Maximum Heating Supply Air Humidity Ratio {kgWater/kgDryAir}
  ,                                       !- Minimum Cooling Supply Air Humidity Ratio {kgWater/kgDryAir}
  ,                                       !- Heating Limit
  ,                                       !- Maximum Heating Air Flow Rate {m3/s}
  ,                                       !- Maximum Sensible Heating Capacity {W}
  ,                                       !- Cooling Limit
  ,                                       !- Maximum Cooling Air Flow Rate {m3/s}
  ,                                       !- Maximum Total Cooling Capacity {W}
  ,                                       !- Heating Availability Schedule Name
  ,                                       !- Cooling Availability Schedule Name
  Humidistat,                             !- Dehumidification Control Type
  ,                                       !- Cooling Sensible Heat Ratio {dimensionless}
  ,                                       !- Dehumidification Setpoint {percent}
  Humidistat,                             !- Humidification Control Type
  ,                                       !- Humidification Setpoint {percent}
  ,                                       !- Outdoor Air Method
  ,                                       !- Outdoor Air Flow Rate per Person {m3/s}
  ,                                       !- Outdoor Air Flow Rate per Zone Floor Area {m3/s-m2}
  ,                                       !- Outdoor Air Flow Rate per Zone {m3/s}
  ,                                       !- Design Specification Outdoor Air Object Name
  ;                                       !- Demand Controlled Ventilation Type

ZoneControl:Humidistat,
  Zone Control Humidistat 1,              !- Name
  Zone 1,                                 !- Zone Name
  Humidifying Setpoint Schedule,          !- Humidifying Setpoint Schedule Name
  ;                                       !- Dehumidifying Setpoint Schedule Name

ScheduleTypeLimits,
  Any Number;                             !- Name

Schedule:Constant,
  Humidifying Setpoint Schedule,          !- Name
  Any Number,                             !- Schedule Type Limits Name
  30.0;                                   !- Hourly Value
"""
    result: ExpandObjectsResult = prepare_and_run_expandobjects(ori_idf_text=ori_idf_text)
    assert result.err_text is not None
    assert (
        'ExpandObjects: Warning: In HVACTemplate:Zone:IdealLoadsAirSystem "Zone 1" a ZoneControl:Humidistat named '
        '"Zone Control Humidistat 1" was already defined for this zone, so the HVACTemplate-generated humidistat '
        "will not be created."
    ) in result.err_text
    assert (
        'ExpandObjects: In HVACTemplate:Zone:IdealLoadsAirSystem "Zone 1" the Dehumidification Control Type field '
        'is Humidistat, but the existing ZoneControl:Humidistat named "Zone Control Humidistat 1" for this zone '
        "has a blank Dehumidifying Setpoint Schedule Name."
    ) in result.err_text

    assert "! HVACTemplate:Zone:IdealLoadsAirSystem," in result.existing_section_text

    assert (
        """
Output:PreprocessorMessage,
  ExpandObjects,                                           !- Preprocessor Name
  Fatal,                                                   !- Error Severity
  In HVACTemplate:Zone:IdealLoadsAirSystem "Zone 1" the Dehumidification,  !- message line
  Control Type field is Humidistat, but the existing ZoneControl:Humidistat,  !- message line
  named "Zone Control Humidistat 1" for this zone has a blank Dehumidifying,  !- message line
  Setpoint Schedule Name.;                                 !- message line"""
        in result.new_section_text
    )


def test_ideal_loads_with_exising_humidistat_but_no_hum_schedule(prepare_and_run_expandobjects):
    """Test IdealLoadsAirSystem with Humidistat but existing ZoneControl:Humidistat has no hum schedule.

    IdealLoadsAirSystem has Humidistat for both dehumidification and humidification control types.
    The existing ZoneControl:Humidistat is missing the Humidifying Schedule => hard error
    """

    ori_idf_text = """
Zone,
  Zone 1;                                 !- Name

HVACTemplate:Zone:IdealLoadsAirSystem,
  Zone 1,                                 !- Zone Name
  ,                                       !- Template Thermostat Name
  ,                                       !- System Availability Schedule Name
  ,                                       !- Maximum Heating Supply Air Temperature {C}
  ,                                       !- Minimum Cooling Supply Air Temperature {C}
  ,                                       !- Maximum Heating Supply Air Humidity Ratio {kgWater/kgDryAir}
  ,                                       !- Minimum Cooling Supply Air Humidity Ratio {kgWater/kgDryAir}
  ,                                       !- Heating Limit
  ,                                       !- Maximum Heating Air Flow Rate {m3/s}
  ,                                       !- Maximum Sensible Heating Capacity {W}
  ,                                       !- Cooling Limit
  ,                                       !- Maximum Cooling Air Flow Rate {m3/s}
  ,                                       !- Maximum Total Cooling Capacity {W}
  ,                                       !- Heating Availability Schedule Name
  ,                                       !- Cooling Availability Schedule Name
  Humidistat,                             !- Dehumidification Control Type
  ,                                       !- Cooling Sensible Heat Ratio {dimensionless}
  ,                                       !- Dehumidification Setpoint {percent}
  Humidistat,                             !- Humidification Control Type
  ,                                       !- Humidification Setpoint {percent}
  ,                                       !- Outdoor Air Method
  ,                                       !- Outdoor Air Flow Rate per Person {m3/s}
  ,                                       !- Outdoor Air Flow Rate per Zone Floor Area {m3/s-m2}
  ,                                       !- Outdoor Air Flow Rate per Zone {m3/s}
  ,                                       !- Design Specification Outdoor Air Object Name
  ;                                       !- Demand Controlled Ventilation Type

ZoneControl:Humidistat,
  Zone Control Humidistat 1,              !- Name
  Zone 1,                                 !- Zone Name
  ,                                       !- Humidifying Setpoint Schedule Name
  Dehumidifying Setpoint Schedule,        !- Dehumidifying Setpoint Schedule Name
  RelativeHumidity;                       !- Control Variable

ScheduleTypeLimits,
  Any Number;                             !- Name

Schedule:Constant,
  Dehumidifying Setpoint Schedule,        !- Name
  Any Number,                             !- Schedule Type Limits Name
  60.0;                                   !- Hourly Value
"""
    result: ExpandObjectsResult = prepare_and_run_expandobjects(ori_idf_text=ori_idf_text)
    assert result.err_text is not None
    assert (
        'ExpandObjects: Warning: In HVACTemplate:Zone:IdealLoadsAirSystem "Zone 1" a ZoneControl:Humidistat named '
        '"Zone Control Humidistat 1" was already defined for this zone, so the HVACTemplate-generated humidistat '
        "will not be created."
    ) in result.err_text
    assert (
        'ExpandObjects: In HVACTemplate:Zone:IdealLoadsAirSystem "Zone 1" the Humidification Control Type field '
        'is Humidistat, but the existing ZoneControl:Humidistat named "Zone Control Humidistat 1" for this zone '
        "has a blank Humidifying Setpoint Schedule Name."
    ) in result.err_text

    assert "! HVACTemplate:Zone:IdealLoadsAirSystem," in result.existing_section_text

    assert (
        """
Output:PreprocessorMessage,
  ExpandObjects,                                           !- Preprocessor Name
  Fatal,                                                   !- Error Severity
  In HVACTemplate:Zone:IdealLoadsAirSystem "Zone 1" the Humidification,  !- message line
  Control Type field is Humidistat, but the existing ZoneControl:Humidistat,  !- message line
  named "Zone Control Humidistat 1" for this zone has a blank Humidifying,  !- message line
  Setpoint Schedule Name.;                                 !- message line"""
        in result.new_section_text
    )
