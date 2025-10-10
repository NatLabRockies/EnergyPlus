// EnergyPlus, Copyright (c) 1996-2025, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
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

#ifndef AirTerminalUnit_hh_INCLUDED
#define AirTerminalUnit_hh_INCLUDED

// C++ Headers
#include <string>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/ScheduleManager.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

// types of air terminal units, refactored from old DataDefineEquip
enum AirTerminalUnitType
{
    notYetDetermined,
    dualDuctConstVolume,
    dualDuctVAV,
    singleDuctVAVReheat,
    singleDuctConstVolReheat,
    singleDuctConstVolNoReheat,
    singleDuctVAVNoReheat,
    singleDuct_SeriesPIU_Reheat,
    singleDuct_ParallelPIU_Reheat,
    singleDuct_ConstVol_4PipeInduc,
    singleDuctVAVReheatVSFan,
    singleDuctCBVAVReheat,
    singleDuctCBVAVNoReheat,
    singleDuctConstVolCooledBeam,
    dualDuctVAVOutdoorAir,
    singleDuctUserDefined,
    singleDuctATMixer,
    singleDuctConstVolFourPipeBeam
};

// base class for all air distribution units.  zone air terminals for connecting to central air handlers
class AirTerminalUnit
{

public: // Methods
    virtual void simulate(EnergyPlusData &state,
                          bool FirstHVACIteration, // TRUE if first HVAC iteration in time step
                          Real64 &NonAirSysOutput  // convective cooling by the beam system [W]
                          ) = 0;

    virtual int getZoneIndex() = 0;

    virtual int getAirLoopNum() = 0;

    virtual Real64 getPrimAirDesignVolFlow() = 0;

    virtual int getTermUnitSizingIndex() = 0;

    virtual void reportTerminalUnit(EnergyPlusData &state) = 0;

protected:                                               // Data
    AirTerminalUnitType terminalType = notYetDetermined; // Type of air distribution unit  //Legacy For use during transition to OO
    std::string name;                                    // name of unit
    std::string unitType;                                // type of unit = e.g. AirTerminal:SingleDuct:ConstantVolume:FourPipeBeam
    int aDUNum = 0;                                      // index of this unit in the corresponding air distribution unit structure
    Sched::Schedule *airAvailSched = nullptr;            // schedule for pimary air availability
    bool airAvailable = false;                           // true if primary air is available
    Real64 vDotDesignPrimAir = 0.0;                      // Design primary air volume flow rate m3/s (autosizable)
    bool vDotDesignPrimAirWasAutosized = false;          // true if user input for design air flow was autsized on input
    Real64 mDotDesignPrimAir = 0.0;                      // Design primary air mass flow rate kg/s
    int airInNodeNum = 0;                                // unit air inlet system node number, air enters into air terminal unit
    int airOutNodeNum = 0;                               // unit air outlet system node number, air enters into zone from air terminal
    int zoneIndex = 0;                                   // zone index for this air terminal unit
    int zoneNodeIndex = 0;                               // index in node structure for the zone node for this air terminal
    int ctrlZoneInNodeIndex = 0;                         // which controlled zone inlet node number corresponds with this unit
    int airLoopNum = 0;                                  // index to airloop that this terminal unit is connected to
    int termUnitSizingNum = 0;                           // index to TermUnitSizing, TermUnitFinalZoneSizing, and more for this air distribution unit
}; // AirTerminalUnit

} // namespace EnergyPlus

#endif // AirTerminalUnit_hh_INCLUDED
