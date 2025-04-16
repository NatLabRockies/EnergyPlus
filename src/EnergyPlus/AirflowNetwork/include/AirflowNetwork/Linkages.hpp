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

#ifndef AIRFLOWNETWORK_LINKAGES_HPP
#define AIRFLOWNETWORK_LINKAGES_HPP

#include "AirflowNetwork/Properties.hpp"
#include "AirflowNetwork/Types.hpp"
#include "EnergyPlus/DataHVACGlobals.hh"
#include "EnergyPlus/EPVector.hh"
#include "EnergyPlus/ScheduleManager.hh"
#include "EnergyPlus/DataSurfaces.hh"
#include <string>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;
struct AirflowNetworkData;

namespace AirflowNetwork {

    // Forward declaration
    struct AirflowElement;

    struct MultizoneSurfaceProp // Surface information
    {
        // Members
        std::string surface_name; // Name of associated EnergyPlus surface
        int surface_number;       // Surface number associated EnergyPlus surface
        Real64 height;            // Surface height
        Real64 width;             // Surface width
        Real64 centroid_height;   // Surface central height in z direction

        std::string opening_name;      // Name of opening component, either simple or detailed large opening TO BE REMOVED
        std::string external_node_name;  // Name of external node, but not used at WPC="INPUT" TO BE REMOVED
        Real64 factor;                // Crack Actual Value or Window Open Factor for Ventilation
        
        // Ventilation Control - needs to be broken into a separate object or objects
        VentControlType ventilation_control_type; // Ventilation control mode number: 1 "Temperature", 2 "ENTHALPIC", 3 "CONSTANT", 4 "NOVENT"
        Real64 open_factor_limit;                 // Limit Value on Multiplier for Modulating Venting Open Factor
        Real64 temperature_lower_limit;           // Lower Value on Inside/Outside Temperature Difference for Modulating the Venting Open Factor with temp control
        Real64 temperature_upper_limit;           // Upper Value on Inside/Outside Temperature Difference for Modulating the Venting Open Factor with temp control
        Real64 enthalpy_lower_limit;              // Lower Value on Inside/Outside Temperature Difference for Modulating the Venting Open Factor with Enthalpic control
        Real64 enthalpy_upper_limit;              // Upper Value on Inside/Outside Temperature Difference for Modulating the Venting Open Factor with Enthalpic control
        std::string VentTempControlSchName;              // Name of ventilation temperature control schedule
        Sched::Schedule *ventTempControlSched = nullptr; // Ventilation temperature control schedule
        std::string VentAvailSchName;                    // Ventilation availability schedule
        Sched::Schedule *ventAvailSched = nullptr;       // Ventilation availability schedule
        int ZonePtr{0};                                  // Pointer to inside face zone
        bool IndVentControl{false};                      // Individual surface venting control

        std::array<int, 2> NodeNums = {0, 0}; // Positive: Zone numbers; 0: External
        Real64 OpenFactor = 0.0;              // Surface factor
        Real64 OpenFactorLast = 0.0;          // Surface factor at previous time step
        bool EMSOpenFactorActuated = false;   // True if EMS actuation is on
        Real64 EMSOpenFactor = 0.0;           // Surface factor value from EMS for override

        // Occupant Ventilation Control
        int occupant_control_number;               // Occupant ventilation control number
        int OpeningStatus = OpenStatus::FreeOperation;         // Open status at current time step
        int PrevOpeningstatus = OpenStatus::FreeOperation;     // Open status at previous time step
        Real64 CloseElapsedTime = 0.0;                         // Elapsed time during closing (min)
        Real64 OpenElapsedTime = 0.0;                          // Elapsed time during closing (min)
        int ClosingProbStatus = ProbabilityCheck::NoAction;    // Closing probability status
        int OpeningProbStatus = ProbabilityCheck::NoAction;    // Opening probability status

        // Non-rectangular geometry parameters
        bool nonrectangular;                  // True if this surface is not rectangular
        EquivRec equivalent_rectangle_method; // Equivalent Rectangle Method input: 1 Height; 2 Base surface aspect ratio; 3 User input aspect ratio
        Real64 user_aspect_ratio;       // user input value when EquivRecMethod = 3

        // Linkage control schedule
        std::string control_schedule_name;

        Real64 Multiplier = 1.0;       // Window multiplier

        // Hybrid ventilation variables
        bool HybridVentClose = false;  // Hybrid ventilation window close control logical
        bool HybridCtrlGlobal = false; // Hybrid ventilation global control logical
        bool HybridCtrlMaster = false; // Hybrid ventilation global control master
        Real64 WindModifier = 1.0;     // Wind modifier from hybrid ventilation control

