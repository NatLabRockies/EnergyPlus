#ifndef fmiFunctions_h
#define fmiFunctions_h

/* This header file must be utilized when compiling a FMU.
   It defines all functions of Co-Simulation Interface.
   In order to have unique function names even if several FMUs
   are compiled together (e.g. for embedded systems), every "real" function name
   is constructed by prepending the function name by
   "MODEL_IDENTIFIER" + "_" where "MODEL_IDENTIFIER" is the short name
   of the model used as the name of the zip-file where the model is stored.
   Therefore, the typical usage is:

      #define MODEL_IDENTIFIER MyModel
      #include "fmiFunctions.h"

   As a result, a function that is defined as "fmiGetDerivatives" in this header file,
   is actually getting the name "MyModel_fmiGetDerivatives".

   Revisions:
   - November 4, 2010: Adapted to specification text:
                       o fmiGetModelTypesPlatform renamed to fmiGetTypesPlatform
                       o fmiInstantiateSlave: Argument GUID     replaced by fmuGUID
                                              Argument mimetype replaced by mimeType
                       o tabs replaced by spaces
   - October 16, 2010: First public Version


   Copyright © 2008-2010, MODELISAR consortium. All rights reserved.
   This file is licensed by the copyright holders under the BSD License
   (http://www.opensource.org/licenses/bsd-license.html):

   ----------------------------------------------------------------------------
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   - Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.
   - Neither the name of the copyright holders nor the names of its
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
   ----------------------------------------------------------------------------
*/

#include "fmiPlatformTypes.h"
#include <stdlib.h>

/* Export fmi functions on Windows */
#ifdef _MSC_VER
#define DllExport __declspec(dllexport)
#else
#define DllExport
#endif

/* Macros to construct the real function name
   (prepend function name by MODEL_IDENTIFIER + "_") */

#define fmiPaste(a, b) a##b
#define fmiPasteB(a, b) fmiPaste(a, b)
#define fmiFullName(name) fmiPasteB(MODEL_IDENTIFIER, name)

/***************************************************
Common Functions
****************************************************/
#define fmiGetTypesPlatform fmiFullName(_fmiGetTypesPlatform)
#define fmiGetVersion fmiFullName(_fmiGetVersion)
#define fmiSetDebugLogging fmiFullName(_fmiSetDebugLogging)

/*Data Exchange*/
#define fmiSetReal fmiFullName(_fmiSetReal)
#define fmiSetInteger fmiFullName(_fmiSetInteger)
#define fmiSetBoolean fmiFullName(_fmiSetBoolean)
#define fmiSetString fmiFullName(_fmiSetString)

#define fmiGetReal fmiFullName(_fmiGetReal)
#define fmiGetInteger fmiFullName(_fmiGetInteger)
#define fmiGetBoolean fmiFullName(_fmiGetBoolean)
#define fmiGetString fmiFullName(_fmiGetString)

/***************************************************
Functions for FMI for Co-Simulation
****************************************************/
#define fmiInstantiateSlave fmiFullName(_fmiInstantiateSlave)
#define fmiInitializeSlave fmiFullName(_fmiInitializeSlave)
#define fmiTerminateSlave fmiFullName(_fmiTerminateSlave)
#define fmiResetSlave fmiFullName(_fmiResetSlave)
#define fmiFreeSlaveInstance fmiFullName(_fmiFreeSlaveInstance)
#define fmiSetRealInputDerivatives fmiFullName(_fmiSetRealInputDerivatives)
#define fmiGetRealOutputDerivatives fmiFullName(_fmiGetRealOutputDerivatives)
#define fmiDoStep fmiFullName(_fmiDoStep)
#define fmiCancelStep fmiFullName(_fmiCancelStep)
#define fmiGetStatus fmiFullName(_fmiGetStatus)
#define fmiGetRealStatus fmiFullName(_fmiGetRealStatus)
#define fmiGetIntegerStatus fmiFullName(_fmiGetIntegerStatus)
#define fmiGetBooleanStatus fmiFullName(_fmiGetBooleanStatus)
#define fmiGetStringStatus fmiFullName(_fmiGetStringStatus)

/* Version number */
#define fmiVersion "1.0"

/* make sure all compiler use the same alignment policies for structures */
#ifdef WIN32
#pragma pack(push, 8)
#endif

