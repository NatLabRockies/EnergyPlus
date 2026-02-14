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

#ifndef HeatPumpWaterToWaterSimple_hh_INCLUDED
#define HeatPumpWaterToWaterSimple_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/PlantComponent.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace HeatPumpWaterToWaterSimple {

    struct GshpSpecs : public PlantComponent
    {
        // Members
        std::string Name;                       // user identifier
        bool checkEquipName;                    // name check flag
        DataPlant::PlantEquipmentType WWHPType; // equipment type num
        bool Available;                         // need an array of logicals--load identifiers of available equipment
        bool ON;                                // simulate the machine at it's operating part load ratio
        bool IsOn;                              // flag that the heat pump is ON during current time step
        bool MustRun;                           // flag that the heat pump is MUST RUN during current time step
        Real64 SourceSideDesignMassFlow;        // Design flow rate (kg/s)

        Real64 LoadSideDesignMassFlow; // Design flow rate (kg/s)

        Real64 RatedLoadVolFlowCool;             // Rated Cooling Load Side Volumetric Flow Rate [m3/s]
        bool ratedLoadVolFlowCoolWasAutoSized;   // true if RatedSourceVolFlowCool was autosize on input
        Real64 RatedSourceVolFlowCool;           // Rated Cooling Source Side Volumetric Flow Rate [m3/s]
        bool ratedSourceVolFlowCoolWasAutoSized; // true if RatedSourceVolFlowCool was autosize on input
        Real64 RatedCapCool;                     // Rated Cooling Capacity [W]
        bool ratedCapCoolWasAutoSized;           // true if RatedCapCool was autosize on input
        Real64 RatedPowerCool;                   // Rated Cooling Power Consumption[W]
        bool ratedPowerCoolWasAutoSized;         // ture if RatedPowerCool was autosize on input
        int CoolCapCurveIndex;                   // Index of the Cooling capacity performance curve
        int CoolPowCurveIndex;                   // Index of the Cooling power consumption curve
        int CoolCapNegativeCounter;              // Counter for number of times cooling capacity curve is <= 0.0
        int CoolCapNegativeIndex;                // Index for recurring warning message regarding cooling capacity curve is <= 0.0
        int CoolPowerNegativeCounter;            // Counter for number of times cooling power curve is <= 0.0
        int CoolPowerNegativeIndex;              // Index for recurring warning message regarding cooling power curve is <= 0.0
        Real64 RatedLoadVolFlowHeat;             // Rated Heating Load Side Volumetric Flow Rate [m3/s]
        bool ratedLoadVolFlowHeatWasAutoSized;   // true if RatedLoadVolFlowHeat was autosize on input
        Real64 RatedSourceVolFlowHeat;           // Rated Heating Source Side Volumetric Flow Rate [m3/s]
        bool ratedSourceVolFlowHeatWasAutoSized; // true if RatedSourceVolFlowHeat was autosize on input
        Real64 RatedCapHeat;                     // Rated Heating Capacity [W]
        bool ratedCapHeatWasAutoSized;           // true if RatedCapHeat was autosize on input
        Real64 RatedPowerHeat;                   // Rated Heating Compressor Power[W]
        bool ratedPowerHeatWasAutoSized;         // true if RatedPowerHeat was autosize on input
        int HeatCapCurveIndex;                   // Index of the Heating capacity performance curve
        int HeatPowCurveIndex;                   // Index of the Heating power consumption curve
        int LoadSideInletNodeNum;                // Load Side Inlet Node
        int LoadSideOutletNodeNum;               // Load Side Outlet Node
        int SourceSideInletNodeNum;              // Source Side Inlet Node
        int SourceSideOutletNodeNum;             // Source Side Outlet Node
        int HeatCapNegativeCounter;              // Counter for number of times heating capacity curve is <= 0.0
        int HeatCapNegativeIndex;                // Index for recurring warning message regarding heating capacity curve is <= 0.0
        int HeatPowerNegativeCounter;            // Counter for number of times heating power curve is <= 0.0
        int HeatPowerNegativeIndex;              // Index for recurring warning message regarding heating power curve is <= 0.0
        // loop topology variables
        PlantLocation SourcePlantLoc; // source side plant loop component location
        PlantLocation LoadPlantLoc;   // load side plant loop component location
        int CondMassFlowIndex;        // index for criteria in PullCompInterconnectTrigger
        Real64 refCOP;                // reference COP used for sizing reference power, user input
        Real64 sizFac;                // component level sizing factor, user input
        std::string companionName;    // name of companion GSHP
        int companionIndex;           // index in GSHP structure for companion heat pump
        bool companionIdentified;     // true if this GSHP has found its companion heat pump

        // Report variables
        Real64 reportPower;                  // Power Consumption [W]
        Real64 reportEnergy;                 // Energy Consumption [J]
        Real64 reportQLoad;                  // Load Side Heat Transfer Rate [W]
        Real64 reportQLoadEnergy;            // Load Side Heat Transfer [J]
        Real64 reportQSource;                // Source Side Heat Transfer Rate [W]
        Real64 reportQSourceEnergy;          // Source Side Heat Transfer [J]
        Real64 reportLoadSideMassFlowRate;   // Load side volumetric flow rate m3/s
        Real64 reportLoadSideInletTemp;      // Load Side outlet temperature degC
        Real64 reportLoadSideOutletTemp;     // Load Side outlet temperature degC
        Real64 reportSourceSideMassFlowRate; // Source side volumetric flow rate m3/s
        Real64 reportSourceSideInletTemp;    // Source Side outlet temperature degC
        Real64 reportSourceSideOutletTemp;   // Source Side outlet temperature degC

        // init flags
        bool MyPlantScanFlag;
        bool MyEnvrnFlag;

        bool myCoolingSizesReported;
        bool myHeatingSizesReported;

        // Default Constructor
        GshpSpecs()
            : checkEquipName(true), WWHPType(DataPlant::PlantEquipmentType::Invalid), Available(false), ON(false), IsOn(false), MustRun(false),
              SourceSideDesignMassFlow(0.0), LoadSideDesignMassFlow(0.0), RatedLoadVolFlowCool(0.0), ratedLoadVolFlowCoolWasAutoSized(false),
              RatedSourceVolFlowCool(0.0), ratedSourceVolFlowCoolWasAutoSized(false), RatedCapCool(0.0), ratedCapCoolWasAutoSized(false),
              RatedPowerCool(0.0), ratedPowerCoolWasAutoSized(false), CoolCapCurveIndex(0), CoolPowCurveIndex(0), CoolCapNegativeCounter(0),
              CoolCapNegativeIndex(0), CoolPowerNegativeCounter(0), CoolPowerNegativeIndex(0), RatedLoadVolFlowHeat(0.0),
              ratedLoadVolFlowHeatWasAutoSized(false), RatedSourceVolFlowHeat(0.0), ratedSourceVolFlowHeatWasAutoSized(false), RatedCapHeat(0.0),
              ratedCapHeatWasAutoSized(false), RatedPowerHeat(0.0), ratedPowerHeatWasAutoSized(false), HeatCapCurveIndex(0), HeatPowCurveIndex(0),
              LoadSideInletNodeNum(0), LoadSideOutletNodeNum(0), SourceSideInletNodeNum(0), SourceSideOutletNodeNum(0), HeatCapNegativeCounter(0),
              HeatCapNegativeIndex(0), HeatPowerNegativeCounter(0), HeatPowerNegativeIndex(0), SourcePlantLoc{}, LoadPlantLoc{}, CondMassFlowIndex(0),
              refCOP(0.0), sizFac(0.0), companionIndex(0), companionIdentified(false), reportPower(0.0), reportEnergy(0.0), reportQLoad(0.0),
              reportQLoadEnergy(0.0), reportQSource(0.0), reportQSourceEnergy(0.0), reportLoadSideMassFlowRate(0.0), reportLoadSideInletTemp(0.0),
              reportLoadSideOutletTemp(0.0), reportSourceSideMassFlowRate(0.0), reportSourceSideInletTemp(0.0), reportSourceSideOutletTemp(0.0),
              MyPlantScanFlag(true), MyEnvrnFlag(true), myCoolingSizesReported(false), myHeatingSizesReported(false)
        {
        }

        virtual ~GshpSpecs() = default;

        static GshpSpecs *factory(EnergyPlusData &state, DataPlant::PlantEquipmentType wwhp_type, std::string_view eir_wwhp_name);

        static void GetWatertoWaterHPInput(EnergyPlusData &state);

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        void getDesignCapacities(
            EnergyPlusData &state, const PlantLocation &calledFromLocation, Real64 &MaxLoad, Real64 &MinLoad, Real64 &OptLoad) override;

        void getSizingFactor(Real64 &sizingFactor) override;

        void InitWatertoWaterHP(EnergyPlusData &state,
                                DataPlant::PlantEquipmentType GSHPTypeNum, // Type of GSHP
                                std::string const &GSHPName,               // User Specified Name of GSHP
                                bool FirstHVACIteration,
                                Real64 MyLoad // Demand Load
        );

        void sizeCoolingWaterToWaterHP(EnergyPlusData &state);

        void sizeHeatingWaterToWaterHP(EnergyPlusData &state);

        void CalcWatertoWaterHPCooling(EnergyPlusData &state, Real64 MyLoad); // Operating Load

        void CalcWatertoWaterHPHeating(EnergyPlusData &state, Real64 MyLoad); // Operating Load

        void UpdateGSHPRecords(EnergyPlusData &state);

        void onInitLoopEquip(EnergyPlusData &state, const PlantLocation &calledFromLocation) override;

        void oneTimeInit(EnergyPlusData &state) override;

        void oneTimeInit_new(EnergyPlusData &state) override;
    };

} // namespace HeatPumpWaterToWaterSimple

struct HeatPumpWaterToWaterSimpleData : BaseGlobalStruct
{

    int NumGSHPs = 0;
    bool GetInputFlag = true;
    Array1D<HeatPumpWaterToWaterSimple::GshpSpecs> GSHP;
    std::unordered_map<std::string, std::string> HeatPumpWaterUniqueNames;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->NumGSHPs = 0;
        this->GetInputFlag = true;
        this->GSHP.deallocate();
        this->HeatPumpWaterUniqueNames.clear();
    }
};

} // namespace EnergyPlus

#endif
