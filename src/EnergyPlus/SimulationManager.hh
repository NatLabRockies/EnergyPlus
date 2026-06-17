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

#ifndef SimulationManager_hh_INCLUDED
#define SimulationManager_hh_INCLUDED

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/FileSystem.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace SimulationManager {

    void ManageSimulation(EnergyPlusData &state);

    void GetProjectData(EnergyPlusData &state);

    void writeInitialPerfLogValues(EnergyPlusData &state, std::string const &currentOverrideModeValue);

    std::string bool_to_string(bool logical);

    void CheckForMisMatchedEnvironmentSpecifications(EnergyPlusData &state);

    void CheckForRequestedReporting(EnergyPlusData &state);

    std::unique_ptr<std::ostream>
    OpenStreamFile(EnergyPlusData &state, const fs::path &fileName, std::ios_base::openmode mode = (std::ios_base::out | std::ios_base::trunc));

    void OpenOutputFiles(EnergyPlusData &state);

    void CloseOutputFiles(EnergyPlusData &state);

    void SetupSimulation(EnergyPlusData &state, bool &ErrorsFound);

    void ReportNodeConnections(EnergyPlusData &state);

    void ReportLoopConnections(EnergyPlusData &state);

    void PostIPProcessing(EnergyPlusData &state);

    void isInputObjectUsed(EnergyPlusData &state);

} // namespace SimulationManager

struct SimulationManagerData : BaseGlobalStruct
{
    bool RunPeriodsInInput = false;
    bool RunControlInInput = false;
    bool PreP_Fatal = false;
    bool WarningOut = true;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state(EnergyPlusData &state) override
    {
        SimulationManager::OpenOutputFiles(state);
        SimulationManager::GetProjectData(state);
    }

    void clear_state() override
    {
        this->RunPeriodsInInput = false;
        this->RunControlInInput = false;
        this->PreP_Fatal = false;
        this->WarningOut = true;
    }
};

void Resimulate(EnergyPlusData &state,
                bool const ResimExt, // Flag to resimulate the exterior energy use simulation
                bool const ResimHB,  // Flag to resimulate the heat balance simulation (including HVAC)
                bool &ResimHVAC      // Flag to resimulate the HVAC simulation
);

} // namespace EnergyPlus

#endif
