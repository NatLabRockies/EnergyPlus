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

// C++ Headers
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataConvergParams.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/HVACInterfaceManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::HVACInterfaceManager {

// MODULE INFORMATION:
//       AUTHOR         Rick Strand
//       DATE WRITTEN   October 1998
//       MODIFIED       na
//       RE-ENGINEERED  na

// PURPOSE OF THIS MODULE:
// This module contains one or more routines for checking the convergence
// of the various HVAC loops and passing information across interface
// boundaries.

// METHODOLOGY EMPLOYED:
// The upper level HVAC managers call the routine(s) contained in this
// module as a last step.  The node information is passed across the
// interface boundary and the logical flag is set if the nodes across
// from each other are not within tolerance.

void UpdateHVACInterface(EnergyPlusData &state,
                         int const AirLoopNum, // airloop number for which air loop this is
                         DataConvergParams::CalledFrom const CalledFrom,
                         int const OutletNode,    // Node number for the outlet of the side of the loop just simulated
                         int const InletNode,     // Node number for the inlet of the side that needs the outlet node data
                         bool &OutOfToleranceFlag // True when the other side of the loop need to be (re)simulated
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   October 1998

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine manages any generic HVAC loop interface.

    // METHODOLOGY EMPLOYED:
    // This is a simple "forward" interface where all of the properties
    // from the outlet of one side of the loop get transferred directly
    // to the inlet node of the corresponding other side of the loop.

    auto &TmpRealARR = state.dataHVACInterfaceMgr->TmpRealARR;
    auto &airLoopConv = state.dataConvergeParams->AirLoopConvergence(AirLoopNum);
    auto &thisInletNode = state.dataLoopNodes->Node(InletNode);
    int const iCall = (int)CalledFrom;

    if ((CalledFrom == DataConvergParams::CalledFrom::AirSystemDemandSide) && (OutletNode == 0)) {
        // Air loop has no return path - only check mass flow and then set return inlet node mass flow to sum of demand side inlet nodes

        airLoopConv.HVACMassFlowNotConverged[iCall] = false;
        airLoopConv.HVACHumRatNotConverged[iCall] = false;
        airLoopConv.HVACTempNotConverged[iCall] = false;
        airLoopConv.HVACEnergyNotConverged[iCall] = false;

        Real64 totDemandSideMassFlow = 0.0;
        Real64 totDemandSideMinAvail = 0.0;
        Real64 totDemandSideMaxAvail = 0.0;
        for (int demIn = 1; demIn <= state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumSupplyNodes; ++demIn) {
            int demInNode = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).ZoneEquipSupplyNodeNum(demIn);
            auto const &node = state.dataLoopNodes->Node(demInNode);
            totDemandSideMassFlow += node.MassFlowRate;
            totDemandSideMinAvail += node.MassFlowRateMinAvail;
            totDemandSideMaxAvail += node.MassFlowRateMaxAvail;
        }
        TmpRealARR = airLoopConv.HVACFlowDemandToSupplyTolValue;
        airLoopConv.HVACFlowDemandToSupplyTolValue[0] = std::abs(totDemandSideMassFlow - thisInletNode.MassFlowRate);
        for (int logIndex = 1; logIndex < DataConvergParams::ConvergLogStackDepth; logIndex++) {
            airLoopConv.HVACFlowDemandToSupplyTolValue[logIndex] = TmpRealARR[logIndex - 1];
        }
        if (airLoopConv.HVACFlowDemandToSupplyTolValue[0] > DataConvergParams::HVACFlowRateToler) {
            airLoopConv.HVACMassFlowNotConverged[iCall] = true;
            OutOfToleranceFlag = true; // Something has changed--resimulate the other side of the loop
        }

        thisInletNode.MassFlowRate = totDemandSideMassFlow;
        thisInletNode.MassFlowRateMinAvail = totDemandSideMinAvail;
        thisInletNode.MassFlowRateMaxAvail = totDemandSideMaxAvail;
        return;
    }

    // Calculate the approximate energy difference across interface for comparison
    Real64 DeltaEnergy =
        DataConvergParams::HVACCpApprox * ((state.dataLoopNodes->Node(OutletNode).MassFlowRate * state.dataLoopNodes->Node(OutletNode).Temp) -
                                           (thisInletNode.MassFlowRate * thisInletNode.Temp));

    if ((CalledFrom == DataConvergParams::CalledFrom::AirSystemDemandSide) && (OutletNode > 0)) {

        airLoopConv.HVACMassFlowNotConverged[iCall] = false;
        airLoopConv.HVACHumRatNotConverged[iCall] = false;
        airLoopConv.HVACTempNotConverged[iCall] = false;
        airLoopConv.HVACEnergyNotConverged[iCall] = false;
        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            airLoopConv.HVACCO2NotConverged[iCall] = false;
        }
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            airLoopConv.HVACGenContamNotConverged[iCall] = false;
        }

        TmpRealARR = airLoopConv.HVACFlowDemandToSupplyTolValue;
        airLoopConv.HVACFlowDemandToSupplyTolValue[0] = std::abs(state.dataLoopNodes->Node(OutletNode).MassFlowRate - thisInletNode.MassFlowRate);
        for (int logIndex = 1; logIndex < DataConvergParams::ConvergLogStackDepth; logIndex++) {
            airLoopConv.HVACFlowDemandToSupplyTolValue[logIndex] = TmpRealARR[logIndex - 1];
        }
        if (airLoopConv.HVACFlowDemandToSupplyTolValue[0] > DataConvergParams::HVACFlowRateToler) {
            airLoopConv.HVACMassFlowNotConverged[iCall] = true;
            OutOfToleranceFlag = true; // Something has changed--resimulate the other side of the loop
        }

        TmpRealARR = airLoopConv.HVACHumDemandToSupplyTolValue;
        airLoopConv.HVACHumDemandToSupplyTolValue[0] = std::abs(state.dataLoopNodes->Node(OutletNode).HumRat - thisInletNode.HumRat);
        for (int logIndex = 1; logIndex < DataConvergParams::ConvergLogStackDepth; logIndex++) {
            airLoopConv.HVACHumDemandToSupplyTolValue[logIndex] = TmpRealARR[logIndex - 1];
        }
        if (airLoopConv.HVACHumDemandToSupplyTolValue[0] > DataConvergParams::HVACHumRatToler) {
            airLoopConv.HVACHumRatNotConverged[iCall] = true;
            OutOfToleranceFlag = true; // Something has changed--resimulate the other side of the loop
        }

        TmpRealARR = airLoopConv.HVACTempDemandToSupplyTolValue;
        airLoopConv.HVACTempDemandToSupplyTolValue[0] = std::abs(state.dataLoopNodes->Node(OutletNode).Temp - thisInletNode.Temp);
        for (int logIndex = 1; logIndex < DataConvergParams::ConvergLogStackDepth; logIndex++) {
            airLoopConv.HVACTempDemandToSupplyTolValue[logIndex] = TmpRealARR[logIndex - 1];
        }
        if (airLoopConv.HVACTempDemandToSupplyTolValue[0] > DataConvergParams::HVACTemperatureToler) {
            airLoopConv.HVACTempNotConverged[iCall] = true;
            OutOfToleranceFlag = true; // Something has changed--resimulate the other side of the loop
        }

        TmpRealARR = airLoopConv.HVACEnergyDemandToSupplyTolValue;
        airLoopConv.HVACEnergyDemandToSupplyTolValue[0] = std::abs(DeltaEnergy);
        for (int logIndex = 1; logIndex < DataConvergParams::ConvergLogStackDepth; logIndex++) {
            airLoopConv.HVACEnergyDemandToSupplyTolValue[logIndex] = TmpRealARR[logIndex - 1];
        }
        if (std::abs(DeltaEnergy) > DataConvergParams::HVACEnergyToler) {
            airLoopConv.HVACEnergyNotConverged[iCall] = true;
            OutOfToleranceFlag = true; // Something has changed--resimulate the other side of the loop
        }

        TmpRealARR = airLoopConv.HVACEnthalpyDemandToSupplyTolValue;
        airLoopConv.HVACEnthalpyDemandToSupplyTolValue[0] = std::abs(state.dataLoopNodes->Node(OutletNode).Enthalpy - thisInletNode.Enthalpy);
        for (int logIndex = 1; logIndex < DataConvergParams::ConvergLogStackDepth; logIndex++) {
            airLoopConv.HVACEnthalpyDemandToSupplyTolValue[logIndex] = TmpRealARR[logIndex - 1];
        }
        if (airLoopConv.HVACEnthalpyDemandToSupplyTolValue[0] > DataConvergParams::HVACEnthalpyToler) {
            OutOfToleranceFlag = true; // Something has changed--resimulate the other side of the loop
        }

        TmpRealARR = airLoopConv.HVACPressureDemandToSupplyTolValue;
        airLoopConv.HVACPressureDemandToSupplyTolValue[0] = std::abs(state.dataLoopNodes->Node(OutletNode).Press - thisInletNode.Press);
        for (int logIndex = 1; logIndex < DataConvergParams::ConvergLogStackDepth; logIndex++) {
            airLoopConv.HVACPressureDemandToSupplyTolValue[logIndex] = TmpRealARR[logIndex - 1];
        }
        if (airLoopConv.HVACPressureDemandToSupplyTolValue[0] > DataConvergParams::HVACPressToler) {
            OutOfToleranceFlag = true; // Something has changed--resimulate the other side of the loop
        }

        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            TmpRealARR = airLoopConv.HVACCO2DemandToSupplyTolValue;
            airLoopConv.HVACCO2DemandToSupplyTolValue[0] = std::abs(state.dataLoopNodes->Node(OutletNode).CO2 - thisInletNode.CO2);
            for (int logIndex = 1; logIndex < DataConvergParams::ConvergLogStackDepth; logIndex++) {
                airLoopConv.HVACCO2DemandToSupplyTolValue[logIndex] = TmpRealARR[logIndex - 1];
            }
            if (airLoopConv.HVACCO2DemandToSupplyTolValue[0] > DataConvergParams::HVACCO2Toler) {
                airLoopConv.HVACCO2NotConverged[iCall] = true;
                OutOfToleranceFlag = true; // Something has changed--resimulate the other side of the loop
            }
        }

        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            TmpRealARR = airLoopConv.HVACGenContamDemandToSupplyTolValue;
            airLoopConv.HVACGenContamDemandToSupplyTolValue[0] = std::abs(state.dataLoopNodes->Node(OutletNode).GenContam - thisInletNode.GenContam);
            for (int logIndex = 1; logIndex < DataConvergParams::ConvergLogStackDepth; logIndex++) {
                airLoopConv.HVACGenContamDemandToSupplyTolValue[logIndex] = TmpRealARR[logIndex - 1];
            }
            if (airLoopConv.HVACGenContamDemandToSupplyTolValue[0] > DataConvergParams::HVACGenContamToler) {
                airLoopConv.HVACGenContamNotConverged[iCall] = true;
                OutOfToleranceFlag = true; // Something has changed--resimulate the other side of the loop
            }
        }

    } else if (CalledFrom == DataConvergParams::CalledFrom::AirSystemSupplySideDeck1) {

        airLoopConv.HVACMassFlowNotConverged[iCall] = false;
        airLoopConv.HVACHumRatNotConverged[iCall] = false;
        airLoopConv.HVACTempNotConverged[iCall] = false;
        airLoopConv.HVACEnergyNotConverged[iCall] = false;
        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            airLoopConv.HVACCO2NotConverged[iCall] = false;
        }
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            airLoopConv.HVACGenContamNotConverged[iCall] = false;
        }

        TmpRealARR = airLoopConv.HVACFlowSupplyDeck1ToDemandTolValue;
        airLoopConv.HVACFlowSupplyDeck1ToDemandTolValue[0] =
            std::abs(state.dataLoopNodes->Node(OutletNode).MassFlowRate - thisInletNode.MassFlowRate);
        for (int logIndex = 1; logIndex < DataConvergParams::ConvergLogStackDepth; logIndex++) {
            airLoopConv.HVACFlowSupplyDeck1ToDemandTolValue[logIndex] = TmpRealARR[logIndex - 1];
        }
        if (airLoopConv.HVACFlowSupplyDeck1ToDemandTolValue[0] > DataConvergParams::HVACFlowRateToler) {
            airLoopConv.HVACMassFlowNotConverged[iCall] = true;
            OutOfToleranceFlag = true; // Something has changed--resimulate the other side of the loop
        }

        TmpRealARR = airLoopConv.HVACHumSupplyDeck1ToDemandTolValue;
        airLoopConv.HVACHumSupplyDeck1ToDemandTolValue[0] = std::abs(state.dataLoopNodes->Node(OutletNode).HumRat - thisInletNode.HumRat);
        for (int logIndex = 1; logIndex < DataConvergParams::ConvergLogStackDepth; logIndex++) {
            airLoopConv.HVACHumSupplyDeck1ToDemandTolValue[logIndex] = TmpRealARR[logIndex - 1];
        }
        if (airLoopConv.HVACHumSupplyDeck1ToDemandTolValue[0] > DataConvergParams::HVACHumRatToler) {
            airLoopConv.HVACHumRatNotConverged[iCall] = true;
            OutOfToleranceFlag = true; // Something has changed--resimulate the other side of the loop
        }

        TmpRealARR = airLoopConv.HVACTempSupplyDeck1ToDemandTolValue;
        airLoopConv.HVACTempSupplyDeck1ToDemandTolValue[0] = std::abs(state.dataLoopNodes->Node(OutletNode).Temp - thisInletNode.Temp);
        for (int logIndex = 1; logIndex < DataConvergParams::ConvergLogStackDepth; logIndex++) {
            airLoopConv.HVACTempSupplyDeck1ToDemandTolValue[logIndex] = TmpRealARR[logIndex - 1];
        }
        if (airLoopConv.HVACTempSupplyDeck1ToDemandTolValue[0] > DataConvergParams::HVACTemperatureToler) {
            airLoopConv.HVACTempNotConverged[iCall] = true;
            OutOfToleranceFlag = true; // Something has changed--resimulate the other side of the loop
        }

        TmpRealARR = airLoopConv.HVACEnergySupplyDeck1ToDemandTolValue;
        airLoopConv.HVACEnergySupplyDeck1ToDemandTolValue[0] = DeltaEnergy;
        for (int logIndex = 1; logIndex < DataConvergParams::ConvergLogStackDepth; logIndex++) {
            airLoopConv.HVACEnergySupplyDeck1ToDemandTolValue[logIndex] = TmpRealARR[logIndex - 1];
        }
        if (std::abs(DeltaEnergy) > DataConvergParams::HVACEnergyToler) {
            airLoopConv.HVACEnergyNotConverged[iCall] = true;
            OutOfToleranceFlag = true; // Something has changed--resimulate the other side of the loop
        }

        TmpRealARR = airLoopConv.HVACEnthalpySupplyDeck1ToDemandTolValue;
        airLoopConv.HVACEnthalpySupplyDeck1ToDemandTolValue[0] = std::abs(state.dataLoopNodes->Node(OutletNode).Enthalpy - thisInletNode.Enthalpy);
        for (int logIndex = 1; logIndex < DataConvergParams::ConvergLogStackDepth; logIndex++) {
            airLoopConv.HVACEnthalpySupplyDeck1ToDemandTolValue[logIndex] = TmpRealARR[logIndex - 1];
        }
        if (airLoopConv.HVACEnthalpySupplyDeck1ToDemandTolValue[0] > DataConvergParams::HVACEnthalpyToler) {
            OutOfToleranceFlag = true; // Something has changed--resimulate the other side of the loop
        }

        TmpRealARR = airLoopConv.HVACPressureSupplyDeck1ToDemandTolValue;
        airLoopConv.HVACPressureSupplyDeck1ToDemandTolValue[0] = std::abs(state.dataLoopNodes->Node(OutletNode).Press - thisInletNode.Press);
        for (int logIndex = 1; logIndex < DataConvergParams::ConvergLogStackDepth; logIndex++) {
            airLoopConv.HVACPressureSupplyDeck1ToDemandTolValue[logIndex] = TmpRealARR[logIndex - 1];
        }
        if (airLoopConv.HVACPressureSupplyDeck1ToDemandTolValue[0] > DataConvergParams::HVACPressToler) {
            OutOfToleranceFlag = true; // Something has changed--resimulate the other side of the loop
        }
        // CO2 check
        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            TmpRealARR = airLoopConv.HVACCO2SupplyDeck1ToDemandTolValue;
            airLoopConv.HVACCO2SupplyDeck1ToDemandTolValue[0] = std::abs(state.dataLoopNodes->Node(OutletNode).CO2 - thisInletNode.CO2);
            for (int logIndex = 1; logIndex < DataConvergParams::ConvergLogStackDepth; logIndex++) {
                airLoopConv.HVACCO2SupplyDeck1ToDemandTolValue[logIndex] = TmpRealARR[logIndex - 1];
            }
            if (airLoopConv.HVACCO2SupplyDeck1ToDemandTolValue[0] > DataConvergParams::HVACCO2Toler) {
                airLoopConv.HVACCO2NotConverged[iCall] = true;
                OutOfToleranceFlag = true; // Something has changed--resimulate the other side of the loop
            }
        }

        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            TmpRealARR = airLoopConv.HVACGenContamSupplyDeck1ToDemandTolValue;
            airLoopConv.HVACGenContamSupplyDeck1ToDemandTolValue[0] =
                std::abs(state.dataLoopNodes->Node(OutletNode).GenContam - thisInletNode.GenContam);
            for (int logIndex = 1; logIndex < DataConvergParams::ConvergLogStackDepth; logIndex++) {
                airLoopConv.HVACGenContamSupplyDeck1ToDemandTolValue[logIndex] = TmpRealARR[logIndex - 1];
            }
            if (airLoopConv.HVACGenContamSupplyDeck1ToDemandTolValue[0] > DataConvergParams::HVACGenContamToler) {
                airLoopConv.HVACGenContamNotConverged[iCall] = true;
                OutOfToleranceFlag = true; // Something has changed--resimulate the other side of the loop
            }
        }

    } else if (CalledFrom == DataConvergParams::CalledFrom::AirSystemSupplySideDeck2) {

        airLoopConv.HVACMassFlowNotConverged[iCall] = false;
        airLoopConv.HVACHumRatNotConverged[iCall] = false;
        airLoopConv.HVACTempNotConverged[iCall] = false;
        airLoopConv.HVACEnergyNotConverged[iCall] = false;
        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            airLoopConv.HVACCO2NotConverged[iCall] = false;
        }
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            airLoopConv.HVACGenContamNotConverged[iCall] = false;
        }

        TmpRealARR = airLoopConv.HVACFlowSupplyDeck2ToDemandTolValue;
        airLoopConv.HVACFlowSupplyDeck2ToDemandTolValue[0] =
            std::abs(state.dataLoopNodes->Node(OutletNode).MassFlowRate - thisInletNode.MassFlowRate);
        for (int logIndex = 1; logIndex < DataConvergParams::ConvergLogStackDepth; logIndex++) {
            airLoopConv.HVACFlowSupplyDeck2ToDemandTolValue[logIndex] = TmpRealARR[logIndex - 1];
        }
        if (airLoopConv.HVACFlowSupplyDeck2ToDemandTolValue[0] > DataConvergParams::HVACFlowRateToler) {
            airLoopConv.HVACMassFlowNotConverged[iCall] = true;
            OutOfToleranceFlag = true; // Something has changed--resimulate the other side of the loop
        }

        TmpRealARR = airLoopConv.HVACHumSupplyDeck2ToDemandTolValue;
        airLoopConv.HVACHumSupplyDeck2ToDemandTolValue[0] = std::abs(state.dataLoopNodes->Node(OutletNode).HumRat - thisInletNode.HumRat);
        for (int logIndex = 1; logIndex < DataConvergParams::ConvergLogStackDepth; logIndex++) {
            airLoopConv.HVACHumSupplyDeck2ToDemandTolValue[logIndex] = TmpRealARR[logIndex - 1];
        }
        if (airLoopConv.HVACHumSupplyDeck2ToDemandTolValue[0] > DataConvergParams::HVACHumRatToler) {
            airLoopConv.HVACHumRatNotConverged[iCall] = true;
            OutOfToleranceFlag = true; // Something has changed--resimulate the other side of the loop
        }

        TmpRealARR = airLoopConv.HVACTempSupplyDeck2ToDemandTolValue;
        airLoopConv.HVACTempSupplyDeck2ToDemandTolValue[0] = std::abs(state.dataLoopNodes->Node(OutletNode).Temp - thisInletNode.Temp);
        for (int logIndex = 1; logIndex < DataConvergParams::ConvergLogStackDepth; logIndex++) {
            airLoopConv.HVACTempSupplyDeck2ToDemandTolValue[logIndex] = TmpRealARR[logIndex - 1];
        }
        if (airLoopConv.HVACTempSupplyDeck2ToDemandTolValue[0] > DataConvergParams::HVACTemperatureToler) {
            airLoopConv.HVACTempNotConverged[iCall] = true;
            OutOfToleranceFlag = true; // Something has changed--resimulate the other side of the loop
        }

        TmpRealARR = airLoopConv.HVACEnergySupplyDeck2ToDemandTolValue;
        airLoopConv.HVACEnergySupplyDeck2ToDemandTolValue[0] = DeltaEnergy;
        for (int logIndex = 1; logIndex < DataConvergParams::ConvergLogStackDepth; logIndex++) {
            airLoopConv.HVACEnergySupplyDeck2ToDemandTolValue[logIndex] = TmpRealARR[logIndex - 1];
        }
        if (std::abs(DeltaEnergy) > DataConvergParams::HVACEnergyToler) {
            airLoopConv.HVACEnergyNotConverged[iCall] = true;
            OutOfToleranceFlag = true; // Something has changed--resimulate the other side of the loop
        }

        TmpRealARR = airLoopConv.HVACEnthalpySupplyDeck2ToDemandTolValue;
        airLoopConv.HVACEnthalpySupplyDeck2ToDemandTolValue[0] = std::abs(state.dataLoopNodes->Node(OutletNode).Enthalpy - thisInletNode.Enthalpy);
        for (int logIndex = 1; logIndex < DataConvergParams::ConvergLogStackDepth; logIndex++) {
            airLoopConv.HVACEnthalpySupplyDeck2ToDemandTolValue[logIndex] = TmpRealARR[logIndex - 1];
        }
        if (airLoopConv.HVACEnthalpySupplyDeck2ToDemandTolValue[0] > DataConvergParams::HVACEnthalpyToler) {
            OutOfToleranceFlag = true; // Something has changed--resimulate the other side of the loop
        }

        TmpRealARR = airLoopConv.HVACPressueSupplyDeck2ToDemandTolValue;
        airLoopConv.HVACPressueSupplyDeck2ToDemandTolValue[0] = std::abs(state.dataLoopNodes->Node(OutletNode).Press - thisInletNode.Press);
        for (int logIndex = 1; logIndex < DataConvergParams::ConvergLogStackDepth; logIndex++) {
            airLoopConv.HVACPressueSupplyDeck2ToDemandTolValue[logIndex] = TmpRealARR[logIndex - 1];
        }
        if (airLoopConv.HVACPressueSupplyDeck2ToDemandTolValue[0] > DataConvergParams::HVACPressToler) {
            OutOfToleranceFlag = true; // Something has changed--resimulate the other side of the loop
        }

        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            TmpRealARR = airLoopConv.HVACCO2SupplyDeck2ToDemandTolValue;
            airLoopConv.HVACCO2SupplyDeck2ToDemandTolValue[0] = std::abs(state.dataLoopNodes->Node(OutletNode).CO2 - thisInletNode.CO2);
            for (int logIndex = 1; logIndex < DataConvergParams::ConvergLogStackDepth; logIndex++) {
                airLoopConv.HVACCO2SupplyDeck2ToDemandTolValue[logIndex] = TmpRealARR[logIndex - 1];
            }
            if (airLoopConv.HVACCO2SupplyDeck2ToDemandTolValue[0] > DataConvergParams::HVACCO2Toler) {
                airLoopConv.HVACCO2NotConverged[iCall] = true;
                OutOfToleranceFlag = true; // Something has changed--resimulate the other side of the loop
            }
        }
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            TmpRealARR = airLoopConv.HVACGenContamSupplyDeck2ToDemandTolValue;
            airLoopConv.HVACGenContamSupplyDeck2ToDemandTolValue[0] =
                std::abs(state.dataLoopNodes->Node(OutletNode).GenContam - thisInletNode.GenContam);
            for (int logIndex = 1; logIndex < DataConvergParams::ConvergLogStackDepth; logIndex++) {
                airLoopConv.HVACGenContamSupplyDeck2ToDemandTolValue[logIndex] = TmpRealARR[logIndex - 1];
            }
            if (airLoopConv.HVACGenContamSupplyDeck2ToDemandTolValue[0] > DataConvergParams::HVACGenContamToler) {
                airLoopConv.HVACGenContamNotConverged[iCall] = true;
                OutOfToleranceFlag = true; // Something has changed--resimulate the other side of the loop
            }
        }
    }

    // Always update the new inlet conditions
    thisInletNode.Temp = state.dataLoopNodes->Node(OutletNode).Temp;
    thisInletNode.MassFlowRate = state.dataLoopNodes->Node(OutletNode).MassFlowRate;
    thisInletNode.MassFlowRateMinAvail = state.dataLoopNodes->Node(OutletNode).MassFlowRateMinAvail;
    thisInletNode.MassFlowRateMaxAvail = state.dataLoopNodes->Node(OutletNode).MassFlowRateMaxAvail;
    thisInletNode.Quality = state.dataLoopNodes->Node(OutletNode).Quality;
    thisInletNode.Press = state.dataLoopNodes->Node(OutletNode).Press;
    thisInletNode.Enthalpy = state.dataLoopNodes->Node(OutletNode).Enthalpy;
    thisInletNode.HumRat = state.dataLoopNodes->Node(OutletNode).HumRat;

    if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
        thisInletNode.CO2 = state.dataLoopNodes->Node(OutletNode).CO2;
    }

    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
        thisInletNode.GenContam = state.dataLoopNodes->Node(OutletNode).GenContam;
    }
}

