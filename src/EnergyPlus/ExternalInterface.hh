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

#ifndef ExternalInterface_hh_INCLUDED
#define ExternalInterface_hh_INCLUDED

// FMI-Related Headers
extern "C" {
#include <FMI/main.h>
}

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/ExternalInterface.hh>
#include <EnergyPlus/FileSystem.hh>
#include <EnergyPlus/OutputProcessor.hh>

// C++ Standard Library Headers
#include <string>

// Objexx Headers
#include <ObjexxFCL/Array1D.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace ExternalInterface {

    // MODULE PARAMETER DEFINITIONS:
    int constexpr maxVar(100000);         // Maximum number of variables to be exchanged
    int constexpr maxErrMsgLength(10000); // Maximum error message length from xml schema validation

    // can't change these to enum class since these are used in getepvariables() in utilXml.c as arguments
    int constexpr indexSchedule(1); // Index for schedule in inpVarTypes
    int constexpr indexVariable(2); // Index for variable in inpVarTypes
    int constexpr indexActuator(3); // Index for actuator in inpVarTypes

    // can't change these to enum class since these are compared to fmistatus defined in hmiModelFunctions.h
    int constexpr fmiOK(0);      // fmiOK
    int constexpr fmiWarning(1); // fmiWarning
    int constexpr fmiDiscard(2); // fmiDiscard
    int constexpr fmiError(3);   // fmiError
    int constexpr fmiFatal(4);   // fmiPending
    int constexpr fmiPending(5); // fmiPending

    struct fmuInputVariableType
    {

        std::string Name;   // Name of FMU input variable
        int ValueReference; // = fmiValueReference specific to FMU variable

        // Default Constructor
        fmuInputVariableType() : Name(std::string()), ValueReference(0)
        {
        }
    };

    struct checkFMUInstanceNameType
    {

        std::string Name; // Name of fmu instance

        // Default Constructor
        checkFMUInstanceNameType() : Name(std::string())
        {
        }
    };

    struct eplusOutputVariableType
    {

        std::string Name;                      // Variable name in EnergyPlus
        std::string VarKey;                    // Key value in EnergyPlus
        Real64 RTSValue;                       // Real value of variable at the Zone Time Step
        int ITSValue;                          // Integer value of variable at the Zone Time Step
        int VarIndex;                          // Index Value of variable
        OutputProcessor::VariableType VarType; // Type of variable at the Zone Time Step
        std::string VarUnits;                  // Units string, may be blank

        // Default Constructor
        eplusOutputVariableType()
            : Name(std::string()), VarKey(std::string()), RTSValue(0.0), ITSValue(0), VarIndex(0), VarType(OutputProcessor::VariableType::Invalid),
              VarUnits(std::string())
        {
        }
    };

    struct fmuOutputVariableScheduleType
    {

        std::string Name;    // Name of fmu output variable --> schedule in energyplus
        Real64 RealVarValue; // = Real value at the Zone Time Step
        int ValueReference;  // = fmiValueReference specific to FMU variable

        // Default Constructor
        fmuOutputVariableScheduleType() : Name(std::string()), RealVarValue(0.0), ValueReference(0)
        {
        }
    };

    struct fmuOutputVariableVariableType
    {

        std::string Name;    // Name of fmu output variable --> variable in energyplus
        Real64 RealVarValue; // = Real value at the Zone Time Step
        int ValueReference;  // = fmiValueReference specific to FMU variable

        // Default Constructor
        fmuOutputVariableVariableType() : Name(std::string()), RealVarValue(0.0), ValueReference(0)
        {
        }
    };

    struct fmuOutputVariableActuatorType
    {

        std::string Name;    // Name of fmu output variable --> actuator in energyplus
        Real64 RealVarValue; // = Real value at the Zone Time Step
        int ValueReference;  // = fmiValueReference specific to FMU variable

        // Default Constructor
        fmuOutputVariableActuatorType() : Name(std::string()), RealVarValue(0.0), ValueReference(0)
        {
        }
    };

    struct eplusInputVariableScheduleType
    {

        std::string Name; // Name of energyplus input variable from Type schedule
        int VarIndex;     // Index Value of this variable
        int InitialValue; // Initial value used during the warmup

        // Default Constructor
        eplusInputVariableScheduleType() : Name(std::string()), VarIndex(0)
        {
            // InitialValue not initialized in default constructor
        }
    };

    struct eplusInputVariableVariableType
    {

        std::string Name; // Name of energyplus input variable from Type variable
        int VarIndex;     // Index Value of this variable

        // Default Constructor
        eplusInputVariableVariableType() : Name(std::string()), VarIndex(0)
        {
        }
    };

    struct eplusInputVariableActuatorType
    {

        std::string Name; // Name of energyplus input variable from Type actuator
        int VarIndex;     // Index Value of this variable

        // Default Constructor
        eplusInputVariableActuatorType() : Name(std::string()), VarIndex(0)
        {
        }
    };

    struct InstanceType
    {

        std::string Name;               // FMU Filename
        std::string modelID;            // FMU modelID
        std::string modelGUID;          // FMU modelGUID
        fs::path WorkingFolder;         // Path to the FMU working folder
        fs::path WorkingFolder_wLib;    // Path to the binaries
        std::string fmiVersionNumber;   // Version number of FMI used
        int NumInputVariablesInFMU;     // Number of input variables in fmu
        int NumInputVariablesInIDF;     // Number of fmus input variables in idf
        int NumOutputVariablesInFMU;    // Number of output variables in fmu
        int NumOutputVariablesInIDF;    // Number of output variables in idf
        int NumOutputVariablesSchedule; // Number of output variables from type schedule
        int NumOutputVariablesVariable; // Number of output variables from type variable
        int NumOutputVariablesActuator; // Number of output variables from type actuator
        int LenModelID;                 // Length of modelID trimmed
        int LenModelGUID;               // Length of modelGUID trimmed
        int LenWorkingFolder;           // Length of working folder trimmed
        int LenWorkingFolder_wLib;      // Length of working folder with libraries trimmed
        fmiComponent fmicomponent;      // FMU instance
        fmiStatus fmistatus;            // Status of fmi
        int Index;                      // Index of FMU
        // Variable Types structure for fmu input variables
        Array1D<fmuInputVariableType> fmuInputVariable;
        // Variable Types structure for checking duplicates fmu input variables
        Array1D<fmuInputVariableType> checkfmuInputVariable;
        // Variable Types structure for energyplus output variables
        Array1D<eplusOutputVariableType> eplusOutputVariable;
        // Variable Types structure for fmu output variables from type schedule
        Array1D<fmuOutputVariableScheduleType> fmuOutputVariableSchedule;
        // Variable Types structure for energyplus input variables from type schedule
        Array1D<eplusInputVariableScheduleType> eplusInputVariableSchedule;
        // Variable Types structure for fmu output variables from type variable
        Array1D<fmuOutputVariableVariableType> fmuOutputVariableVariable;
        // Variable Types structure for energyplus input variables from type variable
        Array1D<eplusInputVariableVariableType> eplusInputVariableVariable;
        // Variable Types structure for fmu output variables from type actuator
        Array1D<fmuOutputVariableActuatorType> fmuOutputVariableActuator;
        // Variable Types structure for energyplus input variables from type actuator
        Array1D<eplusInputVariableActuatorType> eplusInputVariableActuator;

        // Default Constructor
        InstanceType()
            : Name(std::string()), modelID(std::string()), modelGUID(std::string()), WorkingFolder(fs::path()), WorkingFolder_wLib(fs::path()),
              fmiVersionNumber(std::string()), NumInputVariablesInFMU(0), NumInputVariablesInIDF(0), NumOutputVariablesInFMU(0),
              NumOutputVariablesInIDF(0), NumOutputVariablesSchedule(0), NumOutputVariablesVariable(0), NumOutputVariablesActuator(0), LenModelID(0),
              LenModelGUID(0), LenWorkingFolder(0), LenWorkingFolder_wLib(0)
        {
            // fmiStatus, Index, and arrays not initialized in default constructor
        }
    };

    struct FMUType
    {

        std::string Name;                  // FMU Filename
        Real64 TimeOut;                    // Default TimeOut value
        int Visible;                       // Default Visible value
        int Interactive;                   // Default Interactive value
        int LoggingOn;                     // Default LoggingOn value
        int NumInstances;                  // Number of Instances
        int TotNumInputVariablesInIDF;     // Number of input variables
        int TotNumOutputVariablesSchedule; // Number of output variables from type schedule
        int TotNumOutputVariablesVariable; // Number of output variables from type variable
        int TotNumOutputVariablesActuator; // Number of output variables from type actuator
        Array1D<InstanceType> Instance;    // Variable Types structure for energyplus input variables from type actuator

        // Default Constructor
        FMUType()
            : Name(std::string()), TimeOut(0.0), Visible(0), Interactive(0), LoggingOn(0), NumInstances(0), TotNumInputVariablesInIDF(0),
              TotNumOutputVariablesSchedule(0), TotNumOutputVariablesVariable(0), TotNumOutputVariablesActuator(0)
        {
            // Instance not instantiated in default constructor
        }
    };

    // Functions

    void ExternalInterfaceExchangeVariables(EnergyPlusData &state);

    void CloseSocket(EnergyPlusData &state, int FlagToWriteToSocket);

    void InitExternalInterface(EnergyPlusData &state);

    void GetExternalInterfaceInput(EnergyPlusData &state);

    void CalcExternalInterface(EnergyPlusData &state);

    void ParseString(std::string const &str, Array1D_string &ele, int nEle);

    void GetReportVariableKey(EnergyPlusData &state,
                              const Array1D_string &varKeys,
                              int numberOfKeys,
                              const Array1D_string &varNames,
                              Array1D_int &keyVarIndexes,
                              Array1D<OutputProcessor::VariableType> &varTypes);

    std::vector<char> getCharArrayFromString(std::string const &originalString);

    std::string getStringFromCharArray(std::vector<char> originalCharArray);

    void StopExternalInterfaceIfError(EnergyPlusData &state);

    void ValidateRunControl(EnergyPlusData &state);

    void WarnIfExternalInterfaceObjectsAreUsed(EnergyPlusData &state, std::string const &ObjectWord);

    void CalcExternalInterfaceFMUImport(EnergyPlusData &state);

    void InitExternalInterfaceFMUImport(EnergyPlusData &state);

    void InstantiateInitializeFMUImport(EnergyPlusData &state);

    void TerminateResetFreeFMUImport(EnergyPlusData &state, int fmiEndSimulation);

    void GetSetVariablesAndDoStepFMUImport(EnergyPlusData &state);

    void VerifyExternalInterfaceObject(EnergyPlusData &state);

    Real64 GetCurSimStartTimeSeconds(const EnergyPlusData &state);

    std::string trim(std::string const &str);

} // namespace ExternalInterface

