///////////////////////////////////////////////////////
/// \file   parser.h
///
/// \brief  Methods for master program that interacts
///         with an FMU for co-simulation
///
/// \author Wangda Zuo, Thierry S. Nouidui, Michael Wetter
///         Simulation Research Group,
///         LBNL,
///         WZuo@lbl.gov
///
/// \date   2011-11-17
///
/// \version $Id: parser.h 55724 2011-11-17 17:51:58Z wzuo$
///
/// This file provides an interface to an FMU for
/// co-simulation.
///
///
///////////////////////////////////////////////////////
#ifndef _MAIN_H_
#define _MAIN_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#define IS_WINDOWS 1
#else
#define IS_WINDOWS 0
#define HANDLE void *
/* See http://www.yolinux.com/TUTORIALS/LibraryArchives-StaticAndDynamic.html */
#include <sys/stat.h> // for creating dirs on Linux
#endif

#include "fmiFunctions.h"
#include "fmiModelTypes.h"
#include "xml_parser_cosim.h"

FILE *f1 = NULL;
FILE *fmuFil = NULL; /// FMU file name

#if WINDOWS
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

#define true 1
#define false 0

typedef const char *(*fGetModelTypesPlatform)(void);
typedef const char *(*fGetVersion)(void);
typedef fmiComponent (*fInstantiateModel)(fmiString instanceName, fmiString GUID, fmiCallbackFunctions functions, fmiBoolean loggingOn);
typedef void (*fFreeModelInstance)(fmiComponent c);
typedef fmiStatus (*fSetDebugLogging)(fmiComponent c, fmiBoolean loggingOn);
typedef fmiStatus (*fSetTime)(fmiComponent c, fmiReal time);
typedef fmiStatus (*fSetContinuousStates)(fmiComponent c, const fmiReal x[], size_t nx);
typedef fmiStatus (*fCompletedIntegratorStep)(fmiComponent c, fmiBoolean *callEventUpdate);
typedef fmiStatus (*fSetReal)(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiReal value[]);
typedef fmiStatus (*fSetInteger)(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiInteger value[]);
typedef fmiStatus (*fSetBoolean)(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiBoolean value[]);
typedef fmiStatus (*fSetString)(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiString value[]);
typedef fmiStatus (*fInitialize)(fmiComponent c, fmiBoolean toleranceControlled, fmiReal relativeTolerance, fmiEventInfo *eventInfo);
typedef fmiStatus (*fGetDerivatives)(fmiComponent c, fmiReal derivatives[], size_t nx);
typedef fmiStatus (*fGetEventIndicators)(fmiComponent c, fmiReal eventIndicators[], size_t ni);
typedef fmiStatus (*fGetReal)(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiReal value[]);
typedef fmiStatus (*fGetInteger)(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiInteger value[]);
typedef fmiStatus (*fGetBoolean)(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiBoolean value[]);
typedef fmiStatus (*fGetString)(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiString value[]);
typedef fmiStatus (*fEventUpdate)(fmiComponent c, fmiBoolean intermediateResults, fmiEventInfo *eventInfo);
typedef fmiStatus (*fGetContinuousStates)(fmiComponent c, fmiReal states[], size_t nx);
typedef fmiStatus (*fGetNominalContinuousStates)(fmiComponent c, fmiReal x_nominal[], size_t nx);
typedef fmiStatus (*fGetStateValueReferences)(fmiComponent c, fmiValueReference vrx[], size_t nx);
typedef fmiStatus (*fTerminate)(fmiComponent c);

typedef enum
{
    opt_printidf,
    opt_delete,
    opt_unpack,
    opt_help
} option;

typedef struct
{
    ModelDescription *modelDescription;
    HANDLE dllHandle;
    fGetModelTypesPlatform getModelTypesPlatform;
    fGetVersion getVersion;
    fInstantiateModel instantiateModel;
    fFreeModelInstance freeModelInstance;
    fSetDebugLogging setDebugLogging;
    fSetTime setTime;
    fSetContinuousStates setContinuousStates;
    fCompletedIntegratorStep completedIntegratorStep;
    fSetReal setReal;
    fSetInteger setInteger;
    fSetBoolean setBoolean;
    fSetString setString;
    fInitialize initialize;
    fGetDerivatives getDerivatives;
    fGetEventIndicators getEventIndicators;
    fGetReal getReal;
    fGetInteger getInteger;
    fGetBoolean getBoolean;
    fGetString getString;
    fEventUpdate eventUpdate;
    fGetContinuousStates getContinuousStates;
    fGetNominalContinuousStates getNominalContinuousStates;
    fGetStateValueReferences getStateValueReferences;
    fTerminate terminate;
} FMU;

#endif // _MAIN_H_

int callparser(const char *fmuFilNam, const char *tmpPat);

void help(void);
