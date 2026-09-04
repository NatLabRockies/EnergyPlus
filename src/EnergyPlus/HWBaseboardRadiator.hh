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

#ifndef HWBaseboardRadiator_hh_INCLUDED
#define HWBaseboardRadiator_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/Enums.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace HWBaseboardRadiator {

    extern std::string const cCMO_BBRadiator_Water;

    struct HWBaseboardParams
    {
        // Members
        std::string Name;
        DataPlant::PlantEquipmentType EquipType = DataPlant::PlantEquipmentType::Invalid;
        std::string designObjectName; // Design Object
        int DesignObjectPtr = 0;
        Array1D_int SurfacePtr;
        int ZonePtr = 0;
        Sched::Schedule *availSched = nullptr;
        int WaterInletNode = 0;
        int WaterOutletNode = 0;
        int TotSurfToDistrib = 0;
        int ControlCompTypeNum = 0;
        int CompErrIndex = 0;
        Real64 AirMassFlowRate = 0.0;
        Real64 AirMassFlowRateStd = 0.0;
        Real64 WaterTempAvg = 0.0;
        Real64 RatedCapacity = 0.0;
        Real64 UA = 0.0;
        Real64 WaterMassFlowRate = 0.0;
        Real64 WaterMassFlowRateMax = 0.0;
        Real64 WaterMassFlowRateStd = 0.0;
        Real64 WaterVolFlowRateMax = 0.0;
        Real64 WaterInletTempStd = 0.0;
        Real64 WaterInletTemp = 0.0;
        Real64 WaterInletEnthalpy = 0.0;
        Real64 WaterOutletTempStd = 0.0;
        Real64 WaterOutletTemp = 0.0;
        Real64 WaterOutletEnthalpy = 0.0;
        Real64 AirInletTempStd = 0.0;
        Real64 AirInletTemp = 0.0;
        Real64 AirOutletTemp = 0.0;
        Real64 AirInletHumRat = 0.0;
        Real64 AirOutletTempStd = 0.0;
        Real64 FracConvect = 0.0;
        Array1D<Real64> FracDistribToSurf;
        Real64 TotPower = 0.0;
        Real64 Power = 0.0;
        Real64 ConvPower = 0.0;
        Real64 RadPower = 0.0;
        Real64 TotEnergy = 0.0;
        Real64 Energy = 0.0;
        Real64 ConvEnergy = 0.0;
        Real64 RadEnergy = 0.0;
        PlantLocation plantLoc = {};
        int BBLoadReSimIndex = 0;
        int BBMassFlowReSimIndex = 0;
        int BBInletTempFlowReSimIndex = 0;
        int HeatingCapMethod = 0;            // - Method for heating capacity scaled sizing calculation (HeatingDesignCapacity, CapacityPerFloorArea,
                                             // FracOfAutosizedHeatingCapacity)
        Real64 ScaledHeatingCapacity = 0.0;  // - scaled maximum heating capacity {W} or scalable variable of zone HVAC equipment, {-}, or {W/m2}
        Real64 ZeroBBSourceSumHATsurf = 0.0; // used in baseboard energy balance
        // Record keeping variables used to calculate QBBRadSrcAvg locally
        Real64 QBBRadSource = 0.0;       // Need to keep the last value in case we are still iterating
        Real64 QBBRadSrcAvg = 0.0;       // Need to keep the last value in case we are still iterating
        Real64 LastSysTimeElapsed = 0.0; // Need to keep the last value in case we are still iterating
        Real64 LastTimeStepSys = 0.0;    // Need to keep the last value in case we are still iterating
        Real64 LastQBBRadSrc = 0.0;      // Need to keep the last value in case we are still iterating
    };

    struct HWBaseboardDesignData
    {
        // Members
        std::string designName;
        // - Method for heating capacity scaledsizing calculation (HeatingDesignCapacity, CapacityPerFloorArea, FracOfAutosizedHeatingCapacity)
        DataSizing::DesignSizingType HeatingCapMethod = DataSizing::DesignSizingType::Invalid;
        Real64 ScaledHeatingCapacity = 0.0; // scaled maximum heating capacity {W} or scalable variable of zone HVAC equipment, {-}, or {W/m2}
        Real64 Offset = 0.0;
        Real64 FracRadiant = 0.0;
        Real64 FracDistribPerson = 0.0;
    };

    struct HWBaseboardNumericFieldData
    {
        // Members
        Array1D_string FieldNames;

        // Default Constructor
        HWBaseboardNumericFieldData()
        {
        }
    };

    struct HWBaseboardDesignNumericFieldData
    {
        // Members
        Array1D_string FieldNames;

        // Default Constructor
        HWBaseboardDesignNumericFieldData()
        {
        }
    };

    void SimHWBaseboard(EnergyPlusData &state,
                        std::string const &EquipName,
                        int const ControlledZoneNum,
                        bool const FirstHVACIteration,
                        Real64 &PowerMet,
                        int &CompIndex);

    void GetHWBaseboardInput(EnergyPlusData &state);

    void InitHWBaseboard(EnergyPlusData &state, int const BaseboardNum, int const ControlledZoneNum, bool const FirstHVACIteration);

    void SizeHWBaseboard(EnergyPlusData &state, int const BaseboardNum);

    void CalcHWBaseboard(EnergyPlusData &state, int &BaseboardNum, Real64 &LoadMet);

    void UpdateHWBaseboard(EnergyPlusData &state, int const BaseboardNum);

    void UpdateBBRadSourceValAvg(EnergyPlusData &state, bool &HWBaseboardSysOn); // .TRUE. if the radiant system has run this zone time step

    void DistributeBBRadGains(EnergyPlusData &state);

    void ReportHWBaseboard(EnergyPlusData &state, int const BaseboardNum);

    void UpdateHWBaseboardPlantConnection(EnergyPlusData &state,
                                          int const BaseboardTypeNum,                 // type index
                                          std::string const &BaseboardName,           // component name
                                          int const EquipFlowCtrl,                    // Flow control mode for the equipment
                                          int const LoopNum,                          // Plant loop index for where called from
                                          const DataPlant::LoopSideLocation LoopSide, // Plant loop side index for where called from
                                          int &CompIndex,                             // Chiller number pointer
                                          bool const FirstHVACIteration,
                                          bool &InitLoopEquip // If not zero, calculate the max load for operating conditions
    );

    //*****************************************************************************************

} // namespace HWBaseboardRadiator