struct ExternalInterfaceData : BaseGlobalStruct
{
    Real64 tComm = 0.0;
    Real64 tStop = 3600.0;
    Real64 tStart = 0.0;
    Real64 hStep = 15.0;
    bool FlagReIni = false;
    fs::path FMURootWorkingFolder;
    int nInKeys = 3; // Number of input variables available in ExternalInterface (=highest index* number)

    Array1D<ExternalInterface::FMUType> FMU;                                // Variable Types structure
    Array1D<ExternalInterface::FMUType> FMUTemp;                            // Variable Types structure
    Array1D<ExternalInterface::checkFMUInstanceNameType> checkInstanceName; // Variable Types structure for checking instance names

    int NumExternalInterfaces = 0;               // Number of ExternalInterface objects
    int NumExternalInterfacesBCVTB = 0;          // Number of BCVTB ExternalInterface objects
    int NumExternalInterfacesFMUImport = 0;      // Number of FMU ExternalInterface objects
    int NumExternalInterfacesFMUExport = 0;      // Number of FMU ExternalInterface objects
    int NumFMUObjects = 0;                       // Number of FMU objects
    int FMUExportActivate = 0;                   // FMU Export flag
    bool haveExternalInterfaceBCVTB = false;     // Flag for BCVTB interface
    bool haveExternalInterfaceFMUImport = false; // Flag for FMU-Import interface
    bool haveExternalInterfaceFMUExport = false; // Flag for FMU-Export interface
    int simulationStatus = 1; // Status flag. Used to report during which phase an error occurred. (1=initialization, 2=time stepping)

