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

#include <stdio.h>
#include <stdlib.h>

#include <EnergyPlus/api/datatransfer.h>
#include <EnergyPlus/api/runtime.h>
#include <EnergyPlus/api/state.h>

int outdoorDewPointActuator = -1;
int outdoorTempSensor = -1;
int outdoorDewPointSensor = -1;
int zone1_wall1Actuator = -1;
int wallConstruction = -1;
int floorConstruction = -1;
int handlesRetrieved = 0;

void afterZoneTimeStepHandler(EnergyPlusState state)
{
    printf("STARTING A NEW TIME STEP\n");
    if (handlesRetrieved == 0) {
        if (!apiDataFullyReady(state)) {
            return;
        }

        unsigned int arraySize;
        struct APIDataEntry *data = getAPIData(state, &arraySize); // inspect this to see what's available to exchange
        // FILE *fp = fopen("/tmp/data.csv", "w");
        // for (unsigned int a = 0; a < arraySize; a++) {
        //     const struct APIDataEntry d = *(data + a);
        //     fprintf(fp, "%s,%s,%s,%s,%s\n", d.what, d.name, d.key, d.type, d.unit);
        // }
        // fclose(fp);
        char **surfaceNames = getObjectNames(state, "BuildingSurface:Detailed", &arraySize);

        if (arraySize == 0) {
            printf("Encountered a file with no BuildingSurface:Detailed, can't run this script on that file! Aborting!\n");
            exit(1);
        }

        outdoorDewPointActuator = getActuatorHandle(state, "Weather Data", "Outdoor Dew Point", "Environment");
        outdoorTempSensor = getVariableHandle(state, "SITE OUTDOOR AIR DRYBULB TEMPERATURE", "ENVIRONMENT");
        outdoorDewPointSensor = getVariableHandle(state, "SITE OUTDOOR AIR DEWPOINT TEMPERATURE", "ENVIRONMENT");

        zone1_wall1Actuator = getActuatorHandle(state, "Surface", "Construction State", surfaceNames[0]);
        wallConstruction = getConstructionHandle(state, "R13WALL");
        floorConstruction = getConstructionHandle(state, "FLOOR");

        // checking for EMS globals
        int const emsGlobalValid = getEMSGlobalVariableHandle(state, "MaximumEffort");
        int const emsGlobalInvalid = getEMSGlobalVariableHandle(state, "4or5moments");
        int const emsGlobalBuiltIn = getEMSGlobalVariableHandle(state, "WARMUPFLAG");
        if (emsGlobalValid > 0 && emsGlobalInvalid == 0 && emsGlobalBuiltIn == 0) {
            printf("EMS Global handle lookups worked just fine!\n");
        } else {
            printf("EMS Global handle lookup failed.  Make sure to call this with _1ZoneUncontrolled_ForAPITesting.idf\n");
            exit(1);
        }
        setEMSGlobalVariableValue(state, emsGlobalValid, 2.0);
        Real64 const val = getEMSGlobalVariableValue(state, emsGlobalValid);
        if (val < 1.9999 || val > 2.0001) {
            printf("EMS Global assignment/lookup didn't seem to work, odd\n");
            exit(1);
        }

        freeAPIData(data, arraySize);
        freeObjectNames(surfaceNames, arraySize);

        printf("Got handles %d, %d, %d, %d, %d, %d\n",
               outdoorDewPointActuator,
               outdoorTempSensor,
               outdoorDewPointSensor,
               zone1_wall1Actuator,
               wallConstruction,
               floorConstruction);
        if (outdoorDewPointActuator == -1 || outdoorTempSensor == -1 || outdoorDewPointSensor == -1 || zone1_wall1Actuator == -1 ||
            wallConstruction == -1 || floorConstruction == -1) {
            exit(1);
        }

        char *idfPath = inputFilePath(state);
        char *epwPath = epwFilePath(state);
        printf("Got an input file path of: %s, and weather file path of: %s\n", idfPath, epwPath);
        free(idfPath);
        free(epwPath);

        handlesRetrieved = 1;
    }
    setActuatorValue(state, outdoorDewPointActuator, -25.0);
    Real64 oa_temp = getVariableValue(state, outdoorTempSensor);
    printf("Reading outdoor temp via getVariable, value is: %8.4f \n", oa_temp);
    Real64 dp_temp = getVariableValue(state, outdoorDewPointSensor);
    printf("Actuated Dew Point temp value is: %8.4f \n", dp_temp);
    Real64 simTime = currentSimTime(state);
    printf("Current Sim Time: %0.2f \n", simTime);
    const int epwYear = year(state);
    const int runPeriodYear = calendarYear(state);
    printf("year: %i, calendarYear: %i\n", epwYear, runPeriodYear);

    if (oa_temp > 10) {
        printf("Setting Zn001:Wall001 construction (%d) to R13WALL (%d)", zone1_wall1Actuator, wallConstruction);
        setActuatorValue(state, zone1_wall1Actuator, wallConstruction);
    } else {
        printf("Setting Zn001:Wall001 construction (%d) to FLOOR (%d)", zone1_wall1Actuator, floorConstruction);
        setActuatorValue(state, zone1_wall1Actuator, floorConstruction);
    }
}

int main(int argc, const char *argv[])
{
    EnergyPlusState state = stateNew();
    callbackEndOfZoneTimeStepAfterZoneReporting(state, afterZoneTimeStepHandler);
    requestVariable(state, "SITE OUTDOOR AIR DRYBULB TEMPERATURE", "ENVIRONMENT");
    requestVariable(state, "SITE OUTDOOR AIR DEWPOINT TEMPERATURE", "ENVIRONMENT");
    energyplus(state, argc, argv);
    if (handlesRetrieved == 0) {
        fprintf(stderr, "We never got ANY handles\n");
        return 1;
    }
    return 0;
}