struct HWBaseboardRadiatorData : BaseGlobalStruct
{

    Array1D_bool MySizeFlag;
    Array1D_bool CheckEquipName;
    Array1D_bool SetLoopIndexFlag; // get loop number flag
    int NumHWBaseboards = 0;
    int NumHWBaseboardDesignObjs = 0; // Number of HW Baseboard systems design objects
    // Object Data
    Array1D<HWBaseboardRadiator::HWBaseboardParams> HWBaseboard;
    Array1D<HWBaseboardRadiator::HWBaseboardDesignData> HWBaseboardDesignObject;
    Array1D<HWBaseboardRadiator::HWBaseboardNumericFieldData> HWBaseboardNumericFields;
    bool GetInputFlag = true; // One time get input flag
    bool MyOneTimeFlag = true;
    int Iter = 0;
    bool MyEnvrnFlag2 = true;
    Array1D_bool MyEnvrnFlag;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->MySizeFlag.clear();
        this->CheckEquipName.clear();
        this->SetLoopIndexFlag.clear();
        this->NumHWBaseboards = 0;
        this->NumHWBaseboardDesignObjs = 0;
        this->HWBaseboard.clear();
        this->HWBaseboardDesignObject.clear();
        this->HWBaseboardNumericFields.clear();
        this->GetInputFlag = true;
        this->MyOneTimeFlag = true;
        this->MyEnvrnFlag.clear();
        this->Iter = 0;
        this->MyEnvrnFlag2 = true;
    }
};

} // namespace EnergyPlus

#endif
