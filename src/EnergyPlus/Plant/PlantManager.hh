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

#ifndef PlantManager_hh_INCLUDED
#define PlantManager_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/DataPlant.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace PlantManager {

    void ManagePlantLoops(EnergyPlusData &state,
                          bool FirstHVACIteration,
                          bool &SimAirLoops,         // True when the air loops need to be (re)simulated
                          bool &SimZoneEquipment,    // True when zone equipment components need to be (re)simulated
                          bool &SimNonZoneEquipment, // True when non-zone equipment components need to be (re)simulated
                          bool &SimPlantLoops,       // True when some part of Plant needs to be (re)simulated
                          bool &SimElecCircuits      // True when electric circuits need to be (re)simulated
    );

    void GetPlantLoopData(EnergyPlusData &state);

    void GetPlantInput(EnergyPlusData &state);

    void SetupReports(EnergyPlusData &state);

    void fillPlantCondenserTopology(EnergyPlusData &state, DataPlant::PlantLoopData &thisLoop, int &rowCounter);

    // void fillPlantToplogyRow(EnergyPlusData &state,
    //                          const std::string_view &compName,
    //                          const std::string_view &compType,
    //                          const std::string_view &side,
    //                          const std::string_view &parentType,
    //                          const std::string_view &parentName,
    //                          const std::string_view &fluidName,
    //                          const int reportOffset);

    void fillPlantToplogySplitterRow2(EnergyPlusData &state,
                                      const std::string_view &loopType,
                                      const std::string_view &loopName,
                                      const std::string_view &side,
                                      const std::string_view &splitterName,
                                      int &rowCounter);

    void fillPlantToplogyMixerRow2(EnergyPlusData &state,
                                   const std::string_view &loopType,
                                   const std::string_view &loopName,
                                   const std::string_view &side,
                                   const std::string_view &mixerName,
                                   int &rowCounter);

    void fillPlantToplogyComponentRow2(EnergyPlusData &state,
                                       const std::string_view &loopType,
                                       const std::string_view &loopName,
                                       const std::string_view &side,
                                       const std::string_view &branchName,
                                       const std::string_view &compType,
                                       const std::string_view &compName,
                                       int &rowCounter);

    void FillPlantEquipmentOperationLoad(EnergyPlusData &state);

    void InitializeLoops(EnergyPlusData &state,
                         bool FirstHVACIteration); // true if first iteration of the simulation

    void ReInitPlantLoopsAtFirstHVACIteration(EnergyPlusData &state);

    void UpdateNodeThermalHistory(EnergyPlusData const &state);

    void CheckPlantOnAbort(EnergyPlusData &state);

    void InitOneTimePlantSizingInfo(EnergyPlusData &state, int LoopNum); // loop being initialized for sizing

    void SizePlantLoop(EnergyPlusData &state,
                       int LoopNum, // Supply side loop being simulated
                       bool OkayToFinish);

    void ResizePlantLoopLevelSizes(EnergyPlusData &state, int LoopNum);

    void SetupInitialPlantCallingOrder(EnergyPlusData &state);

    void RevisePlantCallingOrder(EnergyPlusData &state);

    int FindLoopSideInCallingOrder(EnergyPlusData &state, int LoopNum, DataPlant::LoopSideLocation LoopSide);

    void SetupBranchControlTypes(EnergyPlusData &state);

    void CheckIfAnyPlant(EnergyPlusData &state);

    void CheckOngoingPlantWarnings(EnergyPlusData &state);

    struct EmptyPlantComponent : PlantComponent
    {
        // this is for purely air side equipment or similar that dont need anything at all done on plant for now
        // this could be a placeholder until those components are more integrated with plant side calcs.
        void simulate([[maybe_unused]] EnergyPlusData &state,
                      [[maybe_unused]] const PlantLocation &calledFromLocation,
                      [[maybe_unused]] bool FirstHVACIteration,
                      [[maybe_unused]] Real64 &CurLoad,
                      [[maybe_unused]] bool RunFlag) override
        {
            // this is empty on purpose
        }

        void oneTimeInit(EnergyPlusData &state) override;

        void oneTimeInit_new(EnergyPlusData &state) override;
    };

} // namespace PlantManager

struct PlantMgrData : BaseGlobalStruct
{

    bool GetCompSizFac = true;
    bool SupplyEnvrnFlag = true;
    bool MySetPointCheckFlag = true;
    Array1D_bool PlantLoopSetPointInitFlag;
    bool MyEnvrnFlag = true;
    int OtherLoopCallingIndex = 0;
    int OtherLoopDemandSideCallingIndex = 0;
    int NewOtherDemandSideCallingIndex = 0;
    int newCallingIndex = 0;
    PlantManager::EmptyPlantComponent dummyPlantComponent;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->GetCompSizFac = true;
        this->SupplyEnvrnFlag = true;
        this->MySetPointCheckFlag = true;
        this->PlantLoopSetPointInitFlag.clear();
        this->MyEnvrnFlag = true;
        this->OtherLoopCallingIndex = 0;
        this->OtherLoopDemandSideCallingIndex = 0;
        this->NewOtherDemandSideCallingIndex = 0;
        this->newCallingIndex = 0;
        this->dummyPlantComponent = {};
    }
};

} // namespace EnergyPlus

#endif