        bool RAFNflag = false; // True if this surface is used in AirflowNetwork:IntraZone:Linkage

        // Error counters
        int ExtLargeOpeningErrCount = 0; // Exterior large opening error count during HVAC system operation
        int ExtLargeOpeningErrIndex = 0; // Exterior large opening error index during HVAC system operation
        int OpenFactorErrCount = 0;      // Large opening error count at Open factor > 1.0
        int OpenFactorErrIndex = 0;      // Large opening error error index at Open factor > 1.0

        MultizoneSurfaceProp()
            : factor(0.0), surface_number(0), NodeNums{{0, 0}}, OpenFactor(0.0), OpenFactorLast(0.0), EMSOpenFactorActuated(false), EMSOpenFactor(0.0),
              height(0.0), width(0.0), centroid_height(0.0), open_factor_limit(0.0), temperature_lower_limit(0.0), temperature_upper_limit(100.0),
              enthalpy_lower_limit(0.0), enthalpy_upper_limit(300000.0), ventilation_control_type(VentControlType::None), ZonePtr(0), IndVentControl(false),
              ExtLargeOpeningErrCount(0), ExtLargeOpeningErrIndex(0), OpenFactorErrCount(0), OpenFactorErrIndex(0), Multiplier(1.0),
              HybridVentClose(false), HybridCtrlGlobal(false), HybridCtrlMaster(false), WindModifier(1.0), occupant_control_number(0),
              OpeningStatus(OpenStatus::FreeOperation), PrevOpeningstatus(OpenStatus::FreeOperation), CloseElapsedTime(0.0), OpenElapsedTime(0.0),
              ClosingProbStatus(ProbabilityCheck::NoAction), OpeningProbStatus(ProbabilityCheck::NoAction), RAFNflag(false), nonrectangular(false),
              equivalent_rectangle_method(EquivRec::Height), user_aspect_ratio(1.0)
        {
        }
        MultizoneSurfaceProp(const std::string &SurfName, const int SurfNum, Real64 Height, Real64 Width, Real64 CHeight, const std::string &OpeningName,
            const std::string &ExternalNodeName, Real64 Factor, VentControlType VentControl, Real64 ModulateFactor, Real64 LowValueTemp, Real64 UpValueTemp,
            Real64 LowValueEnth, Real64 UpValueEnth, const std::string &VentTempControlSchName, const std::string &VentAvailSchName, int OccupantVentilationControlNum,
            bool NonRectangular, EquivRec EquivRectMethod, Real64 UserAspectRatio, const std::string &control_schedule_name) : surface_name(SurfName), surface_number(SurfNum),
            height(Height), width(Width), centroid_height(CHeight), opening_name(OpeningName), external_node_name(ExternalNodeName), factor(Factor),
            ventilation_control_type(VentControl), open_factor_limit(ModulateFactor), temperature_lower_limit(LowValueTemp), temperature_upper_limit(UpValueTemp), 
            enthalpy_lower_limit(LowValueEnth), enthalpy_upper_limit(UpValueEnth), VentTempControlSchName(VentTempControlSchName),
            VentAvailSchName(VentAvailSchName), occupant_control_number(OccupantVentilationControlNum), nonrectangular(NonRectangular), equivalent_rectangle_method(EquivRectMethod),
            user_aspect_ratio(UserAspectRatio), control_schedule_name(control_schedule_name)

        {}
    };

    struct AirflowNetworkLinkage // AirflowNetwork linkage data base class
    {
        // Members
        std::string Name;                     // Provide a unique linkage name
        std::array<std::string, 2> NodeNames; // Names of nodes (limited to 2)
        std::array<Real64, 2> NodeHeights;    // Node heights
        std::string CompName;                 // Name of element
        int CompNum;                          // Element Number
        std::array<int, 2> NodeNums;          // Node numbers
        int LinkNum;                          // Linkage number
        AirflowElement *element;              // Pointer to airflow element
        Real64 control_value;                 // Control value
        Sched::Schedule *control_schedule;     // Control schedule

        // Default Constructor
        AirflowNetworkLinkage() : NodeHeights{{0.0, 0.0}}, CompNum(0), NodeNums{{0, 0}}, LinkNum(0), element(nullptr), control_value(1.0),  control_schedule(nullptr)
        {
        }

        virtual ~AirflowNetworkLinkage()
        {
        }
    };

    struct IntraZoneLinkageProp : public AirflowNetworkLinkage // Intra zone linkage data
    {
        // Members
        std::string SurfaceName; // Connection Surface Name

        // Default Constructor
        IntraZoneLinkageProp() : AirflowNetworkLinkage()
        {
        }
    };

    struct DisSysLinkageProp : public AirflowNetworkLinkage // Distribution system linkage data
    {
        // Members
        std::string ZoneName; // Name of zone
        int ZoneNum;          // Zone Number

