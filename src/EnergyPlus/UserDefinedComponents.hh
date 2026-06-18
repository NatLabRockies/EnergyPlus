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

#ifndef UserDefinedComponents_hh_INCLUDED
#define UserDefinedComponents_hh_INCLUDED

// C++ Headers
#include <vector>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/Enums.hh>
#include <EnergyPlus/PlantComponent.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

int constexpr primaryConnIdx = 0;

namespace UserDefinedComponents {

    struct PlantConnectionStruct
    {
        // Members
        int ErlInitProgramMngr;     // points to an EMS:ProgramManager to run for setup and sizing
        int ErlSimProgramMngr;      // points to an EMS:ProgramManager to run only when this connection is called
        int simPluginLocation;      // If Python Plugins are used to simulate this, this defines the location in the plugin structure
        int initPluginLocation;     // If Python Plugins are used to init this, this defines the location in the plugin structure
        int simCallbackIndex = -1;  // If API Callbacks are used to simulate this, this defines the index in the user-defined callbacks vector
        int initCallbackIndex = -1; // If API Callbacks are used to init this, this defines the index in the user-defined callbacks vector
        PlantLocation plantLoc;
        int InletNodeNum;                       // plant loop inlet node index
        int OutletNodeNum;                      // plant loop outlet node index
        DataPlant::LoopFlowStatus FlowPriority; // how component affects overall loop flow determination
        DataPlant::HowMet HowLoadServed;        // nature of component wrt to plant loop's loads
        Real64 LowOutTempLimit;                 // low limit for outlet temp if MEETSLOADWITHNOMINALCAPACITYLOWOUTLIMIT
        Real64 HiOutTempLimit;                  // hi limit for outlet temp if MEETSLOADWITHNOMINALCAPACITYHIOUTLIMIT
        Real64 MassFlowRateRequest;             // request filled by actuator, might not be satisfied if plant constrained [kg/s]
        Real64 MassFlowRateMin;                 // filled by actuator, reports minimum (hardware) flow rate for component [kg/s]
        Real64 MassFlowRateMax;                 // filled by actuator, reports maximum (hardware) flow rate for component [kg/s]
        Real64 DesignVolumeFlowRate;            // filled by actuator,
        Real64 MyLoad;                          // fills internal variable for user's model to know current load request of supply equip [W]
        Real64 MinLoad;                         // filled by actuator, reports back size for load dispatch routines [W]
        Real64 MaxLoad;                         // filled by actuator, reports back size for load dispatch [W]
        Real64 OptLoad;                         // filled by actuator, reports back size for load dispatch [W]
        Real64 InletRho;                        // fills internal variable, current density for fluid type and inlet temperature [kg/m3]
        Real64 InletCp;                         // fills internal Variable, current specific heat for fluid type and inlet temperature [J/kg-C]
        Real64 InletTemp;                       // fills internal variable, current inlet fluid temperature [C]
        Real64 InletMassFlowRate;               // fills internal variable, current inlet mass flow rate [kg/s]
        Real64 OutletTemp;                      // filled by actuator, component outlet temperature [C]

        // Default Constructor
        PlantConnectionStruct()
            : ErlInitProgramMngr(0), ErlSimProgramMngr(0), simPluginLocation(-1), initPluginLocation(-1), plantLoc{}, InletNodeNum(0),
              OutletNodeNum(0), FlowPriority(DataPlant::LoopFlowStatus::Invalid), HowLoadServed(DataPlant::HowMet::Invalid), LowOutTempLimit(0.0),
              HiOutTempLimit(0.0), MassFlowRateRequest(0.0), MassFlowRateMin(0.0), MassFlowRateMax(0.0), DesignVolumeFlowRate(0.0), MyLoad(0.0),
              MinLoad(0.0), MaxLoad(0.0), OptLoad(0.0), InletRho(0.0), InletCp(0.0), InletTemp(0.0), InletMassFlowRate(0.0), OutletTemp(0.0)
        {
        }
    };

