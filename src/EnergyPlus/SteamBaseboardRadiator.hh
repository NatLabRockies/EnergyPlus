// EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the University of Illinois,
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

#ifndef SteamBaseboardRadiator_hh_INCLUDED
#define SteamBaseboardRadiator_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/FluidProperties.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace SteamBaseboardRadiator {

    struct SteamBaseboardParams
    {
        // Members
        std::string Name;
        DataPlant::PlantEquipmentType EquipType;
        std::string designObjectName; // Design Object
        int DesignObjectPtr;
        Array1D_string SurfaceName;
        Array1D_int SurfacePtr;
        int ZonePtr;
        Sched::Schedule *availSched = nullptr;
        int SteamInletNode;   // Inlet steam baseboard node
        int SteamOutletNode;  // Outlet steam baseboard node
        int TotSurfToDistrib; // Total numbers of the surfaces that the radiant heat gets distributed
        Fluid::RefrigProps *steam = nullptr;
        int ControlCompTypeNum;
        int CompErrIndex;
        Real64 DegOfSubcooling;      // Temperature differences due to subcooling of the condensate [C]
        Real64 SteamMassFlowRate;    // Mass flow rate of steam passing through the heater [kg/s]
        Real64 SteamMassFlowRateMax; // Maximum mass flow rate of steam [kg/s]
        Real64 SteamVolFlowRateMax;  // Maximum volumetric flow rate of steam [m3/s]
        Real64 SteamOutletTemp;      // Outlet steam temperature from the heater [C]
        Real64 SteamInletTemp;       // Inlet steam temperature [C]
        Real64 SteamInletEnthalpy;   // Enthalpy of the steam delivered from the boiler [J/kg]
        Real64 SteamOutletEnthalpy;  // Enthalpy of the steam leaving the heater [J/kg]
        Real64 SteamInletPress;      // Pressure of steam at the inlet of the heater [Pa]
        Real64 SteamOutletPress;     // Pressure of steam at the outlet of the heater [Pa]
        Real64 SteamInletQuality;    // Quality of steam at the inlet of the heater [Pa]
        Real64 SteamOutletQuality;   // Quality of steam at the outlet of the heater [Pa]
        Real64 FracRadiant;          // User defined fraction for radiant heat addition
        Real64 FracConvect;          // Fraction for convective heat addition
        Real64 FracDistribPerson;    // Fraction for radiant heat incident on people
        Array1D<Real64> FracDistribToSurf;
        Real64 TotPower;   // Convective system impact rate that the heater actually meets [W]
        Real64 Power;      // Maximum heating rate [W]
        Real64 ConvPower;  // Convective heating rate [W]
        Real64 RadPower;   // Radiant heating rate [W]
        Real64 TotEnergy;  // Convective system impact energy [J]
        Real64 Energy;     // Maximum heating energy [J]
        Real64 ConvEnergy; // Convective heating energy [J]
        Real64 RadEnergy;  // Radiant heating energy [J]
        PlantLocation plantLoc;
        int BBLoadReSimIndex;
        int BBMassFlowReSimIndex;
        int BBInletTempFlowReSimIndex;
        Real64 QBBSteamRadSource;           // Need to keep the last value in case we are still iterating
        Real64 QBBSteamRadSrcAvg;           // Need to keep the last value in case we are still iterating
        Real64 ZeroBBSteamSourceSumHATsurf; // Equal to the SumHATsurf for all the walls in a zone
                                            // with no source

        // Record keeping variables used to calculate QBBRadSrcAvg locally
        Real64 LastQBBSteamRadSrc; // Need to keep the last value in case we are still iterating
        Real64 LastSysTimeElapsed; // Need to keep the last value in case we are still iterating
        Real64 LastTimeStepSys;    // Need to keep the last value in case we are still iterating

        Real64 ScaledHeatingCapacity; // -  steam baseboard Radiator system scaled maximum heating capacity {W} or scalable variable of zone HVAC
                                      // equipment, {-}, or {W/m2}

        // Default Constructor
        SteamBaseboardParams()
            : EquipType(DataPlant::PlantEquipmentType::Invalid), DesignObjectPtr(0), ZonePtr(0), SteamInletNode(0), SteamOutletNode(0),
              TotSurfToDistrib(0), ControlCompTypeNum(0), CompErrIndex(0), DegOfSubcooling(0.0), SteamMassFlowRate(0.0), SteamMassFlowRateMax(0.0),
              SteamVolFlowRateMax(0.0), SteamOutletTemp(0.0), SteamInletTemp(0.0), SteamInletEnthalpy(0.0), SteamOutletEnthalpy(0.0),
              SteamInletPress(0.0), SteamOutletPress(0.0), SteamInletQuality(0.0), SteamOutletQuality(0.0), FracRadiant(0.0), FracConvect(0.0),
              FracDistribPerson(0.0), TotPower(0.0), Power(0.0), ConvPower(0.0), RadPower(0.0), TotEnergy(0.0), Energy(0.0), ConvEnergy(0.0),
              RadEnergy(0.0), plantLoc{}, BBLoadReSimIndex(0), BBMassFlowReSimIndex(0), BBInletTempFlowReSimIndex(0), QBBSteamRadSource(0.0),
              QBBSteamRadSrcAvg(0.0), ZeroBBSteamSourceSumHATsurf(0.0), LastQBBSteamRadSrc(0.0), LastSysTimeElapsed(0.0), LastTimeStepSys(0.0),
              ScaledHeatingCapacity(0.0)
        {
        }
    };

    struct SteamBaseboardDesignData : SteamBaseboardParams
    {
        // Members
        std::string designName;
        int HeatingCapMethod; // - Method for heating capacity scaledsizing calculation (HeatingDesignCapacity, CapacityPerFloorArea,
        // FracOfAutosizedHeatingCapacity)
        Real64 DesignScaledHeatingCapacity; // - scaled maximum heating capacity {W} or scalable variable of zone HVAC equipment, {-}, or {W/m2}
        Real64 Offset;
        Real64 FracRadiant;
        Real64 FracDistribPerson;

        // Default Constructor
        SteamBaseboardDesignData() : HeatingCapMethod(0), DesignScaledHeatingCapacity(0.0), Offset(0.0), FracRadiant(0.0), FracDistribPerson(0.0)
        {
        }
    };

    struct SteamBaseboardNumericFieldData
    {
        // Members
        Array1D_string FieldNames;

        // Default Constructor
        SteamBaseboardNumericFieldData()
        {
        }
    };

    struct SteamBaseboardDesignNumericFieldData
    {
        // Members
        Array1D_string FieldNames;

        // Default Constructor
        SteamBaseboardDesignNumericFieldData()
        {
        }
    };

    void SimSteamBaseboard(EnergyPlusData &state,
                           std::string const &EquipName,
                           int const ControlledZoneNum,
                           bool const FirstHVACIteration,
                           Real64 &PowerMet,
                           int &CompIndex);

    void GetSteamBaseboardInput(EnergyPlusData &state);

    void InitSteamBaseboard(EnergyPlusData &state, int const BaseboardNum, int const ControlledZoneNum, bool const FirstHVACIteration);

    void SizeSteamBaseboard(EnergyPlusData &state, int const BaseboardNum);

    void CalcSteamBaseboard(EnergyPlusData &state, int &BaseboardNum, Real64 &LoadMet);

    void UpdateSteamBaseboard(EnergyPlusData &state, int const BaseboardNum);

    void UpdateBBSteamRadSourceValAvg(EnergyPlusData &state, bool &SteamBaseboardSysOn); // .TRUE. if the radiant system has run this zone time step

    void DistributeBBSteamRadGains(EnergyPlusData &state);

    void ReportSteamBaseboard(EnergyPlusData &state, int const BaseboardNum);

    void UpdateSteamBaseboardPlantConnection(EnergyPlusData &state,
                                             DataPlant::PlantEquipmentType BaseboardType, // type index
                                             std::string const &BaseboardName,            // component name
                                             int const EquipFlowCtrl,                     // Flow control mode for the equipment
                                             int const LoopNum,                           // Plant loop index for where called from
                                             const DataPlant::LoopSideLocation LoopSide,  // Plant loop side index for where called from
                                             int &CompIndex,                              // Chiller number pointer
                                             bool const FirstHVACIteration,
                                             bool const InitLoopEquip // If not zero, calculate the max load for operating conditions
    );

} // namespace SteamBaseboardRadiator

