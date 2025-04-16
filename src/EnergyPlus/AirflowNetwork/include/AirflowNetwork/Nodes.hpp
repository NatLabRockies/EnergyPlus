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

#ifndef AIRFLOWNETWORK_NODES_HPP
#define AIRFLOWNETWORK_NODES_HPP

#include "AirflowNetwork/Properties.hpp"
#include "AirflowNetwork/Types.hpp"
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/EPVector.hh>
#include <EnergyPlus/ScheduleManager.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;
struct AirflowNetworkData;

namespace AirflowNetwork {

    struct MultizoneZoneProp // Zone information
    {
        // Members
        std::string ZoneName;         // Name of Associated EnergyPlus Thermal Zone
        std::string VentControl;      // Ventilation Control Mode: "TEMPERATURE", "ENTHALPIC", "CONSTANT", or "NOVENT"
        std::string VentAvailSchName; // Ventilation availability schedule
        Real64 Height;                // Nodal height
        Real64 OpenFactor;            // Limit Value on Multiplier for Modulating Venting Open Factor,
        // Not applicable if Vent Control Mode = CONSTANT or NOVENT
        Real64 LowValueTemp; // Lower Value on Inside/Outside Temperature Difference for
        // Modulating the Venting Open Factor with temp control
        Real64 UpValueTemp; // Upper Value on Inside/Outside Temperature Difference for
        // Modulating the Venting Open Factor with temp control
        Real64 LowValueEnth; // Lower Value on Inside/Outside Temperature Difference for
        // Modulating the Venting Open Factor with Enthalpic control
        Real64 UpValueEnth; // Upper Value on Inside/Outside Temperature Difference for
        // Modulating the Venting Open Factor with Enthalpic control
        int ZoneNum;                                     // Zone number associated with ZoneName
        Sched::Schedule *ventTempControlSched = nullptr; // Ventilation temperature control schedule
        int VentCtrNum;                                  // Ventilation control mode number: 1 "Temperature", 2 "ENTHALPIC", 3 "CONSTANT", 4 "NOVENT"
        std::string VentTempControlSchName;              // Name of ventilation temperature control schedule
        Sched::Schedule *ventAvailSched = nullptr;       // Ventilation availability schedule
        std::string SingleSidedCpType;                   // Type of calculation method for single sided wind pressure coefficients
        Real64 BuildWidth;                               // The width of the building along the facade that contains this zone.
        int ASH55PeopleInd;                              // Index of people object with ASH55 comfort calcs for ventilation control
        int CEN15251PeopleInd;                           // Index of people object with CEN15251 comfort calcs for ventilation control
        std::string OccupantVentilationControlName;      // Occupant ventilation control name
        int OccupantVentilationControlNum;               // Occupant ventilation control number
        int RAFNNodeNum;                                 // Index of RAFN node number

        // Default Constructor
        MultizoneZoneProp()
            : VentControl("NoVent"), Height(0.0), OpenFactor(1.0), LowValueTemp(0.0), UpValueTemp(100.0), LowValueEnth(0.0), UpValueEnth(300000.0),
              ZoneNum(0), VentCtrNum(VentControlType::None), SingleSidedCpType("STANDARD"), BuildWidth(10.0), ASH55PeopleInd(0), CEN15251PeopleInd(0),
              OccupantVentilationControlNum(0), RAFNNodeNum(0)
        {
        }
    };

    struct MultizoneExternalNodeProp // External node properties
    {
        // Members
        std::string Name;      // Name of external node
        Real64 azimuth;        // Azimuthal angle of the associated surface
        Real64 height;         // Nodal height
        int ExtNum;            // External node number
        int OutAirNodeNum;     // Outdoor air node number
        int facadeNum;         // Facade number
        int curve;             // Curve ID, replace with pointer after curve refactor
        bool symmetricCurve;   // Symmtric curves are evaluated from 0 to 180, others are evaluated from 0 to 360
        bool useRelativeAngle; // Determines whether the wind angle is relative to the surface or absolute