    struct AirConnectionStruct
    {
        // Members
        int InletNodeNum;          // air inlet node index
        int OutletNodeNum;         // air outlet node index
        Real64 InletRho;           // fills internal variable, current inlet air density [kg/m3]
        Real64 InletCp;            // fills internal variable, current inlet air specific heat [J/kg-c]
        Real64 InletTemp;          // fills internal variable, current inlet air temperature [C]
        Real64 InletHumRat;        // fills internal variable, current inlet air humidity ratio [kg/kg]
        Real64 InletMassFlowRate;  // fills internal variable, current inlet air mass flow rate [kg/s]
        Real64 OutletTemp;         // filled by actuator, component outlet temperature [C]
        Real64 OutletHumRat;       // filled by actuator, component outlet humidity ratio [kg/kg]
        Real64 OutletMassFlowRate; // filled by actuator, component outlet mass flow rate [kg/s]

        // Default Constructor
        AirConnectionStruct()
            : InletNodeNum(0), OutletNodeNum(0), InletRho(0.0), InletCp(0.0), InletTemp(0.0), InletHumRat(0.0), InletMassFlowRate(0.0),
              OutletTemp(0.0), OutletHumRat(0.0), OutletMassFlowRate(0.0)
        {
        }
    };

    struct WaterUseTankConnectionStruct // data for interacting with water use storage system
    {
        // Members
        bool SuppliedByWaterSystem;
        int SupplyTankID;          // index "pointer" to WaterStorage structure
        int SupplyTankDemandARRID; // index "pointer" to demand array inside WaterStorage structure
        Real64 SupplyVdotRequest;
        bool CollectsToWaterSystem;
        int CollectionTankID;          // index "pointer" to Storage TAnk array WaterStorage
        int CollectionTankSupplyARRID; // index pointe to supply Vdot array in WaterStorage
        Real64 CollectedVdot;

        // Default Constructor
        WaterUseTankConnectionStruct()
            : SuppliedByWaterSystem(false), SupplyTankID(0), SupplyTankDemandARRID(0), SupplyVdotRequest(0.0), CollectsToWaterSystem(false),
              CollectionTankID(0), CollectionTankSupplyARRID(0), CollectedVdot(0.0)
        {
        }
    };

    struct ZoneInternalGainsStruct
    {
        // Members
        bool DeviceHasInternalGains;
        int ZoneNum;
        Real64 ConvectionGainRate;
        Real64 ReturnAirConvectionGainRate;
        Real64 ThermalRadiationGainRate;
        Real64 LatentGainRate;
        Real64 ReturnAirLatentGainRate;
        Real64 CarbonDioxideGainRate;
        Real64 GenericContamGainRate;

        // Default Constructor
        ZoneInternalGainsStruct()
            : DeviceHasInternalGains(false), ZoneNum(0), ConvectionGainRate(0.0), ReturnAirConvectionGainRate(0.0), ThermalRadiationGainRate(0.0),
              LatentGainRate(0.0), ReturnAirLatentGainRate(0.0), CarbonDioxideGainRate(0.0), GenericContamGainRate(0.0)
        {
        }
    };

    struct UserPlantComponentStruct : PlantComponent
    {
        // Members
        std::string Name;      // user identifier
        int ErlSimProgramMngr; // EMS:ProgramManager to always run when this model is called
        int simPluginLocation; // If Python Plugins are used to simulate this, this defines the location in the plugin structure
        int simCallbackIndex = -1;
        int NumPlantConnections;                 // count of how many plant loop connections there are
        std::vector<PlantConnectionStruct> Loop; // collect data for each plant loop connection
        AirConnectionStruct Air;
        WaterUseTankConnectionStruct Water;
        ZoneInternalGainsStruct Zone;
        bool myOneTimeFlag;

        // Default Constructor
        UserPlantComponentStruct() : ErlSimProgramMngr(0), simPluginLocation(-1), NumPlantConnections(0), myOneTimeFlag(true)
        {
        }

        static PlantComponent *factory(EnergyPlusData &state, std::string const &objectName);

        void onInitLoopEquip([[maybe_unused]] EnergyPlusData &state, const PlantLocation &calledFromLocation) override;

        void getDesignCapacities(
            EnergyPlusData &state, const PlantLocation &calledFromLocation, Real64 &MaxLoad, Real64 &MinLoad, Real64 &OptLoad) override;

