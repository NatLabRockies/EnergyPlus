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

#ifndef AIRFLOWNETWORK_ELEMENTS_HPP
#define AIRFLOWNETWORK_ELEMENTS_HPP

#include "AirflowNetwork/Properties.hpp"
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/EPVector.hh>
#include <EnergyPlus/ScheduleManager.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;
struct AirflowNetworkData;

namespace AirflowNetwork {

    enum VentControlType // TODO: make enum class
    {
        None = 0,  // Wrong input
        Temp = 1,  // Temperature venting control
        Enth = 2,  // Enthalpy venting control
        Const = 3, // Constant venting control
        ASH55 = 4,
        CEN15251 = 5,
        NoVent = 6,    // No venting
        ZoneLevel = 7, // ZoneLevel control for a heat transfer subsurface
        AdjTemp = 8,   // Temperature venting control based on adjacent zone conditions
        AdjEnth = 9    // Enthalpy venting control based on adjacent zone conditions
    };

    enum OpenStatus // TODO: make enum class
    {
        FreeOperation = 0,     // Free operation
        MinCheckForceOpen = 1, // Force open when opening elapsed time is less than minimum opening time
        MinCheckForceClose = 2 // Force open when closing elapsed time is less than minimum closing time
    };

    enum ProbabilityCheck // TODO: make enum class
    {
        NoAction = 0,    // No action from probability check
        ForceChange = 1, // Force open or close from probability check
        KeepStatus = 2   // Keep status at the previous time step from probability check
    };

    enum class EquivRec
    {
        Height,          // Effective rectangle polygonal height selection
        BaseAspectRatio, // Effective rectangle base surface aspect ratio selection
        UserAspectRatio  // Effective rectangle user input aspect ratio selection
    };

    enum class DuctLineType
    {
        Invalid = -1,
        SupplyTrunk,  // Supply trunk
        SupplyBranch, // SupplyBrnach
        ReturnTrunk,  // Return trunk
        ReturnBranch, // ReturnBrnach
    };

    // Using/Aliasing

    // Data
    // module should be available to other modules and routines.  Thus,
    // all variables in this module must be PUBLIC.

    // MODULE PARAMETER DEFINITIONS:
    enum class iComponentTypeNum : int
    {
        Invalid = 0,
        DOP = 1,  // Detailed large opening component
        SOP = 2,  // Simple opening component
        SCR = 3,  // Surface crack component
        SEL = 4,  // Surface effective leakage ratio component
        PLR = 5,  // Distribution system crack component
        DWC = 6,  // Distribution system duct component
        CVF = 7,  // Distribution system constant volume fan component
        FAN = 8,  // Distribution system detailed fan component
        MRR = 9,  // Distribution system multiple curve fit power law resistant flow component
        DMP = 10, // Distribution system damper component
        ELR = 11, // Distribution system effective leakage ratio component
        CPD = 12, // Distribution system constant pressure drop component
        COI = 13, // Distribution system coil component
        TMU = 14, // Distribution system terminal unit component
        EXF = 15, // Zone exhaust fan
        HEX = 16, // Distribution system heat exchanger
        HOP = 17, // Horizontal opening component
        RVD = 18, // Reheat VAV terminal damper
        OAF = 19, // Distribution system OA
        REL = 20, // Distribution system relief air
        SMF = 21, // Specified mass flow component
        SVF = 22, // Specified volume flow component
        Num
    };

    enum class ComponentType
    {
        // TODO: enum check
        Invalid = -1,
        DOP = 1, // Detailed large opening component
        SOP,     // Simple opening component
        SCR,     // Surface crack component
        SEL,     // Surface effective leakage ratio component
        PLR,     // Distribution system crack component
        DWC,     // Distribution system duct component
        CVF,     // Distribution system constant volume fan component
        FAN,     // Distribution system detailed fan component
        MRR,     // Distribution system multiple curve fit power law resistant flow component
        DMP,     // Distribution system damper component
        ELR,     // Distribution system effective leakage ratio component
        CPD,     // Distribution system constant pressure drop component
        COI,     // Distribution system coil component
        TMU,     // Distribution system terminal unit component
        EXF,     // Zone exhaust fan
        HEX,     // Distribution system heat exchanger
        HOP,     // Horizontal opening component
        RVD,     // Reheat VAV terminal damper
        OAF,     // Distribution system OA
        REL,     // Distribution system relief air
        SMF,     // Specified mass flow component
        SVF,     // Specified volume flow component
        Num
    };

    // EPlus component Type
    enum class iEPlusComponentType : int
    {
        Invalid = 0,
        SCN = 1, // Supply connection
        RCN = 2, // Return connection
        RHT = 3, // Reheat terminal
        FAN = 4, // Fan
        COI = 5, // Heating or cooling coil
        HEX = 6, // Heat exchanger
        RVD = 7, // Reheat VAV terminal damper
        Num
    };

    // EPlus node type
    enum class iEPlusNodeType : int
    {
        Invalid = 0,
        ZIN = 1,  // Zone inlet node
        ZOU = 2,  // Zone outlet node
        SPL = 3,  // Splitter node
        MIX = 4,  // Mixer node
        OAN = 5,  // Outside air system node
        EXT = 6,  // OA system inlet node
        FIN = 7,  // Fan Inlet node
        FOU = 8,  // Fan Outlet Node
        COU = 9,  // Coil Outlet Node
        HXO = 10, // Heat exchanger Outlet Node
        DIN = 11, // Damper Inlet node
        DOU = 12, // Damper Outlet Node
        SPI = 13, // Splitter inlet Node
        SPO = 14, // Splitter Outlet Node
        ZON = 15, // Zone node
        Num
    };

    enum class iWPCCntr : int
    {
        Invalid = 0,
        Input = 1,
        SurfAvg = 2,
        Num
    };

    int constexpr PressureCtrlExhaust = 1;
    int constexpr PressureCtrlRelief = 2;

    // DERIVED TYPE DEFINITIONS:

    // MODULE VARIABLE DECLARATIONS:
    // Node simulation variable in air distribution system
    // Link simulation variable in air distribution system
    // Sensible and latent exchange variable in air distribution system

    void generic_crack(Real64 const coef,        // Flow coefficient
                       Real64 const expn,        // Flow exponent
                       bool const LFLAG,         // Initialization flag.If = 1, use laminar relationship
                       Real64 const PDROP,       // Total pressure drop across a component (P1 - P2) [Pa]
                       const AirState &propN,    // Node 1 properties
                       const AirState &propM,    // Node 2 properties
                       std::array<Real64, 2> &F, // Airflow through the component [kg/s]
                       std::array<Real64, 2> &DF // Partial derivative:  DF/DP
    );

    int GenericDuct(Real64 const Length,      // Duct length
                    Real64 const Diameter,    // Duct diameter
                    bool const LFLAG,         // Initialization flag.If = 1, use laminar relationship
                    Real64 const PDROP,       // Total pressure drop across a component (P1 - P2) [Pa]
                    const AirState &propN,    // Node 1 properties
                    const AirState &propM,    // Node 2 properties
                    std::array<Real64, 2> &F, // Airflow through the component [kg/s]
                    std::array<Real64, 2> &DF // Partial derivative:  DF/DP
    );

    // Types
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