        // Default Constructor
        DisSysLinkageProp() : AirflowNetworkLinkage(), ZoneNum(0)
        {
        }
    };

    struct AirflowNetworkLinkageProp : public AirflowNetworkLinkage // AirflowNetwork linkage data
    {
        // Members
        std::string ZoneName;               // Name of zone
        int ZoneNum;                        // Zone Number
        int DetOpenNum;                     // Large Opening number
        EPlusComponentType ConnectionFlag; // Return and supply connection flag
        bool VAVTermDamper;                 // True if this component is a damper for a VAV terminal
        int LinkageViewFactorObjectNum;
        int AirLoopNum; // Airloop number
        DuctLineType ductLineType;

        // Default Constructor
        AirflowNetworkLinkageProp()
            : AirflowNetworkLinkage(), ZoneNum(0), DetOpenNum(0), ConnectionFlag(EPlusComponentType::Invalid), VAVTermDamper(false),
              LinkageViewFactorObjectNum(0), AirLoopNum(0), ductLineType(DuctLineType::Invalid)
        {
        }
    };

    struct AirflowNetworkLinkSimuData
    {
        // Members
        Real64 FLOW;     // Mass flow rate [kg/s]
        Real64 FLOW2;    // Mass flow rate [kg/s] for two way flow
        Real64 DP;       // Pressure difference across a component
        Real64 VolFLOW;  // Mass flow rate [m3/s]
        Real64 VolFLOW2; // Mass flow rate [m3/s] for two way flow
        Real64 DP1;

        // Default Constructor
        AirflowNetworkLinkSimuData() : FLOW(0.0), FLOW2(0.0), DP(0.0), VolFLOW(0.0), VolFLOW2(0.0), DP1(0.0)
        {
        }
    };

    struct AirflowNetworkLinkReportData
    {
        // Members
        Real64 FLOW;        // Mass flow rate [kg/s]
        Real64 FLOW2;       // Mass flow rate [kg/s] for two way flow
        Real64 VolFLOW;     // Mass flow rate [m^3/s]
        Real64 VolFLOW2;    // Mass flow rate [m^3/s] for two way flow
        Real64 FLOWOFF;     // Mass flow rate during OFF cycle [kg/s]
        Real64 FLOW2OFF;    // Mass flow rate during OFF cycle [kg/s] for two way flow
        Real64 VolFLOWOFF;  // Mass flow rate during OFF cycle [m^3/s]
        Real64 VolFLOW2OFF; // Mass flow rate during OFF cycle [m^3/s] for two way flow
        Real64 DP;          // Average Pressure difference across a component
        Real64 DPON;        // Pressure difference across a component with fan on
        Real64 DPOFF;       // Pressure difference across a component with fan off

        // Default Constructor
        AirflowNetworkLinkReportData()
            : FLOW(0.0), FLOW2(0.0), VolFLOW(0.0), VolFLOW2(0.0), FLOWOFF(0.0), FLOW2OFF(0.0), VolFLOWOFF(0.0), VolFLOW2OFF(0.0), DP(0.0), DPON(0.0),
              DPOFF(0.0)
        {
        }
    };

    struct LinkageSurfaceProp
    {
        // Members
        std::string SurfaceName;
        int SurfaceNum;                 // Name of surface referenced by view factor
        Real64 ViewFactor;              // View factor
        Real64 SurfaceResistanceFactor; // Total radiation heat transfer resistance factor
        Real64 SurfaceRadLoad;          // Duct radiation load from surface [W]

        // Default constructor
        LinkageSurfaceProp() : SurfaceNum(0), ViewFactor(0.0), SurfaceResistanceFactor(0.0), SurfaceRadLoad(0.0)
        {
        }
    };

    struct AirflowNetworkLinkageViewFactorProp
    {
        // Members
        std::string LinkageName;
        Real64 DuctExposureFraction;
        Real64 DuctEmittance;
        Array1D<LinkageSurfaceProp> LinkageSurfaceData;
        int ObjectNum;
        Real64 QRad;
        Real64 QConv;

        AirflowNetworkLinkageViewFactorProp() : DuctExposureFraction(0.0), DuctEmittance(0.0), ObjectNum(0), QRad(0.0), QConv(0.0)
        {
        }
    };

    void handle_nonrectangular_surfaces(EquivRec equivalent_rectangle_method, EPVector<DataSurfaces::SurfaceData> Surface, const std::string &surface_name, int surface_number,
    Real64 &height, Real64 &width, Real64 user_aspect_ratio, EnergyPlusData *state = nullptr);

} // namespace AirflowNetwork

} // namespace EnergyPlus

#endif