void UpdatePlantLoopInterface(EnergyPlusData &state,
                              PlantLocation const &plantLoc,    // The 'outlet node' Location
                              int const ThisLoopSideOutletNode, // Node number for the inlet of the side that needs the outlet node data
                              int const OtherLoopSideInletNode, // Node number for the outlet of the side of the loop just simulated
                              bool &OutOfToleranceFlag,         // True when the other side of the loop need to be (re)simulated
                              DataPlant::CommonPipeType const CommonPipeType)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   October 1998
    //       MODIFIED       na
    //       RE-ENGINEERED  Brent Griffith, Sept. 2010
    //       RE-ENGINEERED  Dan Fisher,     Sept. 2010

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine manages any generic HVAC loop interface.

    // METHODOLOGY EMPLOYED:
    // This is a simple "forward" interface where all of the properties
    // from the outlet of one side of the loop get transfered
    // to the inlet node of the corresponding other side of the loop.
    // Temperatures are 'lagged' by loop capacitance (i.e. a 'tank')
    // between the outlet and inlet nodes.
    // the update from the demand side to the supply side always triggers
    // resimulation of the supply side if any state variable (or energy) is
    // out of tolerance.  Remsimulation of the demand side is only triggered if
    // flow or energy are out of tolerance.  This in effect checks flow and
    // ~.25C temperature difference.

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("UpdatePlantLoopInterface");

    int LoopNum = plantLoc.loopNum;
    DataPlant::LoopSideLocation ThisLoopSideNum = plantLoc.loopSideNum;
    auto &convergence(state.dataConvergeParams->PlantConvergence(LoopNum));

    // reset out of tolerance flags
    convergence.PlantMassFlowNotConverged = false;
    convergence.PlantTempNotConverged = false;

    // set the LoopSide inlet node
    int ThisLoopSideInletNode = state.dataPlnt->PlantLoop(LoopNum).LoopSide(ThisLoopSideNum).NodeNumIn;

    // save the inlet node temp for DeltaEnergy check
    Real64 OldOtherLoopSideInletMdot = state.dataLoopNodes->Node(OtherLoopSideInletNode).MassFlowRate;
    Real64 OldTankOutletTemp = state.dataLoopNodes->Node(OtherLoopSideInletNode).Temp;

    // calculate the specific heat
    Real64 Cp = state.dataPlnt->PlantLoop(LoopNum).glycol->getSpecificHeat(state, OldTankOutletTemp, RoutineName);

    // update the enthalpy
    state.dataLoopNodes->Node(OtherLoopSideInletNode).Enthalpy = Cp * state.dataLoopNodes->Node(OtherLoopSideInletNode).Temp;

    // update the temperatures and flow rates
    auto &flow_demand_to_supply_tol(convergence.PlantFlowDemandToSupplyTolValue);
    auto &flow_supply_to_demand_tol(convergence.PlantFlowSupplyToDemandTolValue);
    Real64 MixedOutletTemp;
    Real64 TankOutletTemp;
    if (CommonPipeType == DataPlant::CommonPipeType::Single || CommonPipeType == DataPlant::CommonPipeType::TwoWay) {
        // update the temperature
        UpdateCommonPipe(state, plantLoc, CommonPipeType, MixedOutletTemp);
        state.dataLoopNodes->Node(OtherLoopSideInletNode).Temp = MixedOutletTemp;
        TankOutletTemp = MixedOutletTemp;
        if (ThisLoopSideNum == DataPlant::LoopSideLocation::Demand) {
            rshift1(flow_demand_to_supply_tol);
            flow_demand_to_supply_tol[0] = std::abs(OldOtherLoopSideInletMdot - state.dataLoopNodes->Node(OtherLoopSideInletNode).MassFlowRate);
            if (flow_demand_to_supply_tol[0] > DataConvergParams::PlantFlowRateToler) {
                convergence.PlantMassFlowNotConverged = true;
            }
        } else {
            rshift1(flow_supply_to_demand_tol);
            flow_supply_to_demand_tol[0] = std::abs(OldOtherLoopSideInletMdot - state.dataLoopNodes->Node(OtherLoopSideInletNode).MassFlowRate);
            if (flow_supply_to_demand_tol[0] > DataConvergParams::PlantFlowRateToler) {
                convergence.PlantMassFlowNotConverged = true;
            }
        }
        // Set the flow rate.  Continuity requires that the flow rates at the half loop inlet and outlet match
        state.dataLoopNodes->Node(ThisLoopSideInletNode).MassFlowRate = state.dataLoopNodes->Node(ThisLoopSideOutletNode).MassFlowRate;
        // Update this LoopSide inlet node Min/MaxAvail to this LoopSide outlet node Min/MaxAvail
        state.dataLoopNodes->Node(ThisLoopSideInletNode).MassFlowRateMinAvail =
            state.dataLoopNodes->Node(ThisLoopSideOutletNode).MassFlowRateMinAvail;
        state.dataLoopNodes->Node(ThisLoopSideInletNode).MassFlowRateMaxAvail =
            state.dataLoopNodes->Node(ThisLoopSideOutletNode).MassFlowRateMaxAvail;

    } else { // no common pipe
        UpdateHalfLoopInletTemp(state, LoopNum, ThisLoopSideNum, TankOutletTemp);
        // update the temperature
        state.dataLoopNodes->Node(OtherLoopSideInletNode).Temp = TankOutletTemp;
        // Set the flow tolerance array
        if (ThisLoopSideNum == DataPlant::LoopSideLocation::Demand) {
            rshift1(flow_demand_to_supply_tol);
            flow_demand_to_supply_tol[0] = std::abs(state.dataLoopNodes->Node(ThisLoopSideOutletNode).MassFlowRate -
                                                    state.dataLoopNodes->Node(OtherLoopSideInletNode).MassFlowRate);
            if (flow_demand_to_supply_tol[0] > DataConvergParams::PlantFlowRateToler) {
                convergence.PlantMassFlowNotConverged = true;
            }
        } else {
            rshift1(flow_supply_to_demand_tol);
            flow_supply_to_demand_tol[0] = std::abs(state.dataLoopNodes->Node(ThisLoopSideOutletNode).MassFlowRate -
                                                    state.dataLoopNodes->Node(OtherLoopSideInletNode).MassFlowRate);
            if (flow_supply_to_demand_tol[0] > DataConvergParams::PlantFlowRateToler) {
                convergence.PlantMassFlowNotConverged = true;
            }
        }
        //    PlantFlowTolValue(PlantQuePtr)  = ABS(Node(ThisLoopSideOutletNode)%MassFlowRate-Node(OtherLoopSideInletNode)%MassFlowRate)
        // Set the flow rate
        state.dataLoopNodes->Node(OtherLoopSideInletNode).MassFlowRate = state.dataLoopNodes->Node(ThisLoopSideOutletNode).MassFlowRate;
        // update the MIN/MAX available flow rates
        state.dataLoopNodes->Node(OtherLoopSideInletNode).MassFlowRateMinAvail =
            state.dataLoopNodes->Node(ThisLoopSideOutletNode).MassFlowRateMinAvail;
        state.dataLoopNodes->Node(OtherLoopSideInletNode).MassFlowRateMaxAvail =
            state.dataLoopNodes->Node(ThisLoopSideOutletNode).MassFlowRateMaxAvail;
        // update Quality.  Note: This update assumes that STEAM cannot be used with common pipes.
        state.dataLoopNodes->Node(OtherLoopSideInletNode).Quality = state.dataLoopNodes->Node(ThisLoopSideOutletNode).Quality;
        // pressure update  Note: This update assumes that PRESSURE SIMULATION cannot be used with common pipes.
        if (state.dataPlnt->PlantLoop(LoopNum).HasPressureComponents) {
            // Don't update pressure, let the pressure simulation handle pressures
        } else {
            // Do update pressure!
            state.dataLoopNodes->Node(OtherLoopSideInletNode).Press = state.dataLoopNodes->Node(ThisLoopSideOutletNode).Press;
        }
    }

    // temperature
    if (ThisLoopSideNum == DataPlant::LoopSideLocation::Demand) {
        auto &temp_demand_to_supply_tol(convergence.PlantTempDemandToSupplyTolValue);
        rshift1(temp_demand_to_supply_tol);
        temp_demand_to_supply_tol[0] = std::abs(OldTankOutletTemp - state.dataLoopNodes->Node(OtherLoopSideInletNode).Temp);
        if (temp_demand_to_supply_tol[0] > DataConvergParams::PlantTemperatureToler) {
            convergence.PlantTempNotConverged = true;
        }
    } else {
        auto &temp_supply_to_demand_tol(convergence.PlantTempSupplyToDemandTolValue);
        rshift1(temp_supply_to_demand_tol);
        temp_supply_to_demand_tol[0] = std::abs(OldTankOutletTemp - state.dataLoopNodes->Node(OtherLoopSideInletNode).Temp);
        if (temp_supply_to_demand_tol[0] > DataConvergParams::PlantTemperatureToler) {
            convergence.PlantTempNotConverged = true;
        }
    }

    // Set out of tolerance flags
    if (ThisLoopSideNum == DataPlant::LoopSideLocation::Demand) {
        if (convergence.PlantMassFlowNotConverged || convergence.PlantTempNotConverged) {
            OutOfToleranceFlag = true;
        }
    } else {
        if (convergence.PlantMassFlowNotConverged) {
            OutOfToleranceFlag = true;
        }
    }
}