    struct MultizoneSurfaceProp // Surface information
    {
        // Members
        std::string SurfName;         // Name of Associated EnergyPlus surface
        std::string OpeningName;      // Name of opening component, either simple or detailed large opening
        std::string ExternalNodeName; // Name of external node, but not used at WPC="INPUT"
        Real64 Factor;                // Crack Actual Value or Window Open Factor for Ventilation
        int SurfNum;                  // Surface number
        std::array<int, 2> NodeNums;  // Positive: Zone numbers; 0: External
        Real64 OpenFactor;            // Surface factor
        Real64 OpenFactorLast;        // Surface factor at previous time step
        bool EMSOpenFactorActuated;   // True if EMS actuation is on
        Real64 EMSOpenFactor;         // Surface factor value from EMS for override
        Real64 Height;                // Surface Height
        Real64 Width;                 // Surface width
        Real64 CHeight;               // Surface central height in z direction
        std::string VentControl;      // Ventilation Control Mode: TEMPERATURE, ENTHALPIC, CONSTANT, ZONELEVEL or NOVENT
        Real64 ModulateFactor;        // Limit Value on Multiplier for Modulating Venting Open Factor
        Real64 LowValueTemp;          // Lower Value on Inside/Outside Temperature Difference for
        // Modulating the Venting Open Factor with temp control
        Real64 UpValueTemp; // Upper Value on Inside/Outside Temperature Difference for
        // Modulating the Venting Open Factor with temp control
        Real64 LowValueEnth; // Lower Value on Inside/Outside Temperature Difference for
        // Modulating the Venting Open Factor with Enthalpic control
        Real64 UpValueEnth; // Upper Value on Inside/Outside Temperature Difference for
        // Modulating the Venting Open Factor with Enthalpic control
        std::string VentTempControlSchName;              // Name of ventilation temperature control schedule
        Sched::Schedule *ventTempControlSched = nullptr; // Ventilation temperature control schedule
        VentControlType VentSurfCtrNum;                  // Ventilation control mode number: 1 "Temperature", 2 "ENTHALPIC", 3 "CONSTANT", 4 "NOVENT"
        std::string VentAvailSchName;                    // Ventilation availability schedule
        Sched::Schedule *ventAvailSched = nullptr;       // Ventilation availability schedule
        int ZonePtr;                                     // Pointer to inside face zone
        bool IndVentControl;                             // Individual surface venting control
        int ExtLargeOpeningErrCount;                     // Exterior large opening error count during HVAC system operation
        int ExtLargeOpeningErrIndex;                     // Exterior large opening error index during HVAC system operation
        int OpenFactorErrCount;                          // Large opening error count at Open factor > 1.0
        int OpenFactorErrIndex;                          // Large opening error error index at Open factor > 1.0
        Real64 Multiplier;                               // Window multiplier
        bool HybridVentClose;                            // Hybrid ventilation window close control logical
        bool HybridCtrlGlobal;                           // Hybrid ventilation global control logical
        bool HybridCtrlMaster;                           // Hybrid ventilation global control master
        Real64 WindModifier;                             // Wind modifier from hybrid ventilation control
        std::string OccupantVentilationControlName;      // Occupant ventilation control name
        int OccupantVentilationControlNum;               // Occupant ventilation control number
        int OpeningStatus;                               // Open status at current time step
        int PrevOpeningstatus;                           // Open status at previous time step
        Real64 CloseElapsedTime;                         // Elapsed time during closing (min)
        Real64 OpenElapsedTime;                          // Elapsed time during closing (min)
        int ClosingProbStatus;                           // Closing probability status
        int OpeningProbStatus;                           // Opening probability status
        bool RAFNflag;                                   // True if this surface is used in AirflowNetwork:IntraZone:Linkage
        bool NonRectangular;                             // True if this surface is not rectangular
        EquivRec EquivRecMethod;        // Equivalent Rectangle Method input: 1 Height; 2 Base surface aspect ratio; 3 User input aspect ratio
        Real64 EquivRecUserAspectRatio; // user input value when EquivRecMethod = 3

        // Default Constructor
        MultizoneSurfaceProp()
            : Factor(0.0), SurfNum(0), NodeNums{{0, 0}}, OpenFactor(0.0), OpenFactorLast(0.0), EMSOpenFactorActuated(false), EMSOpenFactor(0.0),
              Height(0.0), Width(0.0), CHeight(0.0), VentControl("ZONELEVEL"), ModulateFactor(0.0), LowValueTemp(0.0), UpValueTemp(100.0),
              LowValueEnth(0.0), UpValueEnth(300000.0), VentSurfCtrNum(VentControlType::None), ZonePtr(0), IndVentControl(false),
              ExtLargeOpeningErrCount(0), ExtLargeOpeningErrIndex(0), OpenFactorErrCount(0), OpenFactorErrIndex(0), Multiplier(1.0),
              HybridVentClose(false), HybridCtrlGlobal(false), HybridCtrlMaster(false), WindModifier(1.0), OccupantVentilationControlNum(0),
              OpeningStatus(OpenStatus::FreeOperation), PrevOpeningstatus(OpenStatus::FreeOperation), CloseElapsedTime(0.0), OpenElapsedTime(0.0),
              ClosingProbStatus(ProbabilityCheck::NoAction), OpeningProbStatus(ProbabilityCheck::NoAction), RAFNflag(false), NonRectangular(false),
              EquivRecMethod(EquivRec::Height), EquivRecUserAspectRatio(1.0)
        {
        }
    };

    struct AirflowElement
    {
        AirflowElement()
        {
        }

        AirflowElement(const std::string &name) : name(name)
        {
        }

        virtual ~AirflowElement()
        {
        }

        std::string name; // Name of airflow element

        virtual int calculate(EnergyPlusData &state,
                              bool const LFLAG,         // Initialization flag.If = 1, use laminar relationship
                              Real64 const PDROP,       // Total pressure drop across a component (P1 - P2) [Pa]
                              int const i,              // Linkage number
                              const Real64 multiplier,  // Element multiplier
                              const Real64 control,     // Element control signal
                              const AirState &propN,    // Node 1 properties
                              const AirState &propM,    // Node 2 properties
                              std::array<Real64, 2> &F, // Airflow through the component [kg/s]
                              std::array<Real64, 2> &DF // Partial derivative:  DF/DP
                              ) = 0;

        // Make this abstract once all the classes implement it
        virtual int calculate([[maybe_unused]] EnergyPlusData &state,
                              [[maybe_unused]] const Real64 PDROP,       // Total pressure drop across a component (P1 - P2) [Pa]
                              [[maybe_unused]] const Real64 multiplier,  // Element multiplier
                              [[maybe_unused]] const Real64 control,     // Element control signal
                              [[maybe_unused]] const AirState &propN,    // Node 1 properties
                              [[maybe_unused]] const AirState &propM,    // Node 2 properties
                              [[maybe_unused]] std::array<Real64, 2> &F, // Airflow through the component [kg/s]
                              [[maybe_unused]] std::array<Real64, 2> &DF // Partial derivative:  DF/DP
        )
        {
            return 1;
        }

        virtual ComponentType type() = 0;
    };

    int constexpr NrInt = 20; // Number of intervals for a large opening

    struct DetailedOpeningSolver
    {
        // Large opening variables
        EPVector<Real64> DpProf;   // Differential pressure profile for Large Openings [Pa]
        EPVector<Real64> RhoProfF; // Density profile in FROM zone [kg/m3]
        EPVector<Real64> RhoProfT; // Density profile in TO zone [kg/m3]
        Array2D<Real64> DpL;       // Array of stack pressures in link

        void allocate(int number_of_links, int n_dop)
        {
            DpProf.allocate(n_dop * (NrInt + 2));
            RhoProfF.allocate(n_dop * (NrInt + 2));
            RhoProfT.allocate(n_dop * (NrInt + 2));
            DpL.allocate(number_of_links, 2);
        }

