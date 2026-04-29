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

#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/Plant/Branch.hh>
#include <EnergyPlus/PlantUtilities.hh>

namespace EnergyPlus {
namespace DataPlant {

    Real64 BranchData::DetermineBranchFlowRequest(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   September 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This routine will analyze the given branch and determine the representative
        //  flow request.

        // METHODOLOGY EMPLOYED:
        // Several possibilities are available.  In any case, the request is constrained to within
        //  branch outlet min/max avail.  This assumes that the component flow routines will properly
        //  propagate the min/max avail down the branch.
        // Some possibilities for flow request are:
        //  1) take the outlet flow rate -- assumes that the last component wins
        //  2) take the inlet flow rate request -- assumes that the request is propagated up and is good
        //  3) take the maximum request
        //  4) move down the loop and take the maximum "non-load-range-based" request within min/max avail bounds
        //     This assumes that load range based should not request flow for load-rejection purposes, and we
        //     should only "respond" to other component types.

        int const BranchInletNodeNum = this->NodeNumIn;
        int const BranchOutletNodeNum = this->NodeNumOut;
        Real64 OverallFlowRequest = 0.0;

        if (this->controlType != DataBranchAirLoopPlant::ControlType::SeriesActive) {
            OverallFlowRequest = state.dataLoopNodes->Node(BranchInletNodeNum).MassFlowRateRequest;
        } else { // is series active, so take largest request of all the component inlet nodes
            for (int CompCounter = 1; CompCounter <= this->TotalComponents; ++CompCounter) {
                int const CompInletNode = this->Comp(CompCounter).NodeNumIn;
                OverallFlowRequest = max(OverallFlowRequest, state.dataLoopNodes->Node(CompInletNode).MassFlowRateRequest);
            }
        }

        //~ Now use a worker to bound the value to outlet min/max avail
        OverallFlowRequest = PlantUtilities::BoundValueToNodeMinMaxAvail(state, OverallFlowRequest, BranchOutletNodeNum);

        return OverallFlowRequest;
    }

} // namespace DataPlant
} // namespace EnergyPlus