void UpdateHalfLoopInletTemp(EnergyPlusData &state, int const LoopNum, const DataPlant::LoopSideLocation TankInletLoopSide, Real64 &TankOutletTemp)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   September 2001
    //       MODIFIED       Simon Rees, July 2007
    //                      Brent Griffith, Feb. 2010, add LoopNum arg
    //       RE-ENGINEERED  Brent Griffith, Sept 2010, generalize for both loop sides
    //                                           add pump heat from other loop
    //                      B.Griffith and L.Gu, Oct 2011, solve via analytical soln, use average over timestep

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine calculates the new loop side inlet temperature
    // based on the previous temperature of the mixed tank, mass flow rate and the new
    // outlet temperature on the supply side.  The temperature does not
    // pass directly across because the loop has some capacitance. It is
    // called separately but used for both supply-to-demand, and demand-to-supply

    // METHODOLOGY EMPLOYED:
    // This uses a analytical solution for changes in the
    // fluid loop temperature.  The user defines some volume of fluid
    // for the loop which gets converted to a fixed amount of mass.
    // The loop side inlet node is modeled as the outlet of a fully mixed
    // tank. Note that this routine is called repeatedly to re calculate
    // loop capacitance based on current plant conditions

    Real64 SysTimeElapsed = state.dataHVACGlobal->SysTimeElapsed;

    // SUBROUTINE PARAMETER DEFINITIONS:
    Real64 constexpr FracTotLoopMass(0.5); // Fraction of total loop mass assigned to the half loop
    static constexpr std::string_view RoutineName("UpdateHalfLoopInletTemp");

    // find tank inlet and outlet nodes
    DataPlant::LoopSideLocation TankOutletLoopSide = DataPlant::LoopSideOther[static_cast<int>(TankInletLoopSide)];
    int TankInletNode = state.dataPlnt->PlantLoop(LoopNum).LoopSide(TankInletLoopSide).NodeNumOut;
    Real64 TankInletTemp = state.dataLoopNodes->Node(TankInletNode).Temp;

    // This needs to be based on time to deal with system downstepping and repeated timesteps
    Real64 TimeElapsed = (state.dataGlobal->HourOfDay - 1) + state.dataGlobal->TimeStep * state.dataGlobal->TimeStepZone + SysTimeElapsed;
    if (state.dataPlnt->PlantLoop(LoopNum).LoopSide(TankOutletLoopSide).TimeElapsed != TimeElapsed) {
        state.dataPlnt->PlantLoop(LoopNum).LoopSide(TankOutletLoopSide).LastTempInterfaceTankOutlet =
            state.dataPlnt->PlantLoop(LoopNum).LoopSide(TankOutletLoopSide).TempInterfaceTankOutlet;
        state.dataPlnt->PlantLoop(LoopNum).LoopSide(TankOutletLoopSide).TimeElapsed = TimeElapsed;
    }

    Real64 LastTankOutletTemp = state.dataPlnt->PlantLoop(LoopNum).LoopSide(TankOutletLoopSide).LastTempInterfaceTankOutlet;

    // calculate the specific heat for the capacitance calculation
    Real64 Cp = state.dataPlnt->PlantLoop(LoopNum).glycol->getSpecificHeat(state, LastTankOutletTemp, RoutineName);
    // set the fraction of loop mass assigned to each half loop outlet capacitance ('tank') calculation

    // calculate new loop inlet temperature.  The calculation is a simple 'tank' (thermal capacitance) calculation that includes:
    //--half of loop mass.  The other half is accounted for at the other half loop interface
    //--pump heat.  Pump heat for a single loop setpoint with pumps only on the supply side is added at the supply side inlet.
    //   Pump heat for a dual setpoint loop is added to each loop side inlet
    //  The previous tank temperature value is used to prevent accumulation of pump heat during iterations while recalculating
    // tank conditions each call.
    // Analytical solution for ODE, formulated for both final tank temp and average tank temp.

    Real64 TimeStepSeconds = state.dataHVACGlobal->TimeStepSysSec;
    Real64 MassFlowRate = state.dataLoopNodes->Node(TankInletNode).MassFlowRate;
    Real64 PumpHeat = state.dataPlnt->PlantLoop(LoopNum).LoopSide(TankOutletLoopSide).TotalPumpHeat;
    Real64 ThisTankMass = FracTotLoopMass * state.dataPlnt->PlantLoop(LoopNum).Mass;
    Real64 TankFinalTemp;
    Real64 TankAverageTemp;
    if (ThisTankMass <= 0.0) { // no mass, no plant loop volume
        if (MassFlowRate > 0.0) {
            TankFinalTemp = TankInletTemp + PumpHeat / (MassFlowRate * Cp);
            TankAverageTemp = (TankFinalTemp + LastTankOutletTemp) / 2.0;
        } else {
            TankFinalTemp = LastTankOutletTemp;
            TankAverageTemp = LastTankOutletTemp;
        }

    } else { // tank has mass
        if (MassFlowRate > 0.0) {
            Real64 const mdotCp = MassFlowRate * Cp;
            Real64 const mdotCpTempIn = mdotCp * TankInletTemp;
            Real64 const tankMassCp = ThisTankMass * Cp;
            Real64 const ExponentTerm = mdotCp / tankMassCp * TimeStepSeconds;
            if (ExponentTerm >= 700.0) {
                TankFinalTemp = (mdotCp * TankInletTemp + PumpHeat) / mdotCp;

                TankAverageTemp = (tankMassCp / mdotCp * (LastTankOutletTemp - (mdotCpTempIn + PumpHeat) / mdotCp) / TimeStepSeconds +
                                   (mdotCpTempIn + PumpHeat) / mdotCp);
            } else {
                TankFinalTemp = (LastTankOutletTemp - (mdotCpTempIn + PumpHeat) / mdotCp) * std::exp(-ExponentTerm) +
                                (mdotCpTempIn + PumpHeat) / (MassFlowRate * Cp);

                TankAverageTemp = (tankMassCp / mdotCp * (LastTankOutletTemp - (mdotCpTempIn + PumpHeat) / mdotCp) * (1.0 - std::exp(-ExponentTerm)) /
                                       TimeStepSeconds +
                                   (mdotCpTempIn + PumpHeat) / mdotCp);
            }
        } else {
            TankFinalTemp = PumpHeat / (ThisTankMass * Cp) * TimeStepSeconds + LastTankOutletTemp;
            TankAverageTemp = (TankFinalTemp + LastTankOutletTemp) / 2.0;
        }
    }

    // update last tank outlet temperature
    state.dataPlnt->PlantLoop(LoopNum).LoopSide(TankOutletLoopSide).TempInterfaceTankOutlet = TankFinalTemp;

    // update heat transport and heat storage rates
    state.dataPlnt->PlantLoop(LoopNum).LoopSide(TankOutletLoopSide).LoopSideInlet_MdotCpDeltaT =
        (TankInletTemp - TankAverageTemp) * Cp * MassFlowRate;
    state.dataPlnt->PlantLoop(LoopNum).LoopSide(TankOutletLoopSide).LoopSideInlet_McpDTdt =
        (ThisTankMass * Cp * (TankFinalTemp - LastTankOutletTemp)) / TimeStepSeconds;

    // update report variable
    state.dataPlnt->PlantLoop(LoopNum).LoopSide(TankOutletLoopSide).LoopSideInlet_TankTemp = TankAverageTemp;

    TankOutletTemp = TankAverageTemp;
}

