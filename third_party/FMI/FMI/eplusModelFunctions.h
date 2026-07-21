#ifndef eplusModelFunctions_h
#define eplusModelFunctions_h

///////////////////////////////////////////////////////
/// \file   eplusModelFunctions.h
///
///
/// \author Thierry Stephane Nouidui,
///         Simulation Research Group,
///         LBNL,
///         TSNouidui@lbl.gov
///
/// \date   2011-11-11
///
/// This header file defines the energyplus functions
/// that mapped to fmiFunctions and will be exported
/// in .dll
///////////////////////////////////////////////////////

#include "FMI/fmiModelFunctions.h"
#include "fmiModelTypes.h"
#include <stdlib.h>
/* Export fmi functions on Windows */
#ifdef _MSC_VER
#define DllExport __declspec(dllexport)
#define DllImport __declspec(dllexport)

#else
#define DllExport
#endif

/* make sure all compiler use the same alignment policies for structures */
#ifdef WIN32
#pragma pack(push, 8)
#endif

/* Version number */
#define fmiVersion "1.0"

/* reset alignment policy to the one set before reading this file */
#ifdef WIN32
#pragma pack(pop)
#endif

/***************************************************

EnergyPlus Functions for FMI for Co-Simulation

****************************************************/
DllExport fmiComponent fmiEPlusInstantiateSlave(char *fmuResFolder,
                                                fmiInteger *sizefmuResFolder,
                                                fmiReal *timeOut,
                                                fmiInteger *visible,
                                                fmiInteger *interactive,
                                                fmiInteger *loggingOn,
                                                fmiInteger *index);

DllExport fmiStatus fmiEPlusInitializeSlave(fmiComponent *fmuInstance, fmiReal *tStart, fmiInteger *newStep, fmiReal *tStop, fmiInteger *index);

DllExport fmiStatus
fmiEPlusGetReal(fmiComponent *fmuInstance, const fmiValueReference valRef[], fmiReal outValue[], fmiInteger *numOutputs, fmiInteger *index);

DllExport fmiStatus
fmiEPlusSetReal(fmiComponent *fmuInstance, const fmiValueReference valRef[], fmiReal inpVal[], fmiInteger *numInputs, fmiInteger *index);

DllExport fmiStatus fmiEPlusDoStep(fmiComponent *fmuInstance, fmiReal *curCommPoint, fmiReal *commStepSize, fmiInteger *newStep, fmiInteger *index);

DllExport fmiStatus fmiEPlusFreeSlave(fmiComponent *fmuInstance, fmiInteger *index, fmiInteger *fmiEndSimulation);

fmiStatus fmiEPlusResetSlave(fmiComponent *fmuInstance, fmiInteger *index);

DllExport fmiInteger fmiEPlusUnpack(char *fmuName, char *fmuOutputWorkingFolder, fmiInteger *sizefmuName, fmiInteger *sizefmuOutputWorkingFolder);

DllExport fmiInteger addLibPathCurrentWorkingFolder(char *trimfmuOutputWorkingFolder_wLiB,
                                                    char *fmuWorkingFolder,
                                                    fmiInteger *sizefmuWorkingFolder,
                                                    fmiInteger *index);

DllExport fmiValueReference getValueReferenceByNameFMUInputVariables(char *variableName, fmiInteger *sizeVariableName, fmiInteger *index);

DllExport fmiValueReference getValueReferenceByNameFMUOutputVariables(char *variableName, fmiInteger *sizeVariableName, fmiInteger *index);

DllExport fmiInteger
model_ID_GUID(char *fmuInstanceName, char *fmuWorkingFolder, fmiInteger *sizefmuWorkingFolder, fmiInteger *numInputs, fmiInteger *numOutputs);

DllExport fmiInteger addFMURootFolderName(char *fmuOutputWorkingFolder, char *fmuWorkingFolder, fmiInteger *sizefmuWorkingFolder);

DllExport fmiInteger getfmiEPlusVersion(char *fmuWorkingFolder, fmiInteger *sizefmuWorkingFolder, char *fmiVersionNumber, fmiInteger *index);

DllExport fmiInteger checkOperatingSystem(char *errorMessage);

#endif // eplusModelFunctions_h
