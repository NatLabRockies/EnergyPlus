///////////////////////////////////////////////////////////////////
/// \file   main.c
///
///
/// \author Thierry Stephane Nouidui,
///         Simulation Research Group,
///         LBNL,
///         TSNouidui@lbl.gov
///
/// \date   2011-11-11
///
/// This files contains the implementation of all
/// energyplus functions that are needed to mapped
/// to fmiFunctions

/// This file is based on main.c that is copyrighted by
/// QTronic GmbH and that is distributed under the BSD license.
/// The original file, its copyright notice and its license can
/// be found in /Copyright
///
/// The file has been modified for use with the FMU standard for
/// co-simulation. The original file was developed for model exchange.
/// The original file used 0 as indicator for failure and
/// 1 as indicator for success.
/// The new file uses 0 as indicator for success according to STL.
///
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
#include "main.h"
#include "util.h"
#include <ctype.h>
#include <expat.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#define BUFSIZE 4096
#define PATHLEN 1024

#ifdef _MSC_VER
#define LIB_EXT ".dll"
#define LIB_PATH "%s%s%s.dll"
#define FMU_ROOT_DIR "tmp-fmus\\"
#define XML_FILE "\\modelDescription.xml"
#define BIN_WIN "\\binaries\\winxx\\"
#define BIN_WIN32 "\\binaries\\win32\\"
#define BIN_WIN64 "\\binaries\\win64\\"
#if defined(_WIN64)
#define OperSys 2
#elif defined(_WIN32)
#define OperSys 1
#endif
#elif __linux__
#include <dlfcn.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#define LIB_EXT ".so"
#define LIB_PATH "%s%s%s.so"
#define FMU_ROOT_DIR "tmp-fmus/"
#define XML_FILE "/modelDescription.xml"
#define BIN_LIN "/binaries/linuxxx/"
#define BIN_LIN32 "/binaries/linux32/"
#define BIN_LIN64 "/binaries/linux64/"
//#define OperSys 3
//#if defined(__LP32__)
//#define OperSys 3
#if defined(__LP64__) || defined(__x86_64__) || defined(__ia64__) || defined(__amd64__) || defined(__ppc64__)
#define OperSys 4
#else
#define OperSys 3
#endif
#elif __APPLE__
#include <dlfcn.h>
#define LIB_EXT ".dylib"
#define LIB_PATH "%s%s%s.dylib"
#define FMU_ROOT_DIR "tmp-fmus/"
#define XML_FILE "/modelDescription.xml"
#define BIN_MAC "/binaries/darwinxx/"
#define BIN_MAC32 "/binaries/darwin32/"
#define BIN_MAC64 "/binaries/darwin64/"
//#if defined(__LP32__)
//#define OperSys 5
#if defined(__LP64__) || defined(__x86_64__) || defined(__ia64__) || defined(__amd64__) || defined(__ppc64__)
#define OperSys 6
#else
#define OperSys 5
#endif
#endif

// FIXME: Dynamic allocation for FMU instances in addfmuInstances() not working.
// A large array will be instead created. If the number of FMU or FMU instances
// is larger than DELTA, the code will not work.
#define DELTA 100000
FMU **fmuInstances;

fmiComponent c;
FMU *_c;

///////////////////////////////////////////////////////////////////////////////
/// This function checks the operating system and returns an error message
/// if the operating system is not supported.
///
///\param name Error message
///////////////////////////////////////////////////////////////////////////////
fmiInteger checkOperatingSystem(char *errorMessage)
{
#ifdef __APPLE__
    char *msg = "FunctionalMock-up Unit for co-simulation is currently only supported on Windows and Linux.";
    strncpy(errorMessage, msg, strlen(msg));
    return -1;
#endif
    return 0;
}

#if defined _MSC_VER || defined __linux__

static int insNum = 0;
static int arrsize = 0;
static int fmuLocCoun = 0;
static int fmuLocDecoun = 0;

///////////////////////////////////////////////////////////////////////////////
/// Check if string \c name is in the string array \c array[]. This function is
///
///\param name The string to be checked
///\param kind The type of string to be checked
///\param array The list of strings for comparison
///\param n The length of array
///\return The index of matched string in the \c array[].
///        Otherwise, return -1 to indicate error
///////////////////////////////////////////////////////////////////////////////
static int checkName2(const char *name, const char *kind, const char *array[], int n)
{
    int i;
    for (i = 0; i < n; i++) {
        if (!strcmp(name, array[i])) return i;
    }
    printf("Illegal %s %s\n", kind, name);
    return -1;
}

///////////////////////////////////////////////////////////////////////////////
/// Check enum value of input string \c enu.
///
///\param enu String to be checked.
///\return The enum value of string if it is found in the enum.
///        Otherwise, return -1 to indicate an error.
///////////////////////////////////////////////////////////////////////////////
static int checkEnumValue2(const char *enu)
{
    return checkName2(enu, "enum value", enuNames, SIZEOF_ENU);
}
///////////////////////////////////////////////////////////////////////////////
/// Get address of specific function from specific dll
///
///\param fmu FMU instance
///\param modelID FMU model ID
///\param funnam Function name
///\return Address of the specific function
//////////////////////////////////////////////////////////////////////////////
static void *getAdr(FMU *fmu, const char *modelID, const char *functionName)
{
    char name[BUFSIZE];
    void *fp;
    sprintf(name, "%s_%s", modelID, functionName);

#ifdef _MSC_VER
    fp = GetProcAddress(fmu->dllHandle, name);
#else
    fp = dlsym(fmu->dllHandle, name);
#endif
    if (!fp) {
        printf("Error: Function %s not found in FMI functions library.\n", name);
    }
    return fp;
}

////////////////////////////////////////////////////////////////////////////////////
/// Create a list of pointer to FMUs
///
///\param s The Pointer to FMU.
////////////////////////////////////////////////////////////////////////////////////
static void addfmuInstances(FMU *s)
{
    FMU **temp;
    if (fmuLocCoun == arrsize) {
        temp = (FMU **)malloc(sizeof(FMU *) * (DELTA + arrsize));
        arrsize += DELTA;
        memcpy(temp, fmuInstances, sizeof(FMU *) * fmuLocCoun);
        free(fmuInstances);
        fmuInstances = temp;
    }
    fmuInstances[fmuLocCoun++] = s;
}