void UpdateCommonPipe(EnergyPlusData &state,
                      PlantLocation const &TankInletPlantLoc,
                      DataPlant::CommonPipeType const CommonPipeType,
                      Real64 &MixedOutletTemp)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   September 2001
    //       MODIFIED       Simon Rees, July 2007
    //                      Brent Griffith, Feb. 2010, add LoopNum arg
    //       RE-ENGINEERED  Brent Griffith, Sept 2010, generalize for both loop sides
    //                                           add pump heat from other loop
    //                      B.Griffith and L.Gu, Oct 2011, solve via analytical soln, use average over timestep

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine calculates the new loop side inlet temperature
    // based on the previous temperature of the mixed tank, mass flow rate and the new
    // outlet temperature on the supply side.  The temperature does not
    // pass directly across because the loop has some capacitance. It is
    // called separately but used for both supply-to-demand, and demand-to-supply

    // METHODOLOGY EMPLOYED:
    // This uses a analytical solution for changes in the
    // fluid loop temperature.  The user defines some volume of fluid
    // for the loop which gets converted to a fixed amount of mass.
    // The loop side inlet node is modeled as the outlet of a fully mixed
    // tank. Note that this routine is called repeatedly to re calculate
    // loop capacitance based on current plant conditions

    // Using/Aliasing
    Real64 SysTimeElapsed = state.dataHVACGlobal->SysTimeElapsed;

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("UpdateCommonPipe");

    // find tank inlet and outlet nodes
    int LoopNum = TankInletPlantLoc.loopNum;
    DataPlant::LoopSideLocation TankInletLoopSide = TankInletPlantLoc.loopSideNum;
    DataPlant::LoopSideLocation TankOutletLoopSide = DataPlant::LoopSideOther[static_cast<int>(TankInletPlantLoc.loopSideNum)]; // Outlet loopside
    int TankInletNode = state.dataPlnt->PlantLoop(LoopNum).LoopSide(TankInletLoopSide).NodeNumOut;
    int TankOutletNode = state.dataPlnt->PlantLoop(LoopNum).LoopSide(TankOutletLoopSide).NodeNumIn;

    Real64 TankInletTemp = state.dataLoopNodes->Node(TankInletNode).Temp;

    Real64 FracTotLoopMass; // Fraction of total loop mass assigned to the half loop
    if (TankInletLoopSide == DataPlant::LoopSideLocation::Demand) {
        // for common pipe loops, assume 75% of plant loop volume is on the demand side
        FracTotLoopMass = 0.25;
    } else {
        FracTotLoopMass = 0.75;
    }

    // This needs to be based on time to deal with system downstepping and repeated timesteps
    Real64 TimeElapsed = (state.dataGlobal->HourOfDay - 1) + state.dataGlobal->TimeStep * state.dataGlobal->TimeStepZone + SysTimeElapsed;
    if (state.dataPlnt->PlantLoop(LoopNum).LoopSide(TankOutletLoopSide).TimeElapsed != TimeElapsed) {
        state.dataPlnt->PlantLoop(LoopNum).LoopSide(TankOutletLoopSide).LastTempInterfaceTankOutlet =
            state.dataPlnt->PlantLoop(LoopNum).LoopSide(TankOutletLoopSide).TempInterfaceTankOutlet;
        state.dataPlnt->PlantLoop(LoopNum).LoopSide(TankOutletLoopSide).TimeElapsed = TimeElapsed;
    }

    Real64 LastTankOutletTemp = state.dataPlnt->PlantLoop(LoopNum).LoopSide(TankOutletLoopSide).LastTempInterfaceTankOutlet;

    // calculate the specific heat for the capacitance calculation
    Real64 Cp = state.dataPlnt->PlantLoop(LoopNum).glycol->getSpecificHeat(state, LastTankOutletTemp, RoutineName);

    // set the fraction of loop mass assigned to each half loop outlet capacitance ('tank') calculation

    // calculate new loop inlet temperature.  The calculation is a simple 'tank' (thermal capacitance) calculation that includes:
    //--half of loop mass.  The other half is accounted for at the other half loop interface
    //--pump heat.  Pump heat for a single loop setpoint with pumps only on the supply side is added at the supply side inlet.
    // Pump heat for a dual setpoint loop is added to each loop side inlet
    // The previous inlet side temp,'ThisLoopSideTankOutletTemp' is used to prevent accumulation of pump heat during iterations.
    // The placement of the 'tank' for common pipes is *after* the outlet node and *before* the flow split or flow mixing.
    // This requires no logical check in the code since for purposes of temperature calculations, it is identical to the
    // no common pipe case.
    // calculation is separated because for common pipe, a different split for mass fraction is applied
    // The pump heat source is swapped around here compared to no common pipe (so pump heat sort stays on its own side).
    Real64 TimeStepSeconds = state.dataHVACGlobal->TimeStepSysSec;
    Real64 MassFlowRate = state.dataLoopNodes->Node(TankInletNode).MassFlowRate;
    Real64 PumpHeat = state.dataPlnt->PlantLoop(LoopNum).LoopSide(TankInletLoopSide).TotalPumpHeat;
    Real64 ThisTankMass = FracTotLoopMass * state.dataPlnt->PlantLoop(LoopNum).Mass;

    Real64 TankFinalTemp;
    Real64 TankAverageTemp;
    if (ThisTankMass <= 0.0) { // no mass, no plant loop volume
        if (MassFlowRate > 0.0) {
            TankFinalTemp = TankInletTemp + PumpHeat / (MassFlowRate * Cp);
            TankAverageTemp = (TankFinalTemp + LastTankOutletTemp) / 2.0;
        } else {
            TankFinalTemp = LastTankOutletTemp;
            TankAverageTemp = LastTankOutletTemp;
        }

    } else { // tank has mass
        if (MassFlowRate > 0.0) {
            TankFinalTemp = (LastTankOutletTemp - (MassFlowRate * Cp * TankInletTemp + PumpHeat) / (MassFlowRate * Cp)) *
                                std::exp(-(MassFlowRate * Cp) / (ThisTankMass * Cp) * TimeStepSeconds) +
                            (MassFlowRate * Cp * TankInletTemp + PumpHeat) / (MassFlowRate * Cp);
            TankAverageTemp = ((ThisTankMass * Cp) / (MassFlowRate * Cp) *
                                   (LastTankOutletTemp - (MassFlowRate * Cp * TankInletTemp + PumpHeat) / (MassFlowRate * Cp)) *
                                   (1.0 - std::exp(-(MassFlowRate * Cp) / (ThisTankMass * Cp) * TimeStepSeconds)) / TimeStepSeconds +
                               (MassFlowRate * Cp * TankInletTemp + PumpHeat) / (MassFlowRate * Cp));
        } else {

            TankFinalTemp = PumpHeat / (ThisTankMass * Cp) * TimeStepSeconds + LastTankOutletTemp;
            TankAverageTemp = (TankFinalTemp + LastTankOutletTemp) / 2.0;
        }
    }
    // Common Pipe Simulation
    if (CommonPipeType == DataPlant::CommonPipeType::Single) {
        ManageSingleCommonPipe(state, LoopNum, TankOutletLoopSide, TankAverageTemp, MixedOutletTemp);
        // 2-way (controlled) common pipe simulation
    } else if (CommonPipeType == DataPlant::CommonPipeType::TwoWay) {
        PlantLocation TankOutletPlantLoc = {LoopNum, TankOutletLoopSide, 0, 0};

        ManageTwoWayCommonPipe(state, TankOutletPlantLoc, TankAverageTemp);
        MixedOutletTemp = state.dataLoopNodes->Node(TankOutletNode).Temp;
    }

    state.dataPlnt->PlantLoop(LoopNum).LoopSide(TankOutletLoopSide).TempInterfaceTankOutlet = TankFinalTemp;

    state.dataPlnt->PlantLoop(LoopNum).LoopSide(TankOutletLoopSide).LoopSideInlet_TankTemp = TankAverageTemp;
}

