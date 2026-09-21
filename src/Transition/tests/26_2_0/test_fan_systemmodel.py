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


def test_fan_systemmodel_no_trailing_fields(run_transition_test):
    """Fan:SystemModel with no fields past Night Ventilation Mode Pressure Rise: nothing to
    shift, CurArgs stays the same."""
    idf_text = """
  Fan:SystemModel,
    Supply Fan,              !- Name
    ,                        !- Availability Schedule Name
    Fan Inlet Node,          !- Air Inlet Node Name
    Fan Outlet Node,         !- Air Outlet Node Name
    autosize,                !- Design Maximum Air Flow Rate {m3/s}
    Discrete,                !- Speed Control Method
    0.2,                     !- Electric Power Minimum Flow Rate Fraction
    500.0,                   !- Design Pressure Rise {Pa}
    0.9,                     !- Motor Efficiency
    1.0,                     !- Motor In Air Stream Fraction
    autosize,                !- Design Electric Power Consumption {W}
    TotalEfficiencyAndPressure,  !- Design Power Sizing Method
    ,                        !- Electric Power Per Unit Flow Rate {W/(m3/s)}
    ,                        !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}
    0.7,                     !- Fan Total Efficiency
    ,                        !- Electric Power Function of Flow Fraction Curve Name
    250.0;                   !- Night Ventilation Mode Pressure Rise {Pa}
    """

    expected_idf_text = """
  Fan:SystemModel,
    Supply Fan,              !- Name
    ,                        !- Availability Schedule Name
    Fan Inlet Node,          !- Air Inlet Node Name
    Fan Outlet Node,         !- Air Outlet Node Name
    autosize,                !- Design Maximum Air Flow Rate {m3/s}
    Discrete,                !- Speed Control Method
    0.2,                     !- Electric Power Minimum Flow Rate Fraction
    500.0,                   !- Design Pressure Rise {Pa}
    0.9,                     !- Motor Efficiency
    1.0,                     !- Motor In Air Stream Fraction
    autosize,                !- Design Electric Power Consumption {W}
    TotalEfficiencyAndPressure,  !- Design Power Sizing Method
    ,                        !- Electric Power Per Unit Flow Rate {W/(m3/s)}
    ,                        !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}
    0.7,                     !- Fan Total Efficiency
    ,                        !- Electric Power Function of Flow Fraction Curve Name
    250.0;                   !- Night Ventilation Mode Pressure Rise {Pa}
"""

    run_transition_test(idf_text, expected_idf_text)


def test_fan_systemmodel_with_flow_fraction_and_trailing_fields(run_transition_test):
    """Fan:SystemModel with the (now removed) Night Ventilation Mode Flow Fraction populated
    and trailing fields present: the old flow fraction value is dropped, three blank fields
    are inserted in its place, and the remaining fields shift down by two positions."""
    idf_text = """
  Fan:SystemModel,
    Supply Fan,              !- Name
    ,                        !- Availability Schedule Name
    Fan Inlet Node,          !- Air Inlet Node Name
    Fan Outlet Node,         !- Air Outlet Node Name
    autosize,                !- Design Maximum Air Flow Rate {m3/s}
    Discrete,                !- Speed Control Method
    0.2,                     !- Electric Power Minimum Flow Rate Fraction
    500.0,                   !- Design Pressure Rise {Pa}
    0.9,                     !- Motor Efficiency
    1.0,                     !- Motor In Air Stream Fraction
    autosize,                !- Design Electric Power Consumption {W}
    TotalEfficiencyAndPressure,  !- Design Power Sizing Method
    ,                        !- Electric Power Per Unit Flow Rate {W/(m3/s)}
    ,                        !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}
    0.7,                     !- Fan Total Efficiency
    ,                        !- Electric Power Function of Flow Fraction Curve Name
    250.0,                   !- Night Ventilation Mode Pressure Rise {Pa}
    1.0,                     !- Night Ventilation Mode Flow Fraction
    ,                        !- Motor Loss Zone Name
    ,                        !- Motor Loss Radiative Fraction
    Fan Energy;               !- End-Use Subcategory
    """

    expected_idf_text = """
  Fan:SystemModel,
    Supply Fan,              !- Name
    ,                        !- Availability Schedule Name
    Fan Inlet Node,          !- Air Inlet Node Name
    Fan Outlet Node,         !- Air Outlet Node Name
    autosize,                !- Design Maximum Air Flow Rate {m3/s}
    Discrete,                !- Speed Control Method
    0.2,                     !- Electric Power Minimum Flow Rate Fraction
    500.0,                   !- Design Pressure Rise {Pa}
    0.9,                     !- Motor Efficiency
    1.0,                     !- Motor In Air Stream Fraction
    autosize,                !- Design Electric Power Consumption {W}
    TotalEfficiencyAndPressure,  !- Design Power Sizing Method
    ,                        !- Electric Power Per Unit Flow Rate {W/(m3/s)}
    ,                        !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}
    0.7,                     !- Fan Total Efficiency
    ,                        !- Electric Power Function of Flow Fraction Curve Name
    250.0,                   !- Night Ventilation Mode Pressure Rise {Pa}
    ,                        !- Night Ventilation Mode Fan Total Efficiency
    ,                        !- Night Ventilation Mode Motor Efficiency
    ,                        !- Night Ventilation Mode Motor In Air Stream Fraction
    ,                        !- Motor Loss Zone Name
    ,                        !- Motor Loss Radiative Fraction
    Fan Energy;              !- End-Use Subcategory
"""

    run_transition_test(idf_text, expected_idf_text)


