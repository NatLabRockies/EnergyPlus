#ifndef fmiModelFunctions_h
#define fmiModelFunctions_h

/* This header file must be utilized when compiling a model.
   It defines all functions of the Model Execution Interface.
   In order to have unique function names even if several models
   are compiled together (e.g. for embedded systems), every "real" function name
   is constructed by prepending the function name by
   "MODEL_IDENTIFIER" + "_" where "MODEL_IDENTIFIER" is the short name
   of the model used as the name of the zip-file where the model is stored.
   Therefore, the typical usage is:

      #define MODEL_IDENTIFIER MyModel
      #include "fmiModelFunctions.h"

   As a result, a function that is defined as "fmiGetDerivatives" in this header file,
   is actually getting the name "MyModel_fmiGetDerivatives".

   Revisions:
   - Jan. 20, 2010: stateValueReferencesChanged added to struct fmiEventInfo (ticket #27)
                    (by M. Otter, DLR)
                    Added WIN32 pragma to define the struct layout (ticket #34)
                    (by J. Mauss, QTronic)
   - Jan.  4, 2010: Removed argument intermediateResults from fmiInitialize
                    Renamed macro fmiGetModelFunctionsVersion to fmiGetVersion
                    Renamed macro fmiModelFunctionsVersion to fmiVersion
                    Replaced fmiModel by fmiComponent in decl of fmiInstantiateModel
                    (by J. Mauss, QTronic)
   - Dec. 17, 2009: Changed extension "me" to "fmi" (by Martin Otter, DLR).
   - Dez. 14, 2009: Added eventInfo to meInitialize and added
                    meGetNominalContinuousStates (by Martin Otter, DLR)
   - Sept. 9, 2009: Added DllExport (according to Peter Nilsson's suggestion)
                    (by A. Junghanns, QTronic)
   - Sept. 9, 2009: Changes according to FMI-meeting on July 21:
                    meInquireModelTypesVersion     -> meGetModelTypesPlatform
                    meInquireModelFunctionsVersion -> meGetModelFunctionsVersion
                    meSetStates                    -> meSetContinuousStates
                    meGetStates                    -> meGetContinuousStates
                    removal of meInitializeModelClass
                    removal of meGetTime
                    change of arguments of meInstantiateModel
                    change of arguments of meCompletedIntegratorStep
                    (by Martin Otter, DLR):
   - July 19, 2009: Added "me" as prefix to file names (by Martin Otter, DLR).
   - March 2, 2009: Changed function definitions according to the last design
                    meeting with additional improvements (by Martin Otter, DLR).
   - Dec. 3 , 2008: First version by Martin Otter (DLR) and Hans Olsson (Dynasim).


   Copyright © 2008-2009, MODELISAR consortium. All rights reserved.
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

   with the extension:

   You may distribute or publicly perform any modification only under the
   terms of this license.
*/

#include "fmiModelTypes.h"
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
//#define fmiGetModelTypesPlatform      fmiFullName(_fmiGetModelTypesPlatform)	// Zuo: definition in model-ex V1.0
#define fmiGetTypesPlatform fmiFullName(_fmiGetTypesPlatform) // Zuo: definition in co-sim V1.0
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
//#define fmiInstantiateModel           fmiFullName(_fmiInstantiateModel)		// Zuo: definition in model-ex V1.0
#define fmiInstantiateSlave fmiFullName(_fmiInstantiateSlave) // Zuo: definition in co-sim V1.0
//#define fmiInitialize                 fmiFullName(_fmiInitialize)				// Zuo: definition in model-ex V1.0
#define fmiInitializeSlave fmiFullName(_fmiInitializeSlave) // Zuo: definition in co-sim V1.0
//#define fmiTerminate                  fmiFullName(_fmiTerminate)				// Zuo: defintion in modle-ex V1.0
#define fmiTerminateSlave fmiFullName(_fmiTerminateSlave) // Zuo: defnition in co-sim V1.0
//#define fmiFreeModelInstance          fmiFullName(_fmiFreeModelInstance)		// Zuo: definition in model-ex V1.0
#define fmiResetSlave fmiFullName(_fmiResetSlave)                           // Zuo: definition in co-sim V1.0
#define fmiFreeSlaveInstance fmiFullName(_fmiFreeSlaveInstance)             // Zuo: definition in co-sim V1.0
#define fmiSetRealInputDerivatives fmiFullName(_fmiSetRealInputDerivatives) // Zuo: definition in co-sim V1.0
//#define fmiGetDerivatives             fmiFullName(_fmiGetDerivatives)			// Zuo: definition in model-ex V1.0
#define fmiGetRealOutputDerivatives fmiFullName(_fmiGetRealOutputDerivatives) // Zuo: definition in co-sim V1.0
#define fmiDoStep fmiFullName(_fmiDoStep)                                     // Zuo: definition in co-sim V1.0
#define fmiCancelStep fmiFullName(_fmiCancelStep)                             // Zuo: definition in co-sim V1.0
#define fmiGetStatus fmiFullName(_fmiGetStatus)                               // Zuo: definition in co-sim V1.0
#define fmiGetRealStatus fmiFullName(_fmiGetRealStatus)                       // Zuo: definition in co-sim V1.0
#define fmiGetIntegerStatus fmiFullName(_fmiGetIntegerStatus)                 // Zuo: definition in co-sim V1.0
#define fmiGetBooleanStatus fmiFullName(_fmiGetBooleanStatus)                 // Zuo: definition in co-sim V1.0
#define fmiGetStringStatus fmiFullName(_fmiGetStringStatus)                   // Zuo: definition in co-sim V1.0