/* Type definitions */
typedef enum
{
    fmiOK,
    fmiWarning,
    fmiDiscard,
    fmiError,
    fmiFatal,
    fmiPending
} fmiStatus;

typedef void (*fmiCallbackLogger)(fmiComponent c, fmiString instanceName, fmiStatus status, fmiString category, fmiString message, ...);
typedef void *(*fmiCallbackAllocateMemory)(size_t nobj, size_t size);
typedef void (*fmiCallbackFreeMemory)(void *obj);
typedef void (*fmiStepFinished)(fmiComponent c, fmiStatus status);

typedef struct
{
    fmiCallbackLogger logger;
    fmiCallbackAllocateMemory allocateMemory;
    fmiCallbackFreeMemory freeMemory;
    fmiStepFinished stepFinished;
} fmiCallbackFunctions;

typedef struct
{
    fmiBoolean iterationConverged;
    fmiBoolean stateValueReferencesChanged;
    fmiBoolean stateValuesChanged;
    fmiBoolean terminateSimulation;
    fmiBoolean upcomingTimeEvent;
    fmiReal nextEventTime;
} fmiEventInfo;

/* reset alignment policy to the one set before reading this file */
#ifdef WIN32
#pragma pack(pop)
#endif

/***************************************************
Common Functions
****************************************************/

/* Inquire version numbers of header files */
DllExport const char *fmiGetTypesPlatform();
DllExport const char *fmiGetVersion();

DllExport fmiStatus fmiSetDebugLogging(fmiComponent c, fmiBoolean loggingOn);

/* Data Exchange Functions*/
DllExport fmiStatus fmiGetReal(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiReal value[]);
DllExport fmiStatus fmiGetInteger(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiInteger value[]);
DllExport fmiStatus fmiGetBoolean(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiBoolean value[]);
DllExport fmiStatus fmiGetString(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiString value[]);

DllExport fmiStatus fmiSetReal(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiReal value[]);
DllExport fmiStatus fmiSetInteger(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiInteger value[]);
DllExport fmiStatus fmiSetBoolean(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiBoolean value[]);
DllExport fmiStatus fmiSetString(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiString value[]);

/***************************************************
Functions for FMI for Co-Simulation
****************************************************/

/* Creation and destruction of slave instances and setting debug status */
DllExport fmiComponent fmiInstantiateSlave(fmiString instanceName,
                                           fmiString fmuGUID,
                                           fmiString fmuLocation,
                                           fmiString mimeType,
                                           fmiReal timeout,
                                           fmiBoolean visible,
                                           fmiBoolean interactive,
                                           fmiCallbackFunctions functions,
                                           fmiBoolean loggingOn);

DllExport fmiStatus fmiInitializeSlave(fmiComponent c, fmiReal tStart, fmiBoolean StopTimeDefined, fmiReal tStop);

DllExport fmiStatus fmiTerminateSlave(fmiComponent c);
DllExport fmiStatus fmiResetSlave(fmiComponent c);
DllExport void fmiFreeSlaveInstance(fmiComponent c);

DllExport fmiStatus
fmiSetRealInputDerivatives(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiInteger order[], const fmiReal value[]);

DllExport fmiStatus fmiGetRealOutputDerivatives(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiInteger order[], fmiReal value[]);

DllExport fmiStatus fmiCancelStep(fmiComponent c);
DllExport fmiStatus fmiDoStep(fmiComponent c, fmiReal currentCommunicationPoint, fmiReal communicationStepSize, fmiBoolean newStep);

typedef enum
{
    fmiDoStepStatus,
    fmiPendingStatus,
    fmiLastSuccessfulTime
} fmiStatusKind;

DllExport fmiStatus fmiGetStatus(fmiComponent c, const fmiStatusKind s, fmiStatus *value);
DllExport fmiStatus fmiGetRealStatus(fmiComponent c, const fmiStatusKind s, fmiReal *value);
DllExport fmiStatus fmiGetIntegerStatus(fmiComponent c, const fmiStatusKind s, fmiInteger *value);
DllExport fmiStatus fmiGetBooleanStatus(fmiComponent c, const fmiStatusKind s, fmiBoolean *value);
DllExport fmiStatus fmiGetStringStatus(fmiComponent c, const fmiStatusKind s, fmiString *value);

#endif // fmiFunctions_h
