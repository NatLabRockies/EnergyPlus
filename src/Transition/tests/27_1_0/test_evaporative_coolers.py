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


def test_evaporative_cooler_object_type_references_are_renamed(prepare_and_run_transition):
    idf_text = """
  AirLoopHVAC:OutdoorAirSystem:EquipmentList,
    OA Equipment,
    EVAPORATIVECOOLER:DIRECT:RESEARCHSPECIAL,
    Direct Cooler;

  Branch,
    Main Branch,
    ,
    evaporativecooler:indirect:researchspecial,
    Indirect Cooler,
    Branch Inlet,
    Branch Outlet;

  ZoneHVAC:EvaporativeCoolerUnit,
    Zone Evaporative Cooler,
    ,
    ,
    Outdoor Air Inlet,
    Zone Air Inlet,
    Zone Relief Air,
    Fan:OnOff,
    Supply Fan,
    1.0,
    BlowThrough,
    ZoneCoolingLoadOnOffCycling,
    1.0,
    100.0,
    EvaporativeCooler:Direct:ResearchSpecial,
    Direct Cooler,
    EVAPORATIVECOOLER:INDIRECT:RESEARCHSPECIAL,
    Indirect Cooler;
    """

    expected_idf_text = """
  AirLoopHVAC:OutdoorAirSystem:EquipmentList,
    OA Equipment,            !- Name
    EvaporativeCooler:Direct:UserEffectiveness,  !- Component 1 Object Type
    Direct Cooler;           !- Component 1 Name

  Branch,
    Main Branch,             !- Name
    ,                        !- Pressure Drop Curve Name
    EvaporativeCooler:Indirect:UserEffectiveness,  !- Component 1 Object Type
    Indirect Cooler,         !- Component 1 Name
    Branch Inlet,            !- Component 1 Inlet Node Name
    Branch Outlet;           !- Component 1 Outlet Node Name

  ZoneHVAC:EvaporativeCoolerUnit,
    Zone Evaporative Cooler, !- Name
    ,                        !- Availability Schedule Name
    ,                        !- Availability Manager List Name
    Outdoor Air Inlet,       !- Outdoor Air Inlet Node Name
    Zone Air Inlet,          !- Cooler Outlet Node Name
    Zone Relief Air,         !- Zone Relief Air Node Name
    Fan:OnOff,               !- Supply Air Fan Object Type
    Supply Fan,              !- Supply Air Fan Name
    1.0,                     !- Design Supply Air Flow Rate {m3/s}
    BlowThrough,             !- Fan Placement
    ZoneCoolingLoadOnOffCycling,  !- Cooler Unit Control Method
    1.0,                     !- Throttling Range Temperature Difference {deltaC}
    100.0,                   !- Cooling Load Control Threshold Heat Transfer Rate {W}
    EvaporativeCooler:Direct:UserEffectiveness,  !- First Evaporative Cooler Object Type
    Direct Cooler,           !- First Evaporative Cooler Object Name
    EvaporativeCooler:Indirect:UserEffectiveness,  !- Second Evaporative Cooler Object Type
    Indirect Cooler;         !- Second Evaporative Cooler Name
"""

    prepare_and_run_transition(
        ori_idf_text=idf_text,
        expected_idf_text=expected_idf_text,
        version_ori=(26, 2, 0),
        version_new=(27, 1, 0),
    )