def test_fan_systemmodel_with_multispeed_fields(run_transition_test):
    """Fan:SystemModel with Number of Speeds and speed field sets present: all fields after
    Night Ventilation Mode Pressure Rise shift down by two positions."""
    idf_text = """
  Fan:SystemModel,
    Supply Fan,              !- Name
    ,                        !- Availability Schedule Name
    Fan Inlet Node,          !- Air Inlet Node Name
    Fan Outlet Node,         !- Air Outlet Node Name
    autosize,                !- Design Maximum Air Flow Rate {m3/s}
    Discrete,                !- Speed Control Method
    0.2,                     !- Electric Power Minimum Flow Rate Fraction
    500.0,                   !- Design Pressure Rise {Pa}
    0.9,                     !- Motor Efficiency
    1.0,                     !- Motor In Air Stream Fraction
    autosize,                !- Design Electric Power Consumption {W}
    TotalEfficiencyAndPressure,  !- Design Power Sizing Method
    ,                        !- Electric Power Per Unit Flow Rate {W/(m3/s)}
    ,                        !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}
    0.7,                     !- Fan Total Efficiency
    ,                        !- Electric Power Function of Flow Fraction Curve Name
    ,                        !- Night Ventilation Mode Pressure Rise {Pa}
    ,                        !- Night Ventilation Mode Flow Fraction
    ,                        !- Motor Loss Zone Name
    ,                        !- Motor Loss Radiative Fraction
    Fan Energy,               !- End-Use Subcategory
    2,                        !- Number of Speeds
    0.5,                       !- Speed 1 Flow Fraction
    0.3,                       !- Speed 1 Electric Power Fraction
    1.0,                       !- Speed 2 Flow Fraction
    1.0;                       !- Speed 2 Electric Power Fraction
    """

    expected_idf_text = """
  Fan:SystemModel,
    Supply Fan,              !- Name
    ,                        !- Availability Schedule Name
    Fan Inlet Node,          !- Air Inlet Node Name
    Fan Outlet Node,         !- Air Outlet Node Name
    autosize,                !- Design Maximum Air Flow Rate {m3/s}
    Discrete,                !- Speed Control Method
    0.2,                     !- Electric Power Minimum Flow Rate Fraction
    500.0,                   !- Design Pressure Rise {Pa}
    0.9,                     !- Motor Efficiency
    1.0,                     !- Motor In Air Stream Fraction
    autosize,                !- Design Electric Power Consumption {W}
    TotalEfficiencyAndPressure,  !- Design Power Sizing Method
    ,                        !- Electric Power Per Unit Flow Rate {W/(m3/s)}
    ,                        !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}
    0.7,                     !- Fan Total Efficiency
    ,                        !- Electric Power Function of Flow Fraction Curve Name
    ,                        !- Night Ventilation Mode Pressure Rise {Pa}
    ,                        !- Night Ventilation Mode Fan Total Efficiency
    ,                        !- Night Ventilation Mode Motor Efficiency
    ,                        !- Night Ventilation Mode Motor In Air Stream Fraction
    ,                        !- Motor Loss Zone Name
    ,                        !- Motor Loss Radiative Fraction
    Fan Energy,              !- End-Use Subcategory
    2,                       !- Number of Speeds
    0.5,                     !- Speed 1 Flow Fraction
    0.3,                     !- Speed 1 Electric Power Fraction
    1.0,                     !- Speed 2 Flow Fraction
    1.0;                     !- Speed 2 Electric Power Fraction
"""

    run_transition_test(idf_text, expected_idf_text)
