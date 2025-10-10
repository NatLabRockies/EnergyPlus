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

#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/Loop.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::DataPlant {

void PlantLoopData::UpdateLoopSideReportVars(EnergyPlusData &state,
                                             Real64 const OtherSideDemand,   // This is the 'other side' demand, based on other side flow
                                             Real64 const LocalRemLoopDemand // Unmet Demand after equipment has been simulated (report variable)
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dan Fisher
    //       DATE WRITTEN   July 1998
    //       MODIFIED       Aug 2010 Edwin Lee -- add per LoopSide variable support
    //       RE-ENGINEERED  na

    this->InletNodeFlowrate = state.dataLoopNodes->Node(this->LoopSide(DataPlant::LoopSideLocation::Supply).NodeNumIn).MassFlowRate;
    this->InletNodeTemperature = state.dataLoopNodes->Node(this->LoopSide(DataPlant::LoopSideLocation::Supply).NodeNumIn).Temp;
    this->OutletNodeFlowrate = state.dataLoopNodes->Node(this->LoopSide(DataPlant::LoopSideLocation::Supply).NodeNumOut).MassFlowRate;
    this->OutletNodeTemperature = state.dataLoopNodes->Node(this->LoopSide(DataPlant::LoopSideLocation::Supply).NodeNumOut).Temp;

    // In the baseline code, only reported supply side demand. so putting in "SupplySide" IF block for now but might expand later
    if (OtherSideDemand < 0.0) {
        this->CoolingDemand = std::abs(OtherSideDemand);
        this->HeatingDemand = 0.0;
        this->DemandNotDispatched = -LocalRemLoopDemand; //  Setting sign based on old logic for now
    } else {
        this->HeatingDemand = OtherSideDemand;
        this->CoolingDemand = 0.0;
        this->DemandNotDispatched = LocalRemLoopDemand; //  Setting sign based on old logic for now
    }

    this->CalcUnmetPlantDemand(state);
}

void PlantLoopData::CalcUnmetPlantDemand(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   June 2011
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // determine the magnitude of unmet plant loads after the half loop simulation is done

    // METHODOLOGY EMPLOYED:
    // using the loop setpoint node, look at target vs current and
    // calculate a demand based on mass flow times specific heat times delta T

    // Using/Aliasing
    using DataPlant::LoopDemandTol;

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("PlantLoopSolver::EvaluateLoopSetPointLoad");
    static constexpr std::string_view RoutineNameAlt("PlantSupplySide:EvaluateLoopSetPointLoad");

    //~ General variables
    Real64 MassFlowRate;
    Real64 TargetTemp;
    Real64 LoopSetPointTemperature;
    Real64 LoopSetPointTemperatureHi;
    Real64 LoopSetPointTemperatureLo;
    Real64 LoadToHeatingSetPoint;
    Real64 LoadToCoolingSetPoint;
    Real64 DeltaTemp;
    Real64 Cp;
    Real64 EnthalpySteamSatVapor;  // Enthalpy of saturated vapor
    Real64 EnthalpySteamSatLiquid; // Enthalpy of saturated liquid
    Real64 LatentHeatSteam;        // Latent heat of steam
    Real64 LoadToLoopSetPoint;

    // Initialize
    LoadToLoopSetPoint = 0.0;

    // Get temperature at loop setpoint node.
    TargetTemp = state.dataLoopNodes->Node(this->TempSetPointNodeNum).Temp;
    MassFlowRate = state.dataLoopNodes->Node(this->TempSetPointNodeNum).MassFlowRate;

    if (this->FluidType == DataLoopNode::NodeFluidType::Water) {

        Cp = this->glycol->getSpecificHeat(state, TargetTemp, RoutineName);

        switch (this->LoopDemandCalcScheme) {
        case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {

            // Pick up the loop setpoint temperature
            LoopSetPointTemperature = this->LoopSide(DataPlant::LoopSideLocation::Supply).TempSetPoint;
            // Calculate the delta temperature
            DeltaTemp = LoopSetPointTemperature - TargetTemp;

            // Calculate the demand on the loop
            LoadToLoopSetPoint = MassFlowRate * Cp * DeltaTemp;
        } break;
        case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
            // Get the range of setpoints
            LoopSetPointTemperatureHi = state.dataLoopNodes->Node(this->TempSetPointNodeNum).TempSetPointHi;
            LoopSetPointTemperatureLo = state.dataLoopNodes->Node(this->TempSetPointNodeNum).TempSetPointLo;

            // Calculate the demand on the loop
            if (MassFlowRate > 0.0) {
                LoadToHeatingSetPoint = MassFlowRate * Cp * (LoopSetPointTemperatureLo - TargetTemp);
                LoadToCoolingSetPoint = MassFlowRate * Cp * (LoopSetPointTemperatureHi - TargetTemp);
                // Possible combinations:
                // 1  LoadToHeatingSetPoint > 0 & LoadToCoolingSetPoint > 0 -->  Heating required
                // 2  LoadToHeatingSetPoint < 0 & LoadToCoolingSetPoint < 0 -->  Cooling Required
                // 3  LoadToHeatingSetPoint <=0 & LoadToCoolingSetPoint >=0 -->  Dead Band Operation - includes zero load cases
                // 4  LoadToHeatingSetPoint  >  LoadToCoolingSetPoint       -->  Not Feasible if LoopSetPointHi >= LoopSetPointLo
                if (LoadToHeatingSetPoint > 0.0 && LoadToCoolingSetPoint > 0.0) {
                    LoadToLoopSetPoint = LoadToHeatingSetPoint;
                } else if (LoadToHeatingSetPoint < 0.0 && LoadToCoolingSetPoint < 0.0) {
                    LoadToLoopSetPoint = LoadToCoolingSetPoint;
                } else if (LoadToHeatingSetPoint <= 0.0 && LoadToCoolingSetPoint >= 0.0) { // deadband includes zero loads
                    LoadToLoopSetPoint = 0.0;
                }
            } else {
                LoadToLoopSetPoint = 0.0;
            }
        } break;
        default:
            break;
        }

    } else if (this->FluidType == DataLoopNode::NodeFluidType::Steam) {

        Cp = this->glycol->getSpecificHeat(state, TargetTemp, RoutineName);

        switch (this->LoopDemandCalcScheme) {
        case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {

            // Pick up the loop setpoint temperature
            LoopSetPointTemperature = this->LoopSide(DataPlant::LoopSideLocation::Supply).TempSetPoint;

            // Calculate the delta temperature
            DeltaTemp = LoopSetPointTemperature - TargetTemp;

            EnthalpySteamSatVapor = this->steam->getSatEnthalpy(state, LoopSetPointTemperature, 1.0, RoutineNameAlt);
            EnthalpySteamSatLiquid = this->steam->getSatEnthalpy(state, LoopSetPointTemperature, 0.0, RoutineNameAlt);

            LatentHeatSteam = EnthalpySteamSatVapor - EnthalpySteamSatLiquid;

            // Calculate the demand on the loop
            LoadToLoopSetPoint = MassFlowRate * (Cp * DeltaTemp + LatentHeatSteam);
        } break;
        default:
            break;
        }

    } else { // only have two types, water serves for glycol.
    }

    // Trim the demand to zero if it is very small
    if (std::abs(LoadToLoopSetPoint) < LoopDemandTol) {
        LoadToLoopSetPoint = 0.0;
    }

    this->UnmetDemand = LoadToLoopSetPoint;
}