        void
        simulate(EnergyPlusData &state, const PlantLocation &calledFromLocation, bool FirstHVACIteration, Real64 &CurLoad, bool RunFlag) override;

        void initialize(EnergyPlusData &state, int LoopNum, Real64 MyLoad);

        void report(EnergyPlusData &state, int LoopNum);

        void oneTimeInit(EnergyPlusData &state) override;
    };

    struct UserCoilComponentStruct
    {
        // Members
        std::string Name;           // user identifier
        int ErlSimProgramMngr;      // EMS:ProgramManager to always run when this model is called
        int ErlInitProgramMngr;     // EMS:ProgramManager to  run when this model is initialized and setup
        int initPluginLocation;     // If Python Plugins are used to init this, this defines the location in the plugin structure
        int simPluginLocation;      // If Python Plugins are used to simulate this, this defines the location in the plugin structure
        int initCallbackIndex = -1; // If API callbacks are used to init this, this defines the location in the vector
        int simCallbackIndex = -1;  // If API callbacks are used to simulate this, this defines the location in the vector
        int NumAirConnections;      // count of how many air connections there are
        bool PlantIsConnected;
        std::vector<AirConnectionStruct> AirConnections;
        PlantConnectionStruct Loop;
        WaterUseTankConnectionStruct Water;
        ZoneInternalGainsStruct Zone;
        bool myOneTimeFlag;

        // Default Constructor
        UserCoilComponentStruct()
            : ErlSimProgramMngr(0), ErlInitProgramMngr(0), initPluginLocation(-1), simPluginLocation(-1), NumAirConnections(0),
              PlantIsConnected(false), myOneTimeFlag(true)
        {
        }

        void initialize(EnergyPlusData &state);

        void report(EnergyPlusData &state);
    };

    struct UserAirComponentStruct
    {
        // Members
        std::string Name;           // user identifier
        int ErlSimProgramMngr;      // EMS:ProgramManager to always run when this model is called
        int ErlInitProgramMngr;     // EMS:ProgramManager to  run when this model is initialized and setup
        int initPluginLocation;     // If Python Plugins are used to init this, this defines the location in the plugin structure
        int simPluginLocation;      // If Python Plugins are used to simulate this, this defines the location in the plugin structure
        int initCallbackIndex = -1; // If API callbacks are used to init this, this defines the location in the vector
        int simCallbackIndex = -1;  // If API callbacks are used to simulate this, this defines the location in the vector
        AirConnectionStruct SourceAir;
        int NumPlantConnections;                 // count of how many plant loop (demand) connections there are
        std::vector<PlantConnectionStruct> Loop; // collect data for each plant loop connection
        WaterUseTankConnectionStruct Water;
        ZoneInternalGainsStruct Zone;         // for skin losses
        Real64 RemainingOutputToHeatingSP;    // sensible load remaining for device, to heating setpoint [W]
        Real64 RemainingOutputToCoolingSP;    // sensible load remaining for device, negative means cooling [W]
        Real64 RemainingOutputReqToHumidSP;   // latent load remaining for device, to humidification setpoint [kg/s]
        Real64 RemainingOutputReqToDehumidSP; // latent load remaining for device, Negative means dehumidify [kg/s]
        bool myOneTimeFlag;
        AirConnectionStruct AirConnection;

        // Default Constructor
        UserAirComponentStruct()
            : ErlSimProgramMngr(0), ErlInitProgramMngr(0), initPluginLocation(-1), simPluginLocation(-1), NumPlantConnections(0),
              RemainingOutputToHeatingSP(0.0), RemainingOutputToCoolingSP(0.0), RemainingOutputReqToHumidSP(0.0), RemainingOutputReqToDehumidSP(0.0),
              myOneTimeFlag(true)
        {
        }

        virtual ~UserAirComponentStruct() = default;

        virtual void initialize(EnergyPlusData &state, int ZoneNum) = 0;

        virtual void report(EnergyPlusData &state) = 0;
    };

    struct UserZoneHVACForcedAirComponentStruct : public UserAirComponentStruct
    {
        UserZoneHVACForcedAirComponentStruct() : UserAirComponentStruct()
        {
        }

