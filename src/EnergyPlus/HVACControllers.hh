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

#ifndef HVACControllers_hh_INCLUDED
#define HVACControllers_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHVACControllers.hh>
#include <EnergyPlus/DataRootFinder.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/SetPointManager.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace HVACControllers {

    // Using/Aliasing
    using DataAirSystems::DefinePrimaryAirSystem;
    using DataHVACControllers::ControllerAction;
    using DataHVACControllers::ControllerMode;
    using DataHVACControllers::ControllerSimple_Type;
    using DataHVACControllers::iFirstMode;
    using DataHVACControllers::iLastMode;
    using DataRootFinder::RootFinderDataType;

    // Parameters for controls used here
    enum class CtrlVarType
    {
        Invalid = -1,
        NoControlVariable,
        Temperature,
        HumidityRatio,
        TemperatureAndHumidityRatio,
        Flow,
        Num
    };

    struct SolutionTrackerType
    {
        bool DefinedFlag = true;                    // Flag set to TRUE when tracker is up-to-date. FALSE otherwise.
        Real64 ActuatedValue = 0.0;                 // Actuated value
        ControllerMode Mode = ControllerMode::None; // Operational model of controller
    };

    struct ControllerPropsType
    {
        // Members
        std::string ControllerName; // Name of the Controller
        std::string ControllerType; // Type of Controller
        int ControllerType_Num;
        CtrlVarType ControlVar;  // The type of control variable being sensed
        CtrlVarType ActuatorVar; // The variable that the controller will act on ie. flow
        ControllerAction Action; // Controller Action - Reverse or Normal
        // Controller must be initialized to set MinActuated and MaxActuated
        bool InitFirstPass;
        // --------------------
        // Internal data used for optimal restart across successive calls to SimAirLoop()
        // --------------------
        int NumCalcCalls;    // Number of Calc() calls since last call to Reset()
        ControllerMode Mode; // Operational model of controller at current iteration
        // Flag indicating whether the current controller simulation was performed from a cold start
        // or following a speculative warm restart. Set in the ResetController() routine.
        // Used in the CheckController() routine.
        bool DoWarmRestartFlag;
        // Flag used to decide whether or not it is allowed to reuse the intermediate solution from
        // solving the previous controller on the air loop (COLD_START mode only) as the initial guess for
        // the current controller.
        bool ReuseIntermediateSolutionFlag;
        // Flag used to decide whether or not it is possible to reuse the solution from
        // the last call to SimAirLoop() as a possible candidate.
        bool ReusePreviousSolutionFlag;
        // Array of solution trackers. Saved at last call to SimAirLoop() in ManageControllers(iControllerOpEnd)
        // The first tracker is used to track the solution when FirstHVACIteration is TRUE.
        // The second tracker is used to track the solution at FirstHVACIteration is FALSE.
        Array1D<SolutionTrackerType> SolutionTrackers;
        // --------------------
        // Operational limits at min/max avail values for actuated variable and the corresponding sensed values
        // --------------------
        Real64 MaxAvailActuated; // kg/s, The maximum actuated variable currently available.
        // Reset by simulation at each HVAC iteration
        Real64 MaxAvailSensed;   // Sensed value at maximum available actuated variable
        Real64 MinAvailActuated; // kg/s, The minimum actuated variable currently available.
        // Reset by simulation at each HVAC iteration
        Real64 MinAvailSensed; // Sensed value at maximum available actuated variable
        // --------------------
        // User input min/max values for actuated variable
        // --------------------
        Real64 MaxVolFlowActuated; // m3/s, From User input the Max amount for the actuated variable
        Real64 MinVolFlowActuated; // m3/s, From User input the Min amount for the actuated variable
        Real64 MaxActuated;        // kg/s, From User input the Max amount for the actuated variable
        Real64 MinActuated;        // kg/s, From User input the Min amount for the actuated variable
        // --------------------
        // Actuated variable
        // --------------------
        int ActuatedNode;                   // The node that is acted upon by the controller
        Real64 ActuatedValue;               // Value of actuated variable before change by the controller
        Real64 NextActuatedValue;           // The new control actuated value
        PlantLocation ActuatedNodePlantLoc; // Location for actuated node
        DataPlant::PlantEquipmentType WaterCoilType = DataPlant::PlantEquipmentType::Invalid;
        // --------------------
        // Sensed variable
        // --------------------
        int SensedNode;             // The sensed node number from the grid
        bool IsSetPointDefinedFlag; // If TRUE indicates that the setpoint has been defined and can
        // be used to compute DeltaSensed
        Real64 SetPointValue;              // Desired setpoint; set in the SetPoint Manager or computed in Init() routine
        Real64 SensedValue;                // The sensed control variable of any type
        Real64 DeltaSensed;                // Difference of sensed to setpoint value for calculating proportional gain
        Real64 Offset;                     // This is the tolerance or droop from the error
        HVAC::CtrlVarType HumRatCntrlType; // iCtrlVarType_HumRat=4,iCtrlVarType_MaxHumRat=5,iCtrlVarType_MinHumRat=6
        // --------------------
        // Other controller inputs, not yet used
        // --------------------
        std::string LimitType; // Limit type as in HIGH or LOW
        Real64 Range;          // The range or hysteresis of the control limit
        Real64 Limit;          // The Limit value for a Limit Controller
        // --------------------
        // Trace mechanism
        // --------------------
        SharedFileHandle TraceFile;
        bool FirstTraceFlag;   // To detect first individual write operation to individual controller trace file
        int BadActionErrCount; // Counts number of incorrect action errors
        int BadActionErrIndex; // index to recurring error structure for bad action error
        // Fault model for water coil supply air temperature sensor offset
        bool FaultyCoilSATFlag;     // True if the coil has SAT sensor fault
        int FaultyCoilSATIndex;     // Index of the fault object corresponding to the coil
        Real64 FaultyCoilSATOffset; // Coil SAT sensor offset
        bool BypassControllerCalc;  // set true for OA sys water coils
        int AirLoopControllerIndex; // index to controller on specific air loop

        bool HumRatCtrlOverride; // true if TemperatureAndHumidityRatio control switches to humidity ratio control

        // Default Constructor
        ControllerPropsType()
            : ControllerType_Num(ControllerSimple_Type), ControlVar(CtrlVarType::NoControlVariable), ActuatorVar(CtrlVarType::NoControlVariable),
              Action(ControllerAction::NoAction), InitFirstPass(true), NumCalcCalls(0), Mode(ControllerMode::None), DoWarmRestartFlag(false),
              ReuseIntermediateSolutionFlag(false), ReusePreviousSolutionFlag(false), SolutionTrackers(2), MaxAvailActuated(0.0), MaxAvailSensed(0.0),
              MinAvailActuated(0.0), MinAvailSensed(0.0), MaxVolFlowActuated(0.0), MinVolFlowActuated(0.0), MaxActuated(0.0), MinActuated(0.0),
              ActuatedNode(0), ActuatedValue(0.0), NextActuatedValue(0.0), ActuatedNodePlantLoc{}, SensedNode(0), IsSetPointDefinedFlag(false),
              SetPointValue(0.0), SensedValue(0.0), DeltaSensed(0.0), Offset(0.0), HumRatCntrlType(HVAC::CtrlVarType::Invalid), Range(0.0),
              Limit(0.0), FirstTraceFlag(true), BadActionErrCount(0), BadActionErrIndex(0), FaultyCoilSATFlag(false), FaultyCoilSATIndex(0),
              FaultyCoilSATOffset(0.0), BypassControllerCalc(false), AirLoopControllerIndex(0), HumRatCtrlOverride(false)
        {
        }
    };

    struct ControllerStatsType
    {
        // Members
        Array1D_int NumCalls;      // Number of times this controller operated in each mode
        Array1D_int TotIterations; // Total number of iterations required to solve this controller
        Array1D_int MaxIterations; // Maximum number of iterations required to solve this controller

        // Default Constructor
        ControllerStatsType()
            : NumCalls({iFirstMode, iLastMode}, 0), TotIterations({iFirstMode, iLastMode}, 0), MaxIterations({iFirstMode, iLastMode}, 0)
        {
        }
    };

    struct AirLoopStatsType
    {
        // Shared_ptr because we need to put this into an Array1D which is not friendly with move-only types
        SharedFileHandle TraceFile;
        // Used only if > 0. Same size as NumPrimaryAirSys
        bool FirstTraceFlag = true;                   // To detect first trace to air loop trace file
        int NumCalls = 0;                             // Number of times air loop is simulated (number of calls to SimAirLoop)
        int NumFailedWarmRestarts = 0;                // Number of times speculative warm restart was attempted and failed
        int NumSuccessfulWarmRestarts = 0;            // Number of times speculative warm restart was attempted and succeeded
        int TotSimAirLoopComponents = 0;              // Total number of times the SimAirLoopComponents() routine has been invoked
        int MaxSimAirLoopComponents = 0;              // Maximum number of times the SimAirLoopComponents() routine has been invoked
        int TotIterations = 0;                        // Total number of iterations required to solve the controllers on this air loop
        int MaxIterations = 0;                        // Maximum number of iterations required to solve the controllers on this air loop
        Array1D<ControllerStatsType> ControllerStats; // Array of statistics for each controller on this air loop
    };

    void ManageControllers(EnergyPlusData &state,
                           std::string const &ControllerName,
                           int &ControllerIndex,
                           bool FirstHVACIteration,
                           int AirLoopNum, // unused1208
                           DataHVACControllers::ControllerOperation Operation,
                           bool &IsConvergedFlag,
                           bool &IsUpToDateFlag,
                           bool const BypassOAController,
                           ObjexxFCL::Optional_bool AllowWarmRestartFlag = _);

    void GetControllerInput(EnergyPlusData &state);

    void ResetController(EnergyPlusData &state, int ControlNum, bool DoWarmRestartFlag, bool &IsConvergedFlag);

    void InitController(EnergyPlusData &state, int ControlNum, bool &IsConvergedFlag);

    void SizeController(EnergyPlusData &state, int ControlNum);

    void CalcSimpleController(EnergyPlusData &state,
                              int ControlNum,
                              bool FirstHVACIteration,
                              bool &IsConvergedFlag,
                              bool &IsUpToDateFlag,
                              std::string const &ControllerName // used when errors occur
    );

    void FindRootSimpleController(EnergyPlusData &state,
                                  int ControlNum,
                                  bool FirstHVACIteration,
                                  bool &IsConvergedFlag,
                                  bool &IsUpToDateFlag,
                                  std::string const &ControllerName // used when errors occur
    );

    void CheckSimpleController(EnergyPlusData &state, int ControlNum, bool &IsConvergedFlag);

    bool CheckMinActiveController(EnergyPlusData &state, int ControlNum);

    bool CheckMaxActiveController(EnergyPlusData &state, int ControlNum);

    void CheckTempAndHumRatCtrl(EnergyPlusData &state, int ControlNum, bool &IsConvergedFlag);

    void SaveSimpleController(EnergyPlusData &state, int ControlNum, bool FirstHVACIteration, bool IsConvergedFlag);

    void UpdateController(EnergyPlusData &state, int ControlNum);

    void ExitCalcController(
        EnergyPlusData &state, int ControlNum, Real64 NextActuatedValue, ControllerMode Mode, bool &IsConvergedFlag, bool &IsUpToDateFlag);

    void TrackAirLoopControllers(EnergyPlusData &state,
                                 int AirLoopNum,
                                 DataHVACControllers::ControllerWarmRestart WarmRestartStatus,
                                 int AirLoopIterMax,
                                 int AirLoopIterTot,
                                 int AirLoopNumCalls);

    void TrackAirLoopController(EnergyPlusData &state,
                                int AirLoopNum,       // Air loop index
                                int AirLoopControlNum // Controller index on this air loop
    );

    void DumpAirLoopStatistics(EnergyPlusData &state);

    void WriteAirLoopStatistics(EnergyPlusData &state,
                                InputOutputFile &statisticsFile,
                                DefinePrimaryAirSystem const &ThisPrimaryAirSystem,
                                AirLoopStatsType const &ThisAirLoopStats);

    void SetupAirLoopControllersTracer(EnergyPlusData &state, int AirLoopNum);

    void TraceAirLoopControllers(
        EnergyPlusData &state, bool FirstHVACIteration, int AirLoopNum, int AirLoopPass, bool AirLoopConverged, int AirLoopNumCalls);

    void TraceIterationStamp(
        EnergyPlusData &state, InputOutputFile &TraceFile, bool FirstHVACIteration, int AirLoopPass, bool AirLoopConverged, int AirLoopNumCalls);

    void TraceAirLoopController(EnergyPlusData &state, InputOutputFile &TraceFile, int ControlNum);

    void SetupIndividualControllerTracer(EnergyPlusData &state, int ControlNum);

    void TraceIndividualController(EnergyPlusData &state,
                                   int ControlNum,
                                   bool FirstHVACIteration,
                                   int AirLoopPass,
                                   DataHVACControllers::ControllerOperation Operation, // Operation to execute
                                   bool IsConvergedFlag);

    Real64 GetCurrentHVACTime(const EnergyPlusData &state);

    Real64 GetPreviousHVACTime(const EnergyPlusData &state);

    std::string CreateHVACTimeString(const EnergyPlusData &state);

    std::string CreateHVACStepFullString(const EnergyPlusData &state);

    std::string MakeHVACTimeIntervalString(const EnergyPlusData &state);

    void CheckControllerListOrder(EnergyPlusData &state);

    void CheckCoilWaterInletNode(EnergyPlusData &state,
                                 int WaterInletNodeNum, // input actuator node number
                                 bool &NodeNotFound     // true if matching actuator node not found, false if found
    );

    void GetControllerNameAndIndex(EnergyPlusData &state,
                                   int WaterInletNodeNum,       // input actuator node number
                                   std::string &ControllerName, // controller name used by water coil
                                   int &ControllerIndex,        // controller index used by water coil
                                   bool &ErrorsFound            // true if matching actuator node not found
    );

    void GetControllerActuatorNodeNum(EnergyPlusData &state,
                                      std::string const &ControllerName, // name of coil controller
                                      int &WaterInletNodeNum,            // input actuator node number
                                      bool &NodeNotFound                 // true if matching actuator node not found, false if found
    );

    int GetControllerIndex(EnergyPlusData &state,
                           std::string const &ControllerName // name of coil controller
    );

} // namespace HVACControllers

struct HVACControllersData : BaseGlobalStruct
{
    int NumControllers = 0;  // The number of controllers found in the Input
    int NumAirLoopStats = 0; // Same size as NumPrimaryAirSys if controllers
    Array1D_bool CheckEquipName;
    bool GetControllerInputFlag = true;
    bool InitControllerOneTimeFlag = true;
    bool InitControllerSetPointCheckFlag = true;
    Array1D<HVACControllers::ControllerPropsType> ControllerProps;
    Array1D<HVACControllers::RootFinderDataType> RootFinders;
    Array1D<HVACControllers::AirLoopStatsType> AirLoopStats; // Statistics array to analyze computational profile for
    Array1D_bool MyEnvrnFlag;
    Array1D_bool MySizeFlag;
    Array1D_bool MyPlantIndexsFlag;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        new (this) HVACControllersData();
    }
};

} // namespace EnergyPlus

#endif