def test_evaporative_cooler_objects_are_renamed_and_fields_are_preserved(prepare_and_run_transition):
    idf_text = """
  EvaporativeCooler:Direct:ResearchSpecial,
    Direct Cooler,
    Always On,
    0.72,
    Direct Effectiveness Curve,
    1.25,
    42.0,
    111.0,
    Direct Pump Curve,
    Direct Inlet Node,
    Direct Outlet Node,
    Direct Sensor Node,
    Direct Tank,
    0.01,
    3.5,
    10.0,
    22.0,
    30.0;

  EvaporativeCooler:Indirect:ResearchSpecial,
    Indirect Cooler,
    Always On,
    0.73,
    Wetbulb Effectiveness Curve,
    0.44,
    Drybulb Effectiveness Curve,
    84.0,
    122.0,
    Indirect Pump Curve,
    1.55,
    1.15,
    390.0,
    275.0,
    Indirect Fan Curve,
    Primary Inlet Node,
    Primary Outlet Node,
    1.45,
    0.88,
    Secondary Inlet Node,
    Secondary Outlet Node,
    Indirect Sensor Node,
    Relief Inlet Node,
    Indirect Tank,
    0.02,
    4.0,
    9.0,
    23.0,
    31.0;
    """

    expected_idf_text = """
  EvaporativeCooler:Direct:UserEffectiveness,
    Direct Cooler,           !- Name
    Always On,               !- Availability Schedule Name
    0.72,                    !- Cooler Design Effectiveness
    Direct Effectiveness Curve,  !- Effectiveness Flow Ratio Modifier Curve Name
    1.25,                    !- Primary Air Design Flow Rate {m3/s}
    42.0,                    !- Recirculating Water Pump Design Power {W}
    111.0,                   !- Design Water Pump Power Per Unit Flow Rate {W/(m3/s)}
    Direct Pump Curve,       !- Water Pump Power Modifier Curve Name
    Direct Inlet Node,       !- Air Inlet Node Name
    Direct Outlet Node,      !- Air Outlet Node Name
    Direct Sensor Node,      !- Sensor Node Name
    Direct Tank,             !- Water Supply Storage Tank Name
    0.01,                    !- Drift Loss Fraction
    3.5,                     !- Blowdown Concentration Ratio
    10.0,                    !- Evaporative Operation Minimum Drybulb Temperature
    22.0,                    !- Evaporative Operation Maximum Limit Wetbulb Temperature
    30.0;                    !- Evaporative Operation Maximum Limit Drybulb Temperature

  EvaporativeCooler:Indirect:UserEffectiveness,
    Indirect Cooler,         !- Name
    Always On,               !- Availability Schedule Name
    0.73,                    !- Cooler Wetbulb Design Effectiveness
    Wetbulb Effectiveness Curve,  !- Wetbulb Effectiveness Flow Ratio Modifier Curve Name
    0.44,                    !- Cooler Drybulb Design Effectiveness
    Drybulb Effectiveness Curve,  !- Drybulb Effectiveness Flow Ratio Modifier Curve Name
    84.0,                    !- Recirculating Water Pump Design Power {W}
    122.0,                   !- Design Water Pump Power Per Unit Flow Rate {W/(m3/s)}
    Indirect Pump Curve,     !- Water Pump Power Modifier Curve Name
    1.55,                    !- Secondary Air Design Flow Rate {m3/s}
    1.15,                    !- Secondary Air Flow Scaling Factor {dimensionless}
    390.0,                   !- Secondary Air Fan Design Power {W}
    275.0,                   !- Secondary Air Fan Sizing Specific Power {W/(m3/s)}
    Indirect Fan Curve,      !- Secondary Air Fan Power Modifier Curve Name
    Primary Inlet Node,      !- Primary Air Inlet Node Name
    Primary Outlet Node,     !- Primary Air Outlet Node Name
    1.45,                    !- Primary Air Design Flow Rate {m3/s}
    0.88,                    !- Dewpoint Effectiveness Factor
    Secondary Inlet Node,    !- Secondary Air Inlet Node Name
    Secondary Outlet Node,   !- Secondary Air Outlet Node Name
    Indirect Sensor Node,    !- Sensor Node Name
    Relief Inlet Node,       !- Relief Air Inlet Node Name
    Indirect Tank,           !- Water Supply Storage Tank Name
    0.02,                    !- Drift Loss Fraction
    4.0,                     !- Blowdown Concentration Ratio
    9.0,                     !- Evaporative Operation Minimum Limit Secondary Air Drybulb Temperature
    23.0,                    !- Evaporative Operation Maximum Limit Outdoor Wetbulb Temperature
    31.0;                    !- Dry Operation Maximum Limit Outdoor Drybulb Temperature
"""

    prepare_and_run_transition(
        ori_idf_text=idf_text,
        expected_idf_text=expected_idf_text,
        version_ori=(26, 2, 0),
        version_new=(27, 1, 0),
    )