        void initialize(EnergyPlusData &state, int ZoneNum) override;

        void report(EnergyPlusData &state) override;
    };

    struct UserAirTerminalComponentStruct : public UserAirComponentStruct
    {
        int ActualCtrlZoneNum;
        int ADUNum; // index of corresponding air distribution unit

        UserAirTerminalComponentStruct() : UserAirComponentStruct(), ActualCtrlZoneNum(0), ADUNum(0)
        {
        }

        void initialize(EnergyPlusData &state, int ZoneNum) override;

        void report(EnergyPlusData &state) override;
    };

    void SimCoilUserDefined(EnergyPlusData &state,
                            std::string_view EquipName, // user name for component
                            int &CompIndex,
                            int AirLoopNum,
                            bool &HeatingActive,
                            bool &CoolingActive);

    void SimZoneAirUserDefined(EnergyPlusData &state,
                               std::string_view CompName,      // name of the packaged terminal heat pump
                               int ZoneNum,                    // number of zone being served
                               Real64 &SensibleOutputProvided, // sensible capacity delivered to zone
                               Real64 &LatentOutputProvided,   // Latent add/removal  (kg/s), dehumid = negative
                               int &CompIndex                  // index to zone hvac unit
    );

    void SimAirTerminalUserDefined(
        EnergyPlusData &state, std::string_view CompName, bool FirstHVACIteration, int ZoneNum, int ZoneNodeNum, int &CompIndex);

    void GetUserDefinedPlantComponents(EnergyPlusData &state);

    void GetUserDefinedComponents(EnergyPlusData &state);

    void GetUserDefinedAirComponent(EnergyPlusData &state);

    void GetUserDefinedCoilIndex(
        EnergyPlusData &state, std::string const &CoilName, int &CoilIndex, bool &ErrorsFound, std::string const &CurrentModuleObject);

    void GetUserDefinedCoilAirInletNode(
        EnergyPlusData &state, std::string const &CoilName, int &CoilAirInletNode, bool &ErrorsFound, std::string const &CurrentModuleObject);

    void GetUserDefinedCoilAirOutletNode(
        EnergyPlusData &state, std::string const &CoilName, int &CoilAirOutletNode, bool &ErrorsFound, std::string const &CurrentModuleObject);

} // namespace UserDefinedComponents

struct UserDefinedComponentsData : BaseGlobalStruct
{

    int NumUserPlantComps = 0;
    int NumUserCoils = 0;
    int NumUserZoneAir = 0;
    int NumUserAirTerminals = 0;

    bool GetInput = true;
    bool GetAirTerminalInput = true;
    bool GetPlantCompInput = true;

    std::vector<bool> CheckUserPlantCompName;
    std::vector<bool> CheckUserCoilName;
    std::vector<bool> CheckUserZoneAirName;
    std::vector<bool> CheckUserAirTerminal;

    // Object Data
    EPVector<UserDefinedComponents::UserPlantComponentStruct> UserPlantComp;
    EPVector<UserDefinedComponents::UserCoilComponentStruct> UserCoil;
    EPVector<UserDefinedComponents::UserZoneHVACForcedAirComponentStruct> UserZoneAirHVAC;
    EPVector<UserDefinedComponents::UserAirTerminalComponentStruct> UserAirTerminal;

    bool lDummy_EMSActuatedPlantComp = false;
    bool lDummy_GetUserDefComp = false;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->GetInput = true;
        this->GetPlantCompInput = true;
        this->NumUserPlantComps = 0;
        this->NumUserCoils = 0;
        this->NumUserZoneAir = 0;
        this->NumUserAirTerminals = 0;
        this->CheckUserPlantCompName.clear();
        this->CheckUserCoilName.clear();
        this->CheckUserZoneAirName.clear();
        this->CheckUserAirTerminal.clear();
        this->UserPlantComp.deallocate();
        this->UserCoil.deallocate();
        this->UserZoneAirHVAC.deallocate();
        this->UserAirTerminal.deallocate();
        this->lDummy_EMSActuatedPlantComp = false;
        this->lDummy_GetUserDefComp = false;
    }

    // Default Constructor
    UserDefinedComponentsData() = default;
};
} // namespace EnergyPlus

#endif