//#define fmiSetTime                    fmiFullName(_fmiSetTime)					// Zuo: not in co-sim V1.0
//#define fmiSetContinuousStates        fmiFullName(_fmiSetContinuousStates)		// Zuo: not in co-sim V1.0
//#define fmiCompletedIntegratorStep    fmiFullName(_fmiCompletedIntegratorStep)	// Zuo: not in co-sim V1.0
//#define fmiGetEventIndicators         fmiFullName(_fmiGetEventIndicators)		// Zuo: not in co-sim V1.0
//#define fmiEventUpdate                fmiFullName(_fmiEventUpdate)					// Zuo: not in co-sim V1.0
//#define fmiGetContinuousStates        fmiFullName(_fmiGetContinuousStates)			// Zuo: not in co-sim V1.0
//#define fmiGetNominalContinuousStates fmiFullName(_fmiGetNominalContinuousStates)		// Zuo: not in co-sim V1.0
//#define fmiGetStateValueReferences    fmiFullName(_fmiGetStateValueReferences)		// Zuo: not in co-sim V1.0

/* make sure all compiler use the same alignment policies for structures */
#ifdef WIN32
#pragma pack(push, 8)
#endif

/* Version number */
#define fmiVersion "1.0"

/* Type definitions */
typedef enum
{
    fmiOK,
    fmiWarning,
    fmiDiscard,
    fmiError,
    // fmiFatal		// Zuo: definition in model-ex V1.0
    fmiPending // Zuo: definition in co-sim V1.0
} fmiStatus;

typedef void (*fmiCallbackLogger)(fmiComponent c, fmiString instanceName, fmiStatus status, fmiString category, fmiString message, ...);
typedef void *(*fmiCallbackAllocateMemory)(size_t nobj, size_t size);
typedef void (*fmiCallbackFreeMemory)(void *obj);
typedef void (*fmiStepFinished)(fmiComponent c, fmiStatus status); // Zuo: definition in co-sim V1.0