///////////////////////////////////////////////////////////////////////////////
/// Load the given dll and set function pointers in fmu.
/// It changes the names of the standard FMI functions by adding
/// the model identifier and links the new functions with QTronic's FMU structure.
///
///\param dllPat Path of the dll file
///\param modelID FMU model ID
///\param fmu FMU instance
///\return 0 if there is no error occurred
///////////////////////////////////////////////////////////////////////////////
static int loadLib(const char *libPath, const char *modelID, FMU *fmu)
{
#ifdef _MSC_VER
    HINSTANCE h;
#else
    void *h;
#endif

#ifdef _MSC_VER
    h = LoadLibrary(libPath);
    if (h == NULL) {
        printf("Error: unable to load FMI functions library.\n");
        return -1;
    }
    if (!h) {
        printf("Error: unable to load FMI functions library.\n");
        return -1;
    }
#else
    h = dlopen(libPath, RTLD_LAZY);
    if (h == NULL) {
        printf("Error: unable to load  FMI functions library.\n");
        return -1;
    }
    if (!h) {
        printf("Error: unable to load  FMI functions library.\n");
        return -1;
    }
#endif

    fmu->dllHandle = h;
    fmu->getVersion = (fGetVersion)getAdr(fmu, modelID, "fmiGetVersion");
    if (!(fmu->getVersion)) {
        return -1;
    }
    fmu->instantiateSlave = (fInstantiateSlave)getAdr(fmu, modelID, "fmiInstantiateSlave");
    if (!(fmu->instantiateSlave)) {
        return -1;
    }
    fmu->freeSlaveInstance = (fFreeSlaveInstance)getAdr(fmu, modelID, "fmiFreeSlaveInstance");
    if (!(fmu->freeSlaveInstance)) {
        return -1;
    }
    fmu->resetSlaveInstance = (fResetSlaveInstance)getAdr(fmu, modelID, "fmiResetSlave");
    if (!(fmu->resetSlaveInstance)) {
        return -1;
    }
    fmu->setDebugLogging = (fSetDebugLogging)getAdr(fmu, modelID, "fmiSetDebugLogging");
    if (!(fmu->setDebugLogging)) {
        return -1;
    }
    fmu->setReal = (fSetReal)getAdr(fmu, modelID, "fmiSetReal");
    if (!(fmu->setReal)) {
        return -1;
    }
    fmu->setInteger = (fSetInteger)getAdr(fmu, modelID, "fmiSetInteger");
    if (!(fmu->setInteger)) {
        return -1;
    }
    fmu->setBoolean = (fSetBoolean)getAdr(fmu, modelID, "fmiSetBoolean");
    if (!(fmu->setBoolean)) {
        return -1;
    }
    fmu->setString = (fSetString)getAdr(fmu, modelID, "fmiSetString");
    if (!(fmu->setString)) {
        return -1;
    }
    fmu->initializeSlave = (fInitializeSlave)getAdr(fmu, modelID, "fmiInitializeSlave");
    if (!(fmu->initializeSlave)) {
        return -1;
    }
    fmu->getReal = (fGetReal)getAdr(fmu, modelID, "fmiGetReal");
    if (!(fmu->getReal)) {
        return -1;
    }
    fmu->getInteger = (fGetInteger)getAdr(fmu, modelID, "fmiGetInteger");
    if (!(fmu->getInteger)) {
        return -1;
    }
    fmu->getBoolean = (fGetBoolean)getAdr(fmu, modelID, "fmiGetBoolean");
    if (!(fmu->getBoolean)) {
        return -1;
    }
    fmu->getString = (fGetString)getAdr(fmu, modelID, "fmiGetString");
    if (!(fmu->getString)) {
        return -1;
    }
    fmu->doStep = (fDoStep)getAdr(fmu, modelID, "fmiDoStep");
    if (!(fmu->doStep)) {
        return -1;
    }
    return 0; // success
}

static const char *fmiStatusToString(fmiStatus status)
{
    switch (status) {
    case fmiOK:
        return "ok";
    case fmiWarning:
        return "warning";
    case fmiDiscard:
        return "discard";
    case fmiError:
        return "error";
    case fmiPending:
        return "pending";
    default:
        return "?";
    }
}