void ManageSingleCommonPipe(EnergyPlusData &state,
                            int const LoopNum,                          // plant loop number
                            DataPlant::LoopSideLocation const LoopSide, // plant loop side number
                            Real64 const TankOutletTemp, // inlet temperature to the common pipe passed in from the capacitance calculation
                            Real64 &MixedOutletTemp      // inlet temperature to the common pipe passed in from the capacitance calculation
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Sankaranarayanan K P
    //       DATE WRITTEN   November 2006
    //       MODIFIED       B. Griffith, Jan 2010 clean up setup to allow mixing common pipe modes
    //                      B. Griffith, Mar 2010 add LoopNum arg and simplify
    //       RE-ENGINEERED  D. Fisher, Sept. 2010
    //                      B. Griffith, Oct 2011, major rewrite for plant upgrade

    // PURPOSE OF THIS SUBROUTINE:
    // To determine the conditions in common pipe viz., the flow flow temperature and direction of flow.

    // METHODOLOGY EMPLOYED:
    // Determine the flow on both sides of the common pipe. Decide if flow is coming into common pipe
    // or going out of common pipe. After that determine which interface calls the subroutine, i.e. if
    // called from "Demand to Supply" interface or "Supply to Demand" interface. Update the node temperatures
    // accordingly.

    // One time call to set up report variables and set common pipe 'type' flag
    if (!state.dataHVACInterfaceMgr->CommonPipeSetupFinished) {
        SetupCommonPipes(state);
    }

    auto &plantCommonPipe = state.dataHVACInterfaceMgr->PlantCommonPipe(LoopNum);

    // fill local node indexes
    int NodeNumPriIn = state.dataPlnt->PlantLoop(LoopNum).LoopSide(DataPlant::LoopSideLocation::Supply).NodeNumIn;
    int NodeNumPriOut = state.dataPlnt->PlantLoop(LoopNum).LoopSide(DataPlant::LoopSideLocation::Supply).NodeNumOut;
    int NodeNumSecIn = state.dataPlnt->PlantLoop(LoopNum).LoopSide(DataPlant::LoopSideLocation::Demand).NodeNumIn;
    int NodeNumSecOut = state.dataPlnt->PlantLoop(LoopNum).LoopSide(DataPlant::LoopSideLocation::Demand).NodeNumOut;

    if (plantCommonPipe.MyEnvrnFlag && state.dataGlobal->BeginEnvrnFlag) {
        plantCommonPipe.Flow = 0.0;
        plantCommonPipe.Temp = 0.0;
        plantCommonPipe.FlowDir = NoRecircFlow;
        plantCommonPipe.MyEnvrnFlag = false;
    }
    if (!state.dataGlobal->BeginEnvrnFlag) {
        plantCommonPipe.MyEnvrnFlag = true;
    }

    // every time inits
    Real64 MdotSec = state.dataLoopNodes->Node(NodeNumSecOut).MassFlowRate;
    Real64 MdotPri = state.dataLoopNodes->Node(NodeNumPriOut).MassFlowRate;

    Real64 TempSecOutTankOut;
    Real64 TempPriOutTankOut;
    if (LoopSide == DataPlant::LoopSideLocation::Supply) {
        TempSecOutTankOut = TankOutletTemp;
        TempPriOutTankOut = state.dataPlnt->PlantLoop(LoopNum).LoopSide(DataPlant::LoopSideLocation::Demand).LoopSideInlet_TankTemp;
    } else {
        TempPriOutTankOut = TankOutletTemp;
        TempSecOutTankOut = state.dataPlnt->PlantLoop(LoopNum).LoopSide(DataPlant::LoopSideLocation::Supply).LoopSideInlet_TankTemp;
    }

    // first do mass balances and find common pipe flow rate and direction
    Real64 MdotPriRCLeg; // flow rate of primary recirculation thru common pipe kg/s
    Real64 MdotSecRCLeg; // flow rate of secondary recirculation thru common pipe kg/s
    Real64 TempSecInlet; // temperature at secondary inlet deg C
    Real64 TempPriInlet; // temperature at primary inlet deg C
    int CPFlowDir;       // flow direction in single common pipe
    Real64 CommonPipeTemp;
    if (MdotPri > MdotSec) {
        MdotPriRCLeg = MdotPri - MdotSec;
        if (MdotPriRCLeg < DataBranchAirLoopPlant::MassFlowTolerance) {
            MdotPriRCLeg = 0.0;
            CPFlowDir = NoRecircFlow;
        } else {
            CPFlowDir = PrimaryRecirc;
        }
        MdotSecRCLeg = 0.0;
        CommonPipeTemp = TempPriOutTankOut;
    } else if (MdotPri < MdotSec) {
        MdotSecRCLeg = MdotSec - MdotPri;
        if (MdotSecRCLeg < DataBranchAirLoopPlant::MassFlowTolerance) {
            MdotSecRCLeg = 0.0;
            CPFlowDir = NoRecircFlow;
        } else {
            CPFlowDir = SecondaryRecirc;
        }
        MdotPriRCLeg = 0.0;
        CommonPipeTemp = TempSecOutTankOut;
    } else { // equal
        MdotPriRCLeg = 0.0;
        MdotSecRCLeg = 0.0;
        CPFlowDir = NoRecircFlow;
        CommonPipeTemp = (TempPriOutTankOut + TempSecOutTankOut) / 2.0;
    }

    // now calculate inlet temps

    if (MdotSec > 0.0) {
        TempSecInlet = (MdotPri * TempPriOutTankOut + MdotSecRCLeg * TempSecOutTankOut - MdotPriRCLeg * TempPriOutTankOut) / (MdotSec);
    } else {
        TempSecInlet = TempPriOutTankOut;
    }
    if (MdotPri > 0.0) {
        TempPriInlet = (MdotSec * TempSecOutTankOut + MdotPriRCLeg * TempPriOutTankOut - MdotSecRCLeg * TempSecOutTankOut) / (MdotPri);
    } else {
        TempPriInlet = TempSecOutTankOut;
    }

    // Update the Common Pipe Data structure for reporting purposes.
    plantCommonPipe.Flow = max(MdotPriRCLeg, MdotSecRCLeg);
    plantCommonPipe.Temp = CommonPipeTemp;
    plantCommonPipe.FlowDir = CPFlowDir;
    state.dataLoopNodes->Node(NodeNumSecIn).Temp = TempSecInlet;
    state.dataLoopNodes->Node(NodeNumPriIn).Temp = TempPriInlet;

    if (LoopSide == DataPlant::LoopSideLocation::Supply) {
        MixedOutletTemp = TempPriInlet;
    } else {
        MixedOutletTemp = TempSecInlet;
    }
}