        void clear()
        {
            DpProf.clear();
            RhoProfF.clear();
            RhoProfT.clear();
            DpL.clear();
        }

        void presprofile(EnergyPlusData &state,
                         int const il,                  // Linkage number
                         int const Pprof,               // Opening number
                         Real64 const G,                // gravitation field strength [N/kg]
                         const Array1D<Real64> &DpF,    // Stack pressures at start heights of Layers
                         const Array1D<Real64> &DpT,    // Stack pressures at start heights of Layers
                         const Array1D<Real64> &BetaF,  // Density gradients in the FROM zone (starting at linkheight) [Kg/m3/m]
                         const Array1D<Real64> &BetaT,  // Density gradients in the TO zone (starting at linkheight) [Kg/m3/m]
                         const Array1D<Real64> &RhoStF, // Density at the start heights of Layers in the FROM zone
                         const Array1D<Real64> &RhoStT, // Density at the start heights of Layers in the TO zone
                         int const From,                // Number of FROM zone
                         int const To,                  // Number of To zone
                         Real64 const ActLh,            // Actual height of opening [m]
                         Real64 const OwnHeightFactor   // Cosine of deviation angle of the opening plane from the vertical direction
        );

        void pstack(EnergyPlusData &state, std::vector<AirflowNetwork::AirState> &props, Array1D<Real64> &pz);

        Real64 psz(Real64 const Pz0,  // Pressure at altitude z0 [Pa]
                   Real64 const Rho0, // density at altitude z0 [kg/m3]
                   Real64 const beta, // density gradient [kg/m4]
                   Real64 const z0,   // reference altitude [m]
                   Real64 const z,    // altitude[m]
                   Real64 const g     // gravity field strength [N/kg]
        );

        void lclimb(EnergyPlusData &state,
                    Real64 const G,   // gravity field strength [N/kg]
                    Real64 &Rho,      // Density link level (initialized with rho zone) [kg/m3]
                    Real64 const Z,   // Height of the link above the zone reference [m]
                    Real64 &T,        // temperature at link level [C]
                    Real64 &X,        // absolute humidity at link level [kg/kg]
                    Real64 &Dp,       // Stackpressure to the linklevel [Pa]
                    int const zone,   // Zone number
                    Real64 const PZ,  // Zone Pressure (reflevel) [Pa]
                    Real64 const Pbz, // Barometric pressure at entrance level [Pa]
                    Real64 &RhoDr     // Air density of dry air on the link level used
        );
    };

    struct DetailedOpening : public AirflowElement // Large detailed opening component
    {
        // Members
        Real64 FlowCoef;      // Air Mass Flow Coefficient When Window or Door Is Closed [kg/s at 1Pa]
        Real64 FlowExpo;      // Air Mass Flow exponent When Window or Door Is Closed [dimensionless]
        std::string TypeName; // Name of Large vertical opening type
        int LVOType;          // Large vertical opening type number
        Real64 LVOValue;      // Extra crack length for LVO type 1 with multiple operable parts,
        // or Height of pivoting axis for LVO type 2
        int NumFac;         // Number of Opening Factor Values
        Real64 OpenFac1;    // Opening factor #1
        Real64 DischCoeff1; // Discharge coefficient for opening factor #1
        Real64 WidthFac1;   // Width factor for for Opening factor #1
        Real64 HeightFac1;  // Height factor for opening factor #1
        Real64 StartHFac1;  // Start height factor for opening factor #1
        Real64 OpenFac2;    // Opening factor #2
        Real64 DischCoeff2; // Discharge coefficient for opening factor #2
        Real64 WidthFac2;   // Width factor for for Opening factor #2
        Real64 HeightFac2;  // Height factor for opening factor #2
        Real64 StartHFac2;  // Start height factor for opening factor #2
        Real64 OpenFac3;    // Opening factor #3
        Real64 DischCoeff3; // Discharge coefficient for opening factor #3
        Real64 WidthFac3;   // Width factor for for Opening factor #3
        Real64 HeightFac3;  // Height factor for opening factor #3
        Real64 StartHFac3;  // Start height factor for opening factor #3
        Real64 OpenFac4;    // Opening factor #4
        Real64 DischCoeff4; // Discharge coefficient for opening factor #4
        Real64 WidthFac4;   // Width factor for for Opening factor #4
        Real64 HeightFac4;  // Height factor for opening factor #4
        Real64 StartHFac4;  // Start height factor for opening factor #4
        Real64 OpenFactor;  // Opening factor
        int WidthErrCount;  // Width error count
        int WidthErrIndex;  // Width error index
        int HeightErrCount; // Height error count
        int HeightErrIndex; // Height error index

        // Default Constructor
        DetailedOpening()
            : FlowCoef(0.0), FlowExpo(0.0), TypeName("NONPIVOTED"), LVOType(0), LVOValue(0.0), NumFac(0), OpenFac1(0.0), DischCoeff1(0.0),
              WidthFac1(0.0), HeightFac1(0.0), StartHFac1(0.0), OpenFac2(0.0), DischCoeff2(0.0), WidthFac2(0.0), HeightFac2(0.0), StartHFac2(0.0),
              OpenFac3(0.0), DischCoeff3(0.0), WidthFac3(0.0), HeightFac3(0.0), StartHFac3(0.0), OpenFac4(0.0), DischCoeff4(0.0), WidthFac4(0.0),
              HeightFac4(0.0), StartHFac4(0.0), OpenFactor(0.0), WidthErrCount(0), WidthErrIndex(0), HeightErrCount(0), HeightErrIndex(0)
        {
        }

        int calculate([[maybe_unused]] EnergyPlusData &state,
                      bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                      Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                      int const i,                              // Linkage number
                      [[maybe_unused]] const Real64 multiplier, // Element multiplier
                      [[maybe_unused]] const Real64 control,    // Element control signal
                      const AirState &propN,                    // Node 1 properties
                      const AirState &propM,                    // Node 2 properties
                      std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
        );

        virtual ComponentType type()
        {
            return ComponentType::DOP;
        }
    };

    struct SimpleOpening : public AirflowElement // Large simple opening component
    {
        // Members
        Real64 FlowCoef;   // Air Mass Flow Coefficient When Window or Door Is Closed [kg/s at 1Pa]
        Real64 FlowExpo;   // Air Mass Flow exponent When Window or Door Is Closed [dimensionless]
        Real64 MinRhoDiff; // Minimum density difference for two-way flow
        Real64 DischCoeff; // Discharge coefficient at full opening
        Real64 OpenFactor; // Opening factor

        // Default Constructor
        SimpleOpening() : FlowCoef(0.0), FlowExpo(0.0), MinRhoDiff(0.0), DischCoeff(0.0), OpenFactor(0.0)
        {
        }

        int calculate(EnergyPlusData &state,
                      bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                      Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                      int const i,                              // Linkage number
                      [[maybe_unused]] const Real64 multiplier, // Element multiplier
                      [[maybe_unused]] const Real64 control,    // Element control signal
                      const AirState &propN,                    // Node 1 properties
                      const AirState &propM,                    // Node 2 properties
                      std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
        );

        virtual ComponentType type()
        {
            return ComponentType::SOP;
        }
    };