#define MAX_MSG_SIZE 1000
///////////////////////////////////////////////////////////////////////////////
/// Search a fmu for the given variable.
///
///\param fmu FMU
///\param type Type of FMU variable
///\param vr FMI value reference
///\return NULL if not found or vr = fmiUndefinedValueReference
///////////////////////////////////////////////////////////////////////////////
static ScalarVariable *getSV(FMU *fmu, char type, fmiValueReference vr)
{
    int i;
    Elm tp;
    ScalarVariable **vars = fmu->modelDescription->modelVariables;
    if (vr == fmiUndefinedValueReference) return NULL;
    switch (type) {
    case 'r':
        tp = elm_Real;
        break;
    case 'i':
        tp = elm_Integer;
        break;
    case 'b':
        tp = elm_Boolean;
        break;
    case 's':
        tp = elm_String;
        break;
    }
    for (i = 0; vars[i]; i++) {
        ScalarVariable *sv = vars[i];
        if (vr == getValueReference(sv) && tp == sv->typeSpec->type) return sv;
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
/// Replace reference information in message.
/// E.g. #r1365# by variable name and ## by # in message
/// copies the result to buffer.
///
///\param msg The message to replace
///\param buffer The buffer
///\param nBuffer The size of the buffer
///\param fmu FMU The FMU instance
///////////////////////////////////////////////////////////////////////////////
static void replaceRefsInMessage(const char *msg, char *buffer, size_t nBuffer, FMU *fmu)
{
    size_t i = 0; // position in msg
    size_t k = 0; // position in buffer
    size_t n;
    char c = msg[i];
    while (c != '\0' && k < nBuffer) {
        if (c != '#') {
            buffer[k++] = c;
            i++;
            c = msg[i];
        } else {
            char *end = strchr(msg + i + 1, '#');
            if (!end) {
                printf("unmatched '#' in '%s'\n", msg);
                buffer[k++] = '#';
                break;
            }
            n = end - (msg + i);
            if (n == 1) {
                // ## detected, output #
                buffer[k++] = '#';
                i += 2;
                c = msg[i];
            } else {
                char type = msg[i + 1]; // one of ribs
                fmiValueReference vr;
                int nvr = sscanf(msg + i + 2, "%u", &vr);
                if (nvr == 1) {
                    // vr of type detected, e.g. #r12#
                    ScalarVariable *sv = getSV(fmu, type, vr);
                    const char *name = sv ? getName(sv) : "?";
                    sprintf(buffer + k, "%s", name);
                    k += strlen(name);
                    i += (n + 1);
                    c = msg[i];
                } else {
                    // could not parse the number
                    printf("illegal value reference at position %d in '%s'\n", (int)i + 2, msg);
                    buffer[k++] = '#';
                    break;
                }
            }
        }
    } // while
    buffer[k] = '\0';
}

///////////////////////////////////////////////////////////////////////////////
/// FMU logger
///
///\param c FMI component
///\param instanceName FMU instance name
///\param status FMI status
///\param category FMI string
///\param message Message to be recorded
///////////////////////////////////////////////////////////////////////////////
static void fmuLogger(fmiComponent c, fmiString instanceName, fmiStatus status, fmiString category, fmiString message, ...)
{
    char msg[MAX_MSG_SIZE];
    char *copy;
    va_list argp;
    FMU fmu;

    // Replace C format strings
    va_start(argp, message);
    vsprintf(msg, message, argp);

    // Replace e.g. ## and #r12#
    copy = strdup(msg);
    replaceRefsInMessage(copy, msg, MAX_MSG_SIZE, &fmu);
    free(copy);

    // Print the final message
    if (!instanceName) instanceName = "?";
    if (!category) category = "?";
    printf("%s %s (%s): %s\n", fmiStatusToString(status), instanceName, category, msg);
}

static int error(const char *message)
{
    printf("%s\n", message);
    return -1;
}

////////////////////////////////////////////////////////////////
///  This method returns the fmi version number.
///
///\param fmuWorkingFolder Path to the FMU working directory
///\param sizefmuWorkingFolder Size of FMU working directory
///\param modelID FMU modelID
///\param sizemodelID Size of FMU modelID
///\param fmiVersionNumber FMI version number
////////////////////////////////////////////////////////////////
fmiInteger getfmiEPlusVersion(char *fmuWorkingFolder, fmiInteger *sizefmuWorkingFolder, char *fmiVersionNumber, fmiInteger *index)
{

    char *trimfmuWorkingFolder;
    fmiString verID;
    // char* trimmodelID;
    char *err_msg = "Check FMU binaries folder and see whether libraries "
                    "exist for the system architecture of the EnergyPlus version used. Also check whether the FMU has "
                    "been exported for Co-Simulation. FMU for Model Exchange is not supported yet.";

    // allocate memory for the FMU-working folder trimmed
    trimfmuWorkingFolder = (char *)calloc(*sizefmuWorkingFolder + 1, sizeof(char));
    // write fmuWorkingFolder withouth blanks
    strncpy(trimfmuWorkingFolder, fmuWorkingFolder, *sizefmuWorkingFolder);

    // get index value
    _c->index = *index;

    // load lib by specifying path to the binaries
    if (loadLib(trimfmuWorkingFolder, fmuInstances[_c->index]->modelID, fmuInstances[_c->index])) {
        memcpy(fmiVersionNumber, err_msg, strlen(err_msg));
        return -1;
    }

    // free trimmed working folder
    free(trimfmuWorkingFolder);

    // get the modelID of the FMU
    verID = fmuInstances[_c->index]->getVersion();
    memcpy(fmiVersionNumber, verID, strlen(verID) + 1);

    return 0;
}

////////////////////////////////////////////////////////////////
///  This method is an interface to the function in Fortran
///  needed to instantiate an FMU. It returns a pointer to the
///  instantiate FMU.
///
///\param fmuResFolder Path to the FMU resources folder
///\param sizefmuResFolder Size of FMU resources folder
///\param timeOut communication timeout value in milli-seconds
///\param visible Flag to executes the FMU in windowless mode
///\param interactive Flag to execute the FMU in interactive mode
///\param loggingOn Flag to enable or disable debug
///\param index Index of FMU
////////////////////////////////////////////////////////////////
fmiComponent fmiEPlusInstantiateSlave(char *fmuResFolder,
                                      fmiInteger *sizefmuResFolder,
                                      fmiReal *timeOut,
                                      fmiInteger *visible,
                                      fmiInteger *interactive,
                                      fmiInteger *loggingOn,
                                      fmiInteger *index)
{

    char *trimfmuResFolder;

    fmiBoolean visiBoolean;
    fmiBoolean interactBoolean;
    fmiBoolean loggOnBoolean;
    fmiComponent fmuInstance;

    // define callbacks functions
    fmiCallbackFunctions callbacks;
    callbacks.allocateMemory = calloc;
    callbacks.freeMemory = free;
    callbacks.logger = fmuLogger;

    // allocate memory for fmu resources folder
    trimfmuResFolder = (char *)calloc(*sizefmuResFolder + 1, sizeof(char));

    // write fmuResFolder without blanks
    strncpy(trimfmuResFolder, fmuResFolder, *sizefmuResFolder);

    // get index value
    _c->index = *index;

    // map input to fmiBoolean variables
    if (*visible == 0)
        visiBoolean = fmiFalse;
    else
        visiBoolean = fmiTrue;

    if (*interactive == 0)
        interactBoolean = fmiFalse;
    else
        interactBoolean = fmiTrue;

    if (*loggingOn == 0)
        loggOnBoolean = fmiFalse;
    else
        loggOnBoolean = fmiTrue;

    // instantiate FMU
    fmuInstance = fmuInstances[_c->index]->instantiateSlave(fmuInstances[_c->index]->instanceName,
                                                            fmuInstances[_c->index]->modelGUID,
                                                            trimfmuResFolder,
                                                            "",
                                                            *timeOut,
                                                            visiBoolean,
                                                            interactBoolean,
                                                            callbacks,
                                                            loggOnBoolean);
    if (!fmuInstance) {
        printf("Error: failed to instantiate slave in fmiEPlusInstantiateSlave.\n");
        return NULL;
    }

    // free trimfmuResFolder
    free(trimfmuResFolder);
    return fmuInstance;
}

////////////////////////////////////////////////////////////////
///  This method is an interface to the function in Fortran
///  needed to initialize an FMU. It returns an integer-value
///  which indicates whether the initialization was successful
///  or not.
///
///\param fmuInstance FMU instance
///\param tStart Simulation starttime
///\param newStep Flag to accept or reject timestep
///\param tStop Simulation endtime
///\param index Index of FMU
////////////////////////////////////////////////////////////////
fmiStatus fmiEPlusInitializeSlave(fmiComponent *fmuInstance, fmiReal *tStart, fmiInteger *newStep, fmiReal *tStop, fmiInteger *index)
{

    fmiStatus fmiFlag;
    fmiBoolean newStepBoolean;

    // map the newStep to fmiBoolean value
    if (*newStep == 0)
        newStepBoolean = fmiFalse;
    else
        newStepBoolean = fmiTrue;

    // get index value
    _c->index = *index;

    // initialize fmu;
    fmiFlag = fmuInstances[_c->index]->initializeSlave(*fmuInstance, *tStart, newStepBoolean, *tStop);
    if (fmiFlag > fmiWarning) {
        printf("Error: failed to initialize slave in fmiEPlusInitializeSlave!\n");
        return fmiError;
    }
    return fmiFlag;
}

////////////////////////////////////////////////////////////////
///  This method is an interface to the function in Fortran
///  needed to get real variables from the FMU.
///
///\param fmuInstance FMU instance
///\param valRef FMU value references
///\param outValue FMU output values
///\param numOutputs Number of FMU outputs
///\param index Index of FMU
////////////////////////////////////////////////////////////////

fmiStatus fmiEPlusGetReal(fmiComponent *fmuInstance, const fmiValueReference valRef[], fmiReal outValue[], fmiInteger *numOutputs, fmiInteger *index)
{

    fmiStatus fmiFlag;

    // get index value
    _c->index = *index;

    fmiFlag = fmuInstances[_c->index]->getReal(*fmuInstance, valRef, *numOutputs, outValue);
    if (fmiFlag > fmiWarning) {
        printf("Error: failed to get all outputs in fmiEPlusGetReal.\n");
        return fmiError;
    }

    return fmiFlag;
}

////////////////////////////////////////////////////////////////
///  This method is an interface to the function in Fortran
///  needed to set real variables to the FMU. It returns an
///  integer which indicates whether input has been set or not.
///

///\param fmuInstance FMU-instance
///\param valRef FMU value references
///\param inpVal FMU input values
///\param numOutputs Number of FMU inputs
///\param index Index of FMU
////////////////////////////////////////////////////////////////
fmiStatus fmiEPlusSetReal(fmiComponent *fmuInstance, const fmiValueReference valRef[], fmiReal inpVal[], fmiInteger *numInputs, fmiInteger *index)
{

    fmiStatus fmiFlag;

    // get index value
    _c->index = *index;

    fmiFlag = fmuInstances[_c->index]->setReal(*fmuInstance, valRef, *numInputs, inpVal);
    if (fmiFlag > fmiWarning) {
        printf("Error: failed to set all inputs in fmiEPlusSetReal.\n");
        return fmiError;
    }
    return fmiFlag;
}

////////////////////////////////////////////////////////////////
///  This method is an interface to the function in Fortran
///  needed to do the step in FMU. It returns an integer
///  which indicates whether the step was successfully done.
///
///\param fmuInstance FMU instance
///\param curCommPoint fmiEPlusDoStep Current communication intervall
///\param newStep Flag to accept or reject step
///\param commStepSize Size of communication timestep
///\param index Index of FMU
////////////////////////////////////////////////////////////////
fmiStatus fmiEPlusDoStep(fmiComponent *fmuInstance, fmiReal *curCommPoint, fmiReal *commStepSize, fmiInteger *newStep, fmiInteger *index)
{

    fmiBoolean newStepBoolean;
    fmiStatus fmiFlag;

    // map the newStep to the fmiBoolean value
    if (*newStep == 0)
        newStepBoolean = fmiFalse;
    else
        newStepBoolean = fmiTrue;

    // get index value
    _c->index = *index;

    fmiFlag = fmuInstances[_c->index]->doStep(*fmuInstance, *curCommPoint, *commStepSize, newStepBoolean);
    if (fmiFlag > fmiWarning) {
        printf("Error: failed to do Step in fmiEPlusDoStep.\n");
        return fmiError;
    }
    return fmiFlag;
}

////////////////////////////////////////////////////////////////
///  This method is an interface to the function in Fortran
///  needed to free FMU. It returns an integer
///  which indicates whether the FMU was successfully freed.
///
///\param fmuInstance FMU instance
///\param index Index of FMU
///\param fmiEndSimulation Parameter to indicate end of simulation
////////////////////////////////////////////////////////////////

fmiStatus fmiEPlusFreeSlave(fmiComponent *fmuInstance, fmiInteger *index, fmiInteger *fmiEndSimulation)
{

    // get index value
    _c->index = *index;

    // free slave in fmu
    // Freeing doesn't work with Dymola.
    fmuInstances[_c->index]->freeSlaveInstance(*fmuInstance);
    if (*fmiEndSimulation == 1 && fmuInstances != NULL) {
        if (fmuLocDecoun == fmuLocCoun) {
            // Deallocate memory when simulation terminates
            free(fmuInstances);
        }
        fmuLocDecoun++;
    }
    return 0;
}

////////////////////////////////////////////////////////////////
///  This method is an interface to the function in Fortran
///  needed to reset FMU. It returns an integer
///  which indicates whether the FMU was successfully reseted.
///
///\param fmuInstance FMU instance
///\param index Index of FMU
////////////////////////////////////////////////////////////////
fmiStatus fmiEPlusResetSlave(fmiComponent *fmuInstance, fmiInteger *index)
{

    fmiStatus fmiFlag;

    // get index value
    _c->index = *index;

    // reset slave in fmu
    fmiFlag = fmuInstances[_c->index]->resetSlaveInstance(*fmuInstance);
    if (fmiFlag > fmiWarning) {
        printf("Error: failed to reset FMU instance in fmiEPlusResetSlave.\n");
        return fmiWarning;
    }
    return fmiFlag;
}

////////////////////////////////////////////////////////////////
///  This method is an interface to the function in Fortran
///  needed to get value reference of FMU- input variable by Name.
///  It returns the value reference of the given variable.
///
///\param variableName FMU variable name
///\param sizeVariableName Size of FMU-variable name
///\param index Index of FMU
////////////////////////////////////////////////////////////////
fmiValueReference getValueReferenceByNameFMUInputVariables(char *variableName, fmiInteger *sizeVariableName, fmiInteger *index)
{

    char *trimVariableName;
    fmiValueReference valueRef;

    // allocate memory for the FMU-variable trimmed
    trimVariableName = (char *)calloc(*sizeVariableName + 1, sizeof(char));

    // write FMU-variable withouth blanks
    strncpy(trimVariableName, variableName, *sizeVariableName);

    // get index value
    _c->index = *index;

    // check whether the variable exists in the FMU
    if (getVariableByName(fmuInstances[_c->index]->modelDescription, trimVariableName) == NULL) {
        printf("Error: get variable by name failed in fmigetValueReferenceByName. "
               "Please check input variables and modelDescription file again.");
        return -1;
    } else {
        valueRef = getValueReference(getVariableByName(fmuInstances[_c->index]->modelDescription, trimVariableName));
        if (!valueRef) {
            printf("Error: could not get value by reference in fmigetValueReferenceByName. "
                   "Please check input variables and modelDescription file again.");
            return -999;
        }
        if (getCausality(getVariableByName(fmuInstances[_c->index]->modelDescription, trimVariableName)) != enu_input) {
            printf("Error: This is not an FMU input variable. "
                   "Please check input variables and modelDescription file again.");
            return -1;
        }
    }

    // deallocate memory FMU-variable name trimmed
    free(trimVariableName);
    return valueRef;
}

////////////////////////////////////////////////////////////////
///  This method is an interface to the function in Fortran
///  needed to get value reference of FMU output variable by Name.
///  It returns the value reference of the given variable.
///
///\param variableName FMU variable name
///\param sizeVariableName Size of FMU-variable name
///\param index Index of FMU
////////////////////////////////////////////////////////////////
fmiValueReference getValueReferenceByNameFMUOutputVariables(char *variableName, fmiInteger *sizeVariableName, fmiInteger *index)
{

    char *trimVariableName;
    fmiValueReference valueRef;

    // allocate memory for the FMU-variable trimmed
    trimVariableName = (char *)calloc(*sizeVariableName + 1, sizeof(char));

    // write FMU-variable withouth blanks
    strncpy(trimVariableName, variableName, *sizeVariableName);

    // get index value
    _c->index = *index;

    if (getVariableByName(fmuInstances[_c->index]->modelDescription, trimVariableName) == NULL) {
        printf("Error: get variable by name failed in fmigetValueReferenceByName. "
               "Please check output variables and modelDescription file again.");
        return -1;
    } else {
        valueRef = getValueReference(getVariableByName(fmuInstances[_c->index]->modelDescription, trimVariableName));
        if (!valueRef) {
            printf("Error: could not get value by reference in fmigetValueReferenceByName. "
                   "Please check output variables and modelDescription file again.");
            return -999;
        }
        if (getCausality(getVariableByName(fmuInstances[_c->index]->modelDescription, trimVariableName)) != enu_output) {
            printf("Error: This is not an FMU output variable. "
                   "Please check output variables and modelDescription file again.");
            return -1;
        }
    }

    // deallocate memory FMU-variable name trimmed
    free(trimVariableName);
    return valueRef;
}

////////////////////////////////////////////////////////////////
///  This method is an interface to the function in Fortran
///  needed to add //fmus/ to the current working folder
///  of EnergyPlus . It returns an integer
///  which indicates whether the addition was successfully done.
///
///\param fmuWorkingFolder FMU output working folder
///\param fmuWorkingFolder FMU working folder
///\param sizefmuWorkingFolder Size of FMU working folder
////////////////////////////////////////////////////////////////

fmiInteger addFMURootFolderName(char *fmuOutputWorkingFolder, char *fmuWorkingFolder, fmiInteger *sizefmuWorkingFolder)
{
    char *trimfmuWorkingFolder;
    char *trimfmuWorkingFolderWithRoot;

    // allocate memory for the FMU-working folder trimmed
    trimfmuWorkingFolder = (char *)calloc(*sizefmuWorkingFolder + 1, sizeof(char));

    // write fmuWorkingFolder withouth blanks
    strncpy(trimfmuWorkingFolder, fmuWorkingFolder, *sizefmuWorkingFolder);

    trimfmuWorkingFolderWithRoot = (char *)calloc(*sizefmuWorkingFolder + strlen(FMU_ROOT_DIR) + 1, sizeof(char));
    sprintf(trimfmuWorkingFolderWithRoot, "%s%s", trimfmuWorkingFolder, FMU_ROOT_DIR);

    // write fmuOutputWorkingFolder
    strncpy(fmuOutputWorkingFolder, trimfmuWorkingFolderWithRoot, *sizefmuWorkingFolder + strlen(FMU_ROOT_DIR) + 1);

    // deallocate memory FMU-working folder trimmed
    free(trimfmuWorkingFolder);
    // deallocate memory FMU-working folder with root
    free(trimfmuWorkingFolderWithRoot);
    return 0;
}

////////////////////////////////////////////////////////////////
///  This method is an interface to the function in Fortran
///  needed to get the path to the binaries of FMu instance
///  It returns an integer
///  which indicates whether the routine completes successfully.
///
///\param trimfmuOutputWorkingFolder_wLiB FMU Output working folder
/// with path to binaries
///\param fmuWorkingFolder FMU working folder
///\param sizefmuWorkingFolder Size of FMU working folder
///\param index Index of FMU
////////////////////////////////////////////////////////////////

fmiInteger
addLibPathCurrentWorkingFolder(char *trimfmuOutputWorkingFolder_wLiB, char *fmuWorkingFolder, fmiInteger *sizefmuWorkingFolder, fmiInteger *index)
{
    char *trimfmuWorkingFolder;
    char *librPath_w32;
    char *librPath_w64;
    struct stat st;
    fmiInteger len_LibPath;
    fmiBoolean bRes_w32;
    fmiBoolean bRes_w64;
#if __linux__
    char *librPath_l32;
    char *librPath_l64;
    fmiBoolean bRes_l32;
    fmiBoolean bRes_l64;
#endif

    // allocate memory for the FMU-working folder trimmed
    trimfmuWorkingFolder = (char *)calloc(*sizefmuWorkingFolder + 1, sizeof(char));

    // write fmuWorkingFolder withouth blanks
    memcpy(trimfmuWorkingFolder, fmuWorkingFolder, *sizefmuWorkingFolder);

    // get index value
    _c->index = *index;

#ifdef _MSC_VER
    len_LibPath = (fmiInteger)(strlen(trimfmuWorkingFolder) + strlen(BIN_WIN) + strlen(fmuInstances[_c->index]->modelID) + strlen(LIB_EXT));
    librPath_w32 = (char *)calloc(len_LibPath + 1, sizeof(char));

    // write the path to the binaries for Windows 32 bit
    sprintf(librPath_w32, "%s%s%s%s", trimfmuWorkingFolder, BIN_WIN32, fmuInstances[_c->index]->modelID, LIB_EXT);
    // check whether the dlls for Windows 32 bit exist in the binaries folder
    bRes_w32 = (stat(librPath_w32, &st) == 0);

    // write the path to the binaries for Windows 64 bit
    librPath_w64 = (char *)calloc(len_LibPath + 1, sizeof(char));
    sprintf(librPath_w64, "%s%s%s%s", trimfmuWorkingFolder, BIN_WIN64, fmuInstances[_c->index]->modelID, LIB_EXT);
    // check whether the dlls for Windows 32 bit exist in the binaries folder
    bRes_w64 = (stat(librPath_w64, &st) == 0);

    // check whether we have folders for Windows 32 and Windows 64 bit
    if (bRes_w32 && bRes_w64) {
        if (OperSys == 1) {
            memcpy(trimfmuOutputWorkingFolder_wLiB, librPath_w32, strlen(librPath_w32));
        } else {
            memcpy(trimfmuOutputWorkingFolder_wLiB, librPath_w64, strlen(librPath_w64));
        }
    }
    // check whether we just have folder for Windows 32 bit
    else if (bRes_w32 && !bRes_w64) {
        memcpy(trimfmuOutputWorkingFolder_wLiB, librPath_w32, strlen(librPath_w32));
    }
    // check whether we just have folder for Windows 64 bit
    else if (!bRes_w32 && bRes_w64) {
        memcpy(trimfmuOutputWorkingFolder_wLiB, librPath_w64, strlen(librPath_w64));
    } else {
        printf("Error: FMU does not contain binaries folder for this operating system.");
        free(trimfmuWorkingFolder);
        free(librPath_w32);
        free(librPath_w64);
        return -1;
    }
#endif

#if __linux__

    len_LibPath = strlen(trimfmuWorkingFolder) + strlen(BIN_LIN) + strlen(fmuInstances[_c->index]->modelID) + strlen(LIB_EXT);
    librPath_l32 = (char *)calloc(len_LibPath + 10, sizeof(char));

    // write the path to the binaries for Windows 32 bit
    sprintf(librPath_l32, "%s%s%s%s", trimfmuWorkingFolder, BIN_LIN32, fmuInstances[_c->index]->modelID, LIB_EXT);
    // check whether the so for Linux 32 bit exist in the binaries folder
    bRes_l32 = (stat(librPath_l32, &st) == 0);

    // write the path to the binaries for Windows 64 bit
    librPath_l64 = (char *)calloc(len_LibPath + 10, sizeof(char));
    sprintf(librPath_l64, "%s%s%s%s", trimfmuWorkingFolder, BIN_LIN64, fmuInstances[_c->index]->modelID, LIB_EXT);
    // check whether the so for Linux 64 bit exist in the binaries folder
    bRes_l64 = (stat(librPath_l64, &st) == 0);

    // check whether we have folders for Linux 32 and Linux 64 bit
    if (bRes_l32 && bRes_l64) {
        if (OperSys == 3) {
            memcpy(trimfmuOutputWorkingFolder_wLiB, librPath_l32, strlen(librPath_l32));
        } else {
            memcpy(trimfmuOutputWorkingFolder_wLiB, librPath_l64, strlen(librPath_l64));
        }
    }
    // check whether we just have folder for Linux 32 bit
    else if (bRes_l32 && !bRes_l64) {
        memcpy(trimfmuOutputWorkingFolder_wLiB, librPath_l32, strlen(librPath_l32));
    }
    // check whether we just have folder for Linux 64 bit
    else if (!bRes_l32 && bRes_l64) {
        memcpy(trimfmuOutputWorkingFolder_wLiB, librPath_l64, strlen(librPath_l64));
    } else {
        printf("Error: FMU does not contain binaries folder for this operating system.");
        free(trimfmuWorkingFolder);
        free(librPath_l32);
        free(librPath_l64);
        return -1;
    }
#endif

    // deallocate memory FMU-working folder trimmed
    free(trimfmuWorkingFolder);

    if (WINDOWS) {
        // deallocate memory for path for binaries for windows 32 bit
        free(librPath_w32);
        // deallocate memory for path for binaries for windows 64 bit
        free(librPath_w64);
    }

#if __linux__
    // deallocate memory for path for binaries for windows 32 bit
    free(librPath_l32);
    // deallocate memory for path for binaries for windows 64 bit
    free(librPath_l64);
#endif
    return 0;
}

////////////////////////////////////////////////////////////////
///  This method is an interface to the function in Fortran
///  needed to get the modelID and the modelGUID of the FMU.
///  It returns an integer
///  which indicates whether the routine was successfully.
///
///\param fmuInstanceName FMU instance name
///\param fmuWorkingFolder FMU output working folder
///\param sizefmuWorkingFolder Size of FMU working folder
///\param modelID FMU modelID
///\param modelGUID FMU modelGUID
///\param numInputs Number of FMU inputs
///\param numOutputs Number of FMU outputs
////////////////////////////////////////////////////////////////

fmiInteger
model_ID_GUID(char *fmuInstanceName, char *fmuWorkingFolder, fmiInteger *sizefmuWorkingFolder, fmiInteger *numInputs, fmiInteger *numOutputs)
{

    char *xmlPath;
    char *trimfmuWorkingFolder;
    void **list;
    int num_input = 0;
    int num_output = 0;
    int i, j;

    c = (fmiComponent)calloc(1, sizeof(struct FMU));
    _c = (FMU *)c;

    _c->index = insNum;
    addfmuInstances(_c);
    insNum++;
    // retValIns=insNum;

    // allocate memory for the FMU-working folder trimmed
    trimfmuWorkingFolder = (char *)calloc(*sizefmuWorkingFolder + 1, sizeof(char));
    // write fmuWorkingFolder withouth blanks
    strncpy(trimfmuWorkingFolder, fmuWorkingFolder, *sizefmuWorkingFolder);

    // allocate memory for xmlPath
    xmlPath = (char *)calloc(sizeof(char), strlen(trimfmuWorkingFolder) + strlen(XML_FILE) + 1);
    // write path to the FMU
    sprintf(xmlPath, "%s%s", trimfmuWorkingFolder, XML_FILE);

    // parse the xml-file and getModelDescription
    fmuInstances[_c->index]->modelDescription = parse(xmlPath);

    // Initialize instance Name
    fmuInstances[_c->index]->instanceName = (char *)calloc(strlen(fmuInstanceName) + 1, sizeof(char));

    // write fmu instance Name
    strcpy(fmuInstances[_c->index]->instanceName, fmuInstanceName);

    // check whether modelDescription exists or not
    if (!fmuInstances[_c->index]->modelDescription) {
        printf("Error: failed to get the modelDescription in fmiGetModelID.\n");
        free(trimfmuWorkingFolder);
        return -1;
    }
    // get the modelID of the FMU
    fmuInstances[_c->index]->modelID = getModelIdentifier(fmuInstances[_c->index]->modelDescription);

    if (!fmuInstances[_c->index]->modelID) {
        printf("Error: failed to get modelID in fmiGetModelID.\n");
        free(trimfmuWorkingFolder);
        return -1;
    }

    // get the model GUID of the FMU
    fmuInstances[_c->index]->modelGUID = getString(fmuInstances[_c->index]->modelDescription, att_guid);

    if (!fmuInstances[_c->index]->modelGUID) {
        printf("Error: failed to get modelGUID in fmiGetModelGUID.\n");
        free(trimfmuWorkingFolder);
        return -1;
    }

    for (i = 0; i < fmuInstances[_c->index]->modelDescription->n; i += 2)
        if (!strcmp(fmuInstances[_c->index]->modelDescription->attributes[i], "modelIdentifier"))
            ;

    list = (void **)fmuInstances[_c->index]->modelDescription->modelVariables;
    if (list)
        for (j = 0; list[j]; j++) {
            Element *e = (Element *)list[j];
            Enu val = enu_none;

            for (i = 0; i < e->n; i += 2) {
                if (!strcmp(e->attributes[i], "causality")) val = checkEnumValue2(e->attributes[i + 1]);
            }

            // get number of input variables
            if (val == enu_input) {
                num_input = num_input + 1;
            } else if (val == enu_output) {
                num_output = num_output + 1;
            }
        }

    // assign number of inputs found
    *numInputs = num_input;
    // assign number of output founds
    *numOutputs = num_output;

    // deallocate xmlPath
    free(xmlPath);
    // free trimmed working folder
    free(trimfmuWorkingFolder);
    return insNum - 1;
}

// This function is an interface to the function in Fortran that is used to parse the FMU
////////////////////////////////////////////////////////////////
///  This method is an interface to the function in Fortran
///  needed to unpack FMUs. It returns an integer
///  which indicates whether the FMU was successfully unzipped.
///
///\param fmuName FMU fileName
///\param fmuOutputWorkingFolder FMU working folder
///\param sizefmuName Size of FMU name trimmed
///\param sizefmuOutputWorkingFolder Size of FMU working folder trimmed
////////////////////////////////////////////////////////////////
fmiInteger fmiEPlusUnpack(char *fmuName, char *fmuOutputWorkingFolder, fmiInteger *sizefmuName, fmiInteger *sizefmuOutputWorkingFolder)
{

    char *trimfmuOutputWorkingFolder;
    char *trimfmuName;
    int retVal;

    // allocate memory for the FMU-Name trimmed
    trimfmuName = (char *)calloc(*sizefmuName + 1, sizeof(char));

    // allocate memory for the FMU-working folder trimmed
    trimfmuOutputWorkingFolder = (char *)calloc(*sizefmuOutputWorkingFolder + 1, sizeof(char));

    // write fmuName withouth blanks
    strncpy(trimfmuName, fmuName, *sizefmuName);

    // write fmuWorkingFolder withouth blanks
    strncpy(trimfmuOutputWorkingFolder, fmuOutputWorkingFolder, *sizefmuOutputWorkingFolder);

    // unpack FMU in the working folder
    retVal = unpackminizip(trimfmuName, trimfmuOutputWorkingFolder);

    if (retVal != 0) {
        printf("Error: failed to unpack FMU in fmiEPlusUnpack.\n");
        return -1;
    }

    // deallocate memory FMU-Name
    free(trimfmuName);
    // deallocate memory FMU-working folder
    free(trimfmuOutputWorkingFolder);
    return 0;
}

#else
fmiComponent fmiEPlusInstantiateSlave(char *fmuResFolder,
                                      fmiInteger *sizefmuResFolder,
                                      fmiReal *timeOut,
                                      fmiInteger *visible,
                                      fmiInteger *interactive,
                                      fmiInteger *loggingOn,
                                      fmiInteger *index)
{
    (void)fmuResFolder;
    (void)sizefmuResFolder;
    (void)timeOut;
    (void)visible;
    (void)interactive;
    (void)loggingOn;
    (void)index;
    printf("Error: FunctionalMock-up Unit for co-simulation is currently only supported on Windows and Linux.");
    exit(EXIT_FAILURE);
}

fmiStatus fmiEPlusInitializeSlave(fmiComponent *fmuInstance, fmiReal *tStart, fmiInteger *newStep, fmiReal *tStop, fmiInteger *index)
{
    (void)fmuInstance;
    (void)tStart;
    (void)newStep;
    (void)tStop;
    (void)index;
    printf("Error: FunctionalMock-up Unit for co-simulation is currently only supported on Windows and Linux.");
    exit(EXIT_FAILURE);
}

fmiStatus fmiEPlusGetReal(fmiComponent *fmuInstance, const fmiValueReference valRef[], fmiReal outValue[], fmiInteger *numOutputs, fmiInteger *index)
{
    (void)fmuInstance;
    (void)valRef;
    (void)outValue;
    (void)numOutputs;
    (void)index;
    printf("Error: FunctionalMock-up Unit for co-simulation is currently only supported on Windows and Linux.");
    exit(EXIT_FAILURE);
}

fmiStatus fmiEPlusSetReal(fmiComponent *fmuInstance, const fmiValueReference valRef[], fmiReal inpVal[], fmiInteger *numInputs, fmiInteger *index)
{
    (void)fmuInstance;
    (void)valRef;
    (void)inpVal;
    (void)numInputs;
    (void)index;
    printf("Error: FunctionalMock-up Unit for co-simulation is currently only supported on Windows and Linux.");
    exit(EXIT_FAILURE);
}

fmiStatus fmiEPlusDoStep(fmiComponent *fmuInstance, fmiReal *curCommPoint, fmiReal *commStepSize, fmiInteger *newStep, fmiInteger *index)
{
    (void)fmuInstance;
    (void)curCommPoint;
    (void)commStepSize;
    (void)newStep;
    (void)index;
    printf("Error: FunctionalMock-up Unit for co-simulation is currently only supported on Windows and Linux.");
    exit(EXIT_FAILURE);
}

fmiStatus fmiEPlusFreeSlave(fmiComponent *fmuInstance, fmiInteger *index, fmiInteger *fmiEndSimulation)
{
    (void)fmuInstance;
    (void)index;
    (void)fmiEndSimulation;
    printf("Error: FunctionalMock-up Unit for co-simulation is currently only supported on Windows and Linux.");
    exit(EXIT_FAILURE);
}

fmiStatus fmiEPlusResetSlave(fmiComponent *fmuInstance, fmiInteger *index)
{
    (void)fmuInstance;
    (void)index;
    printf("Error: FunctionalMock-up Unit for co-simulation is currently only supported on Windows and Linux.");
    exit(EXIT_FAILURE);
}

fmiInteger fmiEPlusUnpack(char *fmuName, char *fmuOutputWorkingFolder, fmiInteger *sizefmuName, fmiInteger *sizefmuOutputWorkingFolder)
{
    (void)fmuName;
    (void)fmuOutputWorkingFolder;
    (void)sizefmuName;
    (void)sizefmuOutputWorkingFolder;
    printf("Error: FunctionalMock-up Unit for co-simulation is currently only supported on Windows and Linux.");
    exit(EXIT_FAILURE);
}

fmiValueReference getValueReferenceByNameFMUInputVariables(char *variableName, fmiInteger *sizeVariableName, fmiInteger *index)
{
    (void)variableName;
    (void)sizeVariableName;
    (void)index;
    printf("Error: FunctionalMock-up Unit for co-simulation is currently only supported on Windows and Linux.");
    exit(EXIT_FAILURE);
}

fmiValueReference getValueReferenceByNameFMUOutputVariables(char *variableName, fmiInteger *sizeVariableName, fmiInteger *index)
{
    (void)variableName;
    (void)sizeVariableName;
    (void)index;
    printf("Error: FunctionalMock-up Unit for co-simulation is currently only supported on Windows and Linux.");
    exit(EXIT_FAILURE);
}

fmiInteger
model_ID_GUID(char *fmuInstanceName, char *fmuWorkingFolder, fmiInteger *sizefmuWorkingFolder, fmiInteger *numInputs, fmiInteger *numOutputs)
{
    (void)fmuInstanceName;
    (void)fmuWorkingFolder;
    (void)sizefmuWorkingFolder;
    (void)numInputs;
    (void)numOutputs;
    printf("Error: FunctionalMock-up Unit for co-simulation is currently only supported on Windows and Linux.");
    exit(EXIT_FAILURE);
}

fmiInteger addFMURootFolderName(char *fmuOutputWorkingFolder, char *fmuWorkingFolder, fmiInteger *sizefmuWorkingFolder)
{
    (void)fmuOutputWorkingFolder;
    (void)fmuWorkingFolder;
    (void)sizefmuWorkingFolder;
    printf("Error: FunctionalMock-up Unit for co-simulation is currently only supported on Windows and Linux.");
    exit(EXIT_FAILURE);
}

fmiInteger getfmiEPlusVersion(char *fmuWorkingFolder, fmiInteger *sizefmuWorkingFolder, char *fmiVersionNumber, fmiInteger *index)
{
    (void)fmuWorkingFolder;
    (void)sizefmuWorkingFolder;
    (void)fmiVersionNumber;
    (void)index;
    printf("Error: FunctionalMock-up Unit for co-simulation is currently only supported on Windows and Linux.");
    exit(EXIT_FAILURE);
}

fmiInteger
addLibPathCurrentWorkingFolder(char *trimfmuOutputWorkingFolder_wLiB, char *fmuWorkingFolder, fmiInteger *sizefmuWorkingFolder, fmiInteger *index)
{
    (void)trimfmuOutputWorkingFolder_wLiB;
    (void)fmuWorkingFolder;
    (void)sizefmuWorkingFolder;
    (void)index;
    printf("Error: FunctionalMock-up Unit for co-simulation is currently only supported on Windows and Linux.");
    exit(EXIT_FAILURE);
}
#endif

//
// int main ()
//{
//	fmiInteger len1 = 8;
//	fmiInteger len2 = 10;
//	fmiInteger len3 = 100;
//	fmiInteger index;
//	fmiInteger input;
//	fmiInteger output;
//	fmiString version;
//	char libFolder [100]={0};
//	int retVal;
//	fmiEPlusUnpack ("test.fmu", "c:\\temp", &len1, &len2);
//	index = model_ID_GUID ("c:\\temp", &len2, &input, &output);
//	printf ("This is the number of inputs %d\n", input);
//	printf ("This is the number of output %d\n", output);
//	printf ("This is the index %d\n", index);
//
//	addLibPathCurrentWorkingFolder (libFolder, "c:\\temp", &len2, &index);
//
//	printf ("This is the libfolder %s\n", libFolder);
//	retVal = getfmiEPlusVersion (libFolder, &len3, version, &index);
//
//}
