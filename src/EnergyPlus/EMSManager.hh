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

#ifndef EMSManager_hh_INCLUDED
#define EMSManager_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/OutputProcessor.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

// note there are routines that lie outside of the Module at the end of this file

namespace EMSManager {

    // Parameters for EMS Calling Points
    enum class EMSCallFrom
    {
        Invalid = -1,
        ZoneSizing,
        SystemSizing,
        BeginNewEnvironment,
        BeginNewEnvironmentAfterWarmUp,
        BeginTimestepBeforePredictor,
        BeforeHVACManagers,
        AfterHVACManagers,
        HVACIterationLoop,
        EndSystemTimestepBeforeHVACReporting,
        EndSystemTimestepAfterHVACReporting,
        EndZoneTimestepBeforeZoneReporting,
        EndZoneTimestepAfterZoneReporting,
        SetupSimulation,
        ExternalInterface,
        ComponentGetInput,
        UserDefinedComponentModel,
        UnitarySystemSizing,
        BeginZoneTimestepBeforeInitHeatBalance,
        BeginZoneTimestepAfterInitHeatBalance,
        BeginZoneTimestepBeforeSetCurrentWeather,
        Num
    };

    void CheckIfAnyEMS(EnergyPlusData &state);

    void ManageEMS(EnergyPlusData &state,
                   EMSCallFrom iCalledFrom,                              // indicates where subroutine was called from, parameters in DataGlobals.
                   bool &anyProgramRan,                                  // true if any Erl programs ran for this call
                   ObjexxFCL::Optional_int_const ProgramManagerToRun = _ // specific program manager to run
    );

    void InitEMS(EnergyPlusData &state, EMSCallFrom iCalledFrom); // indicates where subroutine was called from, parameters in DataGlobals.

    void ReportEMS(EnergyPlusData &state);

    void GetEMSInput(EnergyPlusData &state);

    void ProcessEMSInput(EnergyPlusData &state, bool reportErrors); // .  If true, then report out errors ,otherwise setup what we can

    void GetVariableTypeAndIndex(
        EnergyPlusData &state, std::string const &VarName, std::string const &VarKeyName, OutputProcessor::VariableType &VarType, int &VarIndex);

    void EchoOutActuatorKeyChoices(EnergyPlusData &state);

    void EchoOutInternalVariableChoices(EnergyPlusData &state);

    void SetupNodeSetPointsAsActuators(EnergyPlusData &state);

    void UpdateEMSTrendVariables(EnergyPlusData &state);

    bool CheckIfNodeSetPointManaged(EnergyPlusData &state,
                                    int NodeNum, // index of node being checked.
                                    HVAC::CtrlVarType SetPointType,
                                    bool byHandle = false);

    bool CheckIfNodeSetPointManagedByEMS(EnergyPlusData &state,
                                         int NodeNum, // index of node being checked.
                                         HVAC::CtrlVarType SetPointType,
                                         bool &ErrorFlag);

    bool CheckIfNodeMoreInfoSensedByEMS(EnergyPlusData &state,
                                        int nodeNum, // index of node being checked.
                                        std::string const &varName);

    bool isScheduleManaged(EnergyPlusData &state, Sched::Schedule *const sched);

    void SetupPrimaryAirSystemAvailMgrAsActuators(EnergyPlusData &state);

    void SetupWindowShadingControlActuators(EnergyPlusData &state);

    void SetupThermostatActuators(EnergyPlusData &state);

    void SetupSurfaceConvectionActuators(EnergyPlusData &state);

    void SetupSurfaceConstructionActuators(EnergyPlusData &state);

    void SetupSurfaceOutdoorBoundaryConditionActuators(EnergyPlusData &state);

    void SetupZoneOutdoorBoundaryConditionActuators(EnergyPlusData &state);

    void SetupZoneInfoAsInternalDataAvail(EnergyPlusData &state);

    void checkForUnusedActuatorsAtEnd(EnergyPlusData &state);

    void checkSetpointNodesAtEnd(EnergyPlusData &state);

} // namespace EMSManager

// Moved these setup EMS actuator routines out of module to solve circular use problems between
//  ScheduleManager and OutputProcessor. Followed pattern used for SetupOutputVariable

void SetupEMSActuator(EnergyPlusData &state,
                      std::string_view cComponentTypeName,
                      std::string_view cUniqueIDName,
                      std::string_view cControlTypeName,
                      std::string_view cUnits,
                      bool &lEMSActuated,
                      Real64 &rValue);

void SetupEMSActuator(EnergyPlusData &state,
                      std::string_view cComponentTypeName,
                      std::string_view cUniqueIDName,
                      std::string_view cControlTypeName,
                      std::string_view cUnits,
                      bool &lEMSActuated,
                      int &iValue);

void SetupEMSActuator(EnergyPlusData &state,
                      std::string_view cComponentTypeName,
                      std::string_view cUniqueIDName,
                      std::string_view cControlTypeName,
                      std::string_view cUnits,
                      bool &lEMSActuated,
                      bool &lValue);

void SetupEMSInternalVariable(
    EnergyPlusData &state, std::string_view cDataTypeName, std::string_view cUniqueIDName, std::string_view cUnits, Real64 &rValue);

void SetupEMSInternalVariable(
    EnergyPlusData &state, std::string_view cDataTypeName, std::string_view cUniqueIDName, std::string_view cUnits, int &iValue);

struct EMSManagerData : BaseGlobalStruct
{

    bool GetEMSUserInput = true; // Flag to prevent input from being read multiple times
    bool ZoneThermostatActuatorsHaveBeenSetup = false;
    bool FinishProcessingUserInput = true; // Flag to indicate still need to process input
    bool lDummy = false;                   // dummy pointer location
    bool lDummy2 = false;                  // dummy pointer location

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
        EMSManager::CheckIfAnyEMS(state);
    }

    void clear_state() override
    {
        GetEMSUserInput = true;
        ZoneThermostatActuatorsHaveBeenSetup = false;
        FinishProcessingUserInput = true;
        this->lDummy = false;
        this->lDummy2 = false;
    }
};

} // namespace EnergyPlus

#endif