    struct HorizontalOpening : public AirflowElement // Large horizontal opening component
    {
        // Members
        Real64 FlowCoef;   // Air Mass Flow Coefficient When Window or Door Is Closed [kg/s at 1Pa]
        Real64 FlowExpo;   // Air Mass Flow exponent When Window or Door Is Closed [dimensionless]
        Real64 Slope;      // Sloping plane angle
        Real64 DischCoeff; // Discharge coefficient at full opening

        // Default Constructor
        HorizontalOpening() : FlowCoef(0.0), FlowExpo(0.0), Slope(0.0), DischCoeff(0.0)
        {
        }

        int calculate(EnergyPlusData &state,
                      bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                      Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                      int const i,                              // Linkage number
                      [[maybe_unused]] const Real64 multiplier, // Element multiplier
                      [[maybe_unused]] const Real64 control,    // Element control signal
                      const AirState &propN,                    // Node 1 properties
                      const AirState &propM,                    // Node 2 properties
                      std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
        );

        virtual ComponentType type()
        {
            return ComponentType::HOP;
        }
    };

    struct SpecifiedMassFlow : public AirflowElement // Specified mass flow element
    {
        // Members
        Real64 mass_flow; // Mass Flow [kg/s]

        // Default Constructor
        SpecifiedMassFlow() : mass_flow(0.0)
        {
        }

        int calculate([[maybe_unused]] EnergyPlusData &state,
                      [[maybe_unused]] bool const LFLAG,      // Initialization flag.If = 1, use laminar relationship
                      [[maybe_unused]] Real64 const PDROP,    // Total pressure drop across a component (P1 - P2) [Pa]
                      [[maybe_unused]] int const i,           // Linkage number
                      const Real64 multiplier,                // Element multiplier
                      const Real64 control,                   // Element control signal
                      [[maybe_unused]] const AirState &propN, // Node 1 properties
                      [[maybe_unused]] const AirState &propM, // Node 2 properties
                      std::array<Real64, 2> &F,               // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF               // Partial derivative:  DF/DP
        );

        virtual ComponentType type()
        {
            return ComponentType::SMF;
        }
    };

    struct SpecifiedVolumeFlow : public AirflowElement // Specified mass flow element
    {
        // Members
        Real64 volume_flow; // Volume Flow [m3/s]

        // Default Constructor
        SpecifiedVolumeFlow() : volume_flow(0.0)
        {
        }

        int calculate([[maybe_unused]] EnergyPlusData &state,
                      [[maybe_unused]] bool const LFLAG,   // Initialization flag.If = 1, use laminar relationship
                      [[maybe_unused]] Real64 const PDROP, // Total pressure drop across a component (P1 - P2) [Pa]
                      [[maybe_unused]] int const i,        // Linkage number
                      const Real64 multiplier,             // Element multiplier
                      const Real64 control,                // Element control signal
                      const AirState &propN,               // Node 1 properties
                      const AirState &propM,               // Node 2 properties
                      std::array<Real64, 2> &F,            // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF            // Partial derivative:  DF/DP
        );

        virtual ComponentType type()
        {
            return ComponentType::SVF;
        }
    };

    struct ReferenceConditions // Surface crack standard conditions
    {
        // Members
        std::string name;      // Name of standard conditions component
        Real64 temperature;    // Standard temperature for crack data
        Real64 pressure;       // Standard barometric pressure for crack data
        Real64 humidity_ratio; // Standard humidity ratio for crack data

        ReferenceConditions(const std::string &name, Real64 temperature = 20.0, Real64 pressure = 101325.0, Real64 humidity_ratio = 0.0)
            : name(name), temperature(temperature), pressure(pressure), humidity_ratio(humidity_ratio)
        {
        }

        // ReferenceConditions(Real64 temperature = 20.0, Real64 pressure = 101325.0, Real64 humidityRatio = 0.0)
        //    : temperature(temperature), pressure(pressure), humidityRatio(humidityRatio)
        //{
        //}
    };

    struct SurfaceCrack : public AirflowElement // Surface crack component
    {
        // Members
        Real64 coefficient;         // Air Mass Flow Coefficient When Window or Door Is Closed [kg/s at 1Pa]
        Real64 exponent;            // Air Mass Flow exponent When Window or Door Is Closed [dimensionless]
        Real64 reference_density;   // Reference density for crack data
        Real64 reference_viscosity; // Reference viscosity for crack data

        // Default Constructor
        SurfaceCrack()
            : coefficient(0.0), exponent(0.0), reference_density(AIRDENSITY_CONSTEXPR(101325.0, 20.0, 0.0)),
              reference_viscosity(AIRDYNAMICVISCOSITY_CONSTEXPR(20.0))
        {
        }

        int calculate([[maybe_unused]] EnergyPlusData &state,
                      bool const linear,            // Initialization flag.If = 1, use laminar relationship
                      Real64 const PDROP,           // Total pressure drop across a component (P1 - P2) [Pa]
                      [[maybe_unused]] int const i, // Linkage number
                      const Real64 multiplier,      // Element multiplier
                      const Real64 control,         // Element control signal
                      const AirState &propN,        // Node 1 properties
                      const AirState &propM,        // Node 2 properties
                      std::array<Real64, 2> &F,     // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF     // Partial derivative:  DF/DP
        );

        virtual int calculate(EnergyPlusData &state,
                              const Real64 pdrop,       // Total pressure drop across a component (P1 - P2) [Pa]
                              const Real64 multiplier,  // Element multiplier
                              const Real64 control,     // Element control signal
                              const AirState &propN,    // Node 1 properties
                              const AirState &propM,    // Node 2 properties
                              std::array<Real64, 2> &F, // Airflow through the component [kg/s]
                              std::array<Real64, 2> &DF // Partial derivative:  DF/DP
        );

        virtual ComponentType type()
        {
            return ComponentType::SCR;
        }
    };

    struct EffectiveLeakageArea : public AirflowElement // Surface effective leakage area component
    {
        // Members
        Real64 ELA;         // Effective leakage area
        Real64 DischCoeff;  // Discharge coefficient
        Real64 RefDeltaP;   // Reference pressure difference
        Real64 FlowExpo;    // Air Mass Flow exponent When Window or Door Is Closed
        Real64 TestDeltaP;  // Testing pressure difference
        Real64 TestDisCoef; // Testing Discharge coefficient

        // Default Constructor
        EffectiveLeakageArea() : ELA(0.0), DischCoeff(0.0), RefDeltaP(0.0), FlowExpo(0.0), TestDeltaP(0.0), TestDisCoef(0.0)
        {
        }

        int calculate([[maybe_unused]] EnergyPlusData &state,
                      bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                      Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                      [[maybe_unused]] int const i,             // Linkage number
                      [[maybe_unused]] const Real64 multiplier, // Element multiplier
                      [[maybe_unused]] const Real64 control,    // Element control signal
                      const AirState &propN,                    // Node 1 properties
                      const AirState &propM,                    // Node 2 properties
                      std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
        );

        int calculate([[maybe_unused]] EnergyPlusData &state,
                      Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                      [[maybe_unused]] const Real64 multiplier, // Element multiplier
                      [[maybe_unused]] const Real64 control,    // Element control signal
                      const AirState &propN,                    // Node 1 properties
                      const AirState &propM,                    // Node 2 properties
                      std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
        );

        virtual ComponentType type()
        {
            return ComponentType::SEL;
        }
    };

