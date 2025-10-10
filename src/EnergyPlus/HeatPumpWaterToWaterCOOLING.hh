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

#ifndef HeatPumpWaterToWaterCOOLING_hh_INCLUDED
#define HeatPumpWaterToWaterCOOLING_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantComponent.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace HeatPumpWaterToWaterCOOLING {

    struct GshpPeCoolingSpecs : PlantComponent // Needs Some Modifications talk with Dr.Fisher and decide....
    {
        // Members
        std::string Name; // user identifier
        DataPlant::PlantEquipmentType WWHPPlantTypeOfNum;

        Fluid::RefrigProps *refrig = nullptr;
        bool Available;                  // need an array of logicals--load identifiers of available equipment
        bool ON;                         // simulate the machine at it's operating part load ratio
        Real64 COP;                      // Coefficient of Performance of the machine
        Real64 NomCap;                   // Nominal Capacity of the HeatPump
        Real64 MinPartLoadRat;           // Minimum operating Part Load Ratio
        Real64 MaxPartLoadRat;           // Maximum operating Part Load Ratio
        Real64 OptPartLoadRat;           // Optimal operating Part Load Ratio
        Real64 LoadSideVolFlowRate;      // Design Flow Rate on the Load side
        Real64 LoadSideDesignMassFlow;   // Design flow rate (kg/s)
        Real64 SourceSideVolFlowRate;    // Design Flow Rate on th Source Side
        Real64 SourceSideDesignMassFlow; // Design flow rate (kg/s)
        int SourceSideInletNodeNum;      // Node number on the inlet side of the plant
        int SourceSideOutletNodeNum;     // Node number on the outlet side of the plant
        int LoadSideInletNodeNum;        // Node number on the inlet side of the Load Side
        int LoadSideOutletNodeNum;       // Node number on the outlet side of the Load Side
        Real64 SourceSideUACoeff;        // Source Side heat transfer coeff
        Real64 LoadSideUACoeff;          // Load Side heat transfer coeff
        Real64 CompPistonDisp;           // compressor piston displacement
        Real64 CompClearanceFactor;      // compressor clearance factor
        Real64 CompSucPressDrop;         // deltap ,  compressor suction and discharge pressure drop
        Real64 SuperheatTemp;            // deltatsh , super heating
        Real64 PowerLosses;              // constant part of electro mechanical power losses
        Real64 LossFactor;               // loss factor used ot define the electro mechanical loss
        //  that is supposed to be proportional to the theoretical power
        Real64 HighPressCutoff; // Maximum Design Pressure on the Load Side
        Real64 LowPressCutoff;  // Minimum Design Pressure on the Source Side
        bool IsOn;
        bool MustRun;
        // loop topology variables
        PlantLocation SourcePlantLoc; // source side plant loop component location
        PlantLocation LoadPlantLoc;   // load side plant loop component location
        int CondMassFlowIndex;        // index for criteria in PullCompInterconnectTrigger

        // Members
        Real64 Power;                     // Power Consumption Watts
        Real64 Energy;                    // Energy Consumption Joules
        Real64 QLoad;                     // Load Side heat transfer rate Watts
        Real64 QLoadEnergy;               // Load Side heat transfer Joules
        Real64 QSource;                   // Source Side heat transfer rate Watts
        Real64 QSourceEnergy;             // Source Side heat transfer Joules
        Real64 LoadSideWaterInletTemp;    // Load Side outlet temperature °C
        Real64 SourceSideWaterInletTemp;  // Source Side outlet temperature °C
        Real64 LoadSideWaterOutletTemp;   // Load Side outlet temperature °C
        Real64 SourceSideWaterOutletTemp; // Source Side outlet temperature °C
        int Running;                      // On reporting Flag

        Real64 LoadSideWaterMassFlowRate;
        Real64 SourceSideWaterMassFlowRate;

        bool plantScanFlag;
        bool beginEnvironFlag;

        // Default Constructor
        GshpPeCoolingSpecs()
            : WWHPPlantTypeOfNum(DataPlant::PlantEquipmentType::Invalid), Available(false), ON(false), COP(0.0), NomCap(0.0), MinPartLoadRat(0.0),
              MaxPartLoadRat(0.0), OptPartLoadRat(0.0), LoadSideVolFlowRate(0.0), LoadSideDesignMassFlow(0.0), SourceSideVolFlowRate(0.0),
              SourceSideDesignMassFlow(0.0), SourceSideInletNodeNum(0), SourceSideOutletNodeNum(0), LoadSideInletNodeNum(0), LoadSideOutletNodeNum(0),
              SourceSideUACoeff(0.0), LoadSideUACoeff(0.0), CompPistonDisp(0.0), CompClearanceFactor(0.0), CompSucPressDrop(0.0), SuperheatTemp(0.0),
              PowerLosses(0.0), LossFactor(0.0), HighPressCutoff(0.0), LowPressCutoff(0.0), IsOn(false), MustRun(false), SourcePlantLoc{},
              LoadPlantLoc{}, CondMassFlowIndex(0), Power(0.0), Energy(0.0), QLoad(0.0), QLoadEnergy(0.0), QSource(0.0), QSourceEnergy(0.0),
              LoadSideWaterInletTemp(0.0), SourceSideWaterInletTemp(0.0), LoadSideWaterOutletTemp(0.0), SourceSideWaterOutletTemp(0.0), Running(0),
              LoadSideWaterMassFlowRate(0.0), SourceSideWaterMassFlowRate(0.0), plantScanFlag(true), beginEnvironFlag(true)
        {
        }

        virtual ~GshpPeCoolingSpecs() = default;

        static GshpPeCoolingSpecs *factory(EnergyPlusData &state, const std::string &objectName);

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        void getDesignCapacities(
            EnergyPlusData &state, const PlantLocation &calledFromLocation, Real64 &MaxLoad, Real64 &MinLoad, Real64 &OptLoad) override;

        void onInitLoopEquip([[maybe_unused]] EnergyPlusData &state, [[maybe_unused]] const PlantLocation &calledFromLocation) override;

        void initialize(EnergyPlusData &state);

        void calculate(EnergyPlusData &state, Real64 &MyLoad);

        void update(EnergyPlusData &state);

        void oneTimeInit(EnergyPlusData &state) override;

        void oneTimeInit_new(EnergyPlusData &state) override;
    };

    void GetGshpInput(EnergyPlusData &state);

} // namespace HeatPumpWaterToWaterCOOLING

struct HeatPumpWaterToWaterCOOLINGData : BaseGlobalStruct
{

    int NumGSHPs = 0;
    bool GetWWHPCoolingInput = true;
    Array1D<HeatPumpWaterToWaterCOOLING::GshpPeCoolingSpecs> GSHP;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->NumGSHPs = 0;
        this->GetWWHPCoolingInput = true;
        this->GSHP.deallocate();
    }
};

} // namespace EnergyPlus

#endif