void ManageTwoWayCommonPipe(EnergyPlusData &state, PlantLocation const &plantLoc, Real64 const TankOutletTemp)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   June 2011
    //       MODIFIED       na
    //       RE-ENGINEERED  B. Griffith, Oct 2011.  rewrite

    // PURPOSE OF THIS SUBROUTINE:
    // manage two-way common pipe modeling at half-loop interface

    // METHODOLOGY EMPLOYED:
    // calculate mixed temperatures and various flow rates
    // sequential substitution of system of equations

    // REFERENCES:
    // reimplementation of CheckTwoWayCommonPipeConditions by Sankaranarayanan K P Jan 2007

    // SUBROUTINE PARAMETER DEFINITIONS:
    enum class UpdateType
    {
        DemandLedPrimaryInlet,
        DemandLedSecondaryInlet,
        SupplyLedPrimaryInlet,
        SupplyLedSecondaryInlet
    } curCallingCase = UpdateType::SupplyLedPrimaryInlet;
    constexpr int MaxIterLimitCaseA(8);
    constexpr int MaxIterLimitCaseB(4);

    // one time setups
    if (!state.dataHVACInterfaceMgr->CommonPipeSetupFinished) {
        SetupCommonPipes(state);
    }

    auto &plantCommonPipe(state.dataHVACInterfaceMgr->PlantCommonPipe(plantLoc.loopNum));
    auto &thisPlantLoop = state.dataPlnt->PlantLoop(plantLoc.loopNum);

    // fill local node indexes
    int const NodeNumPriIn = thisPlantLoop.LoopSide(DataPlant::LoopSideLocation::Supply).NodeNumIn;
    int const NodeNumPriOut = thisPlantLoop.LoopSide(DataPlant::LoopSideLocation::Supply).NodeNumOut;
    int const NodeNumSecIn = thisPlantLoop.LoopSide(DataPlant::LoopSideLocation::Demand).NodeNumIn;
    int const NodeNumSecOut = thisPlantLoop.LoopSide(DataPlant::LoopSideLocation::Demand).NodeNumOut;

    // begin environment inits
    if (plantCommonPipe.MyEnvrnFlag && state.dataGlobal->BeginEnvrnFlag) {
        plantCommonPipe.PriToSecFlow = 0.0;
        plantCommonPipe.SecToPriFlow = 0.0;
        plantCommonPipe.PriCPLegFlow = 0.0;
        plantCommonPipe.SecCPLegFlow = 0.0;
        plantCommonPipe.MyEnvrnFlag = false;
    }

    if (!state.dataGlobal->BeginEnvrnFlag) {
        plantCommonPipe.MyEnvrnFlag = true;
    }

    // every time inits
    Real64 MdotSec = state.dataLoopNodes->Node(NodeNumSecOut).MassFlowRate; // assume known and fixed by demand side operation
    Real64 TempCPPrimaryCntrlSetPoint = state.dataLoopNodes->Node(NodeNumPriIn).TempSetPoint;
    Real64 TempCPSecondaryCntrlSetPoint = state.dataLoopNodes->Node(NodeNumSecIn).TempSetPoint;

    // 6 unknowns follow, fill with current values
    Real64 MdotPriToSec = plantCommonPipe.PriToSecFlow;
    Real64 MdotPriRCLeg = plantCommonPipe.PriCPLegFlow;
    Real64 MdotSecRCLeg = plantCommonPipe.SecCPLegFlow;
    Real64 TempSecInlet = state.dataLoopNodes->Node(NodeNumSecIn).Temp;
    Real64 TempPriInlet = state.dataLoopNodes->Node(NodeNumPriIn).Temp;
    Real64 MdotPri =
        state.dataLoopNodes->Node(NodeNumPriOut).MassFlowRate; // may or may not be an unknown, If variable speed primary side, then unknown

    Real64 TempPriOutTankOut;
    Real64 TempSecOutTankOut;
    if (plantLoc.loopSideNum == DataPlant::LoopSideLocation::Supply) {
        TempSecOutTankOut = TankOutletTemp;
        TempPriOutTankOut = thisPlantLoop.LoopSide(DataPlant::LoopSideLocation::Demand).LoopSideInlet_TankTemp;
    } else {
        TempPriOutTankOut = TankOutletTemp;
        TempSecOutTankOut = thisPlantLoop.LoopSide(DataPlant::LoopSideLocation::Supply).LoopSideInlet_TankTemp;
    }

    // determine current case
    // which side is being updated
    // commonpipe control point is the inlet of one of the half loops
    if (plantLoc.loopSideNum == DataPlant::LoopSideLocation::Supply) { // update primary inlet
        if (thisPlantLoop.LoopSide(DataPlant::LoopSideLocation::Supply).InletNodeSetPt &&
            !thisPlantLoop.LoopSide(DataPlant::LoopSideLocation::Demand).InletNodeSetPt) {
            curCallingCase = UpdateType::SupplyLedPrimaryInlet;

        } else if (!thisPlantLoop.LoopSide(DataPlant::LoopSideLocation::Supply).InletNodeSetPt &&
                   thisPlantLoop.LoopSide(DataPlant::LoopSideLocation::Demand).InletNodeSetPt) {
            curCallingCase = UpdateType::DemandLedPrimaryInlet;
        }
    } else { // update secondary inlet
        if (thisPlantLoop.LoopSide(DataPlant::LoopSideLocation::Supply).InletNodeSetPt &&
            !thisPlantLoop.LoopSide(DataPlant::LoopSideLocation::Demand).InletNodeSetPt) {
            curCallingCase = UpdateType::SupplyLedSecondaryInlet;

        } else if (!thisPlantLoop.LoopSide(DataPlant::LoopSideLocation::Supply).InletNodeSetPt &&
                   thisPlantLoop.LoopSide(DataPlant::LoopSideLocation::Demand).InletNodeSetPt) {
            curCallingCase = UpdateType::DemandLedSecondaryInlet;
        }
    }

    switch (curCallingCase) {
    case UpdateType::SupplyLedPrimaryInlet:
    case UpdateType::SupplyLedSecondaryInlet:
        // CASE A, Primary/Supply Led
        // six equations and six unknowns (although one has a setpoint)
        for (int loop = 1; loop <= MaxIterLimitCaseA; ++loop) {

            // eq 1
            if (std::abs(TempSecOutTankOut - TempCPPrimaryCntrlSetPoint) > DataPlant::DeltaTempTol) {
                MdotPriToSec = MdotPriRCLeg * (TempCPPrimaryCntrlSetPoint - TempPriOutTankOut) / (TempSecOutTankOut - TempCPPrimaryCntrlSetPoint);
                if (MdotPriToSec < DataBranchAirLoopPlant::MassFlowTolerance) {
                    MdotPriToSec = 0.0;
                }
                if (MdotPriToSec > MdotSec) {
                    MdotPriToSec = MdotSec;
                }
            } else {
                MdotPriToSec = MdotSec; //  what to do (?)
            }
            // eq. 5
            MdotPriRCLeg = MdotPri - MdotPriToSec;
            if (MdotPriRCLeg < DataBranchAirLoopPlant::MassFlowTolerance) {
                MdotPriRCLeg = 0.0;
            }

            // eq. 4
            MdotSecRCLeg = MdotSec - MdotPriToSec;
            if (MdotSecRCLeg < DataBranchAirLoopPlant::MassFlowTolerance) {
                MdotSecRCLeg = 0.0;
            }

            // eq  6
            if ((MdotPriToSec + MdotSecRCLeg) > DataBranchAirLoopPlant::MassFlowTolerance) {
                TempSecInlet = (MdotPriToSec * TempPriOutTankOut + MdotSecRCLeg * TempSecOutTankOut) / (MdotPriToSec + MdotSecRCLeg);
            } else {
                TempSecInlet = TempPriOutTankOut;
            }

            // eq. 3
            if ((plantCommonPipe.SupplySideInletPumpType == FlowType::Variable) && (curCallingCase == UpdateType::SupplyLedPrimaryInlet)) {
                // MdotPri is a variable to be calculated and flow request needs to be made
                if (std::abs(TempCPPrimaryCntrlSetPoint) > DataPlant::DeltaTempTol) {

                    MdotPri = (MdotPriRCLeg * TempPriOutTankOut + MdotPriToSec * TempSecOutTankOut) / (TempCPPrimaryCntrlSetPoint);

                    if (MdotPri < DataBranchAirLoopPlant::MassFlowTolerance) {
                        MdotPri = 0.0;
                    }
                } else {
                    MdotPri = MdotSec;
                }
                PlantLocation thisPlantLoc = {plantLoc.loopNum, DataPlant::LoopSideLocation::Supply, 1, 0};
                PlantUtilities::SetActuatedBranchFlowRate(state, MdotPri, NodeNumPriIn, thisPlantLoc, false);
            }

            // eq. 2
            if ((MdotPriToSec + MdotPriRCLeg) > DataBranchAirLoopPlant::MassFlowTolerance) {
                TempPriInlet = (MdotPriToSec * TempSecOutTankOut + MdotPriRCLeg * TempPriOutTankOut) / (MdotPriToSec + MdotPriRCLeg);
            } else {
                TempPriInlet = TempSecOutTankOut;
            }
        }
        break;
    case UpdateType::DemandLedPrimaryInlet:
    case UpdateType::DemandLedSecondaryInlet:
        // case B. Secondary/demand led

        // six equations and six unknowns (although one has a setpoint)
        for (int loop = 1; loop <= MaxIterLimitCaseB; ++loop) {
            // eq 1,
            if (std::abs(TempPriOutTankOut - TempSecOutTankOut) > DataPlant::DeltaTempTol) {
                MdotPriToSec = MdotSec * (TempCPSecondaryCntrlSetPoint - TempSecOutTankOut) / (TempPriOutTankOut - TempSecOutTankOut);
                if (MdotPriToSec < DataBranchAirLoopPlant::MassFlowTolerance) {
                    MdotPriToSec = 0.0;
                }
                if (MdotPriToSec > MdotSec) {
                    MdotPriToSec = MdotSec;
                }
            } else {
                MdotPriToSec = MdotSec;
            }

            // eq. 2,
            if ((MdotPriToSec + MdotPriRCLeg) > DataBranchAirLoopPlant::MassFlowTolerance) {
                TempPriInlet = (MdotPriToSec * TempSecOutTankOut + MdotPriRCLeg * TempPriOutTankOut) / (MdotPriToSec + MdotPriRCLeg);
            } else {
                TempPriInlet = TempSecOutTankOut;
            }

            // eq. 3
            if ((plantCommonPipe.SupplySideInletPumpType == FlowType::Variable) && (curCallingCase == UpdateType::DemandLedPrimaryInlet)) {
                // MdotPri is a variable to be calculated and flow request made
                if (std::abs(TempPriOutTankOut - TempPriInlet) > DataPlant::DeltaTempTol) {
                    MdotPri = MdotSec * (TempCPSecondaryCntrlSetPoint - TempSecOutTankOut) / (TempPriOutTankOut - TempPriInlet);
                    if (MdotPri < DataBranchAirLoopPlant::MassFlowTolerance) {
                        MdotPri = 0.0;
                    }
                } else {
                    MdotPri = MdotSec;
                }
                PlantLocation thisPlantLoc = {plantLoc.loopNum, DataPlant::LoopSideLocation::Supply, 1, 0};
                PlantUtilities::SetActuatedBranchFlowRate(state, MdotPri, NodeNumPriIn, thisPlantLoc, false);
            }

            // eq. 4
            MdotSecRCLeg = MdotSec - MdotPriToSec;
            if (MdotSecRCLeg < DataBranchAirLoopPlant::MassFlowTolerance) {
                MdotSecRCLeg = 0.0;
            }

            // eq. 5
            MdotPriRCLeg = MdotPri - MdotPriToSec;
            if (MdotPriRCLeg < DataBranchAirLoopPlant::MassFlowTolerance) {
                MdotPriRCLeg = 0.0;
            }

            // eq  6
            if ((MdotPriToSec + MdotSecRCLeg) > DataBranchAirLoopPlant::MassFlowTolerance) {
                TempSecInlet = (MdotPriToSec * TempPriOutTankOut + MdotSecRCLeg * TempSecOutTankOut) / (MdotPriToSec + MdotSecRCLeg);
            } else {
                TempSecInlet = TempPriOutTankOut;
            }
        }
    }

    // update
    plantCommonPipe.PriToSecFlow = MdotPriToSec;
    plantCommonPipe.SecToPriFlow = MdotPriToSec;
    plantCommonPipe.PriCPLegFlow = MdotPriRCLeg;
    plantCommonPipe.SecCPLegFlow = MdotSecRCLeg;
    state.dataLoopNodes->Node(NodeNumSecIn).Temp = TempSecInlet;
    state.dataLoopNodes->Node(NodeNumPriIn).Temp = TempPriInlet;
}