    struct ZoneExhaustFan : public AirflowElement // Zone exhaust fan component
    {
        // Members
        Real64 FlowRate;                  // mass flow rate
        Sched::Schedule *sched = nullptr; // Schedule pointer
        Real64 FlowCoef;                  // Air Mass Flow Coefficient [kg/s at 1Pa]
        Real64 FlowExpo;                  // Air Mass Flow exponent [dimensionless]
        Real64 StandardT;                 // Standard temperature for crack data
        Real64 StandardP;                 // Standard barometric pressure for crack data
        Real64 StandardW;                 // Standard humidity ratio for crack data
        int InletNode;                    // Inlet node number
        int OutletNode;                   // Outlet node number
        int EPlusZoneNum;                 // Zone number
        int PressCtrlNum;                 // pressure control number

        // Default Constructor
        ZoneExhaustFan()
            : FlowRate(0.0), FlowCoef(0.0), FlowExpo(0.0), StandardT(0.0), StandardP(0.0), StandardW(0.0), InletNode(0), OutletNode(0),
              EPlusZoneNum(0), PressCtrlNum(0)
        {
        }

        int calculate(EnergyPlusData &state,
                      bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                      Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                      int const i,                              // Linkage number
                      [[maybe_unused]] const Real64 multiplier, // Element multiplier
                      [[maybe_unused]] const Real64 control,    // Element control signal
                      const AirState &propN,                    // Node 1 properties
                      const AirState &propM,                    // Node 2 properties
                      std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
        );

        int calculate(EnergyPlusData &state,
                      Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                      [[maybe_unused]] const Real64 multiplier, // Element multiplier
                      const Real64 control,                     // Element control signal
                      const AirState &propN,                    // Node 1 properties
                      const AirState &propM,                    // Node 2 properties
                      std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
        );

