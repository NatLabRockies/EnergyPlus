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

#ifndef HVACInterfaceManager_hh_INCLUDED
#define HVACInterfaceManager_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataConvergParams.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/Enums.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace HVACInterfaceManager {

    // Common Pipe Recirc Flow Directions
    // can't change these to enum class since these are used in SetupOutputVariable()
    constexpr int NoRecircFlow(0);
    constexpr int PrimaryRecirc(1);   // flow from Supply-outlet/Demand-inlet to Supply-inlet/demand-outlet
    constexpr int SecondaryRecirc(2); // flow from Supply-inlet/Demand-outlet to Supply-outlet/demand-inlet

    enum class FlowType
    {
        Invalid = -1,
        Constant,
        Variable,
        Num
    };

    struct CommonPipeData
    {
        // Members
        DataPlant::CommonPipeType CommonPipeType = DataPlant::CommonPipeType::No; // type of common pipe used if any
        FlowType SupplySideInletPumpType = FlowType::Invalid;
        FlowType DemandSideInletPumpType = FlowType::Invalid;
        // Following report variables are used in uncontrolled common pipe
        int FlowDir = 0;   // Direction in which flow is in Common Pipe
        Real64 Flow = 0.0; // Flow in the Common Pipe
        Real64 Temp = 0.0;
        // Following report variables are used in two-way common pipe
        Real64 SecCPLegFlow = 0.0;       // Mass flow in the secondary side Common pipe leg
        Real64 PriCPLegFlow = 0.0;       // Mass flow in the primary side Common pipe leg
        Real64 SecToPriFlow = 0.0;       // Mass flow in the pipe from Secondary to primary side
        Real64 PriToSecFlow = 0.0;       // Mass flow in the pipe from primary to Secondary side
        Real64 PriInTemp = 0.0;          // Temperature at primary inlet node
        Real64 PriOutTemp = 0.0;         // Temperature at primary outlet node
        Real64 SecInTemp = 0.0;          // Temperature at secondary inlet node
        Real64 SecOutTemp = 0.0;         // Temperature at secondary outlet node
        Real64 PriInletSetPoint = 0.0;   // Setpoint at Primary inlet node
        Real64 SecInletSetPoint = 0.0;   // Setpoint at Secondary inlet node
        bool PriInletControlled = false; // True if Primary inlet node is controlled
        bool SecInletControlled = false; // True if secondary inlet is controlled
        Real64 PriFlowRequest = 0.0;     // total flow request on supply side.
        bool MyEnvrnFlag = true;
    };

    void UpdateHVACInterface(EnergyPlusData &state,
                             int AirLoopNum, // airloop number for which air loop this is
                             DataConvergParams::CalledFrom CalledFrom,
                             int OutletNode,          // Node number for the outlet of the side of the loop just simulated
                             int InletNode,           // Node number for the inlet of the side that needs the outlet node data
                             bool &OutOfToleranceFlag // True when the other side of the loop need to be (re)simulated
    );

    void UpdatePlantLoopInterface(EnergyPlusData &state,
                                  PlantLocation const &plantLoc, // The 'outlet node' Location
                                  int ThisLoopSideOutletNode,    // Node number for the inlet of the side that needs the outlet node data
                                  int OtherLoopSideInletNode,    // Node number for the outlet of the side of the loop just simulated
                                  bool &OutOfToleranceFlag,      // True when the other side of the loop need to be (re)simulated
                                  DataPlant::CommonPipeType CommonPipeType);

    void UpdateHalfLoopInletTemp(EnergyPlusData &state, int LoopNum, DataPlant::LoopSideLocation TankInletLoopSide, Real64 &TankOutletTemp);

    void UpdateCommonPipe(EnergyPlusData &state,
                          const PlantLocation &TankInletPlantLoc,
                          DataPlant::CommonPipeType CommonPipeType,
                          Real64 &MixedOutletTemp);

    void ManageSingleCommonPipe(EnergyPlusData &state,
                                int LoopNum,                          // plant loop number
                                DataPlant::LoopSideLocation LoopSide, // plant loop side number
                                Real64 TankOutletTemp,  // inlet temperature to the common pipe passed in from the capacitance calculation
                                Real64 &MixedOutletTemp // inlet temperature to the common pipe passed in from the capacitance calculation
    );

    void ManageTwoWayCommonPipe(EnergyPlusData &state, PlantLocation const &plantLoc, Real64 TankOutletTemp);

    void SetupCommonPipes(EnergyPlusData &state);

    // In-Place Right Shift by 1 of Array Elements
    inline void rshift1(std::array<Real64, DataConvergParams::ConvergLogStackDepth> &a)
    {
        Real64 lastVal = a[a.size() - 1];
        for (unsigned int i = a.size() - 1; i > 0; --i) {
            a[i] = a[i - 1];
        }
        a[0] = lastVal;
    }

} // namespace HVACInterfaceManager

struct HVACInterfaceManagerData : BaseGlobalStruct
{

    bool CommonPipeSetupFinished = false;
    Array1D<HVACInterfaceManager::CommonPipeData> PlantCommonPipe;
    Array1D<Real64> TmpRealARR = Array1D<Real64>(DataConvergParams::ConvergLogStackDepth); // Tuned Made static

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->CommonPipeSetupFinished = false;
        this->PlantCommonPipe.deallocate();
    }
};

} // namespace EnergyPlus

#endif
