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

#include <ObjexxFCL/member.functions.hh>

#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataConvergParams.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HVACInterfaceManager.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/LoopSide.hh>
#include <EnergyPlus/PlantCondLoopOperation.hh>
#include <EnergyPlus/PlantPressureSystem.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Pumps.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {
namespace DataPlant {

    void HalfLoopData::solve(EnergyPlusData &state, bool const FirstHVACIteration, bool &ReSimOtherSideNeeded)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHORS:         Dan Fisher, Sankaranarayanan K P, Edwin Lee
        //       DATE WRITTEN:    April 1998
        //       MODIFIED         June 2005(Work in the Plant Super Manager Module)
        //                        July 2006
        //       RE-ENGINEERED    July 2010

        // PURPOSE OF THIS SUBROUTINE:
        // SimSupplyFlowSolution is the driver routine for plant loops.  It performs
        //  the following tasks for each half loop (supply or demand side):
        // 1. Calculate flow request for half loop
        // 2. Predict Loop Flow
        // 3. Simulate the inlet branch
        // 4. Simulate the parallel branches, distributing load if necessary
        // 5. Set flow rates on parallel branches
        // 6. Simulate outlet branch and update node and report variables

        // METHODOLOGY EMPLOYED:
        // The algorithm operates on a predictor/corrector flow setting method by simulating all available loop components
        // based on component requested flow rates, then enforcing continuity on all loop branch flows by calling
        // the flow resolver and locking those flows down.  Available components are then re-simulated using the
        // corrected flow rates.

        auto &thisPlantLoop = state.dataPlnt->PlantLoop(this->plantLoc.loopNum);
        int ThisSideInletNode = this->NodeNumIn;

        this->InitialDemandToLoopSetPoint = 0.0;
        this->CurrentAlterationsToDemand = 0.0;
        this->UpdatedDemandToLoopSetPoint = 0.0;

        // The following block is related to validating the flow control paths of the loop side
        // Since the control types are scheduled, I think BeginTimeStep should be a decent check frequency
        if (state.dataGlobal->BeginTimeStepFlag && this->OncePerTimeStepOperations) {

            // Initialize loop side controls -- could just be done for one loop since this routine inherently
            //  loops over all plant/condenser loops.  Not sure if the penalty is worth investigating.
            PlantCondLoopOperation::InitLoadDistribution(state, FirstHVACIteration);

            // Now that the op scheme types are updated, do LoopSide validation
            this->ValidateFlowControlPaths(state);

            // Set the flag to false so we won't do these again this time step
            this->OncePerTimeStepOperations = false;

        } else {

            // Set the flag to true so that it is activated for the next time step
            this->OncePerTimeStepOperations = true;
        }

        // Do pressure system initialize if this is the demand side (therefore once per whole loop)
        if (this->plantLoc.loopSideNum == DataPlant::LoopSideLocation::Demand) {
            PlantPressureSystem::SimPressureDropSystem(state, this->plantLoc.loopNum, FirstHVACIteration, DataPlant::PressureCall::Init);
        }

        // Turn on any previously disabled branches due to constant speed branch pump issue
        this->TurnOnAllLoopSideBranches();

        LoopSideLocation OtherLoopSide = static_cast<LoopSideLocation>(LoopSideOther[static_cast<int>(this->plantLoc.loopSideNum)]);
        // Do the actual simulation here every time
        this->DoFlowAndLoadSolutionPass(state, OtherLoopSide, ThisSideInletNode, FirstHVACIteration);

        // On constant speed branch pump loop sides we need to re-simulate
        if (this->hasConstSpeedBranchPumps) {
            // turn off any pumps connected to unloaded equipment and re-do the flow/load solution pass
            this->DisableAnyBranchPumpsConnectedToUnloadedEquipment();
            this->DoFlowAndLoadSolutionPass(state, OtherLoopSide, ThisSideInletNode, FirstHVACIteration);
        }