void SetupCommonPipes(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   Jan. 2010
    //       MODIFIED       B. Griffith Oct. 2011
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // collect allocation, outputs, and other set up for common pipes

    state.dataHVACInterfaceMgr->PlantCommonPipe.allocate(state.dataPlnt->TotNumLoops);

    for (int CurLoopNum = 1; CurLoopNum <= state.dataPlnt->TotNumLoops; ++CurLoopNum) {

        // reference to easily lookup the first item once
        auto &thisPlantLoop = state.dataPlnt->PlantLoop(CurLoopNum);
        auto &thisCommonPipe = state.dataHVACInterfaceMgr->PlantCommonPipe(CurLoopNum);
        auto const &first_demand_component_type = thisPlantLoop.LoopSide(DataPlant::LoopSideLocation::Demand).Branch(1).Comp(1).Type;
        auto const &first_supply_component_type = thisPlantLoop.LoopSide(DataPlant::LoopSideLocation::Supply).Branch(1).Comp(1).Type;

        switch (thisPlantLoop.CommonPipeType) {
        case DataPlant::CommonPipeType::No:
            thisCommonPipe.CommonPipeType = DataPlant::CommonPipeType::No;
            break;
        case DataPlant::CommonPipeType::Single: // Uncontrolled ('single') common pipe
            thisCommonPipe.CommonPipeType = DataPlant::CommonPipeType::Single;
            SetupOutputVariable(state,
                                "Plant Common Pipe Mass Flow Rate",
                                Constant::Units::kg_s,
                                thisCommonPipe.Flow,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisPlantLoop.Name);
            SetupOutputVariable(state,
                                "Plant Common Pipe Temperature",
                                Constant::Units::C,
                                thisCommonPipe.Temp,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisPlantLoop.Name);
            SetupOutputVariable(state,
                                "Plant Common Pipe Flow Direction Status",
                                Constant::Units::None,
                                thisCommonPipe.FlowDir,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisPlantLoop.Name);

            if (first_supply_component_type == DataPlant::PlantEquipmentType::PumpVariableSpeed) {
                // If/when the model supports variable-pumping primary, this can be removed.
                ShowWarningError(state, "SetupCommonPipes: detected variable speed pump on supply inlet of CommonPipe plant loop");
                ShowContinueError(state, format("Occurs on plant loop name = {}", thisPlantLoop.Name));
                ShowContinueError(state, "The common pipe model does not support varying the flow rate on the primary/supply side");
                ShowContinueError(state, "The primary/supply side will operate as if constant speed, and the simulation continues");
            }
            break;
        case DataPlant::CommonPipeType::TwoWay: // Controlled ('two-way') common pipe
            thisCommonPipe.CommonPipeType = DataPlant::CommonPipeType::TwoWay;
            SetupOutputVariable(state,
                                "Plant Common Pipe Primary Mass Flow Rate",
                                Constant::Units::kg_s,
                                thisCommonPipe.PriCPLegFlow,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisPlantLoop.Name);
            SetupOutputVariable(state,
                                "Plant Common Pipe Secondary Mass Flow Rate",
                                Constant::Units::kg_s,
                                thisCommonPipe.SecCPLegFlow,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisPlantLoop.Name);
            SetupOutputVariable(state,
                                "Plant Common Pipe Primary to Secondary Mass Flow Rate",
                                Constant::Units::kg_s,
                                thisCommonPipe.PriToSecFlow,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisPlantLoop.Name);
            SetupOutputVariable(state,
                                "Plant Common Pipe Secondary to Primary Mass Flow Rate",
                                Constant::Units::kg_s,
                                thisCommonPipe.SecToPriFlow,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisPlantLoop.Name);

            // check type of pump on supply side inlet
            if (first_supply_component_type == DataPlant::PlantEquipmentType::PumpConstantSpeed) {
                thisCommonPipe.SupplySideInletPumpType = FlowType::Constant;
            } else if (first_supply_component_type == DataPlant::PlantEquipmentType::PumpVariableSpeed) {
                thisCommonPipe.SupplySideInletPumpType = FlowType::Variable;
                // If/when the model supports variable-pumping primary, this can be removed.
                ShowWarningError(state, "SetupCommonPipes: detected variable speed pump on supply inlet of TwoWayCommonPipe plant loop");
                ShowContinueError(state, format("Occurs on plant loop name = {}", thisPlantLoop.Name));
                ShowContinueError(state, "The common pipe model does not support varying the flow rate on the primary/supply side");
                ShowContinueError(state, "The primary/supply side will operate as if constant speed, and the simulation continues");
            }
            // check type of pump on demand side inlet
            if (first_demand_component_type == DataPlant::PlantEquipmentType::PumpConstantSpeed) {
                thisCommonPipe.DemandSideInletPumpType = FlowType::Constant;
            } else if (first_demand_component_type == DataPlant::PlantEquipmentType::PumpVariableSpeed) {
                thisCommonPipe.DemandSideInletPumpType = FlowType::Variable;
            }
            break;
        default:
            assert(false);
        }
    }

    state.dataHVACInterfaceMgr->CommonPipeSetupFinished = true;
}

} // namespace EnergyPlus::HVACInterfaceManager
