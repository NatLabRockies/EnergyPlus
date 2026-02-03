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

#ifndef PlantTopologyBranch_hh_INCLUDED
#define PlantTopologyBranch_hh_INCLUDED

#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/Plant/Component.hh>

namespace EnergyPlus {
namespace DataPlant {

    struct BranchData
    {
        // Members
        std::string Name; // Name of the branch
        DataBranchAirLoopPlant::ControlType controlType;
        Real64 RequestedMassFlow;
        bool HasConstantSpeedBranchPump;    // true if branch has a constant speed branch pump
        Real64 ConstantSpeedBranchMassFlow; // nominal flow rate if constant speed branch pump on
        int BranchLevel;
        int FlowErrCount;    // For recurring error counting
        int FlowErrIndex;    // For recurring error index
        int TotalComponents; // Total number of components on the branch
        int NodeNumIn;       // Component inlet node number
        int NodeNumOut;      // Component outlet node number
        bool IsBypass;
        int PumpIndex;
        Real64 PumpSizFac;
        bool EMSCtrlOverrideOn;      // if true, EMS is calling to override branch operation avail
        Real64 EMSCtrlOverrideValue; // value set by EMS system for branch override controls
        Array1D<CompData> Comp;      // Component type list
        bool HasPressureComponents;
        Real64 PressureDrop;
        DataBranchAirLoopPlant::PressureCurveType PressureCurveType; // Either none, pressure curve, or generic curve
        int PressureCurveIndex;                                      // Curve: index for pressure drop calculations
        Real64 PressureEffectiveK;
        bool disableOverrideForCSBranchPumping;
        int lastComponentSimulated;

        // Default Constructor
        BranchData()
            : controlType(DataBranchAirLoopPlant::ControlType::Invalid), RequestedMassFlow(0.0), HasConstantSpeedBranchPump(false),
              ConstantSpeedBranchMassFlow(0.0), BranchLevel(0), FlowErrCount(0), FlowErrIndex(0), TotalComponents(0), NodeNumIn(0), NodeNumOut(0),
              IsBypass(false), PumpIndex(0), PumpSizFac(1.0), EMSCtrlOverrideOn(false), EMSCtrlOverrideValue(0.0), HasPressureComponents(false),
              PressureDrop(0.0), PressureCurveType(DataBranchAirLoopPlant::PressureCurveType::Invalid), PressureCurveIndex(0),
              PressureEffectiveK(0.0), disableOverrideForCSBranchPumping(false), lastComponentSimulated(0)
        {
        }

        // Max abs of Comp array MyLoad values //Autodesk:Tuned For replacement of any( abs( Comp.MyLoad() > SmallLoad ) usage
        Real64 max_abs_Comp_MyLoad() const
        {
            Real64 load(0.0); // Return value
            for (int i = Comp.l(), e = Comp.u(); i <= e; ++i) {
                load = max(load, std::abs(Comp(i).MyLoad));
            }
            return load;
        }

        Real64 DetermineBranchFlowRequest(EnergyPlusData &state);
    };

} // namespace DataPlant
} // namespace EnergyPlus

#endif