        virtual ComponentType type()
        {
            return ComponentType::EXF;
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

    struct DeltaCpProp
    {
        // Members
        Array1D<Real64> WindDir; // Wind direction

        // Default Constructor
        DeltaCpProp()
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
        Real64 control;                       // Control value

        // Default Constructor
        AirflowNetworkLinkage() : NodeHeights{{0.0, 0.0}}, CompNum(0), NodeNums{{0, 0}}, LinkNum(0), element(nullptr), control(1.0)
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

    struct DisSysNodeProp // CP Value
    {
        // Members
        std::string Name;          // Name of node
        std::string EPlusName;     // EnergyPlus node name
        std::string EPlusType;     // EnergyPlus node type
        Real64 Height;             // Nodal height
        int EPlusNodeNum;          // EPlus node number
        int AirLoopNum;            // AirLoop number
        int EPlusZoneInletNodeNum; // Air terminal outlet

        // Default Constructor
        DisSysNodeProp() : Height(0.0), EPlusNodeNum(0), AirLoopNum(0)
        {
        }
    };

    struct DuctLeak : public AirflowElement // duct leak component
    {
        // Members
        Real64 FlowCoef; // Air Mass Flow Coefficient [kg/s at 1Pa]
        Real64 FlowExpo; // Air Mass Flow exponent [dimensionless]

        // Default Constructor
        DuctLeak() : FlowCoef(0.0), FlowExpo(0.0)
        {
        }

        int calculate(EnergyPlusData &state,
                      bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                      Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                      int const i,                              // Linkage number
                      [[maybe_unused]] const Real64 multiplier, // Element multiplier
                      [[maybe_unused]] const Real64 control,    // Element control signal
                      const AirState &propN,                    // Node 1 properties
                      const AirState &propM,                    // Node 2 properties
                      std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
        );

        int calculate(EnergyPlusData &state,
                      Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                      [[maybe_unused]] const Real64 multiplier, // Element multiplier
                      [[maybe_unused]] const Real64 control,    // Element control signal
                      const AirState &propN,                    // Node 1 properties
                      const AirState &propM,                    // Node 2 properties
                      std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
        );

        virtual ComponentType type()
        {
            return ComponentType::PLR;
        }
    };

    struct EffectiveLeakageRatio : public AirflowElement // effective leakage ratio component
    {
        // Members
        Real64 ELR;      // Value of effective leakage ratio
        Real64 FlowRate; // Maximum airflow rate
        Real64 RefPres;  // Reference pressure difference
        Real64 FlowExpo; // Air Mass Flow exponent

        // Default Constructor
        EffectiveLeakageRatio() : ELR(0.0), FlowRate(0.0), RefPres(0.0), FlowExpo(0.0)
        {
        }

        int calculate([[maybe_unused]] EnergyPlusData &state,
                      bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                      Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                      [[maybe_unused]] int const i,             // Linkage number
                      [[maybe_unused]] const Real64 multiplier, // Element multiplier
                      [[maybe_unused]] const Real64 control,    // Element control signal
                      const AirState &propN,                    // Node 1 properties
                      const AirState &propM,                    // Node 2 properties
                      std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
        );

        int calculate([[maybe_unused]] EnergyPlusData &state,
                      Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                      [[maybe_unused]] const Real64 multiplier, // Element multiplier
                      [[maybe_unused]] const Real64 control,    // Element control signal
                      const AirState &propN,                    // Node 1 properties
                      const AirState &propM,                    // Node 2 properties
                      std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
        );

        virtual ComponentType type()
        {
            return ComponentType::ELR;
        }
    };

    struct Duct : public AirflowElement // Duct component
    {
        // Members
        Real64 L;                 // Duct length [m]
        Real64 hydraulicDiameter; // Hydraulic diameter [m]
        Real64 A;                 // Cross section area [m2]
        Real64 roughness;         // Surface roughness [m]
        Real64 TurDynCoef;        // Turbulent dynamic loss coefficient
        Real64 UThermConduct;     // Conduction heat transmittance [W/m2-K]
        Real64 UMoisture;         // Overall moisture transmittance [kg/m2]
        Real64 InsideConvCoeff;   // Inside convection coefficient [W/m2-K]
        Real64 OutsideConvCoeff;  // Outside convection coefficient [W/m2-K]
        Real64 MThermal;          // Thermal capacity [J/K]
        Real64 MMoisture;         // Moisture capacity [kg]
        Real64 LamDynCoef;        // Laminar dynamic loss coefficient
        Real64 LamFriCoef;        // Laminar friction loss coefficient
        Real64 InitLamCoef;       // Coefficient of linear initialization
        Real64 RelRough;          // e/D: relative roughness,
        Real64 RelL;              // L/D: relative length,
        Real64 g;                 // 1/sqrt(Darcy friction factor),
        Real64 A1;                // 1.14 - 0.868589*ln(e/D),

        // Default Constructor
        Duct()
            : L(0.0), hydraulicDiameter(0.0), A(0.0), roughness(0.0), TurDynCoef(0.0), UThermConduct(0.0), UMoisture(0.0), InsideConvCoeff(0.0),
              OutsideConvCoeff(0.0), MThermal(0.0), MMoisture(0.0), LamDynCoef(0.0), LamFriCoef(0.0), InitLamCoef(0.0), RelRough(0.0), RelL(0.0),
              g(0.0), A1(0.0)
        {
        }

        int calculate([[maybe_unused]] EnergyPlusData &state,
                      bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                      Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                      [[maybe_unused]] int const i,             // Linkage number
                      [[maybe_unused]] const Real64 multiplier, // Element multiplier
                      [[maybe_unused]] const Real64 control,    // Element control signal
                      const AirState &propN,                    // Node 1 properties
                      const AirState &propM,                    // Node 2 properties
                      std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
        );

        int calculate([[maybe_unused]] EnergyPlusData &state,
                      Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                      [[maybe_unused]] const Real64 multiplier, // Element multiplier
                      [[maybe_unused]] const Real64 control,    // Element control signal
                      const AirState &propN,                    // Node 1 properties
                      const AirState &propM,                    // Node 2 properties
                      std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
        );

        virtual ComponentType type()
        {
            return ComponentType::DWC;
        }
    };

    struct Damper : public AirflowElement // Damper component
    {
        // Members
        Real64 LTP;      // Value for laminar turbulent transition
        Real64 LamFlow;  // Laminar flow coefficient
        Real64 TurFlow;  // Turbulent flow coefficient
        Real64 FlowExpo; // Air Mass Flow exponent
        Real64 FlowMin;  // Minimum control air mass rate
        Real64 FlowMax;  // Maximum control air mass rate
        Real64 A0;       // First polynomial coefficient of the control variable (constant coefficient)
        Real64 A1;       // Second polynomial coefficient of the control variable (linear coefficient)
        Real64 A2;       // Third polynomial coefficient of the control variable (quadratic coefficient)
        Real64 A3;       // Fourth polynomial coefficient of the control variable (cubic coefficient)

        // Default Constructor
        Damper() : LTP(0.0), LamFlow(0.0), TurFlow(0.0), FlowExpo(0.0), FlowMin(0.0), FlowMax(0.0), A0(0.0), A1(0.0), A2(0.0), A3(0.0)
        {
        }

        int calculate([[maybe_unused]] EnergyPlusData &state,
                      bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                      Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                      int const i,                              // Linkage number
                      [[maybe_unused]] const Real64 multiplier, // Element multiplier
                      [[maybe_unused]] const Real64 control,    // Element control signal
                      const AirState &propN,                    // Node 1 properties
                      const AirState &propM,                    // Node 2 properties
                      std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
        );

        int calculate([[maybe_unused]] EnergyPlusData &state,
                      Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                      [[maybe_unused]] const Real64 multiplier, // Element multiplier
                      const Real64 control,                     // Element control signal
                      const AirState &propN,                    // Node 1 properties
                      const AirState &propM,                    // Node 2 properties
                      std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
        );

        virtual ComponentType type()
        {
            return ComponentType::DMP;
        }
    };

    struct ConstantVolumeFan : public AirflowElement // Constant volume fan component
    {
        // Members
        Real64 FlowRate;           // Air volume flow rate
        Real64 Ctrl;               // Control ratio
        HVAC::FanType fanType;     // Fan type: Constant volume or ONOFF
        int FanIndex;              // Fan index
        int InletNode;             // Inlet node number
        int OutletNode;            // Outlet node number
        Real64 MaxAirMassFlowRate; // Max Specified MAss Flow Rate of Damper [kg/s]
        int AirLoopNum;            // Air loop number
        bool FanModelFlag;         // True, this fan is FAN:SYSTEMMODEL

        // Default Constructor
        ConstantVolumeFan()
            : FlowRate(0.0), Ctrl(0.0), fanType(HVAC::FanType::Invalid), FanIndex(0), InletNode(0), OutletNode(0), MaxAirMassFlowRate(0.0),
              AirLoopNum(0), FanModelFlag(false)
        {
        }

        int calculate(EnergyPlusData &state,
                      bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                      Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                      int const i,                              // Linkage number
                      [[maybe_unused]] const Real64 multiplier, // Element multiplier
                      [[maybe_unused]] const Real64 control,    // Element control signal
                      const AirState &propN,                    // Node 1 properties
                      const AirState &propM,                    // Node 2 properties
                      std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
        );

        virtual ComponentType type()
        {
            return ComponentType::CVF;
        }
    };

    struct DetailedFan : public AirflowElement // Detailed fan component
    {
        // Members
        Real64 FlowCoef;       // Coefficient for linear initialization [kg/s at 1Pa]
        Real64 FlowExpo;       // Turbulent flow coefficient [dimensionless]
        Real64 RhoAir;         // Reference air density
        Real64 Qfree;          // Free delivery flow at P=0
        Real64 Pshut;          // Shutoff pressure at Q=0
        Real64 TranRat;        // Flow coefficient at laminar/turbulent transition
        int n;                 // Number of ranges for fan performance curve
        Array1D<Real64> Coeff; // Coefficients of fan performance curve.
        // Each range has a min flow rate and 4 coefficients

        // Default Constructor
        DetailedFan() : FlowCoef(0.0), FlowExpo(0.0), RhoAir(0.0), Qfree(0.0), Pshut(0.0), TranRat(0.0)
        {
        }

        int calculate([[maybe_unused]] EnergyPlusData &state,
                      bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                      Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                      int const i,                              // Linkage number
                      [[maybe_unused]] const Real64 multiplier, // Element multiplier
                      [[maybe_unused]] const Real64 control,    // Element control signal
                      const AirState &propN,                    // Node 1 properties
                      const AirState &propM,                    // Node 2 properties
                      std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
        );

        int calculate(EnergyPlusData &state,
                      Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                      [[maybe_unused]] const Real64 multiplier, // Element multiplier
                      const Real64 control,                     // Element control signal
                      const AirState &propN,                    // Node 1 properties
                      const AirState &propM,                    // Node 2 properties
                      std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
        );

        virtual ComponentType type()
        {
            return ComponentType::FAN;
        }
    };

    struct DisSysCompCoilProp : public AirflowElement // Coil component
    {
        // Members
        std::string EPlusType;    // EnergyPlus coil type
        Real64 L;                 // Air path length
        Real64 hydraulicDiameter; // Air path hydraulic diameter
        int AirLoopNum;           // AirLoop number

        // Default Constructor
        DisSysCompCoilProp() : L(0.0), hydraulicDiameter(0.0), AirLoopNum(0)
        {
        }

        int calculate([[maybe_unused]] EnergyPlusData &state,
                      bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                      Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                      int const i,                              // Linkage number
                      [[maybe_unused]] const Real64 multiplier, // Element multiplier
                      [[maybe_unused]] const Real64 control,    // Element control signal
                      const AirState &propN,                    // Node 1 properties
                      const AirState &propM,                    // Node 2 properties
                      std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
        );

        int calculate([[maybe_unused]] EnergyPlusData &state,
                      Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                      [[maybe_unused]] const Real64 multiplier, // Element multiplier
                      [[maybe_unused]] const Real64 control,    // Element control signal
                      const AirState &propN,                    // Node 1 properties
                      const AirState &propM,                    // Node 2 properties
                      std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
        );

        virtual ComponentType type()
        {
            return ComponentType::COI;
        }
    };

    struct DisSysCompHXProp : public AirflowElement // Coil component
    {
        // Members
        std::string EPlusType;    // EnergyPlus coil type
        Real64 L;                 // Air path length
        Real64 hydraulicDiameter; // Air path hydraulic diameter
        bool CoilParentExists;    // Is a coil component

        // Default Constructor
        DisSysCompHXProp() : L(0.0), hydraulicDiameter(0.0), CoilParentExists(false)
        {
        }

        int calculate([[maybe_unused]] EnergyPlusData &state,
                      bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                      Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                      [[maybe_unused]] int const i,             // Linkage number
                      [[maybe_unused]] const Real64 multiplier, // Element multiplier
                      [[maybe_unused]] const Real64 control,    // Element control signal
                      const AirState &propN,                    // Node 1 properties
                      const AirState &propM,                    // Node 2 properties
                      std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
        );

        int calculate([[maybe_unused]] EnergyPlusData &state,
                      Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                      [[maybe_unused]] const Real64 multiplier, // Element multiplier
                      [[maybe_unused]] const Real64 control,    // Element control signal
                      const AirState &propN,                    // Node 1 properties
                      const AirState &propM,                    // Node 2 properties
                      std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
        );

        virtual ComponentType type()
        {
            return ComponentType::HEX;
        }
    };

    struct DisSysCompTermUnitProp : public AirflowElement // Terminal unit component
    {
        // Members
        std::string EPlusType;    // EnergyPlus coil type
        Real64 L;                 // Air path length
        Real64 hydraulicDiameter; // Air path hydraulic diameter
        int DamperInletNode;      // Damper inlet node number
        int DamperOutletNode;     // Damper outlet node number
        int AirLoopNum;           // AirLoop number

        // Default Constructor
        DisSysCompTermUnitProp() : L(0.0), hydraulicDiameter(0.0), DamperInletNode(0), DamperOutletNode(0), AirLoopNum(0)
        {
        }

        int calculate([[maybe_unused]] EnergyPlusData &state,
                      bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                      Real64 const PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                      int const i,                              // Linkage number
                      [[maybe_unused]] const Real64 multiplier, // Element multiplier
                      [[maybe_unused]] const Real64 control,    // Element control signal
                      const AirState &propN,                    // Node 1 properties
                      const AirState &propM,                    // Node 2 properties
                      std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
        );

        virtual ComponentType type()
        {
            return ComponentType::TMU;
        }
    };

    struct ConstantPressureDrop : public AirflowElement // Constant pressure drop component
    {
        // Members
        Real64 A;  // cross section area
        Real64 DP; // Pressure difference across the component

        // Default Constructor
        ConstantPressureDrop() : A(0.0), DP(0.0)
        {
        }

        int calculate([[maybe_unused]] EnergyPlusData &state,
                      bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                      const Real64 PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                      int const i,                              // Linkage number
                      [[maybe_unused]] const Real64 multiplier, // Element multiplier
                      [[maybe_unused]] const Real64 control,    // Element control signal
                      const AirState &propN,                    // Node 1 properties
                      const AirState &propM,                    // Node 2 properties
                      std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                      std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
        );

        virtual ComponentType type()
        {
            return ComponentType::CPD;
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

    struct AirflowNetworkNodeProp // AirflowNetwork nodal data
    {
        // Members
        std::string Name;      // Provide a unique node name
        std::string NodeType;  // Provide node type "External", "Thermal Zone" or "Other"
        std::string EPlusNode; // EnergyPlus node name
        Real64 NodeHeight;     // Node height [m]
        int NodeNum;           // Node number
        int NodeTypeNum;       // Node type with integer number
        // 0: Calculated, 1: Given pressure;
        std::string EPlusZoneName; // EnergyPlus node name
        int EPlusZoneNum;          // E+ zone number
        int EPlusNodeNum;
        int ExtNodeNum;
        int OutAirNodeNum;
        iEPlusNodeType EPlusTypeNum;
        int RAFNNodeNum; // RoomAir model node number
        int NumOfLinks;  // Number of links for RoomAir model
        int AirLoopNum;  // AirLoop number

        // Default Constructor
        AirflowNetworkNodeProp()
            : NodeHeight(0.0), NodeNum(0), NodeTypeNum(0), EPlusZoneNum(0), EPlusNodeNum(0), ExtNodeNum(0), OutAirNodeNum(0),
              EPlusTypeNum(iEPlusNodeType::Invalid), RAFNNodeNum(0), NumOfLinks(0), AirLoopNum(0)
        {
        }
    };

    struct AirflowNetworkCompProp // AirflowNetwork element data
    {
        // Members
        std::string Name;                 // Provide a unique element name
        iComponentTypeNum CompTypeNum;    // Provide numeric equivalent for AirflowNetworkCompType
        int TypeNum;                      // Component number under same component type
        int CompNum;                      // General component number
        std::string EPlusName;            // Provide a unique element name
        std::string EPlusCompName;        // Provide EPlus component name or Other
        std::string EPlusType;            // Provide EPlus type, such as terminal reheat, coil, etc. 9/30/03 or Other
        iEPlusComponentType EPlusTypeNum; // Provide EPlus component type

        // Default Constructor
        AirflowNetworkCompProp() : CompTypeNum(iComponentTypeNum::Invalid), TypeNum(0), CompNum(0), EPlusTypeNum(iEPlusComponentType::Invalid)
        {
        }
    };

    struct AirflowNetworkLinkageProp : public AirflowNetworkLinkage // AirflowNetwork linkage data
    {
        // Members
        std::string ZoneName;               // Name of zone
        int ZoneNum;                        // Zone Number
        int DetOpenNum;                     // Large Opening number
        iEPlusComponentType ConnectionFlag; // Return and supply connection flag
        bool VAVTermDamper;                 // True if this component is a damper for a VAV terminal
        int LinkageViewFactorObjectNum;
        int AirLoopNum; // Airloop number
        DuctLineType ductLineType;

        // Default Constructor
        AirflowNetworkLinkageProp()
            : AirflowNetworkLinkage(), ZoneNum(0), DetOpenNum(0), ConnectionFlag(iEPlusComponentType::Invalid), VAVTermDamper(false),
              LinkageViewFactorObjectNum(0), AirLoopNum(0), ductLineType(DuctLineType::Invalid)
        {
        }
    };

    struct PressureControllerProp
    {
        // Members
        std::string Name;                             // Provide a unique object name
        std::string ZoneName;                         // Name of the zone that is being controlled
        int ZoneNum;                                  // Zone number
        int AFNNodeNum;                               // AFN node number
        std::string ControlObjectType;                // The control type to be used for pressure control
        std::string ControlObjectName;                // Corresponding control type name
        int ControlTypeSet;                           // Control type set to be used for pressure control
        Sched::Schedule *availSched = nullptr;        // Availability schedule pointer
        Sched::Schedule *presSetpointSched = nullptr; // Pressure setpoint schedule pointer
        int AirLoopNum;                               // Air loop number
        int OANodeNum;                                // outdoor air node number
        bool bypass;                                  // Can not perform pressure control as true
        Real64 PresCtrlMassRate;

        // Default Constructor
        PressureControllerProp() : ZoneNum(0), AFNNodeNum(0), ControlTypeSet(0), AirLoopNum(0), OANodeNum(0), bypass(false), PresCtrlMassRate(0.0)
        {
        }
    };

    struct OutdoorAirFan : public AirflowElement // OA fan component
    {
        // Members
        Sched::Schedule *sched = nullptr; // Schedule pointer
        Real64 FlowCoef;                  // Air Mass Flow Coefficient [kg/s at 1Pa]
        Real64 FlowExpo;                  // Air Mass Flow exponent [dimensionless]
        Real64 StandardT;                 // Standard temperature for crack data [C]
        Real64 StandardP;                 // Standard barometric pressure for crack data [Pa]
        Real64 StandardW;                 // Standard humidity ratio for crack data [kg/kg]
        int InletNode;                    // Inlet node number
        int OutletNode;                   // Outlet node number
        int OAMixerNum;                   // OA Mixer number
        int PressCtrlNum;                 // Pressure control number

        // Default Constructor
        OutdoorAirFan()
            : FlowCoef(0.0), FlowExpo(0.0), StandardT(0.0), StandardP(0.0), StandardW(0.0), InletNode(0), OutletNode(0), OAMixerNum(0),
              PressCtrlNum(0)
        {
        }

        virtual ~OutdoorAirFan()
        {
        }

        virtual int calculate(EnergyPlusData &state,
                              bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                              const Real64 PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                              int const i,                              // Linkage number
                              [[maybe_unused]] const Real64 multiplier, // Element multiplier
                              [[maybe_unused]] const Real64 control,    // Element control signal
                              const AirState &propN,                    // Node 1 properties
                              const AirState &propM,                    // Node 2 properties
                              std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                              std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
        );

        virtual ComponentType type()
        {
            return ComponentType::OAF;
        }
    };

    struct ReliefFlow : public OutdoorAirFan // OA fan component
    {

        // Default Constructor
        ReliefFlow() : OutdoorAirFan()
        {
        }

        virtual int calculate(EnergyPlusData &state,
                              bool const LFLAG,                         // Initialization flag.If = 1, use laminar relationship
                              const Real64 PDROP,                       // Total pressure drop across a component (P1 - P2) [Pa]
                              int const i,                              // Linkage number
                              [[maybe_unused]] const Real64 multiplier, // Element multiplier
                              [[maybe_unused]] const Real64 control,    // Element control signal
                              const AirState &propN,                    // Node 1 properties
                              const AirState &propM,                    // Node 2 properties
                              std::array<Real64, 2> &F,                 // Airflow through the component [kg/s]
                              std::array<Real64, 2> &DF                 // Partial derivative:  DF/DP
        );

        virtual ComponentType type()
        {
            return ComponentType::REL;
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

    struct AirflowNetworkExchangeProp
    {
        // Members
        Real64 MultiZoneSen;
        Real64 MultiZoneLat;
        Real64 LeakSen;
        Real64 LeakLat;
        Real64 CondSen;
        Real64 DiffLat;
        Real64 RadGain;
        Real64 TotalSen;
        Real64 TotalLat;
        Real64 SumMCp;
        Real64 SumMCpT;
        Real64 SumMVCp;
        Real64 SumMVCpT;
        Real64 SumMHr;
        Real64 SumMHrW;
        Real64 SumMMCp;
        Real64 SumMMCpT;
        Real64 SumMMHr;
        Real64 SumMMHrW;
        Real64 SumMHrCO;
        Real64 SumMMHrCO;
        Real64 TotalCO2;
        Real64 SumMHrGC;
        Real64 SumMMHrGC;
        Real64 TotalGC;

        // Default Constructor
        AirflowNetworkExchangeProp()
            : MultiZoneSen(0.0), MultiZoneLat(0.0), LeakSen(0.0), LeakLat(0.0), CondSen(0.0), DiffLat(0.0), RadGain(0.0), TotalSen(0.0),
              TotalLat(0.0), SumMCp(0.0), SumMCpT(0.0), SumMVCp(0.0), SumMVCpT(0.0), SumMHr(0.0), SumMHrW(0.0), SumMMCp(0.0), SumMMCpT(0.0),
              SumMMHr(0.0), SumMMHrW(0.0), SumMHrCO(0.0), SumMMHrCO(0.0), TotalCO2(0.0), SumMHrGC(0.0), SumMMHrGC(0.0), TotalGC(0.0)
        {
        }
    };

    struct AiflowNetworkReportProp
    {
        // Members
        Real64 MultiZoneInfiSenGainW;
        Real64 MultiZoneInfiSenGainJ;
        Real64 MultiZoneInfiSenLossW;
        Real64 MultiZoneInfiSenLossJ;
        Real64 MultiZoneVentSenGainW;
        Real64 MultiZoneVentSenGainJ;
        Real64 MultiZoneVentSenLossW;
        Real64 MultiZoneVentSenLossJ;
        Real64 MultiZoneMixSenGainW;
        Real64 MultiZoneMixSenGainJ;
        Real64 MultiZoneMixSenLossW;
        Real64 MultiZoneMixSenLossJ;
        Real64 MultiZoneInfiLatGainW;
        Real64 MultiZoneInfiLatGainJ;
        Real64 MultiZoneInfiLatLossW;
        Real64 MultiZoneInfiLatLossJ;
        Real64 MultiZoneVentLatGainW;
        Real64 MultiZoneVentLatGainJ;
        Real64 MultiZoneVentLatLossW;
        Real64 MultiZoneVentLatLossJ;
        Real64 MultiZoneMixLatGainW;
        Real64 MultiZoneMixLatGainJ;
        Real64 MultiZoneMixLatLossW;
        Real64 MultiZoneMixLatLossJ;
        Real64 LeakSenGainW;
        Real64 LeakSenGainJ;
        Real64 LeakSenLossW;
        Real64 LeakSenLossJ;
        Real64 LeakLatGainW;
        Real64 LeakLatGainJ;
        Real64 LeakLatLossW;
        Real64 LeakLatLossJ;
        Real64 CondSenGainW;
        Real64 CondSenGainJ;
        Real64 CondSenLossW;
        Real64 CondSenLossJ;
        Real64 DiffLatGainW;
        Real64 DiffLatGainJ;
        Real64 DiffLatLossW;
        Real64 DiffLatLossJ;
        Real64 RadGainW;
        Real64 RadGainJ;
        Real64 RadLossW;
        Real64 RadLossJ;
        Real64 TotalSenGainW;
        Real64 TotalSenGainJ;
        Real64 TotalSenLossW;
        Real64 TotalSenLossJ;
        Real64 TotalLatGainW;
        Real64 TotalLatGainJ;
        Real64 TotalLatLossW;
        Real64 TotalLatLossJ;
        bool OnOffFlag;

        // Default Constructor
        AiflowNetworkReportProp()
            : MultiZoneInfiSenGainW(0.0), MultiZoneInfiSenGainJ(0.0), MultiZoneInfiSenLossW(0.0), MultiZoneInfiSenLossJ(0.0),
              MultiZoneVentSenGainW(0.0), MultiZoneVentSenGainJ(0.0), MultiZoneVentSenLossW(0.0), MultiZoneVentSenLossJ(0.0),
              MultiZoneMixSenGainW(0.0), MultiZoneMixSenGainJ(0.0), MultiZoneMixSenLossW(0.0), MultiZoneMixSenLossJ(0.0), MultiZoneInfiLatGainW(0.0),
              MultiZoneInfiLatGainJ(0.0), MultiZoneInfiLatLossW(0.0), MultiZoneInfiLatLossJ(0.0), MultiZoneVentLatGainW(0.0),
              MultiZoneVentLatGainJ(0.0), MultiZoneVentLatLossW(0.0), MultiZoneVentLatLossJ(0.0), MultiZoneMixLatGainW(0.0),
              MultiZoneMixLatGainJ(0.0), MultiZoneMixLatLossW(0.0), MultiZoneMixLatLossJ(0.0), LeakSenGainW(0.0), LeakSenGainJ(0.0),
              LeakSenLossW(0.0), LeakSenLossJ(0.0), LeakLatGainW(0.0), LeakLatGainJ(0.0), LeakLatLossW(0.0), LeakLatLossJ(0.0), CondSenGainW(0.0),
              CondSenGainJ(0.0), CondSenLossW(0.0), CondSenLossJ(0.0), DiffLatGainW(0.0), DiffLatGainJ(0.0), DiffLatLossW(0.0), DiffLatLossJ(0.0),
              RadGainW(0.0), RadGainJ(0.0), RadLossW(0.0), RadLossJ(0.0), TotalSenGainW(0.0), TotalSenGainJ(0.0), TotalSenLossW(0.0),
              TotalSenLossJ(0.0), TotalLatGainW(0.0), TotalLatGainJ(0.0), TotalLatLossW(0.0), TotalLatLossJ(0.0), OnOffFlag(false)
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

} // namespace AirflowNetwork

} // namespace EnergyPlus

#endif
