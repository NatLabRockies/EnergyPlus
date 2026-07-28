/* -------------------------------------------------------------------------
///////////////////////////////////////////////////////
/// \file   main.h
///
/// \author Thierry Stephane Nouidui,
///         Simulation Research Group,
///         LBNL,
///         TSNouidui@lbl.gov
///
/// \date   2011-10-11
///
/// This files contains the definition of all
/// energyplus functions that are needed to mapped
/// to fmiFunctions

/// This file is based on main.h that is copyrighted by
/// QTronic GmbH and that is distributed under the BSD license.
/// Function types for all FMU functions and a struct to wrap a FMU dll.
/// Copyright 2010 QTronic GmbH. All rights reserved.
/// -------------------------------------------------------------------------
*/

#ifndef main_h
#define main_h

// Use windows.h only for Windows
#ifdef _WIN32
#include <windows.h>
#define WINDOWS 1
#else
#define WINDOWS 0
#define HANDLE void *
/* See http://www.yolinux.com/TUTORIALS/LibraryArchives-StaticAndDynamic.html */
#include <sys/stat.h> // for creating dirs on Linux
#endif

#include "eplusModelFunctions.h"
#include "fmiModelFunctions.h"
#include "xml_parser_cosim.h"

typedef const char *(*fGetTypesPlatform)(void);
typedef const char *(*fGetVersion)(void);
typedef fmiComponent (*fInstantiateSlave)(fmiString instanceName,
                                          fmiString GUID,
                                          fmiString fmuLocation,
                                          fmiString mimeType,
                                          fmiReal timeout,
                                          fmiBoolean visible,
                                          fmiBoolean interactive,
                                          fmiCallbackFunctions functions,
                                          fmiBoolean loggingOn);
typedef void (*fFreeSlaveInstance)(fmiComponent c);
typedef fmiStatus (*fResetSlaveInstance)(fmiComponent c);
typedef fmiStatus (*fSetDebugLogging)(fmiComponent c, fmiBoolean loggingOn);
typedef fmiStatus (*fSetReal)(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiReal value[]);
typedef fmiStatus (*fSetInteger)(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiInteger value[]);
typedef fmiStatus (*fSetBoolean)(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiBoolean value[]);
typedef fmiStatus (*fSetString)(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiString value[]);
typedef fmiStatus (*fInitializeSlave)(fmiComponent c, fmiReal tStart, fmiBoolean StopTimeDefined, fmiReal tStop);

typedef fmiStatus (*fGetReal)(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiReal value[]);
typedef fmiStatus (*fGetInteger)(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiInteger value[]);
typedef fmiStatus (*fGetBoolean)(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiBoolean value[]);
typedef fmiStatus (*fGetString)(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiString value[]);

typedef fmiStatus (*fDoStep)(fmiComponent c,
                             fmiReal currentCommunicationPoint,
                             fmiReal communicationStepSize,
                             fmiBoolean newStep); // Zuo: add for co-sim

typedef struct FMU
{
    int index;
    char *instanceName;
    fmiString modelID;
    fmiString modelGUID;
    ModelDescription *modelDescription;
    HANDLE dllHandle;
    fGetTypesPlatform getTypesPlatform;
    fGetVersion getVersion;
    fInstantiateSlave instantiateSlave;
    fFreeSlaveInstance freeSlaveInstance;
    fResetSlaveInstance resetSlaveInstance;
    fSetDebugLogging setDebugLogging;
    fSetReal setReal;
    fSetInteger setInteger;
    fSetBoolean setBoolean;
    fSetString setString;
    fInitializeSlave initializeSlave;
    fGetReal getReal;
    fGetInteger getInteger;
    fGetBoolean getBoolean;
    fGetString getString;
    fDoStep doStep;

} FMU;

fmiComponent fmiEPlusInstantiateSlave(char *fmuResFolder,
                                      fmiInteger *sizefmuResFolder,
                                      fmiReal *timeOut,
                                      fmiInteger *visible,
                                      fmiInteger *interactive,
                                      fmiInteger *loggingOn,
                                      fmiInteger *index);

fmiStatus fmiEPlusInitializeSlave(fmiComponent *fmuInstance, fmiReal *tStart, fmiInteger *newStep, fmiReal *tStop, fmiInteger *index);

fmiStatus fmiEPlusGetReal(fmiComponent *fmuInstance, const fmiValueReference valRef[], fmiReal outValue[], fmiInteger *numOutputs, fmiInteger *index);

fmiStatus fmiEPlusSetReal(fmiComponent *fmuInstance, const fmiValueReference valRef[], fmiReal inpVal[], fmiInteger *numInputs, fmiInteger *index);

fmiStatus fmiEPlusDoStep(fmiComponent *fmuInstance, fmiReal *curCommPoint, fmiReal *commStepSize, fmiInteger *newStep, fmiInteger *index);

fmiStatus fmiEPlusFreeSlave(fmiComponent *fmuInstance, fmiInteger *index, fmiInteger *fmiEndSimulation);

fmiStatus fmiEPlusResetSlave(fmiComponent *fmuInstance, fmiInteger *index);

fmiInteger fmiEPlusUnpack(char *fmuName, char *fmuOutputWorkingFolder, fmiInteger *sizefmuName, fmiInteger *sizefmuOutputWorkingFolder);

fmiInteger
addLibPathCurrentWorkingFolder(char *trimfmuOutputWorkingFolder_wLiB, char *fmuWorkingFolder, fmiInteger *sizefmuWorkingFolder, fmiInteger *index);

fmiValueReference getValueReferenceByNameFMUInputVariables(char *variableName, fmiInteger *sizeVariableName, fmiInteger *index);

fmiValueReference getValueReferenceByNameFMUOutputVariables(char *variableName, fmiInteger *sizeVariableName, fmiInteger *index);

fmiInteger
model_ID_GUID(char *fmuInstanceName, char *fmuWorkingFolder, fmiInteger *sizefmuWorkingFolder, fmiInteger *numInputs, fmiInteger *numOutputs);

fmiInteger addFMURootFolderName(char *fmuOutputWorkingFolder, char *fmuWorkingFolder, fmiInteger *sizefmuWorkingFolder);

fmiInteger getfmiEPlusVersion(char *fmuWorkingFolder, fmiInteger *sizefmuWorkingFolder, char *fmiVersionNumber, fmiInteger *index);

fmiInteger checkOperatingSystem(char *errorMessage);

#endif // main_h