struct SteamBaseboardRadiatorData : BaseGlobalStruct
{

    std::string const cCMO_BBRadiator_Steam = "ZoneHVAC:Baseboard:RadiantConvective:Steam";
    std::string const cCMO_BBRadiator_Steam_Design = "ZoneHVAC:Baseboard:RadiantConvective:Steam:Design";
    int NumSteamBaseboards = 0;
    int NumSteamBaseboardsDesign = 0;

    Array1D_bool MySizeFlag;
    Array1D_bool CheckEquipName;
    Array1D_bool CheckDesignObjectName;
    Array1D_bool SetLoopIndexFlag; // get loop number flag

    bool GetInputFlag = true; // one time get input flag
    bool MyOneTimeFlag = true;
    bool ZoneEquipmentListChecked = false;
    Array1D_bool MyEnvrnFlag;

    Array1D<SteamBaseboardRadiator::SteamBaseboardParams> SteamBaseboard;
    Array1D<SteamBaseboardRadiator::SteamBaseboardDesignData> SteamBaseboardDesign;
    Array1D<SteamBaseboardRadiator::SteamBaseboardNumericFieldData> SteamBaseboardNumericFields;
    Array1D<SteamBaseboardRadiator::SteamBaseboardDesignNumericFieldData> SteamBaseboardDesignNumericFields;
    Array1D_string SteamBaseboardDesignNames; // Array that contains the names of Design objects

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        NumSteamBaseboards = 0;
        MySizeFlag.clear();
        MyEnvrnFlag.clear();
        CheckEquipName.clear();
        SetLoopIndexFlag.clear();
        GetInputFlag = true;
        MyOneTimeFlag = true;
        ZoneEquipmentListChecked = false;
        SteamBaseboard.clear();
        SteamBaseboardNumericFields.clear();
        SteamBaseboardDesignNames.clear();
    }

    // Default Constructor
    SteamBaseboardRadiatorData() = default;
};
} // namespace EnergyPlus

#endif
