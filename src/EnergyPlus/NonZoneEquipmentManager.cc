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

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NonZoneEquipmentManager.hh>
#include <EnergyPlus/WaterThermalTanks.hh>
#include <EnergyPlus/WaterUse.hh>

namespace EnergyPlus {

namespace NonZoneEquipmentManager {

    // MODULE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   January 2004
    //       MODIFIED       Hudson, ORNL July 2007
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:

    // METHODOLOGY EMPLOYED: na

    // REFERENCES: na
    // OTHER NOTES: na
    // USE STATEMENTS: na

    // Data
    // MODULE PARAMETER DEFINITIONS: na
    // MODULE VARIABLE DECLARATIONS: na

    // SUBROUTINE SPECIFICATIONS:

    // MODULE SUBROUTINES:

    // Functions

    void ManageNonZoneEquipment(EnergyPlusData &state,
                                bool const FirstHVACIteration,
                                bool &SimNonZoneEquipment // Simulation convergence flag
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher
        //       DATE WRITTEN   Sept. 2000
        //       RE-ENGINEERED  Richard Liesen
        //       DATE MODIFIED  February 2003
        //       MODIFIED       Hudson, ORNL July 2007
        //       MODIFIED       B. Griffith, NREL, April 2008,
        //                      added calls for just heat recovery part of chillers
        //       MODIFIED       Removed much for plant upgrade, 2011

        // PURPOSE OF THIS SUBROUTINE:
        // This routine checks the input file for any non-zone equipment objects and gets their input.
        // Zone equipment objects are generally triggered to "get input" when they are called for simulation
        // by the ZoneEquipmentManager because they are referenced by a Zone Equipment List.  In the case of
        // the NonZoneEquipmentManager, it does not yet have a list of non-zone equipment, so it must make
        // one here before it knows what to call for simulation.

        // Using/Aliasing
        using WaterThermalTanks::SimulateWaterHeaterStandAlone;
        using WaterUse::SimulateWaterUse;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        auto &CountNonZoneEquip = state.dataGlobal->CountNonZoneEquip;

        if (CountNonZoneEquip) {
            state.dataGlobal->NumOfWaterHeater = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "WaterHeater:Mixed") +
                                                 state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "WaterHeater:Stratified");
            CountNonZoneEquip = false;
        }

        SimulateWaterUse(state, FirstHVACIteration); // simulate non-plant loop water use.

        if (!state.dataGlobal->ZoneSizingCalc) {
            for (int WaterHeaterNum = 1; WaterHeaterNum <= state.dataGlobal->NumOfWaterHeater; ++WaterHeaterNum) {
                SimulateWaterHeaterStandAlone(state, WaterHeaterNum, FirstHVACIteration);
            }
        }

        if (FirstHVACIteration) {
            SimNonZoneEquipment = true;
        } else {
            SimNonZoneEquipment = false;
        }
    }

} // namespace NonZoneEquipmentManager

} // namespace EnergyPlus
