// EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the University of Illinois,
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

#ifndef PlantPressureSystem_hh_INCLUDED
#define PlantPressureSystem_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace PlantPressureSystem {

    void SimPressureDropSystem(
        EnergyPlusData &state,
        int LoopNum,                                                                    // Plant Loop to update pressure information
        bool FirstHVACIteration,                                                        // System flag
        DataPlant::PressureCall CallType,                                               // Enumerated call type
        DataPlant::LoopSideLocation LoopSideNum = DataPlant::LoopSideLocation::Invalid, // Loop side num for specific branch simulation
        ObjexxFCL::Optional_int_const BranchNum = _                                     // Branch num for specific branch simulation
    );

    void InitPressureDrop(EnergyPlusData &state, int LoopNum, bool FirstHVACIteration);

    void BranchPressureDrop(EnergyPlusData &state,
                            int LoopNum,                             // Plant Loop Index
                            DataPlant::LoopSideLocation LoopSideNum, // LoopSide on Plant Loop LoopNum
                            int BranchNum                            // Branch Index on LoopSide LoopSideNum
    );

    void UpdatePressureDrop(EnergyPlusData &state, int LoopNum);

    void DistributePressureOnBranch(EnergyPlusData &state,
                                    const int LoopNum,
                                    const DataPlant::LoopSideLocation LoopSideNum,
                                    const int BranchNum,
                                    Real64 &BranchPressureDrop,
                                    bool &PumpFound);

    void PassPressureAcrossMixer(EnergyPlusData &state,
                                 const int LoopNum,
                                 const DataPlant::LoopSideLocation LoopSideNum,
                                 const Real64 MixerPressure,
                                 const int NumBranchesOnLoopSide);

    void PassPressureAcrossSplitter(EnergyPlusData &state,
                                    const int LoopNum,
                                    const DataPlant::LoopSideLocation LoopSideNum,
                                    const Real64 SplitterInletPressure);

    void PassPressureAcrossInterface(EnergyPlusData &state, int LoopNum);

    Real64 ResolveLoopFlowVsPressure(EnergyPlusData &state,
                                     int LoopNum,            // - Index of which plant/condenser loop is being simulated
                                     Real64 SystemMassFlow,  // - Initial "guess" at system mass flow rate [kg/s]
                                     int PumpCurveNum,       // - Pump curve to use when calling the curve manager for psi = f(phi)
                                     Real64 PumpSpeed,       // - Pump rotational speed, [rps] (revs per second)
                                     Real64 PumpImpellerDia, // - Nominal pump impeller diameter [m]
                                     Real64 MinPhi,          // - Minimum allowable value of phi, requested by the pump manager from curve mgr
                                     Real64 MaxPhi           // - Maximum allowable value of phi, requested by the pump manager from curve mgr
    );

} // namespace PlantPressureSystem

struct PlantPressureSysData : BaseGlobalStruct
{

    bool InitPressureDropOneTimeInit = true;
    Array1D_bool LoopInit;
    std::array<bool, static_cast<int>(DataPlant::LoopSideLocation::Num)> FullParallelBranchSetFound{false, false};
    bool CommonPipeErrorEncountered = false;
    int ErrorCounter = 0; // For proper error handling
    int ZeroKWarningCounter = 0;
    int MaxIterWarningCounter = 0;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->InitPressureDropOneTimeInit = true;
        this->LoopInit.clear();
        this->FullParallelBranchSetFound = {false, false};
        this->CommonPipeErrorEncountered = false;
        this->ErrorCounter = 0;
        this->ZeroKWarningCounter = 0;
        this->MaxIterWarningCounter = 0;
    }
};

} // namespace EnergyPlus

#endif