        // Default Constructor
        MultizoneExternalNodeProp()
            : azimuth(0.0), height(0.0), ExtNum(0), OutAirNodeNum(0), facadeNum(0), curve(0), symmetricCurve(false), useRelativeAngle(false)
        {
        }
    };

    struct IntraZoneNodeProp // Intra zone node data
    {
        // Members
        std::string Name;         // Name of node
        std::string RAFNNodeName; // RoomAir model node name
        Real64 Height;            // Nodal height
        int RAFNNodeNum;          // RoomAir model node number
        int ZoneNum;              // Zone number
        int AFNZoneNum;           // MultiZone number

        // Default Constructor
        IntraZoneNodeProp() : Height(0.0), RAFNNodeNum(0), ZoneNum(0), AFNZoneNum(0)
        {
        }
    };

    struct DisSysNodeProp // CP Value
    {
        // Members
        std::string Name;      // Name of node
        std::string EPlusName; // EnergyPlus node name
        std::string EPlusType; // EnergyPlus node type
        Real64 Height;         // Nodal height
        int EPlusNodeNum;      // EPlus node number
        int AirLoopNum;        // AirLoop number

        // Default Constructor
        DisSysNodeProp() : Height(0.0), EPlusNodeNum(0), AirLoopNum(0)
        {
        }
    };

    struct AirflowNetworkNodeProp // AirflowNetwork nodal data
    {
        // Members
        std::string Name;      // Provide a unique node name
        std::string NodeType;  // Provide node type "External", "Thermal Zone" or "Other"
        std::string EPlusNode; // EnergyPlus node name
        Real64 NodeHeight;     // Node height [m]
        int NodeNum;           // Node number
        int NodeTypeNum;       // Node type with integer number: 0 - Calculated, 1 - Given pressure;
        std::string EPlusZoneName; // EnergyPlus node name
        int EPlusZoneNum;          // E+ zone number
        int EPlusNodeNum;
        int ExtNodeNum;
        int OutAirNodeNum;
        EPlusNodeType EPlusTypeNum;
        int RAFNNodeNum; // RoomAir model node number
        int NumOfLinks;  // Number of links for RoomAir model
        int AirLoopNum;  // AirLoop number

        // Default Constructor
        AirflowNetworkNodeProp()
            : NodeHeight(0.0), NodeNum(0), NodeTypeNum(0), EPlusZoneNum(0), EPlusNodeNum(0), ExtNodeNum(0), OutAirNodeNum(0),
              EPlusTypeNum(EPlusNodeType::Invalid), RAFNNodeNum(0), NumOfLinks(0), AirLoopNum(0)
        {
        }
    };

    struct AirflowNetworkNodeSimuData // Node variable for simulation
    {
        // Members
        Real64 TZ;       // Temperature [C]
        Real64 WZ;       // Humidity ratio [kg/kg]
        Real64 PZ;       // Pressure [Pa]
        Real64 CO2Z;     // CO2 [ppm]
        Real64 GCZ;      // Generic contaminant [ppm]
        Real64 TZlast;   // Temperature [C] at previous time step
        Real64 WZlast;   // Humidity ratio [kg/kg] at previous time step
        Real64 CO2Zlast; // CO2 [ppm] at previous time step
        Real64 GCZlast;  // Generic contaminant [ppm] at previous time step

        // Default Constructor
        AirflowNetworkNodeSimuData() : TZ(0.0), WZ(0.0), PZ(0.0), CO2Z(0.0), GCZ(0.0), TZlast(0.0), WZlast(0.0), CO2Zlast(0.0), GCZlast(0.0)
        {
        }
    };

    struct AirflowNetworkNodeReportData // Node variable for simulation
    {
        // Members
        Real64 PZ;    // Average Pressure [Pa]
        Real64 PZON;  // Pressure with fan on [Pa]
        Real64 PZOFF; // Pressure with fan off [Pa]

        // Default Constructor
        AirflowNetworkNodeReportData() : PZ(0.0), PZON(0.0), PZOFF(0.0)
        {
        }
    };

} // namespace AirflowNetwork

} // namespace EnergyPlus

#endif