void PlantLoopData::CheckLoopExitNode(EnergyPlusData &state, bool const FirstHVACIteration)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dan Fisher
    //       DATE WRITTEN   October 1998
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine sets the temperature
    // and mass flow rate of the plant loop supply side exit
    // node.  As written, the routine calculates the exit
    // temperature based on the fraction of loop demand met
    // by the plant equipment.  This assumes that each piece
    // of operating plant equipment produced chilled/hot water
    // at the loop setpoint temperature.

    // Using/Aliasing

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int LoopInlet;  // plant loop inlet node num.
    int LoopOutlet; // plant loop outlet node num.

    // set local variables: loop inlet and outlet nodes
    auto &Supply = this->LoopSide(DataPlant::LoopSideLocation::Supply);
    LoopInlet = Supply.NodeNumIn;
    LoopOutlet = Supply.NodeNumOut;
    // Check continuity invalid...loop pumps now turned on and off
    if (!FirstHVACIteration && !state.dataGlobal->WarmupFlag) {
        if (std::abs(state.dataLoopNodes->Node(LoopOutlet).MassFlowRate - state.dataLoopNodes->Node(LoopInlet).MassFlowRate) >
            DataBranchAirLoopPlant::MassFlowTolerance) {
            if (this->MFErrIndex == 0) {
                ShowWarningError(state,
                                 "PlantSupplySide: PlantLoop=\"" + this->Name +
                                     "\", Error (CheckLoopExitNode) -- Mass Flow Rate Calculation. Outlet and Inlet differ by more than tolerance.");
                ShowContinueErrorTimeStamp(state, "");
                ShowContinueError(state,
                                  format("Loop inlet node={}, flowrate={:.4R} kg/s",
                                         state.dataLoopNodes->NodeID(LoopInlet),
                                         state.dataLoopNodes->Node(LoopInlet).MassFlowRate));
                ShowContinueError(state,
                                  format("Loop outlet node={}, flowrate={:.4R} kg/s",
                                         state.dataLoopNodes->NodeID(LoopOutlet),
                                         state.dataLoopNodes->Node(LoopOutlet).MassFlowRate));
                ShowContinueError(state, "This loop might be helped by a bypass.");
            }
            ShowRecurringWarningErrorAtEnd(
                state, "PlantSupplySide: PlantLoop=\"" + this->Name + "\", Error -- Mass Flow Rate Calculation -- continues ** ", this->MFErrIndex);
        }
    }
    // Reset Max loop flow rate based on pump performance
    state.dataLoopNodes->Node(LoopOutlet).MassFlowRateMax = state.dataLoopNodes->Node(LoopInlet).MassFlowRateMax;
}

} // namespace EnergyPlus::DataPlant