    Array1D<int> keyVarIndexes;                      // Array index for specific key name
    Array1D<OutputProcessor::VariableType> varTypes; // Types of variables in keyVarIndexes
    Array1D<int> varInd;                             // Index of ErlVariables for ExternalInterface
    int socketFD = -1;                               // socket file descriptor
    bool ErrorsFound = false;                        // Set to true if errors are found
    bool noMoreValues = false;                       // Flag, true if no more values will be sent by the server

    Array1D<std::string> varKeys;     // Keys of report variables used for data exchange
    Array1D<std::string> varNames;    // Names of report variables used for data exchange
    Array1D<int> inpVarTypes;         // Names of report variables used for data exchange
    Array1D<std::string> inpVarNames; // Names of report variables used for data exchange

    bool configuredControlPoints = false; // True if control points have been configured
    bool useEMS = false;                  // Will be set to true if ExternalInterface writes to EMS variables or actuators

    bool firstCall = true;
    bool showContinuationWithoutUpdate = true;
    bool GetInputFlag = true; // First time, input is "gotten"
    bool InitExternalInterfacefirstCall = true;
    bool FirstCallGetSetDoStep = true; // Flag to check when External Interface is called first time
    bool FirstCallIni = true;          // First time, input has been read
    bool FirstCallDesignDays = true;   // Flag fo first call during warmup
    bool FirstCallWUp = true;          // Flag fo first call during warmup
    bool FirstCallTStep = true;        // Flag for first call during time stepping
    int fmiEndSimulation = 0;          // Flag to indicate end of simulation