typedef struct
{
    fmiCallbackLogger logger;
    fmiCallbackAllocateMemory allocateMemory;
    fmiCallbackFreeMemory freeMemory;
    fmiStepFinished stepFinished; // Zuo: definition in co-sim V1.0
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
/* Inquire version numbers of header files */
// DllExport const char* fmiGetModelTypesPlatform();	//Zuo: definition in model-ex V1.0
DllExport const char *fmiGetTypesPlatform(); // Zuo: definition in co-sim V1.0
DllExport const char *fmiGetVersion();

DllExport fmiStatus fmiSetDebugLogging(fmiComponent c, fmiBoolean loggingOn); // Zuo: definition in co-sim V1.0

/***************************************************

Functions for FMI for Co-Simulation

****************************************************/
/* Creation and destruction of model instances and setting debug status */
// Zuo: definition in model-ex V1.0
/*DllExport fmiComponent fmiInstantiateModel (fmiString            instanceName,
                                                                                             fmiString            GUID,
                                                                                             fmiCallbackFunctions functions,
                                                                                             fmiBoolean           loggingOn); */
// Zuo: definition in co-sim V1.0
DllExport fmiComponent fmiInstantiateSlave(fmiString instanceName,
                                           fmiString GUID,
                                           fmiString fmuLocation,
                                           fmiString mimeType,
                                           fmiReal timeout,
                                           fmiBoolean visible,
                                           fmiBoolean interactive,
                                           fmiCallbackFunctions functions,
                                           fmiBoolean loggingOn);
// Zuo: definition in model-ex V1.0
// DllExport fmiStatus fmiInitialize(fmiComponent c, fmiBoolean toleranceControlled,
// fmiReal relativeTolerance, fmiEventInfo* eventInfo);
// Zuo: definition in co-sim V1.0
DllExport fmiStatus fmiInitializeSlave(fmiComponent c, fmiReal tStart, fmiBoolean StopTimeDefined, fmiReal tStop);

DllExport fmiStatus fmiTerminateSlave(fmiComponent c); // Zuo: definition in co-sim V1.0
DllExport fmiStatus fmiResetSlave(fmiComponent c);     // Zuo: definition in co-sim V1.0
// DllExport void      fmiFreeModelInstance(fmiComponent c);				// Zuo: definition in model-ex V1.0
DllExport void fmiFreeSlaveInstance(fmiComponent c); // Zuo: definition in co-sim V1.0

// Zuo: definition in co-sim V1.0
DllExport fmiStatus
fmiSetRealInputDerivatives(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiInteger order[], const fmiReal value[]);

// Zuo: definition in co-sim V1.0
DllExport fmiStatus fmiGetRealOutputDerivatives(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiInteger order[], fmiReal value[]);

DllExport fmiStatus fmiCancelStep(fmiComponent c);

DllExport fmiStatus fmiDoStep(fmiComponent c, fmiReal currentCommunicationPoint, fmiReal communicationStepSize, fmiBoolean newStep);

// DllExport fmiStatus fmiSetDebugLogging  (fmiComponent c, fmiBoolean loggingOn);	// Zuo definition in model-ex V1.0

typedef enum
{
    fmiDoStepStatus,
    fmiPendingStatus,
    fmiLastSuccessfulTime
} fmiStatusKind;

/* Providing independent variables and re-initialization of caching */
// DllExport fmiStatus fmiSetTime                (fmiComponent c, fmiReal time);					// Zuo: not in co-sim V1.0
// DllExport fmiStatus fmiSetContinuousStates    (fmiComponent c, const fmiReal x[], size_t nx);	// Zuo: not in co-sim V1.0
// DllExport fmiStatus fmiCompletedIntegratorStep(fmiComponent c, fmiBoolean* callEventUpdate);	// Zuo: not in co-sim V1.0
DllExport fmiStatus fmiSetReal(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiReal value[]);
DllExport fmiStatus fmiSetInteger(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiInteger value[]);
DllExport fmiStatus fmiSetBoolean(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiBoolean value[]);
DllExport fmiStatus fmiSetString(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiString value[]);

/* Evaluation of the model equations */

// DllExport fmiStatus fmiGetDerivatives    (fmiComponent c, fmiReal derivatives[]    , size_t nx);	// Zuo: not in co-sim V1.0
// DllExport fmiStatus fmiGetEventIndicators(fmiComponent c, fmiReal eventIndicators[], size_t ni);	// Zuo: not in cos-im V1.0
// DllExport fmiStatus fmiEventUpdate               (fmiComponent c, fmiBoolean intermediateResults, fmiEventInfo* eventInfo);	// Zuo: not in co-sim
// V1.0
// DllExport fmiStatus fmiGetContinuousStates       (fmiComponent c, fmiReal states[], size_t nx);								// Zuo:
// not in co-sim V1.0 DllExport fmiStatus fmiGetNominalContinuousStates(fmiComponent c, fmiReal x_nominal[], size_t nx);
// // Zuo: not in co-sim V1.0 DllExport fmiStatus fmiGetStateValueReferences   (fmiComponent c, fmiValueReference vrx[], size_t nx);
// // Zuo: not in co-sim V1.0
// DllExport fmiStatus fmiTerminate                 (fmiComponent c);
// // Zuo: not in co-sim V1.0

#endif // fmiModelFunctions_h