        // A couple things are specific to which LoopSide we are on  // TODO: This whole block needs to be moved up to the loop level
        if (this->plantLoc.loopSideNum == DataPlant::LoopSideLocation::Demand) {

            // Pass the loop information via the HVAC interface manager
            HVACInterfaceManager::UpdatePlantLoopInterface(state,
                                                           this->plantLoc,
                                                           thisPlantLoop.LoopSide(DataPlant::LoopSideLocation::Demand).NodeNumOut,
                                                           thisPlantLoop.LoopSide(DataPlant::LoopSideLocation::Supply).NodeNumIn,
                                                           ReSimOtherSideNeeded,
                                                           thisPlantLoop.CommonPipeType);

        } else { // LoopSide == LoopSideLocation::Supply

            // Update pressure drop reporting, calculate total loop pressure drop for use elsewhere
            PlantPressureSystem::SimPressureDropSystem(state, this->plantLoc.loopNum, FirstHVACIteration, DataPlant::PressureCall::Update);

            // Pass the loop information via the HVAC interface manager (only the flow)
            HVACInterfaceManager::UpdatePlantLoopInterface(state,
                                                           this->plantLoc,
                                                           thisPlantLoop.LoopSide(DataPlant::LoopSideLocation::Supply).NodeNumOut,
                                                           thisPlantLoop.LoopSide(DataPlant::LoopSideLocation::Demand).NodeNumIn,
                                                           ReSimOtherSideNeeded,
                                                           thisPlantLoop.CommonPipeType);

            // Update the loop outlet node conditions
            state.dataPlnt->PlantLoop(this->plantLoc.loopNum)
                .CheckLoopExitNode(state, FirstHVACIteration); // TODO: This is a loop level check, move out

            state.dataPlnt->PlantLoop(this->plantLoc.loopNum)
                .UpdateLoopSideReportVars(state, this->InitialDemandToLoopSetPointSAVED, this->LoadToLoopSetPointThatWasntMet);
        }
    }

    void HalfLoopData::ValidateFlowControlPaths(EnergyPlusData &state)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   July 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This routine will scan all the loop side paths and validate the component topology according
        //  to current topology rules and regulations.

        // METHODOLOGY EMPLOYED:
        // Scan this loop side and begin by scanning the first branch, then follow with the remainder of the flow paths
        //  - this would be from splitter outlet nodes all the way to the loop side outlet node.
        // The current rules are that "other types" of components (as defined below in the references) can be placed along each
        //  flow path as needed.  At this point, any number of "load-range based" components can be placed along the flow
        //  path.  After this, the user is allowed to place another set of any number of "other types" of components.
        // The key restriction is that an "other type" of component may not be sandwiched by "load-range based" components.
        // This is due to the load range based needing to be simulated all at once along each flow path.

        // REFERENCES:
        // "other types" of components: basically not load-range based heat transfer components.  This would include:
        //    - demand based components such as coils
        //    - component setpoint based operating components
        //    - heat exchanger components including waterside economizers
        // "load-range based" components are heat transfer components which are controlled based on a single load range.
        //    - currently only one load range based scheme is available at a given time, although other control types
        //      may be enabled, such as component setpoint.
        // Pumps are separate components since the pump heat is not accounted for in the flow path order.
        //  Improvements during the demand side rewrite has allowed pumps to be placed as -not- the first component on a branch
        //  Pumps can be placed anywhere, even between load-range based components, since they will not affect loop load

        // RETURN VALUE:
        // Returns a control validator flow structure, including a flag for successful or not, then if not successful
        //  the other values are filled in such as location on the loop where the error occurred and a message error description

        // FUNCTION PARAMETER DEFINITIONS:
        int constexpr Parallel(1);
        int constexpr Outlet(2);

        //~ Initialze
        bool EncounteredLRB = false;
        bool EncounteredNonLRBAfterLRB = false;
        int const NumParallelPaths = this->TotalBranches - 2;

        // We'll start by stepping through the first branch, which may be the only branch
        // If we find a load range based, then trip the flag and just keep moving
        // If we only have one branch and all is good, then RETURN early
        // If we have parallel branches, then start looping through each flow path to
        //  decide if it is a valid path.
        // If any one path is invalid then all is wrong
        int firstBranchIndex = 1;
        for (int CompIndex = 1; CompIndex <= this->Branch(firstBranchIndex).TotalComponents; ++CompIndex) {

            auto const &this_component(this->Branch(firstBranchIndex).Comp(CompIndex));

            {
                switch (this_component.CurOpSchemeType) {
                case OpScheme::HeatingRB:
                case OpScheme::CoolingRB: { //~ load range based
                    if (EncounteredNonLRBAfterLRB) {
                        // We must have already encountered a LRB, then a non-LRB, and now another LRB, this is bad
                        ShowSevereError(state, "Plant topology problem on \"" + this->loopSideDescription + "\"");
                        ShowContinueError(state, "PlaLoad range based components are separated by other control type components.");
                        ShowContinueError(state, "Load Range Based should be grouped together on each flow path.");
                        ShowFatalError(state, "Plant topology issue causes program termination");
                    } else {
                        EncounteredLRB = true;
                    }
                    break;
                }
                case DataPlant::OpScheme::Pump: { //~ pump
                    // For now this is just a placeholder, because I think pumps will be available anywhere,
                    //  and they won't affect the load distribution
                    break;
                }
                case DataPlant::OpScheme::NoControl: { //~ Such as pipes
                    // For now this is just a placeholder, because these components shouldn't cause a problem anywhere...
                    break;
                }
                case DataPlant::OpScheme::Invalid: { //~ Uninitialized, this should be a sufficient place to catch for this on branch 1
                    // throw fatal
                    ShowSevereError(state,
                                    "ValidateFlowControlPaths: Uninitialized operation scheme type for component Name: " + this_component.Name);
                    ShowFatalError(state, "ValidateFlowControlPaths: developer notice, Inlet path validation loop");
                    break;
                }
                default: { //~ Other control type
                    if (EncounteredLRB) {
                        EncounteredNonLRBAfterLRB = true;
                    } else {
                        // For now don't do anything, but we'll see...
                    }
                }
                }
            }
        }

        // Return early if we only needed to do the one branch
        if (NumParallelPaths <= 0) {
            return;
        }

        // Now, if we have multiple parallel branches, I think the easiest way is to go all the way from the inlet node
        //  of each parallel branch to the loop outlet node and check the flow path
        // This way we don't have to remember the conditions on each of the parallel branches when we would finally move
        //  to analyzing the outlet node when all done
        // This will reduce allocation on the heap because we will keep from storing that array
        // For each parallel path, we will need to check two branches: the parallel branch and the LoopSide outlet branch
        for (int PathCounter = 1; PathCounter <= NumParallelPaths; ++PathCounter) {
            for (int ParallelOrOutletIndex = Parallel; ParallelOrOutletIndex <= Outlet; ++ParallelOrOutletIndex) {
                int BranchIndex;
                if (ParallelOrOutletIndex == Parallel) {
                    // The branch index will be the current pathtype + 1 to add the inlet branch
                    BranchIndex = PathCounter + 1;
                } else { // ParallelOrOutletIndex == Outlet
                    // The branch index will be the LoopSide outlet node
                    BranchIndex = this->TotalBranches;
                }

                // Now that we have the branch index, let's do the control type check over all the components
                for (int CompIndex = 1; CompIndex <= this->Branch(BranchIndex).TotalComponents; ++CompIndex) {

                    auto const &this_component(this->Branch(BranchIndex).Comp(CompIndex));

                    {
                        switch (this_component.CurOpSchemeType) {
                        case OpScheme::HeatingRB:
                        case OpScheme::CoolingRB: { //~ load range based
                            if (EncounteredNonLRBAfterLRB) {
                                // We must have already encountered a LRB, then a non-LRB, and now another LRB, this is bad
                                ShowSevereError(state, "Plant topology problem on \"" + this->loopSideDescription + "\"");
                                ShowContinueError(state, "Load range based components are separated by other control type components.");
                                ShowContinueError(state, "Load Range Based should be grouped together on each flow path.");
                                ShowFatalError(state, "Plant topology issue causes program termination");
                            } else {
                                EncounteredLRB = true;
                            }
                            break;
                        }

                        case DataPlant::OpScheme::NoControl: { //~ Such as pipes
                            // For now this is just a placeholder, because these components shouldn't cause a problem anywhere...
                            break;
                        }
                        case DataPlant::OpScheme::Pump: { //~ pump
                            // For now this is just a placeholder, because I think pumps will be available anywhere,
                            //  and they won't affect the load distribution
                            break;
                        }
                        case DataPlant::OpScheme::Invalid: { //~ Uninitialized, this should be sufficient place to
                                                             // catch for this on other branches
                            // throw fatal error
                            ShowSevereError(
                                state, "ValidateFlowControlPaths: Uninitialized operation scheme type for component Name: " + this_component.Name);
                            ShowFatalError(state, "ValidateFlowControlPaths: developer notice, problem in Parallel path validation loop");
                            break;
                        }
                        default: { //~ Other control type
                            if (EncounteredLRB) {
                                EncounteredNonLRBAfterLRB = true;
                            } else {
                                // For now don't do anything, but we'll see...
                            }
                        }
                        }
                    }

                } //~ CompIndex

            } //~ Parallel and Outlet Branches

        } //~ Parallel Paths
    }

    bool HalfLoopData::CheckPlantConvergence(bool const FirstHVACIteration)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   Summer 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This routine checks the history values in the convergence arrays of this loop/LoopSide combination

        // METHODOLOGY EMPLOYED:
        // On FirstHVAC, we are not converged yet, thus forcing at least two iterations
        // Calculate the average of each related variable history (generalized: could be any number of history terms)
        // If any of the history terms do not match this average, then at least one value is different, so not converged
        // Although this routine appears to check for convergence, it is also used to check for stuck (max iteration) conditions
        //  in cases where demand side (air loop, for example) equipment is "fighting" with the plant loop
        // The result of this routine can help the plant "lock-in" and take action to stop the iteration

        // Using/Aliasing
        using namespace DataPlant;
        using namespace DataLoopNode;

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        Real64 InletAvgTemp;
        Real64 InletAvgMdot;
        Real64 OutletAvgTemp;
        Real64 OutletAvgMdot;

        if (FirstHVACIteration) {
            return false;
        }

        InletAvgTemp = sum(this->InletNode.TemperatureHistory) / size(this->InletNode.TemperatureHistory);
        if (any_ne(this->InletNode.TemperatureHistory, InletAvgTemp)) {
            return false;
        }

        InletAvgMdot = sum(this->InletNode.MassFlowRateHistory) / size(this->InletNode.MassFlowRateHistory);
        if (any_ne(this->InletNode.MassFlowRateHistory, InletAvgMdot)) {
            return false;
        }

        OutletAvgTemp = sum(this->OutletNode.TemperatureHistory) / size(this->OutletNode.TemperatureHistory);
        if (any_ne(this->OutletNode.TemperatureHistory, OutletAvgTemp)) {
            return false;
        }

        OutletAvgMdot = sum(this->OutletNode.MassFlowRateHistory) / size(this->OutletNode.MassFlowRateHistory);
        if (any_ne(this->OutletNode.MassFlowRateHistory, OutletAvgMdot)) {
            return false;
        }

        // If we made it this far, we're good!
        return true;
    }

    void HalfLoopData::PushBranchFlowCharacteristics(EnergyPlusData &state,
                                                     int const BranchNum,
                                                     Real64 const ValueToPush,
                                                     bool const FirstHVACIteration // TRUE if First HVAC iteration of Time step
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   September 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This routine takes the flow resolved flow rate and pushes it
        //  down a branch.  In the process, if an externally connected
        //  component (air-water coil for example) is found to have a
        //  differing flow rate, the air sim flag is tripped to true, but
        //  the flow resolved flow rate is pushed down the loop to allow
        //  the plant to finish successfully.

        // METHODOLOGY EMPLOYED:
        // Push mass flow rate and max avail down each branch.  If the component
        //  is connected (or could be, for now) to an external loop such as
        //  an air loop, the current component outlet mass flow is checked
        //  vs the current resolved mass flow.  If the mass flow doesn't match,
        //  the air sim flag is tripped to true.

        // Currently this routine is only performed for starved branches, when
        //  the coil is requesting too much flow, more than the plant can provide.
        // If this were moved to every call type, including a minimum plant flow,
        //  you would need to provide a mass flow and min/max avail to push
        //  down the branch as well.

        // Using/Aliasing
        using namespace DataPlant; // Use the entire module to allow all TypeOf's, would be a huge ONLY list

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int CompCounter;
        int BranchInletNode;
        DataPlant::PlantEquipmentType ComponentType;
        Real64 MassFlowRateFound;
        Real64 MassFlow;
        bool PlantIsRigid;

        auto &this_branch(this->Branch(BranchNum));

        BranchInletNode = this_branch.NodeNumIn;

        //~ Possible error handling if needed
        if (ValueToPush != state.dataLoopNodes->Node(BranchInletNode).MassFlowRate) {
            // Diagnostic problem, flow resolver isn't calling this routine properly
        }

        //~ This section would really be useful more later on if this routine has more logic regarding what to push down the branch
        MassFlow = ValueToPush;
        // MinAvail = ValueToPush
        // MaxAvail = ValueToPush

        PlantIsRigid = this->CheckPlantConvergence(FirstHVACIteration);

        //~ Loop across all component outlet nodes and update their mass flow and max avail
        for (CompCounter = 1; CompCounter <= this_branch.TotalComponents; ++CompCounter) {

            auto const &this_comp(this_branch.Comp(CompCounter));

            //~ Pick up some values for convenience
            int ComponentInletNode = this_comp.NodeNumIn;
            int ComponentOutletNode = this_comp.NodeNumOut;
            MassFlowRateFound = state.dataLoopNodes->Node(ComponentOutletNode).MassFlowRate;
            ComponentType = this_comp.Type;

            //~ Push the values through
            state.dataLoopNodes->Node(ComponentOutletNode).MassFlowRate = MassFlow;

            if (PlantIsRigid) {
                state.dataLoopNodes->Node(ComponentInletNode).MassFlowRateMinAvail = MassFlow;
                state.dataLoopNodes->Node(ComponentInletNode).MassFlowRateMaxAvail = MassFlow;
                state.dataLoopNodes->Node(ComponentOutletNode).MassFlowRateMinAvail = MassFlow;
                state.dataLoopNodes->Node(ComponentOutletNode).MassFlowRateMaxAvail = MassFlow;
            }
            // Node(ComponentOutletNode)%MassFlowRateMinAvail = MinAvail
            // no this is 2-way valve which messes up flow options
            //      for demand components Node(ComponentOutletNode)%MassFlowRateMaxAvail = MaxAvail

            //~ If this value matches then we are good to move to the next component
            if (std::abs(MassFlow - MassFlowRateFound) < CriteriaDelta_MassFlowRate) {
                continue;
            }
            //~ Since there is a difference, we have to decide what to do based on the component type:
            //~  For plant connections, don't do anything, it SHOULD work itself out
            //~  For air connections, trip the LoopSide air flag
            //~  Similar for zone, none zone, and electric load center
            {
                switch (ComponentType) {

                    // possibly air-connected components
                case DataPlant::PlantEquipmentType::CoilWaterCooling:
                case DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling:
                case DataPlant::PlantEquipmentType::CoilWaterSimpleHeating:
                case DataPlant::PlantEquipmentType::CoilSteamAirHeating:
                case DataPlant::PlantEquipmentType::CoilWAHPHeatingEquationFit:
                case DataPlant::PlantEquipmentType::CoilWAHPCoolingEquationFit:
                case DataPlant::PlantEquipmentType::CoilWAHPHeatingParamEst:
                case DataPlant::PlantEquipmentType::CoilWAHPCoolingParamEst:
                case DataPlant::PlantEquipmentType::CoilUserDefined:
                case DataPlant::PlantEquipmentType::CoilVSWAHPCoolingEquationFit:
                case DataPlant::PlantEquipmentType::CoilVSWAHPHeatingEquationFit:
                case DataPlant::PlantEquipmentType::PackagedTESCoolingCoil: {
                    this->SimAirLoopsNeeded = true;
                    // sometimes these coils are children in ZoneHVAC equipment
                    // PlantLoop(LoopNum)%LoopSide(LoopSideNum)%SimZoneEquipNeeded= .TRUE.
                    break;
                }

                    // zone connected components
                case DataPlant::PlantEquipmentType::CoolingPanel_Simple:
                case DataPlant::PlantEquipmentType::Baseboard_Conv_Water:
                case DataPlant::PlantEquipmentType::Baseboard_Rad_Conv_Steam:
                case DataPlant::PlantEquipmentType::Baseboard_Rad_Conv_Water:
                case DataPlant::PlantEquipmentType::LowTempRadiant_VarFlow:
                case DataPlant::PlantEquipmentType::LowTempRadiant_ConstFlow:
                case DataPlant::PlantEquipmentType::CooledBeamAirTerminal:
                case DataPlant::PlantEquipmentType::ZoneHVACAirUserDefined:
                case DataPlant::PlantEquipmentType::AirTerminalUserDefined:
                case DataPlant::PlantEquipmentType::FourPipeBeamAirTerminal: {
                    this->SimZoneEquipNeeded = true;
                    break;
                }

                // electric center connected components
                case DataPlant::PlantEquipmentType::Generator_FCExhaust:
                case DataPlant::PlantEquipmentType::Generator_FCStackCooler:
                case DataPlant::PlantEquipmentType::Generator_MicroCHP:
                case DataPlant::PlantEquipmentType::Generator_MicroTurbine:
                case DataPlant::PlantEquipmentType::Generator_ICEngine:
                case DataPlant::PlantEquipmentType::Generator_CTurbine: {
                    this->SimElectLoadCentrNeeded = true;
                    break;
                }

                default:
                    break;
                }
            }
        }
    }

    void HalfLoopData::TurnOnAllLoopSideBranches()
    {
        for (int branchNum = 2; branchNum <= this->TotalBranches - 1; ++branchNum) {
            auto &branch = this->Branch(branchNum);
            branch.disableOverrideForCSBranchPumping = false;
        }
    }

    void HalfLoopData::SimulateAllLoopSideBranches(EnergyPlusData &state,
                                                   Real64 const ThisLoopSideFlow,
                                                   bool const FirstHVACIteration,
                                                   bool &LoopShutDownFlag)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   July 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This routine will step through all branch groups (single branch .OR. inlet/parallels/outlet)
        //  and call the branch group simulation routine.  This routine also calls to update the splitter
        //  and mixer.

        // METHODOLOGY EMPLOYED:
        // The number of branch groups is either 1 or 3.  1 would be a single branch half-loop.  3 would
        //  be the minimum for an inlet/parallels/outlet set.  The number of branch groups can then be
        //  calculated as #BrGrps = 1 + 2*L; where L is zero for single half loop and one for parallel-type set.
        //  This calculation can be reduced to the logical/integer conversion as shown in the code.
        // The simulation then steps through each branch group.  If there are parallel branches, the splitter is
        //  updated on flowlock=0 to pass information through, then after the parallel branches the mixer is always
        //  updated.  The outlet branch "group" is then simulated.

        // SUBROUTINE PARAMETER DEFINITIONS:
        int constexpr InletBranchOrOneBranchHalfLoop(1);
        int constexpr ParallelBranchSet(2);
        int constexpr OutletBranch(3);

        int NumBranchGroups = 1;
        if (this->TotalBranches > 1) {
            NumBranchGroups = 3;
        }

        // reset branch starting component index back to zero before each pass
        for (int BranchCounter = 1; BranchCounter <= this->TotalBranches; ++BranchCounter) {
            this->Branch(BranchCounter).lastComponentSimulated = 0;
        }

        for (int BranchGroup = 1; BranchGroup <= NumBranchGroups; ++BranchGroup) {

            if ((BranchGroup > 1) && (this->TotalBranches == 1)) {
                break;
            }

            switch (BranchGroup) {
            case InletBranchOrOneBranchHalfLoop:
                this->SimulateLoopSideBranchGroup(state, 1, 1, ThisLoopSideFlow, FirstHVACIteration, LoopShutDownFlag);
                break;
            case ParallelBranchSet:
                this->UpdatePlantSplitter(state);
                this->SimulateLoopSideBranchGroup(state, 2, this->TotalBranches - 1, ThisLoopSideFlow, FirstHVACIteration, LoopShutDownFlag);
                this->UpdatePlantMixer(state);
                break;
            case OutletBranch:
                this->SimulateLoopSideBranchGroup(
                    state, this->TotalBranches, this->TotalBranches, ThisLoopSideFlow, FirstHVACIteration, LoopShutDownFlag);
                break;
            }
        }
    }

    void HalfLoopData::AdjustPumpFlowRequestByEMSControls(int const BranchNum, int const CompNum, Real64 &FlowToRequest)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   April 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // modify flow request to pump simulation if EMS is overriding pump component

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        auto &this_branch(this->Branch(BranchNum));
        auto &this_comp(this_branch.Comp(CompNum));

        if ((this->EMSCtrl) && (this->EMSValue <= 0.0)) {
            FlowToRequest = 0.0;
            return;
        }

        if ((this_branch.EMSCtrlOverrideOn) && (this_branch.EMSCtrlOverrideValue <= 0.0)) {
            FlowToRequest = 0.0;
            return;
        }

        if (this_comp.EMSLoadOverrideOn) {
            if (this_comp.EMSLoadOverrideValue == 0.0) {
                FlowToRequest = 0.0;
            }
        }
    }

    void HalfLoopData::DisableAnyBranchPumpsConnectedToUnloadedEquipment()
    {
        for (int branchNum = 2; branchNum <= this->TotalBranches - 1; ++branchNum) {
            auto &branch = this->Branch(branchNum);
            Real64 totalDispatchedLoadOnBranch = 0.0;
            for (int compNum = 1; compNum <= branch.TotalComponents; ++compNum) {
                auto const &component = branch.Comp(compNum);
                auto const &t = component.Type;
                if (t == DataPlant::PlantEquipmentType::PumpConstantSpeed || t == DataPlant::PlantEquipmentType::PumpBankConstantSpeed ||
                    t == DataPlant::PlantEquipmentType::PumpVariableSpeed || t == DataPlant::PlantEquipmentType::PumpBankVariableSpeed) {
                    // don't do anything
                } else {
                    totalDispatchedLoadOnBranch += component.MyLoad;
                }
            }
            if (std::abs(totalDispatchedLoadOnBranch) < 0.001) {
                branch.disableOverrideForCSBranchPumping = true;
            }
        }
    }

    Real64 HalfLoopData::EvaluateLoopSetPointLoad(EnergyPlusData &state, int const FirstBranchNum, int const LastBranchNum, Real64 ThisLoopSideFlow)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   August 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // Return value
        Real64 LoadToLoopSetPoint = 0.0; // function result

        static constexpr std::string_view RoutineName("PlantLoopSolver::EvaluateLoopSetPointLoad");
        static constexpr std::string_view RoutineNameAlt("PlantSupplySide:EvaluateLoopSetPointLoad");

        //~ General variables
        Real64 SumMdotTimesTemp = 0.0;
        Real64 SumMdot = 0.0;

        auto &thisPlantLoop = state.dataPlnt->PlantLoop(this->plantLoc.loopNum);

        // We will place one specialized case in here for common pipe simulations.
        // If we are doing a common pipe simulation, and there is greater other-side flow than this side,
        //  then the "other side" demand needs to include getting the flow through the common pipe to the same setpoint
        //  as the flow going through the actual supply side
        if (this->hasConstSpeedBranchPumps && this->plantLoc.loopSideNum == DataPlant::LoopSideLocation::Supply &&
            thisPlantLoop.CommonPipeType != DataPlant::CommonPipeType::No) {
            const DataPlant::LoopSideLocation OtherSide = LoopSideOther[static_cast<int>(this->plantLoc.loopSideNum)];
            const int otherSideOutletNodeNum = thisPlantLoop.LoopSide(OtherSide).NodeNumOut;
            Real64 commonPipeFlow = state.dataLoopNodes->Node(otherSideOutletNodeNum).MassFlowRate - ThisLoopSideFlow;
            Real64 otherSideExitingTemperature = state.dataLoopNodes->Node(otherSideOutletNodeNum).Temp;
            SumMdotTimesTemp += otherSideExitingTemperature * commonPipeFlow;
            SumMdot += commonPipeFlow;
        }

        // Sweep across flow paths in this group and calculate the deltaT and then the load
        int BranchIndex = 0; // ~ This is a 1 - n value within the current branch group
        for (int BranchCounter = FirstBranchNum; BranchCounter <= LastBranchNum; ++BranchCounter) {

            ++BranchIndex;

            //~ Always start from the last component we did the last time around + 1 and
            //~  try to make it all the way to the end of the loop
            int StartingComponent = this->Branch(BranchCounter).lastComponentSimulated + 1;
            int EnteringNodeNum = this->Branch(BranchCounter).Comp(StartingComponent).NodeNumIn;

            Real64 EnteringTemperature = state.dataLoopNodes->Node(EnteringNodeNum).Temp;
            Real64 MassFlowRate = state.dataLoopNodes->Node(EnteringNodeNum).MassFlowRate;

            SumMdotTimesTemp += EnteringTemperature * MassFlowRate;
            SumMdot += MassFlowRate;
        }

        if (SumMdot < DataBranchAirLoopPlant::MassFlowTolerance) {
            return 0.0;
        }

        Real64 WeightedInletTemp = SumMdotTimesTemp / SumMdot;

        if (thisPlantLoop.FluidType == DataLoopNode::NodeFluidType::Water) {

            Real64 Cp = thisPlantLoop.glycol->getSpecificHeat(state, WeightedInletTemp, RoutineName);

            {

                if (thisPlantLoop.LoopDemandCalcScheme == DataPlant::LoopDemandCalcScheme::SingleSetPoint) {

                    // Pick up the loop setpoint temperature
                    Real64 LoopSetPointTemperature = this->TempSetPoint;
                    // Calculate the delta temperature
                    Real64 DeltaTemp = LoopSetPointTemperature - WeightedInletTemp;

                    // Calculate the demand on the loop
                    LoadToLoopSetPoint = SumMdot * Cp * DeltaTemp;

                } else if (thisPlantLoop.LoopDemandCalcScheme == DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand) {

                    // Get the range of setpoints
                    Real64 LoopSetPointTemperatureHi = state.dataLoopNodes->Node(thisPlantLoop.TempSetPointNodeNum).TempSetPointHi;
                    Real64 LoopSetPointTemperatureLo = state.dataLoopNodes->Node(thisPlantLoop.TempSetPointNodeNum).TempSetPointLo;

                    // Calculate the demand on the loop
                    if (SumMdot > 0.0) {
                        Real64 LoadToHeatingSetPoint = SumMdot * Cp * (LoopSetPointTemperatureLo - WeightedInletTemp);
                        Real64 LoadToCoolingSetPoint = SumMdot * Cp * (LoopSetPointTemperatureHi - WeightedInletTemp);
                        // Possible combinations:
                        // 1  LoadToHeatingSetPoint > 0 & LoadToCoolingSetPoint > 0 -->  Heating required
                        // 2  LoadToHeatingSetPoint < 0 & LoadToCoolingSetPoint < 0 -->  Cooling Required
                        // 3  LoadToHeatingSetPoint <=0 & LoadToCoolingSetPoint >=0 -->  Dead Band Operation - includes zero load cases
                        // 4  LoadToHeatingSetPoint  >  LoadToCoolingSetPoint       -->  Not Feasible if LoopSetPointHi >= LoopSetPointLo
                        // First trap bad set-points
                        if (LoadToHeatingSetPoint > LoadToCoolingSetPoint) {
                            ShowSevereError(state,
                                            "Plant Loop: the Plant Loop Demand Calculation Scheme is set to DualSetPointDeadBand, but the "
                                            "heating-related low setpoint appears to be above the cooling-related high setpoint.");
                            ShowContinueError(state,
                                              "For example, if using SetpointManager:Scheduled:DualSetpoint, then check that the low setpoint is "
                                              "below the high setpoint.");
                            ShowContinueError(state, "Occurs in PlantLoop=" + thisPlantLoop.Name);
                            ShowContinueError(
                                state,
                                format("LoadToHeatingSetPoint={:.3R}, LoadToCoolingSetPoint={:.3R}", LoadToHeatingSetPoint, LoadToCoolingSetPoint));
                            ShowContinueError(state, format("Loop Heating Low Setpoint={:.2R}", LoopSetPointTemperatureLo));
                            ShowContinueError(state, format("Loop Cooling High Setpoint={:.2R}", LoopSetPointTemperatureHi));

                            ShowFatalError(state, "Program terminates due to above conditions.");
                        }
                        if (LoadToHeatingSetPoint > 0.0 && LoadToCoolingSetPoint > 0.0) {
                            LoadToLoopSetPoint = LoadToHeatingSetPoint;
                        } else if (LoadToHeatingSetPoint < 0.0 && LoadToCoolingSetPoint < 0.0) {
                            LoadToLoopSetPoint = LoadToCoolingSetPoint;
                        } else if (LoadToHeatingSetPoint <= 0.0 && LoadToCoolingSetPoint >= 0.0) { // deadband includes zero loads
                            LoadToLoopSetPoint = 0.0;
                        } else {
                            ShowSevereError(state,
                                            "DualSetPointWithDeadBand: Unanticipated combination of heating and cooling loads - report to EnergyPlus "
                                            "Development Team");
                            ShowContinueError(state, "occurs in PlantLoop=" + thisPlantLoop.Name);
                            ShowContinueError(
                                state,
                                format("LoadToHeatingSetPoint={:.3R}, LoadToCoolingSetPoint={:.3R}", LoadToHeatingSetPoint, LoadToCoolingSetPoint));
                            ShowContinueError(state, format("Loop Heating Setpoint={:.2R}", LoopSetPointTemperatureLo));
                            ShowContinueError(state, format("Loop Cooling Setpoint={:.2R}", LoopSetPointTemperatureHi));
                            ShowFatalError(state, "Program terminates due to above conditions.");
                        }
                    } else {
                        LoadToLoopSetPoint = 0.0;
                    }
                }
            }

        } else if (thisPlantLoop.FluidType == DataLoopNode::NodeFluidType::Steam) {

            Real64 Cp = thisPlantLoop.glycol->getSpecificHeat(state, WeightedInletTemp, RoutineName);

            {

                if (thisPlantLoop.LoopDemandCalcScheme == DataPlant::LoopDemandCalcScheme::SingleSetPoint) {

                    // Pick up the loop setpoint temperature
                    Real64 LoopSetPointTemperature = this->TempSetPoint;

                    // Calculate the delta temperature
                    Real64 DeltaTemp = LoopSetPointTemperature - WeightedInletTemp;

                    auto *steam = Fluid::GetSteam(state);
                    Real64 EnthalpySteamSatVapor = steam->getSatEnthalpy(state, LoopSetPointTemperature, 1.0, RoutineNameAlt);
                    Real64 EnthalpySteamSatLiquid = steam->getSatEnthalpy(state, LoopSetPointTemperature, 0.0, RoutineNameAlt);

                    Real64 LatentHeatSteam = EnthalpySteamSatVapor - EnthalpySteamSatLiquid;

                    // Calculate the demand on the loop
                    LoadToLoopSetPoint = SumMdot * (Cp * DeltaTemp + LatentHeatSteam);
                }
            }

        } else { // only have two types, water serves for glycol.
        }

        // Trim the demand to zero if it is very small
        if (std::abs(LoadToLoopSetPoint) < DataPlant::LoopDemandTol) {
            LoadToLoopSetPoint = 0.0;
        }

        return LoadToLoopSetPoint;
    }

    Real64 HalfLoopData::CalcOtherSideDemand(EnergyPlusData &state, Real64 ThisLoopSideFlow)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   August 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // To evaluate the demand to hit the loop setpoint based on the loop side inlet conditions

        // METHODOLOGY EMPLOYED:
        // This routine will simply call the evaluate loop setpoint routine but call it from
        //  the very beginning of this loop side, so that it is basically for the entire loop side

        // FUNCTION PARAMETER DEFINITIONS:
        return this->EvaluateLoopSetPointLoad(state, 1, 1, ThisLoopSideFlow);
    }

    Real64 HalfLoopData::SetupLoopFlowRequest(EnergyPlusData &state, const LoopSideLocation OtherSide)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR:          Dan Fisher, Edwin Lee
        //       DATE WRITTEN:    August 2010
        //       MODIFIED:        na
        //       RE-ENGINEERED:   na

        // PURPOSE OF THIS SUBROUTINE:
        // This routine sets up the flow request values and sums them up for each loop side
        // Then makes a decision on the desired loop flow based on loop configuration

        // METHODOLOGY EMPLOYED:
        // Scan through the components on this loop side, and look at the mass flow request
        //  values on components inlet node.
        // Check common pipe/pumping configuration for this loop side and the other loop side
        //  to determine what the LoopSide should flow

        //~ Initialize
        Real64 LoopFlow = 0.0; // Once all flow requests are evaluated, this is the desired flow on this side

        // reference
        auto &loop(state.dataPlnt->PlantLoop(this->plantLoc.loopNum));

        //~ First we need to set up the flow requests on each LoopSide
        for (DataPlant::LoopSideLocation LoopSideCounter : DataPlant::LoopSideKeys) {
            // Clear things out for this LoopSide
            Real64 InletBranchRequestNeedAndTurnOn = 0.0;
            Real64 InletBranchRequestNeedIfOn = 0.0;
            Real64 ParallelBranchRequestsNeedAndTurnOn(0.0);
            Real64 ParallelBranchRequestsNeedIfOn(0.0);
            Real64 OutletBranchRequestNeedAndTurnOn = 0.0;
            Real64 OutletBranchRequestNeedIfOn = 0.0;

            // reference
            auto &loop_side(loop.LoopSide(LoopSideCounter));

            loop_side.flowRequestNeedIfOn = 0.0;
            loop_side.flowRequestNeedAndTurnOn = 0.0;
            loop_side.flowRequestFinal = 0.0;
            loop_side.hasConstSpeedBranchPumps = false;

            // Now loop through all the branches on this LoopSide and get flow requests
            int const NumBranchesOnThisLoopSide = loop_side.TotalBranches;
            int ParallelBranchIndex = 0;
            for (int BranchCounter = 1; BranchCounter <= NumBranchesOnThisLoopSide; ++BranchCounter) {
                Real64 ThisBranchFlowRequestNeedAndTurnOn = 0.0;
                Real64 ThisBranchFlowRequestNeedIfOn = 0.0;

                // reference
                auto &branch(loop_side.Branch(BranchCounter));

                if (BranchCounter > 1 && BranchCounter < NumBranchesOnThisLoopSide) {
                    ++ParallelBranchIndex;
                }

                if (branch.disableOverrideForCSBranchPumping) {
                    branch.RequestedMassFlow = 0.0;
                    continue;
                }

                int const NumCompsOnThisBranch = branch.TotalComponents;
                for (int CompCounter = 1; CompCounter <= NumCompsOnThisBranch; ++CompCounter) {

                    // reference
                    auto &component(branch.Comp(CompCounter));

                    int NodeToCheckRequest = component.NodeNumIn;
                    LoopFlowStatus FlowPriorityStatus = component.FlowPriority;

                    // reference
                    auto const &node_with_request = state.dataLoopNodes->Node(NodeToCheckRequest);

                    if (!DataPlant::PlantEquipmentTypeIsPump[static_cast<int>(component.Type)]) {

                        if (FlowPriorityStatus == DataPlant::LoopFlowStatus::Invalid) {
                            // do nothing
                        } else if (FlowPriorityStatus == DataPlant::LoopFlowStatus::NeedyAndTurnsLoopOn) {
                            ThisBranchFlowRequestNeedAndTurnOn = max(ThisBranchFlowRequestNeedAndTurnOn, node_with_request.MassFlowRateRequest);
                            ThisBranchFlowRequestNeedIfOn = max(ThisBranchFlowRequestNeedIfOn, node_with_request.MassFlowRateRequest);
                        } else if (FlowPriorityStatus == DataPlant::LoopFlowStatus::NeedyIfLoopOn) {
                            ThisBranchFlowRequestNeedIfOn = max(ThisBranchFlowRequestNeedIfOn, node_with_request.MassFlowRateRequest);
                        } else if (FlowPriorityStatus == DataPlant::LoopFlowStatus::TakesWhatGets) {
                            // do nothing
                        }
                    } else { // handle pumps differently
                        if ((BranchCounter == 1) && (LoopSideCounter == DataPlant::LoopSideLocation::Supply) &&
                            (loop.CommonPipeType == DataPlant::CommonPipeType::TwoWay)) {
                            // special primary side flow request for two way common pipe
                            int const CompIndex = component.CompNum;
                            switch (component.Type) {
                            // remove var speed pumps from this case statement if can set MassFlowRateRequest
                            case DataPlant::PlantEquipmentType::PumpConstantSpeed:
                            case DataPlant::PlantEquipmentType::PumpVariableSpeed:
                            case DataPlant::PlantEquipmentType::PumpBankVariableSpeed:
                                if (CompIndex > 0) {
                                    ThisBranchFlowRequestNeedIfOn =
                                        max(ThisBranchFlowRequestNeedIfOn, state.dataPumps->PumpEquip(CompIndex).MassFlowRateMax);
                                }
                                break;
                            case DataPlant::PlantEquipmentType::PumpBankConstantSpeed:
                                if (CompIndex > 0) {
                                    ThisBranchFlowRequestNeedIfOn = max(ThisBranchFlowRequestNeedIfOn,
                                                                        state.dataPumps->PumpEquip(CompIndex).MassFlowRateMax /
                                                                            state.dataPumps->PumpEquip(CompIndex).NumPumpsInBank);
                                }
                                break;
                            default:
                                ThisBranchFlowRequestNeedIfOn = max(ThisBranchFlowRequestNeedIfOn, node_with_request.MassFlowRateRequest);
                                break;
                            }

                        } else if ((BranchCounter == 1) && (LoopSideCounter == DataPlant::LoopSideLocation::Supply) &&
                                   (loop.CommonPipeType == DataPlant::CommonPipeType::Single)) {
                            int const CompIndex = component.CompNum;
                            switch (component.Type) {
                            // remove var speed pumps from this case statement if can set MassFlowRateRequest
                            case DataPlant::PlantEquipmentType::PumpConstantSpeed:
                            case DataPlant::PlantEquipmentType::PumpVariableSpeed:
                            case DataPlant::PlantEquipmentType::PumpBankVariableSpeed: {
                                if (CompIndex > 0) {
                                    ThisBranchFlowRequestNeedIfOn =
                                        max(ThisBranchFlowRequestNeedIfOn, state.dataPumps->PumpEquip(CompIndex).MassFlowRateMax);
                                }
                                break;
                            }
                            case DataPlant::PlantEquipmentType::PumpBankConstantSpeed:
                                if (CompIndex > 0) {
                                    ThisBranchFlowRequestNeedIfOn = max(ThisBranchFlowRequestNeedIfOn,
                                                                        state.dataPumps->PumpEquip(CompIndex).MassFlowRateMax /
                                                                            state.dataPumps->PumpEquip(CompIndex).NumPumpsInBank);
                                }
                                break;
                            default:
                                ThisBranchFlowRequestNeedIfOn = max(ThisBranchFlowRequestNeedIfOn, node_with_request.MassFlowRateRequest);
                            }
                        } else {
                            int const CompIndex = component.CompNum;
                            switch (component.Type) {
                            case DataPlant::PlantEquipmentType::PumpConstantSpeed:
                                if (CompIndex > 0) {
                                    auto const &this_pump(state.dataPumps->PumpEquip(CompIndex));
                                    if (ParallelBranchIndex >= 1) { // branch pump
                                        if (branch.max_abs_Comp_MyLoad() > HVAC::SmallLoad) {
                                            ThisBranchFlowRequestNeedIfOn = max(ThisBranchFlowRequestNeedIfOn, this_pump.MassFlowRateMax);
                                        } else if (loop.CommonPipeType != DataPlant::CommonPipeType::No) { // common pipe and constant branch pumps
                                            ThisBranchFlowRequestNeedIfOn = max(ThisBranchFlowRequestNeedIfOn, this_pump.MassFlowRateMax);
                                        }
                                        loop_side.hasConstSpeedBranchPumps = true;
                                        branch.HasConstantSpeedBranchPump = true;
                                        branch.ConstantSpeedBranchMassFlow = this_pump.MassFlowRateMax;
                                    } else { // inlet pump
                                        ThisBranchFlowRequestNeedIfOn = max(ThisBranchFlowRequestNeedIfOn, this_pump.MassFlowRateMax);
                                    }
                                }
                                break;
                            case DataPlant::PlantEquipmentType::PumpBankConstantSpeed:
                                if (CompIndex > 0) {
                                    auto const &this_pump(state.dataPumps->PumpEquip(CompIndex));
                                    if (ParallelBranchIndex >= 1) { // branch pump
                                        if (branch.max_abs_Comp_MyLoad() > HVAC::SmallLoad) {
                                            ThisBranchFlowRequestNeedIfOn =
                                                max(ThisBranchFlowRequestNeedIfOn, this_pump.MassFlowRateMax / this_pump.NumPumpsInBank);
                                        } else if (loop.CommonPipeType != DataPlant::CommonPipeType::No) { // common pipe and constant branch pumps
                                            ThisBranchFlowRequestNeedIfOn =
                                                max(ThisBranchFlowRequestNeedIfOn, this_pump.MassFlowRateMax / this_pump.NumPumpsInBank);
                                        }
                                        loop_side.hasConstSpeedBranchPumps = true;
                                        branch.HasConstantSpeedBranchPump = true;
                                        branch.ConstantSpeedBranchMassFlow = this_pump.MassFlowRateMax / this_pump.NumPumpsInBank;
                                    } else { // inlet pump
                                        ThisBranchFlowRequestNeedIfOn =
                                            max(ThisBranchFlowRequestNeedIfOn, this_pump.MassFlowRateMax / this_pump.NumPumpsInBank);
                                    }
                                }
                                break;

                                // overwrite here for branch pumps
                            case DataPlant::PlantEquipmentType::PumpVariableSpeed:
                            case DataPlant::PlantEquipmentType::PumpBankVariableSpeed:
                            case DataPlant::PlantEquipmentType::PumpCondensate:
                                if (component.CompNum > 0) {
                                    auto &this_pump(state.dataPumps->PumpEquip(component.CompNum));
                                    this_pump.LoopSolverOverwriteFlag = false;
                                }
                            default:
                                break;
                            }
                        }
                    }
                }
                if (BranchCounter == 1) { // inlet branch
                    InletBranchRequestNeedAndTurnOn = ThisBranchFlowRequestNeedAndTurnOn;
                    InletBranchRequestNeedIfOn = ThisBranchFlowRequestNeedIfOn;
                } else if (BranchCounter < NumBranchesOnThisLoopSide) { // branchcounter = 1 is already caught
                    ParallelBranchRequestsNeedAndTurnOn += ThisBranchFlowRequestNeedAndTurnOn;
                    ParallelBranchRequestsNeedIfOn += ThisBranchFlowRequestNeedIfOn;
                } else if (BranchCounter == NumBranchesOnThisLoopSide) { // outlet branch
                    OutletBranchRequestNeedAndTurnOn = ThisBranchFlowRequestNeedAndTurnOn;
                    OutletBranchRequestNeedIfOn = ThisBranchFlowRequestNeedIfOn;
                }

                branch.RequestedMassFlow = max(ThisBranchFlowRequestNeedIfOn, ThisBranchFlowRequestNeedAndTurnOn);
            }
            loop_side.flowRequestNeedAndTurnOn =
                max(InletBranchRequestNeedAndTurnOn, ParallelBranchRequestsNeedAndTurnOn, OutletBranchRequestNeedAndTurnOn);
            loop_side.flowRequestNeedIfOn = max(InletBranchRequestNeedIfOn, ParallelBranchRequestsNeedIfOn, OutletBranchRequestNeedIfOn);
        }

        auto &this_loop_side(loop.LoopSide(this->plantLoc.loopSideNum));
        auto &other_loop_side(loop.LoopSide(OtherSide));

        //~ Now that we have calculated each sides different status's requests, process to find final
        if ((this_loop_side.flowRequestNeedAndTurnOn + other_loop_side.flowRequestNeedAndTurnOn) < DataBranchAirLoopPlant::MassFlowTolerance) {
            this_loop_side.flowRequestFinal = 0.0;
            other_loop_side.flowRequestFinal = 0.0;
        } else { // some flow is needed and loop should try to run
            this_loop_side.flowRequestFinal = max(this_loop_side.flowRequestNeedAndTurnOn, this_loop_side.flowRequestNeedIfOn);
            other_loop_side.flowRequestFinal = max(other_loop_side.flowRequestNeedAndTurnOn, other_loop_side.flowRequestNeedIfOn);
        }
        // now store final flow requests on each loop side data structure
        this_loop_side.FlowRequest = this_loop_side.flowRequestFinal;
        other_loop_side.FlowRequest = other_loop_side.flowRequestFinal;

        if (loop.CommonPipeType == DataPlant::CommonPipeType::No) {
            // we may or may not have a pump on this side, but the flow request is the larger of the two side's final
            if ((!this_loop_side.hasConstSpeedBranchPumps) && (!other_loop_side.hasConstSpeedBranchPumps)) {
                LoopFlow = max(this_loop_side.flowRequestFinal, other_loop_side.flowRequestFinal);
            } else { // account for stepped loop flow rates required of branch pumps

                // rules for setting flow when there are constant speed branch pumps.
                // 1. Check if above routines already selected a loop flow rate based on the constant speed branches, if so then just use it
                if (this_loop_side.hasConstSpeedBranchPumps && (this_loop_side.flowRequestFinal >= other_loop_side.flowRequestFinal)) {
                    // okay, just use basic logic
                    LoopFlow = max(this_loop_side.flowRequestFinal, other_loop_side.flowRequestFinal);
                } else if (other_loop_side.hasConstSpeedBranchPumps && (this_loop_side.flowRequestFinal <= other_loop_side.flowRequestFinal)) {
                    // okay, just use basic logic
                    LoopFlow = max(this_loop_side.flowRequestFinal, other_loop_side.flowRequestFinal);
                } else { // not okay, we have a case that will likely need special correcting
                    //  2. determine which loop side has the stepped data
                    DataPlant::LoopSideLocation LoopSideIndex = DataPlant::LoopSideLocation::Invalid;
                    if (this_loop_side.hasConstSpeedBranchPumps && (this_loop_side.flowRequestFinal < other_loop_side.flowRequestFinal)) {
                        LoopSideIndex = this->plantLoc.loopSideNum;
                    } else if (other_loop_side.hasConstSpeedBranchPumps && (other_loop_side.flowRequestFinal < this_loop_side.flowRequestFinal)) {
                        LoopSideIndex = OtherSide;
                    }
                    auto &loop_side(loop.LoopSide(LoopSideIndex));

                    // 3. step through and find out needed information
                    // 3a.  search the loop side with branch pumps and find the steps available with non-zero Myloads
                    // 3b.  search the loop side with branch pumps and find the steps available with zero Myloads
                    //                    LoadedConstantSpeedBranchFlowRateSteps = 0.0;
                    Real64 LoadedConstantSpeedBranchFlowRateSteps_sum = 0.0;
                    this_loop_side.noLoadConstantSpeedBranchFlowRateSteps = 0.0;
                    Real64 NoLoadConstantSpeedBranchFlowRateSteps_sum = 0.0;
                    int ParallelBranchIndex = 0;
                    int const NumBranchesOnThisLoopSide = loop_side.TotalBranches;
                    auto const &loop_branches(loop_side.Branch);
                    for (int BranchCounter = 1; BranchCounter <= NumBranchesOnThisLoopSide; ++BranchCounter) {
                        auto const &loop_branch(loop_branches(BranchCounter));
                        if (BranchCounter > 1 && BranchCounter < NumBranchesOnThisLoopSide) {
                            ++ParallelBranchIndex;
                        }
                        if (loop_branch.HasConstantSpeedBranchPump) {
                            Real64 const branch_mass_flow(loop_branch.ConstantSpeedBranchMassFlow);
                            if (loop_branch.max_abs_Comp_MyLoad() > HVAC::SmallLoad) {
                                LoadedConstantSpeedBranchFlowRateSteps_sum += branch_mass_flow;
                            } else {
                                this_loop_side.noLoadConstantSpeedBranchFlowRateSteps(ParallelBranchIndex) = branch_mass_flow;
                                NoLoadConstantSpeedBranchFlowRateSteps_sum += branch_mass_flow;
                            }
                        }
                    }

                    // 4. allocate which branches to use,
                    Real64 tmpLoopFlow = max(this_loop_side.flowRequestFinal, other_loop_side.flowRequestFinal);
                    Real64 MaxBranchPumpLoopSideFlow = LoadedConstantSpeedBranchFlowRateSteps_sum + NoLoadConstantSpeedBranchFlowRateSteps_sum;
                    tmpLoopFlow = min(tmpLoopFlow, MaxBranchPumpLoopSideFlow);
                    //  4b. first use all the branches with non-zero MyLoad
                    if (tmpLoopFlow > LoadedConstantSpeedBranchFlowRateSteps_sum) {
                        Real64 AccumFlowSteps = LoadedConstantSpeedBranchFlowRateSteps_sum;
                        ParallelBranchIndex = 0;
                        for (int BranchCounter = 1; BranchCounter <= NumBranchesOnThisLoopSide; ++BranchCounter) {
                            if (BranchCounter > 1 && BranchCounter < NumBranchesOnThisLoopSide) {
                                ++ParallelBranchIndex;
                            } else {
                                continue;
                            }
                            Real64 const steps = this_loop_side.noLoadConstantSpeedBranchFlowRateSteps(ParallelBranchIndex);
                            if (steps > 0.0) { // add in branches with zero MyLoad  in branch input order until satisfied
                                if (tmpLoopFlow > AccumFlowSteps) {
                                    if (tmpLoopFlow <= AccumFlowSteps + steps) { // found it set requests and exit
                                        tmpLoopFlow = AccumFlowSteps + steps;
                                        loop_side.Branch(BranchCounter).RequestedMassFlow = steps;
                                        LoopFlow = tmpLoopFlow;
                                        break;
                                    } else {
                                        AccumFlowSteps += steps;
                                        loop_side.Branch(BranchCounter).RequestedMassFlow = steps;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } else if (loop.CommonPipeType == DataPlant::CommonPipeType::TwoWay) {
            LoopFlow = this_loop_side.flowRequestFinal;
        } else if (loop.CommonPipeType == DataPlant::CommonPipeType::Single) {
            LoopFlow = this_loop_side.flowRequestFinal;
        }

        // overrides the loop solver flow request to allow loop pump to turn off when not in use
        if (this_loop_side.TotalPumps == 1) {
            if (LoopFlow < HVAC::VerySmallMassFlow) { // Update from dataconvergetols...
                for (int BranchCounter = 1; BranchCounter <= this_loop_side.TotalBranches; ++BranchCounter) {
                    // reference
                    auto &branch(this_loop_side.Branch(BranchCounter));
                    int const NumCompsOnThisBranch = branch.TotalComponents;
                    for (int CompCounter = 1; CompCounter <= NumCompsOnThisBranch; ++CompCounter) {
                        auto const &component(branch.Comp(CompCounter));
                        switch (component.Type) {
                        case DataPlant::PlantEquipmentType::PumpVariableSpeed:
                        case DataPlant::PlantEquipmentType::PumpBankVariableSpeed:
                        case DataPlant::PlantEquipmentType::PumpCondensate:
                            if (component.CompNum > 0) {
                                auto &this_pump(state.dataPumps->PumpEquip(component.CompNum));
                                this_pump.LoopSolverOverwriteFlag = true;
                            }
                        default:
                            break;
                        }
                    }
                }
            }
        }

        return LoopFlow;
    }

    void HalfLoopData::DoFlowAndLoadSolutionPass(EnergyPlusData &state, LoopSideLocation OtherSide, int ThisSideInletNode, bool FirstHVACIteration)
    {

        // This is passed in-out deep down into the depths where the load op manager calls EMS and EMS can shut down pumps
        bool LoopShutDownFlag = false;

        // First thing is to setup mass flow request information
        Real64 ThisLoopSideFlowRequest = this->SetupLoopFlowRequest(state, OtherSide);

        // Now we know what the loop would "like" to run at, let's see the pump
        // operation range (min/max avail) to see whether it is possible this time around
        Real64 ThisLoopSideFlow = this->DetermineLoopSideFlowRate(state, ThisSideInletNode, ThisLoopSideFlowRequest);

        for (auto &branch : this->Branch) {
            branch.lastComponentSimulated = 0;
        }

        // We also need to establish a baseline "other-side-based" loop demand based on this possible flow rate
        this->InitialDemandToLoopSetPoint = this->CalcOtherSideDemand(state, ThisLoopSideFlow);
        this->UpdatedDemandToLoopSetPoint = this->InitialDemandToLoopSetPoint;
        this->LoadToLoopSetPointThatWasntMet = 0.0;

        // We now have a loop side flow request, along with inlet min/max avails.
        // We can now make a first pass through the component simulation, requesting flow as necessary.
        // Normal "supply side" components will set a mass flow rate on their outlet node to request flow,
        // while "Demand side" components will set a a mass flow request on their inlet node to request flow.
        this->FlowLock = DataPlant::FlowLock::Unlocked;
        this->SimulateAllLoopSideBranches(state, ThisLoopSideFlow, FirstHVACIteration, LoopShutDownFlag);

        // discussion/comments about loop solver/flow resolver interaction
        // At this point, the components have been simulated.  They should have either:
        //  - logged a massflowrequest
        //  - or logged a MassFlowRate
        // We need to decide what the components are going to do on FlowLock=0.
        // If we want all control here at the solver level, the components just need to
        //  log their MassFlowRate on their outlet nodes, or some other mechanism.
        // Then the loop solver can scan the branch and get the max, and this will be the requested
        //  flow rate for the branch.
        // The loop solver will then set this as the branch outlet mass flow rate in preparation
        //  for the flow resolver.
        // The loop solver may need to do something to the inlet/outlet branch, but I'm not sure yet.
        // The following comment block is what I had already thought of, and it may still make sense.

        // Now that all the flow requests have been logged, we need to prepare them for the
        //  flow resolver.  This will just take the requests and determine the desired flow
        //  request for that branch according to pump placement, pump type, and other component
        //  conditions.  In many cases, this will just be to simply take the max request from
        //  the branch, which will already be within pumping limits for that flow path.
        // We can then call the flow resolver to lock down branch inlet flow rates.

        // The flow resolver takes information such as requested flows and min/max available flows and
        //  sets the corrected flow on the inlet to each parallel branch
        this->ResolveParallelFlows(state, ThisLoopSideFlow, FirstHVACIteration);

        // Re-Initialize variables for this next pass
        this->InitialDemandToLoopSetPointSAVED = this->InitialDemandToLoopSetPoint;
        this->CurrentAlterationsToDemand = 0.0;
        this->UpdatedDemandToLoopSetPoint = this->InitialDemandToLoopSetPoint;

        // Now that flow rates have been resolved, we just need to set the flow lock status
        //  flag, and resimulate.  During this simulation each component will still use the
        //  SetFlowRequest routine, but this routine will also set the outlet flow rate
        //  equal to the inlet flow rate, according to flowlock logic.
        this->FlowLock = DataPlant::FlowLock::Locked;
        this->SimulateAllLoopSideBranches(state, ThisLoopSideFlow, FirstHVACIteration, LoopShutDownFlag);
    }

    void HalfLoopData::ResolveParallelFlows(EnergyPlusData &state,
                                            Real64 const ThisLoopSideFlow, // [kg/s]  total flow to be split
                                            bool const FirstHVACIteration  // TRUE if First HVAC iteration of Time step
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brandon Anderson, Dan Fisher
        //       DATE WRITTEN   October 1999
        //       MODIFIED       May 2005 Sankaranarayanan K P, Rich Liesen
        //       RE-ENGINEERED  Sept 2010 Dan Fisher, Brent Griffith for demand side update

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine takes the overall loop side flow and distributes
        // it among parallel branches. this is the main implementation of
        // flow splitting for plant splitter/mixer

        // METHODOLOGY EMPLOYED:
        // Flow through the branches is currently determined by
        // the active component on the branch, as well as the
        // order of the branches following the splitter.
        // SimPlantEquipment is run first, and the active components
        // request their flow.  These flows are compared and a simple
        // algorithm balances flow in the branches.  The flow in these
        // branches is then locked down, via MassFlowRateMaxAvail and MinAvail
        // SimPlant Equipment is then run again in order to get correct
        // properties.  Finally, Max/MinAvail are reset for the next time step.

        // Using/Aliasing

        // SUBROUTINE PARAMETER DEFINITIONS:
        int constexpr LoopSideSingleBranch(1); // For readability

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 ActiveFlowRate;        // The flow available when cycling through branches
        Real64 PassiveFlowRate;       // The flow available when cycling through branches
        Real64 FracFlow;              // The flow available when cycling through branches
        Real64 ThisBranchRequestFrac; // The request ratio
        Real64 totalMax;              // The flow available when cycling through branches
        Real64 FlowRemaining;         // The flow available when cycling through branches
        int LastNodeOnBranch;         // intermediate value used for better readabilty
        int FirstNodeOnBranch;        // intermediate value used for better readabilty
        Real64 BranchFlowReq;
        Real64 BranchMinAvail;
        Real64 BranchMaxAvail;
        Real64 ParallelBranchMaxAvail;
        Real64 ParallelBranchMinAvail;
        Real64 TotParallelBranchFlowReq;
        Real64 StartingFlowRate;
        Real64 ThisBranchRequest;

        // If there is no splitter then there is no continuity to enforce.
        if (!this->Splitter.Exists) {

            // If there's only one branch, then RETURN
            if (this->TotalBranches == 1) {
                // The branch should just try to meet the request previously calculated.  This should be good,
                // just need to make sure that during FlowUnlocked, no one constrained Min/Max farther.
                // This would have been propagated down the branch, so we can check the outlet node min/max avail for this.
                auto const &this_single_branch(this->Branch(LoopSideSingleBranch));
                LastNodeOnBranch = this_single_branch.NodeNumOut;
                FirstNodeOnBranch = this_single_branch.NodeNumIn;
                BranchMinAvail = state.dataLoopNodes->Node(LastNodeOnBranch).MassFlowRateMinAvail;
                BranchMaxAvail = state.dataLoopNodes->Node(LastNodeOnBranch).MassFlowRateMaxAvail;
                state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate = min(max(ThisLoopSideFlow, BranchMinAvail), BranchMaxAvail);
                // now with flow locked, this single branch will just ran at the specified flow rate, so we are done
                return;
            } else {
                ShowSevereError(state, "Plant topology problem on \"" + this->loopSideDescription + "\"");
                ShowContinueError(state, "There are multiple branches, yet no splitter.  This is an invalid configuration.");
                ShowContinueError(state, "Add a set of connectors, use put components on a single branch.");
                ShowFatalError(state, "Invalid plant topology causes program termination.");
                return;
            }
        }

        // If a splitter/mixer combination exist on the loop
        if (this->Splitter.Exists && this->Mixer.Exists) {

            // Zero out local variables
            TotParallelBranchFlowReq = 0.0;
            int NumSplitOutlets = this->Splitter.TotalOutletNodes;
            if (NumSplitOutlets < 1) {
                ShowSevereError(state, "Plant topology problem on \"" + this->loopSideDescription + "\"");
                ShowContinueError(state, "Diagnostic error in PlantLoopSolver::ResolveParallelFlows.");
                ShowContinueError(state, "Splitter improperly specified, no splitter outlets.");
                ShowFatalError(state, "Invalid plant topology causes program termination.");
            }

            int NumActiveBranches = 0;
            ParallelBranchMaxAvail = 0.0;
            ParallelBranchMinAvail = 0.0;
            for (int iBranch = 1; iBranch <= NumSplitOutlets; ++iBranch) {

                int BranchNum = this->Splitter.BranchNumOut(iBranch);
                auto &this_branch(this->Branch(BranchNum));
                int SplitterBranchOut = this->Splitter.BranchNumOut(iBranch);
                auto const &this_splitter_outlet_branch(this->Branch(SplitterBranchOut));
                LastNodeOnBranch = this_branch.NodeNumOut;
                FirstNodeOnBranch = this_branch.NodeNumIn;
                BranchFlowReq = this_branch.DetermineBranchFlowRequest(state);
                this_branch.RequestedMassFlow = BranchFlowReq; // store this for later use in logic for remaining flow allocations
                // now, if we are have branch pumps, here is the situation:
                // constant speed pumps lock in a flow request on the inlet node
                // variable speed pumps which have other components on the branch do not log a request themselves
                // the DetermineBranchFlowRequest routine only looks at the branch inlet node
                // for variable speed branch pumps then, this won't work because the branch will be requesting zero
                // so let's adjust for this here to make sure these branches get good representation
                // This comment above is not true, for series active branches, DetermineBranchFlowRequest does scan down the branch's
                // components already, no need to loop over components
                BranchMinAvail = state.dataLoopNodes->Node(LastNodeOnBranch).MassFlowRateMinAvail;
                BranchMaxAvail = state.dataLoopNodes->Node(LastNodeOnBranch).MassFlowRateMaxAvail;
                //            !sum the branch flow requests to a total parallel branch flow request
                bool activeBranch = this_splitter_outlet_branch.controlType == DataBranchAirLoopPlant::ControlType::Active;
                bool isSeriesActiveAndRequesting =
                    (this_splitter_outlet_branch.controlType == DataBranchAirLoopPlant::ControlType::SeriesActive) && (BranchFlowReq > 0.0);
                if (activeBranch || isSeriesActiveAndRequesting) { // revised logic for series active
                    TotParallelBranchFlowReq += BranchFlowReq;
                    ++NumActiveBranches;
                }
                state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate = BranchFlowReq;
                state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRateMinAvail = BranchMinAvail;
                state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRateMaxAvail = BranchMaxAvail;
                ParallelBranchMaxAvail += BranchMaxAvail;
                ParallelBranchMinAvail += BranchMinAvail;
            }
            //            ! Find branch number and flow rates at splitter inlet
            int SplitterBranchIn = this->Splitter.BranchNumIn;
            int FirstNodeOnBranchIn = this->Branch(SplitterBranchIn).NodeNumIn;
            //            ! Find branch number and flow rates at mixer outlet
            int MixerBranchOut = this->Mixer.BranchNumOut;
            LastNodeOnBranch = this->Branch(MixerBranchOut).NodeNumOut;
            int FirstNodeOnBranchOut = this->Branch(MixerBranchOut).NodeNumIn;

            auto &first_branch_inlet_node(state.dataLoopNodes->Node(FirstNodeOnBranchIn));
            auto &last_branch_inlet_node(state.dataLoopNodes->Node(FirstNodeOnBranchOut));

            // Reset branch inlet node flow rates for the first and last branch on loop
            first_branch_inlet_node.MassFlowRate = ThisLoopSideFlow;
            last_branch_inlet_node.MassFlowRate = ThisLoopSideFlow;

            // Reset branch inlet node Min/MaxAvails for the first and last branch on loop
            first_branch_inlet_node.MassFlowRateMaxAvail = min(first_branch_inlet_node.MassFlowRateMaxAvail, ParallelBranchMaxAvail);
            first_branch_inlet_node.MassFlowRateMaxAvail =
                min(first_branch_inlet_node.MassFlowRateMaxAvail, last_branch_inlet_node.MassFlowRateMaxAvail);
            first_branch_inlet_node.MassFlowRateMinAvail = max(first_branch_inlet_node.MassFlowRateMinAvail, ParallelBranchMinAvail);
            first_branch_inlet_node.MassFlowRateMinAvail =
                max(first_branch_inlet_node.MassFlowRateMinAvail, last_branch_inlet_node.MassFlowRateMinAvail);
            last_branch_inlet_node.MassFlowRateMinAvail = first_branch_inlet_node.MassFlowRateMinAvail;
            last_branch_inlet_node.MassFlowRateMaxAvail = first_branch_inlet_node.MassFlowRateMaxAvail;

            // Initialize the remaining flow variable
            FlowRemaining = ThisLoopSideFlow;

            // Initialize flow on passive, bypass and uncontrolled parallel branches to zero.  For these branches
            // MinAvail is not enforced
            for (int OutletNum = 1; OutletNum <= NumSplitOutlets; ++OutletNum) {
                int SplitterBranchOut = this->Splitter.BranchNumOut(OutletNum);
                FirstNodeOnBranch = this->Branch(SplitterBranchOut).NodeNumIn;
                if (this->Branch(SplitterBranchOut).controlType != DataBranchAirLoopPlant::ControlType::Active &&
                    this->Branch(SplitterBranchOut).controlType != DataBranchAirLoopPlant::ControlType::SeriesActive) {
                    state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate = 0.0;
                    this->PushBranchFlowCharacteristics(
                        state, SplitterBranchOut, state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate, FirstHVACIteration);
                }
            }

            // IF SUFFICIENT FLOW TO MEET ALL PARALLEL BRANCH FLOW REQUESTS
            if (FlowRemaining < DataBranchAirLoopPlant::MassFlowTolerance) { // no flow available at all for splitter
                for (int OutletNum = 1; OutletNum <= NumSplitOutlets; ++OutletNum) {
                    int SplitterBranchOut = this->Splitter.BranchNumOut(OutletNum);
                    for (int CompCounter = 1; CompCounter <= this->Branch(SplitterBranchOut).TotalComponents; ++CompCounter) {

                        FirstNodeOnBranch = this->Branch(SplitterBranchOut).NodeNumIn;
                        int CompInletNode = this->Branch(SplitterBranchOut).Comp(CompCounter).NodeNumIn;
                        int CompOutletNode = this->Branch(SplitterBranchOut).Comp(CompCounter).NodeNumOut;
                        state.dataLoopNodes->Node(CompInletNode).MassFlowRate = 0.0;
                        state.dataLoopNodes->Node(CompInletNode).MassFlowRateMaxAvail = 0.0;
                        state.dataLoopNodes->Node(CompOutletNode).MassFlowRate = 0.0;
                        state.dataLoopNodes->Node(CompOutletNode).MassFlowRateMaxAvail = 0.0;
                    }
                }
                return;
            } else if (FlowRemaining >= TotParallelBranchFlowReq) {

                // 1) Satisfy flow demand of ACTIVE splitter outlet branches
                for (int OutletNum = 1; OutletNum <= NumSplitOutlets; ++OutletNum) {
                    int SplitterBranchOut = this->Splitter.BranchNumOut(OutletNum);
                    FirstNodeOnBranch = this->Branch(SplitterBranchOut).NodeNumIn;
                    if (this->Branch(SplitterBranchOut).controlType == DataBranchAirLoopPlant::ControlType::Active ||
                        this->Branch(SplitterBranchOut).controlType == DataBranchAirLoopPlant::ControlType::SeriesActive) {
                        // branch flow is min of requested flow and remaining flow
                        state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate =
                            min(state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate, FlowRemaining);
                        if (state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate < DataBranchAirLoopPlant::MassFlowTolerance) {
                            state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate = 0.0;
                        }
                        this->PushBranchFlowCharacteristics(
                            state, SplitterBranchOut, state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate, FirstHVACIteration);
                        FlowRemaining -= state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate;
                        if (FlowRemaining < DataBranchAirLoopPlant::MassFlowTolerance) {
                            FlowRemaining = 0.0;
                        }
                    }
                }
                // IF the active branches take the entire loop flow, return
                if (FlowRemaining == 0.0) {
                    return;
                }

                // 2) Distribute remaining flow to PASSIVE branches
                totalMax = 0.0;
                for (int OutletNum = 1; OutletNum <= NumSplitOutlets; ++OutletNum) {
                    int SplitterBranchOut = this->Splitter.BranchNumOut(OutletNum);
                    FirstNodeOnBranch = this->Branch(SplitterBranchOut).NodeNumIn;
                    if (this->Branch(SplitterBranchOut).controlType == DataBranchAirLoopPlant::ControlType::Passive) {
                        // Calculate the total max available
                        totalMax += state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRateMaxAvail;
                    }
                }

                if (totalMax > 0) {
                    for (int OutletNum = 1; OutletNum <= NumSplitOutlets; ++OutletNum) {
                        int SplitterBranchOut = this->Splitter.BranchNumOut(OutletNum);
                        FirstNodeOnBranch = this->Branch(SplitterBranchOut).NodeNumIn;
                        if (this->Branch(SplitterBranchOut).controlType == DataBranchAirLoopPlant::ControlType::Passive) {
                            FracFlow = FlowRemaining / totalMax;
                            if (FracFlow <= 1.0) { // the passive branches will take all the flow
                                PassiveFlowRate = FracFlow * state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRateMaxAvail;
                                // Check against FlowRemaining
                                PassiveFlowRate = min(FlowRemaining, PassiveFlowRate);
                                // Allow FlowRequest to be increased to meet minimum on branch
                                PassiveFlowRate = max(PassiveFlowRate, state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRateMinAvail);
                                FlowRemaining = max((FlowRemaining - PassiveFlowRate), 0.0);
                                state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate = PassiveFlowRate;
                            } else { // Each Branch receives maximum flow and BYPASS must be used
                                state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate =
                                    min(state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRateMaxAvail, FlowRemaining);
                                FlowRemaining -= state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate;
                            }
                            this->PushBranchFlowCharacteristics(
                                state, SplitterBranchOut, state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate, FirstHVACIteration);
                        }
                    }
                } // totalMax <=0 and flow should be assigned to active branches
                // IF the passive branches take the remaining loop flow, return
                if (FlowRemaining == 0.0) {
                    return;
                }

                // 3) Distribute remaining flow to the BYPASS
                for (int OutletNum = 1; OutletNum <= this->Splitter.TotalOutletNodes; ++OutletNum) {
                    int SplitterBranchOut = this->Splitter.BranchNumOut(OutletNum);
                    FirstNodeOnBranch = this->Branch(SplitterBranchOut).NodeNumIn;
                    if (this->Branch(SplitterBranchOut).controlType == DataBranchAirLoopPlant::ControlType::Bypass) {
                        state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate =
                            min(FlowRemaining, state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRateMaxAvail);
                        this->PushBranchFlowCharacteristics(
                            state, SplitterBranchOut, state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate, FirstHVACIteration);
                        FlowRemaining -= state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate;
                    }
                }
                // IF the bypass take the remaining loop flow, return
                if (FlowRemaining == 0.0) {
                    return;
                }

                // 4) If PASSIVE branches and BYPASS are at max and there's still flow, distribute remaining flow to ACTIVE branches but only those
                // that had a non-zero flow request. Try to leave branches off that wanted to be off.
                if (NumActiveBranches > 0) {
                    ActiveFlowRate = FlowRemaining / NumActiveBranches; // denominator now only includes active branches that wanted to be "on"
                    for (int OutletNum = 1; OutletNum <= NumSplitOutlets; ++OutletNum) {
                        int SplitterBranchOut = this->Splitter.BranchNumOut(OutletNum);
                        FirstNodeOnBranch = this->Branch(SplitterBranchOut).NodeNumIn;
                        bool branchIsActive = this->Branch(SplitterBranchOut).controlType == DataBranchAirLoopPlant::ControlType::Active;
                        bool branchIsSeriesActiveAndRequesting =
                            this->Branch(SplitterBranchOut).controlType == DataBranchAirLoopPlant::ControlType::SeriesActive &&
                            this->Branch(SplitterBranchOut).RequestedMassFlow > 0.0;
                        if (branchIsActive || branchIsSeriesActiveAndRequesting) { // only series active branches that want to be "on"
                            // check Remaining flow (should be correct!)
                            ActiveFlowRate = min(ActiveFlowRate, FlowRemaining);
                            // set the flow rate to the MIN((MassFlowRate+AvtiveFlowRate), MaxAvail)
                            StartingFlowRate = state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate;
                            state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate =
                                min((state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate + ActiveFlowRate),
                                    state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRateMaxAvail);
                            this->PushBranchFlowCharacteristics(
                                state, SplitterBranchOut, state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate, FirstHVACIteration);
                            // adjust the remaining flow
                            FlowRemaining -= (state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate - StartingFlowRate);
                        }
                        if (FlowRemaining == 0) {
                            break;
                        }
                    }
                    // IF the active branches take the remaining loop flow, return
                    if (FlowRemaining == 0.0) {
                        return;
                    }

                    // 5)  Step 4) could have left ACTIVE branches < MaxAvail.  Check to makes sure all ACTIVE branches are at MaxAvail
                    for (int OutletNum = 1; OutletNum <= NumSplitOutlets; ++OutletNum) {
                        int SplitterBranchOut = this->Splitter.BranchNumOut(OutletNum);
                        FirstNodeOnBranch = this->Branch(SplitterBranchOut).NodeNumIn;
                        if (this->Branch(SplitterBranchOut).controlType == DataBranchAirLoopPlant::ControlType::Active ||
                            this->Branch(SplitterBranchOut).controlType == DataBranchAirLoopPlant::ControlType::SeriesActive) {
                            StartingFlowRate = state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate;
                            ActiveFlowRate =
                                min(FlowRemaining, (state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRateMaxAvail - StartingFlowRate));
                            FlowRemaining -= ActiveFlowRate;
                            state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate = StartingFlowRate + ActiveFlowRate;
                            this->PushBranchFlowCharacteristics(
                                state, SplitterBranchOut, state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate, FirstHVACIteration);
                        }
                    }
                }
                // IF the active branches take the remaining loop flow, return
                if (FlowRemaining == 0.0) {
                    return;
                }

                // 6) Adjust Inlet branch and outlet branch flow rates to match parallel branch rate
                TotParallelBranchFlowReq = 0.0;
                for (int iBranch = 1; iBranch <= NumSplitOutlets; ++iBranch) {
                    int BranchNum = this->Splitter.BranchNumOut(iBranch);
                    FirstNodeOnBranch = this->Branch(BranchNum).NodeNumIn;
                    // calculate parallel branch flow rate
                    TotParallelBranchFlowReq += state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate;
                }
                // Reset the flow on the splitter inlet branch
                SplitterBranchIn = this->Splitter.BranchNumIn;
                FirstNodeOnBranchIn = this->Branch(SplitterBranchIn).NodeNumIn;
                state.dataLoopNodes->Node(FirstNodeOnBranchIn).MassFlowRate = TotParallelBranchFlowReq;
                this->PushBranchFlowCharacteristics(
                    state, SplitterBranchIn, state.dataLoopNodes->Node(FirstNodeOnBranchIn).MassFlowRate, FirstHVACIteration);
                // Reset the flow on the Mixer outlet branch
                MixerBranchOut = this->Mixer.BranchNumOut;
                FirstNodeOnBranchOut = this->Branch(MixerBranchOut).NodeNumIn;
                state.dataLoopNodes->Node(FirstNodeOnBranchOut).MassFlowRate = TotParallelBranchFlowReq;
                this->PushBranchFlowCharacteristics(
                    state, MixerBranchOut, state.dataLoopNodes->Node(FirstNodeOnBranchOut).MassFlowRate, FirstHVACIteration);
                return;

                // IF INSUFFICIENT FLOW TO MEET ALL PARALLEL BRANCH FLOW REQUESTS
            } else {

                // 1) apportion flow based on requested fraction of total
                for (int OutletNum = 1; OutletNum <= NumSplitOutlets; ++OutletNum) {

                    int SplitterBranchOut = this->Splitter.BranchNumOut(OutletNum);
                    ThisBranchRequest = this->Branch(SplitterBranchOut).DetermineBranchFlowRequest(state);
                    FirstNodeOnBranch = this->Branch(SplitterBranchOut).NodeNumIn;
                    auto &this_splitter_outlet_branch(this->Branch(SplitterBranchOut));

                    if ((this_splitter_outlet_branch.controlType == DataBranchAirLoopPlant::ControlType::Active) ||
                        (this_splitter_outlet_branch.controlType == DataBranchAirLoopPlant::ControlType::SeriesActive)) {

                        // since we are calculating this fraction based on the total parallel request calculated above, we must mimic the logic to
                        // make sure the math works every time that means we must make the variable speed pump correction here as well.
                        for (int CompCounter = 1; CompCounter <= this_splitter_outlet_branch.TotalComponents; ++CompCounter) {

                            auto const &this_comp(this_splitter_outlet_branch.Comp(CompCounter));

                            // if this isn't a variable speed pump then just keep cycling
                            if ((this_comp.Type != PlantEquipmentType::PumpVariableSpeed) &&
                                (this_comp.Type != PlantEquipmentType::PumpBankVariableSpeed)) {
                                continue;
                            }

                            int CompInletNode = this_comp.NodeNumIn;
                            ThisBranchRequest = max(ThisBranchRequest, state.dataLoopNodes->Node(CompInletNode).MassFlowRateRequest);
                        }

                        ThisBranchRequestFrac = ThisBranchRequest / TotParallelBranchFlowReq;
                        //    FracFlow = state.dataLoopNodes->Node(FirstNodeOnBranch)%MassFlowRate/TotParallelBranchFlowReq
                        //    state.dataLoopNodes->Node(FirstNodeOnBranch)%MassFlowRate = MIN((FracFlow *
                        //    state.dataLoopNodes->Node(FirstNodeOnBranch)%MassFlowRate),FlowRemaining)
                        state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate = ThisBranchRequestFrac * ThisLoopSideFlow;
                        this->PushBranchFlowCharacteristics(
                            state, SplitterBranchOut, state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate, FirstHVACIteration);
                        FlowRemaining -= state.dataLoopNodes->Node(FirstNodeOnBranch).MassFlowRate;
                    }
                }

                // 1b) check if flow all apportioned
                if (FlowRemaining > DataBranchAirLoopPlant::MassFlowTolerance) {
                    // Call fatal diagnostic error. !The math should work out!
                    ShowSevereError(state, "ResolveParallelFlows: Dev note, failed to redistribute restricted flow");
                    ShowContinueErrorTimeStamp(state, "");
                    ShowContinueError(state, format("Loop side flow = {:.8R} (kg/s)", ThisLoopSideFlow));
                    ShowContinueError(state, format("Flow Remaining = {:.8R} (kg/s)", FlowRemaining));
                    ShowContinueError(state, format("Parallel Branch requests  = {:.8R} (kg/s)", TotParallelBranchFlowReq));
                }

                // 2)  ! Reset the flow on the Mixer outlet branch
                MixerBranchOut = this->Mixer.BranchNumOut;
                FirstNodeOnBranchOut = this->Branch(MixerBranchOut).NodeNumIn;
                state.dataLoopNodes->Node(FirstNodeOnBranchOut).MassFlowRate = TotParallelBranchFlowReq;
                this->PushBranchFlowCharacteristics(
                    state, MixerBranchOut, state.dataLoopNodes->Node(FirstNodeOnBranchOut).MassFlowRate, FirstHVACIteration);

            } // Total flow requested >= or < Total parallel request

        } // Splitter/Mixer exists
    }

    void HalfLoopData::SimulateLoopSideBranchGroup(EnergyPlusData &state,
                                                   int const FirstBranchNum,
                                                   int const LastBranchNum,
                                                   Real64 FlowRequest,
                                                   bool const FirstHVACIteration,
                                                   bool &LoopShutDownFlag)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   July 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This routine will manage the component simulation on a single set of parallel branches
        // This routine also reverts to a single branch simulation if there isn't a set of parallel branches

        // METHODOLOGY EMPLOYED:
        // Loop through all components, and simulate first the non-load range based on each branch.
        // When a load-range based (LRB) is encountered, the simulation moves to the next branch to do non-LRB components.
        // When all paths are exhausted the simulation begins simulating LRB components.  Before each comp, the load distribution
        //  engine is called to handle the load distribution for this current pass.  If load is successfully distributed, this is
        //  flagged, and not called again.  If load is not distributed (i.e. this component isn't ON right now), then the
        //  load distribution engine will be called again before the next component.
        // After all load distribution is done and those components are complete, the simulation moves back to do any
        //  remaining components that may be downstream.

        //~ Flags
        bool LoadDistributionWasPerformed;

        //~ General variables
        Real64 LoadToLoopSetPoint;
        PlantLocation PumpLocation;
        LoadToLoopSetPoint = 0.0;

        // We now know what plant simulation region is available to us, let's simulate this group
        bool EncounteredLRBObjDuringPass1(false);
        for (int BranchCounter = FirstBranchNum; BranchCounter <= LastBranchNum; ++BranchCounter) {
            auto &branch(this->Branch(BranchCounter));

            //~ Always start from the last component we did the last time around + 1 and
            //~  try to make it all the way to the end of the loop
            int const StartingComponent = branch.lastComponentSimulated + 1;
            int const EndingComponent = branch.TotalComponents;
            for (int CompCounter = StartingComponent; CompCounter <= EndingComponent; ++CompCounter) {

                auto &this_comp(branch.Comp(CompCounter));
                PlantLocation this_plantLoc = {this->plantLoc.loopNum, this->plantLoc.loopSideNum, BranchCounter, CompCounter};
                DataPlant::OpScheme const CurOpSchemeType(this_comp.CurOpSchemeType);

                switch (CurOpSchemeType) {
                case DataPlant::OpScheme::WSEcon: //~ coils
                    this_comp.MyLoad = UpdatedDemandToLoopSetPoint;
                    branch.Comp(CompCounter).simulate(state, FirstHVACIteration);
                    break;
                case DataPlant::OpScheme::Pump: //~ pump
                    if (this->BranchPumpsExist) {
                        SimulateSinglePump(state, this_comp.location, branch.RequestedMassFlow);
                    } else {
                        SimulateSinglePump(state, this_comp.location, FlowRequest);
                    }
                    break;
                case DataPlant::OpScheme::CompSetPtBased:
                    PlantCondLoopOperation::ManagePlantLoadDistribution(state,
                                                                        this_plantLoc,
                                                                        LoadToLoopSetPoint,
                                                                        LoadToLoopSetPointThatWasntMet,
                                                                        FirstHVACIteration,
                                                                        LoopShutDownFlag,
                                                                        LoadDistributionWasPerformed);
                    branch.Comp(CompCounter).simulate(state, FirstHVACIteration);
                    break;
                case DataPlant::OpScheme::EMS:
                    if (this->plantLoc.loopSideNum == DataPlant::LoopSideLocation::Supply) {
                        int const curCompOpSchemePtr = this_comp.CurCompLevelOpNum;
                        int const OpSchemePtr = this_comp.OpScheme(curCompOpSchemePtr).OpSchemePtr;
                        state.dataPlnt->PlantLoop(this->plantLoc.loopNum).OpScheme(OpSchemePtr).EMSIntVarLoopDemandRate = InitialDemandToLoopSetPoint;
                    }
                    PlantCondLoopOperation::ManagePlantLoadDistribution(state,
                                                                        this_plantLoc,
                                                                        UpdatedDemandToLoopSetPoint,
                                                                        LoadToLoopSetPointThatWasntMet,
                                                                        FirstHVACIteration,
                                                                        LoopShutDownFlag,
                                                                        LoadDistributionWasPerformed);
                    branch.Comp(CompCounter).simulate(state, FirstHVACIteration);
                    break;
                case OpScheme::WetBulbRB:
                case OpScheme::DryBulbRB:
                case OpScheme::DewPointRB:
                case OpScheme::RelHumRB:
                case OpScheme::DryBulbTDB:
                case OpScheme::WetBulbTDB:
                case OpScheme::DewPointTDB:
                case OpScheme::HeatingRB:
                case OpScheme::CoolingRB: { //~ load range based
                    EncounteredLRBObjDuringPass1 = true;
                    goto components_end; // don't do any more components on this branch
                }
                default: // demand, etc.
                    branch.Comp(CompCounter).simulate(state, FirstHVACIteration);
                }

                // Update loop demand as needed for changes this component may have made
                this->UpdateAnyLoopDemandAlterations(state, BranchCounter, CompCounter);

                //~ If we didn't EXIT early, we must have simulated, so update array
                branch.lastComponentSimulated = CompCounter;

            } //~ CompCounter
        components_end:;

            if (this->FlowLock == DataPlant::FlowLock::Locked) {
                PlantPressureSystem::SimPressureDropSystem(
                    state, this->plantLoc.loopNum, FirstHVACIteration, DataPlant::PressureCall::Calc, this->plantLoc.loopSideNum, BranchCounter);
            }

        } //~ BranchCounter

        // So now we have made one pass through all of the available components on these branches, skipping load based
        // If we didn't encounter any load based objects during the first pass, then we must be done!
        if (!EncounteredLRBObjDuringPass1) {
            return;
        }

        // If we have load based now, we should go ahead and distribute the load
        // If not then this branch group is done, since flow path validation was previously done
        LoadToLoopSetPoint = UpdatedDemandToLoopSetPoint;
        LoadDistributionWasPerformed = false;

        // The way the load distribution is set up, I think I should call this for every load range based component
        //  encountered until distribution is actually performed.  If we don't call for each component then we may
        //  call for a component that is not on the current equip list and then nothing would come on.
        bool EncounteredNonLBObjDuringPass2(false);
        for (int BranchCounter = FirstBranchNum; BranchCounter <= LastBranchNum; ++BranchCounter) {
            auto &branch(this->Branch(BranchCounter));

            //~ Always start from the last component we did the last time around + 1 and
            //~  try to make it all the way to the end of the loop
            int const StartingComponent = branch.lastComponentSimulated + 1;
            int const EndingComponent = branch.TotalComponents;
            for (int CompCounter = StartingComponent; CompCounter <= EndingComponent; ++CompCounter) {
                PlantLocation this_plantLoc = {this->plantLoc.loopNum, this->plantLoc.loopSideNum, BranchCounter, CompCounter};

                DataPlant::OpScheme const CurOpSchemeType(branch.Comp(CompCounter).CurOpSchemeType);

                switch (CurOpSchemeType) {
                case DataPlant::OpScheme::NoControl: //~ pipes, for example
                    branch.Comp(CompCounter).simulate(state, FirstHVACIteration);
                    break;
                case DataPlant::OpScheme::Demand:
                case DataPlant::OpScheme::CompSetPtBased:
                case DataPlant::OpScheme::FreeRejection: //~ other control types
                    EncounteredNonLBObjDuringPass2 = true;
                    goto components2_end;       // don't do anymore components on this branch
                case DataPlant::OpScheme::Pump: //~ pump
                    PumpLocation.loopNum = this->plantLoc.loopNum;
                    PumpLocation.loopSideNum = this->plantLoc.loopSideNum;
                    PumpLocation.branchNum = BranchCounter;
                    PumpLocation.compNum = CompCounter;
                    if (this->BranchPumpsExist) {
                        SimulateSinglePump(state, PumpLocation, branch.RequestedMassFlow);
                    } else {
                        SimulateSinglePump(state, PumpLocation, FlowRequest);
                    }
                    break;
                case OpScheme::WetBulbRB:
                case OpScheme::DryBulbRB:
                case OpScheme::DewPointRB:
                case OpScheme::RelHumRB:
                case OpScheme::DryBulbTDB:
                case OpScheme::WetBulbTDB:
                case OpScheme::DewPointTDB:
                case OpScheme::HeatingRB:
                case OpScheme::CoolingRB: {              //~ load range based
                    if (!LoadDistributionWasPerformed) { //~ Still need to distribute load among load range based components
                        PlantCondLoopOperation::ManagePlantLoadDistribution(state,
                                                                            this_plantLoc,
                                                                            LoadToLoopSetPoint,
                                                                            LoadToLoopSetPointThatWasntMet,
                                                                            FirstHVACIteration,
                                                                            LoopShutDownFlag,
                                                                            LoadDistributionWasPerformed);
                    }
                    branch.Comp(CompCounter).simulate(state, FirstHVACIteration);
                    break;
                }
                default:
                    break;
                }

                //~ If we didn't EXIT early, we must have simulated, so update array
                branch.lastComponentSimulated = CompCounter;

            } //~ CompCounter
        components2_end:;

            //~ If we are locked, go ahead and simulate the pressure components on this branch
            if (this->FlowLock == DataPlant::FlowLock::Locked) {
                PlantPressureSystem::SimPressureDropSystem(
                    state, this->plantLoc.loopNum, FirstHVACIteration, DataPlant::PressureCall::Calc, this->plantLoc.loopSideNum, BranchCounter);
            }

        } //~ BranchCounter

        // So now we have made the load range based pass through all the components on each branch
        // If we didn't see any other component types, then we are done, go away
        if (!EncounteredNonLBObjDuringPass2) {
            return;
        }

        // If we did encounter other objects than we just need to go back through and simulate them
        for (int BranchCounter = FirstBranchNum; BranchCounter <= LastBranchNum; ++BranchCounter) {
            auto &branch(this->Branch(BranchCounter));

            //~ Always start from the last component we did the last time around + 1 and
            //~  try to make it all the way to the end of the loop
            int const StartingComponent = branch.lastComponentSimulated + 1;
            int const EndingComponent = branch.TotalComponents;
            for (int CompCounter = StartingComponent; CompCounter <= EndingComponent; ++CompCounter) {

                DataPlant::OpScheme const CurOpSchemeType(branch.Comp(CompCounter).CurOpSchemeType);

                switch (CurOpSchemeType) {
                case DataPlant::OpScheme::Demand: //~ coils
                    branch.Comp(CompCounter).simulate(state, FirstHVACIteration);
                    break;
                case DataPlant::OpScheme::Pump: //~ pump
                    PumpLocation.loopNum = this->plantLoc.loopNum;
                    PumpLocation.loopSideNum = this->plantLoc.loopSideNum;
                    PumpLocation.branchNum = BranchCounter;
                    PumpLocation.compNum = CompCounter;
                    if (this->BranchPumpsExist) {
                        SimulateSinglePump(state, PumpLocation, branch.RequestedMassFlow);
                    } else {
                        SimulateSinglePump(state, PumpLocation, FlowRequest);
                    }
                    break;
                case OpScheme::HeatingRB:
                case OpScheme::CoolingRB: { //~ load range based
                    ShowFatalError(state, "Encountered Load Based Object after other components, invalid.");
                    break;
                }
                default:
                    //~ Typical control equipment
                    branch.Comp(CompCounter).simulate(state, FirstHVACIteration);
                }

                //~ If we didn't EXIT early, we must have simulated, so update array
                branch.lastComponentSimulated = CompCounter;

            } //~ CompCounter

            if (this->FlowLock == DataPlant::FlowLock::Locked) {
                PlantPressureSystem::SimPressureDropSystem(
                    state, this->plantLoc.loopNum, FirstHVACIteration, DataPlant::PressureCall::Calc, this->plantLoc.loopSideNum, BranchCounter);
            }

        } //~ BranchCounter

        // I suppose I could do a check on the last component simulated to make sure we actually exhausted all branches
        // This would be the "THIRD" check on flow validation, but would be OK
    }

    void HalfLoopData::UpdateAnyLoopDemandAlterations(EnergyPlusData &state, int const BranchNum, int const CompNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   August 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This routine will analyze the given component and determine if any
        //  alterations need to be made to the current loop demand value.  If so,
        //  it will make the changes to the module level loop demand variables.

        // METHODOLOGY EMPLOYED:
        // Components will always supply a useful delta T, even if it happens to be zero
        // For flow rate, make decisions based on the component's current operating scheme type:
        //    Demand based: these components will have a flow request on their inlet node
        //    Pump: these components will not be included, as they no longer include heat at the pump
        //    component setpoint: these components will have a flow request

        //    on their outlet node corresponding to their calculated delta T
        //    load range based: these components do not 'alter' the load, they reject the load
        //    Therefore they are not included

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("PlantLoopSolver::UpdateAnyLoopDemandAlterations");

        // Init to zero, so that if we don't find anything, we exit early
        Real64 ComponentMassFlowRate(0.0);

        auto const &this_comp(this->Branch(BranchNum).Comp(CompNum));

        // Get information
        int const InletNode(this_comp.NodeNumIn);
        int const OutletNode(this_comp.NodeNumOut);

        if (this->FlowLock == DataPlant::FlowLock::Unlocked) {

            switch (this_comp.CurOpSchemeType) {
            case OpScheme::HeatingRB:
            case OpScheme::CoolingRB: { //~ load range based
                break;                  // Don't do anything for load based components
            }

            default: {
                // pumps pipes, etc. will be lumped in here with other component types, but they will have no delta T anyway
                ComponentMassFlowRate = state.dataLoopNodes->Node(InletNode).MassFlowRateRequest;
                // make sure components like economizers use the mass flow request
                break;
            }
            }

        } else if (this->FlowLock == DataPlant::FlowLock::Locked) {

            // For locked flow just use the mass flow rate

            switch (this_comp.CurOpSchemeType) {
            case OpScheme::HeatingRB:
            case OpScheme::CoolingRB: { //~ load range based
                break;                  // Don't do anything for load based components
            }
            default: {
                // pumps pipes, etc. will be lumped in here with other component types, but they will have no delta T anyway
                ComponentMassFlowRate = state.dataLoopNodes->Node(OutletNode).MassFlowRate;
            }
            }

        } else { // flow pump query? problem?
        }

        // Leave early if there wasn't a mass flow rate or request
        if (ComponentMassFlowRate < DataBranchAirLoopPlant::MassFlowTolerance) {
            return;
        }

        // Get an average temperature for the property call
        Real64 const InletTemp(state.dataLoopNodes->Node(InletNode).Temp);
        Real64 const OutletTemp(state.dataLoopNodes->Node(OutletNode).Temp);
        Real64 const AverageTemp((InletTemp + OutletTemp) / 2.0);
        Real64 const ComponentCp(state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, AverageTemp, RoutineName));

        // Calculate the load altered by this component
        Real64 const LoadAlteration(ComponentMassFlowRate * ComponentCp * (OutletTemp - InletTemp));

        // Now alter the module level variables
        this->CurrentAlterationsToDemand += LoadAlteration;
        this->UpdatedDemandToLoopSetPoint = this->InitialDemandToLoopSetPoint - this->CurrentAlterationsToDemand;
    }

    void HalfLoopData::SimulateSinglePump(EnergyPlusData &state, PlantLocation const SpecificPumpLocation, Real64 &SpecificPumpFlowRate)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   July 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        auto &loop(state.dataPlnt->PlantLoop(SpecificPumpLocation.loopNum));
        auto &loop_side(loop.LoopSide(SpecificPumpLocation.loopSideNum));
        auto &loop_side_branch(loop_side.Branch(SpecificPumpLocation.branchNum));
        auto &comp(loop_side_branch.Comp(SpecificPumpLocation.compNum));
        int const PumpIndex = comp.IndexInLoopSidePumps;
        auto &pump(loop_side.Pumps(PumpIndex));

        this->AdjustPumpFlowRequestByEMSControls(SpecificPumpLocation.branchNum, SpecificPumpLocation.compNum, SpecificPumpFlowRate);

        // Call SimPumps, routine takes a flow request, and returns some info about the status of the pump
        bool DummyThisPumpRunning;
        Pumps::SimPumps(state,
                        pump.PumpName,
                        SpecificPumpLocation.loopNum,
                        SpecificPumpFlowRate,
                        DummyThisPumpRunning,
                        loop_side_branch.PumpIndex,
                        pump.PumpHeatToFluid);

        //~ Pull some state information from the pump outlet node
        pump.CurrentMinAvail = state.dataLoopNodes->Node(pump.PumpOutletNode).MassFlowRateMinAvail;
        pump.CurrentMaxAvail = state.dataLoopNodes->Node(pump.PumpOutletNode).MassFlowRateMaxAvail;

        //~ Update the LoopSide pump heat totality here
        if (loop_side.TotalPumps > 0) {
            loop_side.TotalPumpHeat = sum(loop_side.Pumps, &DataPlant::LoopSidePumpInformation::PumpHeatToFluid);
        }
    }

    void HalfLoopData::SimulateAllLoopSidePumps(EnergyPlusData &state,
                                                ObjexxFCL::Optional<PlantLocation const> SpecificPumpLocation,
                                                ObjexxFCL::Optional<Real64 const> SpecificPumpFlowRate)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   July 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        int PumpIndexStart;
        int PumpIndexEnd;
        int PumpLoopNum;
        DataPlant::LoopSideLocation PumpLoopSideNum;

        // If we have a specific loop/side/br/comp, then find the index and only do that one, otherwise do all pumps on the loop side
        if (present(SpecificPumpLocation)) {
            PumpLoopNum = SpecificPumpLocation().loopNum;
            PumpLoopSideNum = SpecificPumpLocation().loopSideNum;
            int const PumpBranchNum = SpecificPumpLocation().branchNum;
            int const PumpCompNum = SpecificPumpLocation().compNum;
            PumpIndexStart =
                state.dataPlnt->PlantLoop(PumpLoopNum).LoopSide(PumpLoopSideNum).Branch(PumpBranchNum).Comp(PumpCompNum).IndexInLoopSidePumps;
            PumpIndexEnd = PumpIndexStart;
        } else {
            PumpLoopNum = this->plantLoc.loopNum;
            PumpLoopSideNum = this->plantLoc.loopSideNum;
            PumpIndexStart = 1;
            PumpIndexEnd = this->TotalPumps;
        }

        // If we have a flow rate to hit, then go for it, otherwise, just operate in request mode with zero flow
        Real64 FlowToRequest;
        if (present(SpecificPumpFlowRate)) {
            FlowToRequest = SpecificPumpFlowRate;
        } else {
            FlowToRequest = 0.0;
        }

        //~ Now loop through all the pumps and simulate them, keeping track of their status
        auto &loop_side(state.dataPlnt->PlantLoop(PumpLoopNum).LoopSide(PumpLoopSideNum));
        auto &loop_side_branch(loop_side.Branch);
        for (int PumpCounter = PumpIndexStart; PumpCounter <= PumpIndexEnd; ++PumpCounter) {

            //~ Set some variables
            auto &pump(loop_side.Pumps(PumpCounter));
            int const PumpBranchNum = pump.BranchNum;
            int const PumpCompNum = pump.CompNum;
            int const PumpOutletNode = pump.PumpOutletNode;

            this->AdjustPumpFlowRequestByEMSControls(PumpBranchNum, PumpCompNum, FlowToRequest);

            // Call SimPumps, routine takes a flow request, and returns some info about the status of the pump
            bool DummyThisPumpRunning;
            Pumps::SimPumps(state,
                            pump.PumpName,
                            PumpLoopNum,
                            FlowToRequest,
                            DummyThisPumpRunning,
                            loop_side_branch(PumpBranchNum).PumpIndex,
                            pump.PumpHeatToFluid);

            //~ Pull some state information from the pump outlet node
            Real64 const ThisPumpMinAvail = state.dataLoopNodes->Node(PumpOutletNode).MassFlowRateMinAvail;
            Real64 const ThisPumpMaxAvail = state.dataLoopNodes->Node(PumpOutletNode).MassFlowRateMaxAvail;

            //~ Now update the data structure
            pump.CurrentMinAvail = ThisPumpMinAvail;
            pump.CurrentMaxAvail = ThisPumpMaxAvail;
        }

        //~ Update the LoopSide pump heat totality here
        if (loop_side.TotalPumps > 0) {
            loop_side.TotalPumpHeat = sum(loop_side.Pumps, &DataPlant::LoopSidePumpInformation::PumpHeatToFluid);
        }
    }

    Real64 HalfLoopData::DetermineLoopSideFlowRate(EnergyPlusData &state, int ThisSideInletNode, Real64 ThisSideLoopFlowRequest)
    {
        Real64 ThisLoopSideFlow = ThisSideLoopFlowRequest;
        Real64 TotalPumpMinAvailFlow = 0.0;
        Real64 TotalPumpMaxAvailFlow = 0.0;
        if (allocated(this->Pumps)) {

            //~ Initialize pump values
            for (auto &e : this->Pumps) {
                e.CurrentMinAvail = 0.0;
                e.CurrentMaxAvail = 0.0;
            }
            this->FlowLock = DataPlant::FlowLock::PumpQuery;

            //~ Simulate pumps
            this->SimulateAllLoopSidePumps(state);

            //~ Calculate totals
            for (auto const &e : this->Pumps) {
                TotalPumpMinAvailFlow += e.CurrentMinAvail;
                TotalPumpMaxAvailFlow += e.CurrentMaxAvail;
            }

            // Use the pump min/max avail to attempt to constrain the loop side flow
            ThisLoopSideFlow = PlantUtilities::BoundValueToWithinTwoValues(ThisLoopSideFlow, TotalPumpMinAvailFlow, TotalPumpMaxAvailFlow);
        }

        // Now we check flow restriction from the other side, both min and max avail.
        // Doing this last basically means it wins, so the pump should pull down to meet the flow restriction
        ThisLoopSideFlow = PlantUtilities::BoundValueToNodeMinMaxAvail(state, ThisLoopSideFlow, ThisSideInletNode);

        // Final preparation of loop inlet min/max avail if pumps exist
        if (allocated(this->Pumps)) {
            // At this point, the pump limits should have been obeyed unless a flow restriction was encountered from the other side
            // The pump may, however, have even tighter constraints than the other side
            // At this point, the inlet node doesn't know anything about those limits
            // Since we have already honored the other side flow restriction, try to honor the pump limits here
            PlantUtilities::TightenNodeMinMaxAvails(state, ThisSideInletNode, TotalPumpMinAvailFlow, TotalPumpMaxAvailFlow);
        }

        // Now reset the entering mass flow rate to the decided-upon flow rate
        state.dataLoopNodes->Node(ThisSideInletNode).MassFlowRate = ThisLoopSideFlow;
        return ThisLoopSideFlow;
    }

    void HalfLoopData::UpdatePlantMixer(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brandon Anderson, Dan Fisher
        //       DATE WRITTEN   October 1999
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // calculate the outlet conditions at the mixer
        // this is expected to only be called for loops with a mixer

        // Find mixer outlet node number
        int const MixerOutletNode = this->Mixer.NodeNumOut;

        // Find corresponding splitter inlet node number--correspondence, but currently
        //  hard code things to a single split/mix setting it to the mixer number
        int const SplitterInNode = this->Splitter.NodeNumIn;
        // Initialize Mixer outlet temp and mass flow rate
        Real64 MixerOutletTemp = 0.0;
        Real64 MixerOutletMassFlow = 0.0;
        Real64 MixerOutletMassFlowMaxAvail = 0.0;
        Real64 MixerOutletMassFlowMinAvail = 0.0;
        Real64 MixerOutletPress = 0.0;
        Real64 MixerOutletQuality = 0.0;

        // Calculate Mixer outlet mass flow rate
        for (int InletNodeNum = 1; InletNodeNum <= this->Mixer.TotalInletNodes; ++InletNodeNum) {
            int const MixerInletNode = this->Mixer.NodeNumIn(InletNodeNum);
            MixerOutletMassFlow += state.dataLoopNodes->Node(MixerInletNode).MassFlowRate;
        }

        // Calculate Mixer outlet temperature
        for (int InletNodeNum = 1; InletNodeNum <= this->Mixer.TotalInletNodes; ++InletNodeNum) {
            int const MixerInletNode = this->Mixer.NodeNumIn(InletNodeNum);
            if (MixerOutletMassFlow > 0.0) {
                Real64 const MixerInletMassFlow = state.dataLoopNodes->Node(MixerInletNode).MassFlowRate;
                Real64 const MassFrac = MixerInletMassFlow / MixerOutletMassFlow;
                // mass flow weighted temp and enthalpy for each mixer inlet
                MixerOutletTemp += MassFrac * state.dataLoopNodes->Node(MixerInletNode).Temp;
                MixerOutletQuality += MassFrac * state.dataLoopNodes->Node(MixerInletNode).Quality;
                MixerOutletMassFlowMaxAvail += state.dataLoopNodes->Node(MixerInletNode).MassFlowRateMaxAvail;
                MixerOutletMassFlowMinAvail += state.dataLoopNodes->Node(MixerInletNode).MassFlowRateMinAvail;
                MixerOutletPress = max(MixerOutletPress, state.dataLoopNodes->Node(MixerInletNode).Press);
            } else { // MixerOutletMassFlow <=0, then perform the 'no flow' update.
                MixerOutletTemp = state.dataLoopNodes->Node(SplitterInNode).Temp;
                MixerOutletQuality = state.dataLoopNodes->Node(SplitterInNode).Quality;
                MixerOutletMassFlowMaxAvail = state.dataLoopNodes->Node(SplitterInNode).MassFlowRateMaxAvail;
                MixerOutletMassFlowMinAvail = state.dataLoopNodes->Node(SplitterInNode).MassFlowRateMinAvail;
                MixerOutletPress = state.dataLoopNodes->Node(SplitterInNode).Press;
                break;
            }
        }

        state.dataLoopNodes->Node(MixerOutletNode).MassFlowRate = MixerOutletMassFlow;
        state.dataLoopNodes->Node(MixerOutletNode).Temp = MixerOutletTemp;
        if (state.dataPlnt->PlantLoop(this->plantLoc.loopNum).HasPressureComponents) {
            // Don't update pressure, let pressure system handle this...
        } else {
            // Go ahead and update!
            state.dataLoopNodes->Node(MixerOutletNode).Press = MixerOutletPress;
        }
        state.dataLoopNodes->Node(MixerOutletNode).Quality = MixerOutletQuality;

        // set max/min avails on mixer outlet to be consistent with the following rules
        // 1.  limited by the max/min avails on splitter inlet
        // 2.  limited by the sum of max/min avails for each branch's mixer inlet node

        state.dataLoopNodes->Node(MixerOutletNode).MassFlowRateMaxAvail =
            min(MixerOutletMassFlowMaxAvail, state.dataLoopNodes->Node(SplitterInNode).MassFlowRateMaxAvail);
        state.dataLoopNodes->Node(MixerOutletNode).MassFlowRateMinAvail =
            max(MixerOutletMassFlowMinAvail, state.dataLoopNodes->Node(SplitterInNode).MassFlowRateMinAvail);
    }

    void HalfLoopData::UpdatePlantSplitter(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brandon Anderson, Dan Fisher
        //       DATE WRITTEN   October 1999
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set the outlet conditions of the splitter

        // Update Temperatures across splitter
        if (this->Splitter.Exists) {

            // Set branch number at splitter inlet
            int const SplitterInletNode = this->Splitter.NodeNumIn;

            // Loop over outlet nodes
            for (int CurNode = 1; CurNode <= this->Splitter.TotalOutletNodes; ++CurNode) {
                int const SplitterOutletNode = this->Splitter.NodeNumOut(CurNode);

                // Inlet Temp equals exit Temp to all outlet branches
                state.dataLoopNodes->Node(SplitterOutletNode).Temp = state.dataLoopNodes->Node(SplitterInletNode).Temp;
                state.dataLoopNodes->Node(SplitterOutletNode).TempMin = state.dataLoopNodes->Node(SplitterInletNode).TempMin;
                state.dataLoopNodes->Node(SplitterOutletNode).TempMax = state.dataLoopNodes->Node(SplitterInletNode).TempMax;
                if (state.dataPlnt->PlantLoop(this->plantLoc.loopNum).HasPressureComponents) {
                    // Don't update pressure, let pressure system handle this...
                } else {
                    // Go ahead and update!
                    state.dataLoopNodes->Node(SplitterOutletNode).Press = state.dataLoopNodes->Node(SplitterInletNode).Press;
                }
                state.dataLoopNodes->Node(SplitterOutletNode).Quality = state.dataLoopNodes->Node(SplitterInletNode).Quality;

                // These two blocks and the following one which I added need to be cleaned up
                // I think we will always pass maxavail down the splitter, min avail is the issue.
                // Changed to include hardware max in next line
                state.dataLoopNodes->Node(SplitterOutletNode).MassFlowRateMaxAvail = min(
                    state.dataLoopNodes->Node(SplitterInletNode).MassFlowRateMaxAvail, state.dataLoopNodes->Node(SplitterOutletNode).MassFlowRateMax);
                state.dataLoopNodes->Node(SplitterOutletNode).MassFlowRateMinAvail = 0.0;

                // Not sure about passing min avail if it is nonzero.  I am testing a pump with nonzero
                // min flow rate, and it is causing problems because this routine passes zero down.  Perhaps if
                // it is a single parallel branch, we are safe to assume we need to just pass it down.
                // But need to test for multiple branches (or at least think about it), to see what we need to do...
                if (this->Splitter.TotalOutletNodes == 1) {
                    state.dataLoopNodes->Node(SplitterOutletNode).MassFlowRateMinAvail =
                        state.dataLoopNodes->Node(SplitterInletNode).MassFlowRateMinAvail;
                }
            }
        }
    }

} // namespace DataPlant
} // namespace EnergyPlus
