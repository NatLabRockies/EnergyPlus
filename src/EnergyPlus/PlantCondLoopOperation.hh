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

#ifndef PlantCondLoopOperation_hh_INCLUDED
#define PlantCondLoopOperation_hh_INCLUDED

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/DataPlant.hh>

namespace EnergyPlus {

namespace PlantCondLoopOperation {

    void ManagePlantLoadDistribution(EnergyPlusData &state,
                                     PlantLocation const &plantLoc, // PlantLoop data structure Location struct
                                     Real64 const LoopDemand,
                                     Real64 &RemLoopDemand,
                                     bool const FirstHVACIteration,
                                     bool &LoopShutDownFlag, // EMS flag to tell loop solver to shut down pumps
                                     bool &LoadDistributionWasPerformed);

    void GetPlantOperationInput(EnergyPlusData &state, bool &GetInputOK);

    void GetOperationSchemeInput(EnergyPlusData &state);

    void FindRangeBasedOrUncontrolledInput(EnergyPlusData &state,
                                           std::string &CurrentModuleObject, // for ease in renaming
                                           int const NumSchemes,             // May be set here and passed on
                                           int const LoopNum,                // May be set here and passed on
                                           int const SchemeNum,              // May be set here and passed on
                                           bool &ErrorsFound                 // May be set here and passed on
    );

    void FindDeltaTempRangeInput(EnergyPlusData &state,
                                 DataLoopNode::ConnectionObjectType const CurrentModuleObject, // for ease in renaming
                                 int const NumSchemes,                                         // May be set here and passed on
                                 int const LoopNum,                                            // May be set here and passed on
                                 int const SchemeNum,                                          // May be set here and passed on
                                 bool &ErrorsFound                                             // May be set here and passed on
    );

    void LoadEquipList(EnergyPlusData &state,
                       int const LoopNum,   // May be set here and passed on
                       int const SchemeNum, // May be set here and passed on
                       int const ListNum,   // May be set here and passed on
                       bool &ErrorsFound    // May be set here and passed on
    );

    void FindCompSPInput(EnergyPlusData &state,
                         std::string &CurrentModuleObject, // for ease in renaming
                         int const NumSchemes,             // May be set here and passed on
                         int const LoopNum,                // May be set here and passed on
                         int const SchemeNum,              // May be set here and passed on
                         bool &ErrorsFound                 // May be set here and passed on
    );

    void GetChillerHeaterChangeoverOpSchemeInput(EnergyPlusData &state,
                                                 std::string &CurrentModuleObject, // for ease in renaming
                                                 int const NumSchemes,             // may be set here and passed on
                                                 bool &ErrorsFound                 // may be set here and passed on
    );

    void GetUserDefinedOpSchemeInput(EnergyPlusData &state,
                                     std::string &CurrentModuleObject, // for ease in renaming
                                     int const NumSchemes,             // May be set here and passed on
                                     int const LoopNum,                // May be set here and passed on
                                     int const SchemeNum,              // May be set here and passed on
                                     bool &ErrorsFound                 // May be set here and passed on
    );

    void InitLoadDistribution(EnergyPlusData &state, bool const FirstHVACIteration);

    void DistributePlantLoad(EnergyPlusData &state,
                             int const LoopNum,
                             const DataPlant::LoopSideLocation LoopSideNum,
                             int const CurSchemePtr, // use as index in PlantLoop()OpScheme() data structure
                             int const ListPtr,      // use as index in PlantLoop()OpScheme() data structure
                             Real64 const LoopDemand,
                             Real64 &RemLoopDemand);

    void AdjustChangeInLoadForLastStageUpperRangeLimit(EnergyPlusData &state,
                                                       int const LoopNum,         // component topology
                                                       int const CurOpSchemePtr,  // current active operation scheme
                                                       int const CurEquipListPtr, // current equipment list
                                                       Real64 &ChangeInLoad       // positive magnitude of load change
    );

    void AdjustChangeInLoadByHowServed(EnergyPlusData &state,
                                       PlantLocation const &plantLoc, // component topology
                                       Real64 &ChangeInLoad           // positive magnitude of load change
    );

    void FindCompSPLoad(EnergyPlusData &state,
                        PlantLocation const &plantLoc,
                        int const OpNum // index for Plant()%LoopSide()%Branch()%Comp()%OpScheme()
    );

    void DistributeUserDefinedPlantLoad(EnergyPlusData &state,
                                        PlantLocation const &plantLoc,
                                        int const CurCompLevelOpNum, // index for Plant()%LoopSide()%Branch()%Comp()%OpScheme()
                                        int const CurSchemePtr,
                                        Real64 const LoopDemand,
                                        Real64 &RemLoopDemand);

    Real64 FindRangeVariable(EnergyPlusData &state,
                             int const LoopNum,                // PlantLoop data structure loop counter
                             int const CurSchemePtr,           // set by PL()%LoopSide()%Branch()%Comp()%OpScheme()%OpSchemePtr
                             DataPlant::OpScheme CurSchemeType // identifier set in PlantData
    );

    void TurnOnPlantLoopPipes(EnergyPlusData &state, int const LoopNum, const DataPlant::LoopSideLocation LoopSideNum);

    void TurnOffLoopEquipment(EnergyPlusData &state, int const LoopNum);

    void TurnOffLoopSideEquipment(EnergyPlusData &state, int const LoopNum, const DataPlant::LoopSideLocation LoopSideNum);

    void SetupPlantEMSActuators(EnergyPlusData &state);

    void ActivateEMSControls(EnergyPlusData &state, PlantLocation const &plantLoc, bool &LoopShutDownFlag);

    void AdjustChangeInLoadByEMSControls(EnergyPlusData &state,
                                         PlantLocation const &plantLoc,
                                         Real64 &ChangeInLoad // positive magnitude of load change
    );

} // namespace PlantCondLoopOperation

struct PlantCondLoopOperationData : BaseGlobalStruct
{

    bool GetPlantOpInput = true;
    bool InitLoadDistributionOneTimeFlag = true;
    bool LoadEquipListOneTimeFlag = true;
    int TotNumLists = 0;
    Array1D_string EquipListsNameList;
    Array1D<DataPlant::LoopType> EquipListsTypeList;
    Array1D_int EquipListsIndexList;
    bool lDummy = false; // for User-defined component load dispatch
    bool LoadSupervisoryChillerHeaterOpScheme = true;
    Array1D<DataPlant::ChillerHeaterSupervisoryOperationData> ChillerHeaterSupervisoryOperationSchemes;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->GetPlantOpInput = true;
        this->InitLoadDistributionOneTimeFlag = true;
        this->LoadEquipListOneTimeFlag = true;
        this->LoadSupervisoryChillerHeaterOpScheme = true;
        this->ChillerHeaterSupervisoryOperationSchemes.clear();
        this->TotNumLists = 0;
        this->EquipListsNameList.clear();
        this->EquipListsTypeList.clear();
        this->EquipListsIndexList.clear();
        this->lDummy = false;
    }
};

} // namespace EnergyPlus

#endif