    fs::path const socCfgFilPath = "socket.cfg"; // socket configuration file
    std::unordered_map<std::string, std::string> UniqueFMUInputVarNames;

    int nOutVal; // Number of output values (E+ -> ExternalInterface)
    int nInpVar; // Number of input values (ExternalInterface -> E+)

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->tComm = 0.0;
        this->tStop = 3600.0;
        this->tStart = 0.0;
        this->hStep = 15.0;
        this->FlagReIni = false;
        this->FMURootWorkingFolder.clear();
        this->nInKeys = 3; // Number of input variables available in ExternalInterface (=highest index* number)

        this->FMU.clear();               // Variable Types structure
        this->FMUTemp.clear();           // Variable Types structure
        this->checkInstanceName.clear(); // Variable Types structure for checking instance names

        this->NumExternalInterfaces = 0;              // Number of ExternalInterface objects
        this->NumExternalInterfacesBCVTB = 0;         // Number of BCVTB ExternalInterface objects
        this->NumExternalInterfacesFMUImport = 0;     // Number of FMU ExternalInterface objects
        this->NumExternalInterfacesFMUExport = 0;     // Number of FMU ExternalInterface objects
        this->NumFMUObjects = 0;                      // Number of FMU objects
        this->FMUExportActivate = 0;                  // FMU Export flag
        this->haveExternalInterfaceBCVTB = false;     // Flag for BCVTB interface
        this->haveExternalInterfaceFMUImport = false; // Flag for FMU-Import interface
        this->haveExternalInterfaceFMUExport = false; // Flag for FMU-Export interface
        this->simulationStatus = 1; // Status flag. Used to report during which phase an error occurred. (1=initialization, 2=time stepping)

        this->keyVarIndexes.clear(); // Array index for specific key name
        this->varTypes.clear();      // Types of variables in keyVarIndexes
        this->varInd.clear();        // Index of ErlVariables for ExternalInterface
        this->socketFD = -1;         // socket file descriptor
        this->ErrorsFound = false;   // Set to true if errors are found
        this->noMoreValues = false;  // Flag, true if no more values will be sent by the server

        this->varKeys.clear();     // Keys of report variables used for data exchange
        this->varNames.clear();    // Names of report variables used for data exchange
        this->inpVarTypes.clear(); // Names of report variables used for data exchange
        this->inpVarNames.clear(); // Names of report variables used for data exchange

        this->configuredControlPoints = false; // True if control points have been configured
        this->useEMS = false;                  // Will be set to true if ExternalInterface writes to EMS variables or actuators
        this->firstCall = true;
        this->showContinuationWithoutUpdate = true;
        this->GetInputFlag = true; // First time, input is "gotten"
        this->InitExternalInterfacefirstCall = true;
        this->FirstCallGetSetDoStep = true; // Flag to check when External Interface is called first time
        this->FirstCallIni = true;          // First time, input has been read
        this->FirstCallDesignDays = true;   // Flag fo first call during warmup
        this->FirstCallWUp = true;          // Flag fo first call during warmup
        this->FirstCallTStep = true;        // Flag for first call during time stepping
        this->fmiEndSimulation = 0;         // Flag to indicate end of simulation
        this->UniqueFMUInputVarNames.clear();

        // these were statistics without an initial value
        //        int nOutVal;       // Number of output values (E+ -> ExternalInterface)
        //        int nInpVar;       // Number of input values (ExternalInterface -> E+)
    }
};

} // namespace EnergyPlus

#endif
