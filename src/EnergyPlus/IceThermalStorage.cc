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
#include <cassert>
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/IceThermalStorage.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace IceThermalStorage {

    // MODULE INFORMATION:
    //       AUTHOR         Pyeongchan Ihm
    //       DATE WRITTEN   April 2002
    //       MODIFIED       Modified Refined model, added Simple model, by Guo Zhou, Oct 2002
    //                      Remove chiller, make just a storage tank, Michael J. Witte, Sep 2005
    //                      Added detailed ice storage model, Rick Strand, Feb 2006
    //                      B. Griffith, Sept 2010, plant upgrades, fluid properties
    //                      Enhancements to detailed ice storage model, Rick Strand, Aug 2012
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // This module simulates the performance of Ice Thermal Storage

    // METHODOLOGY EMPLOYED:
    // Once the PlantLoopManager determines that the Ice Thermal Storage
    // is available to meet a loop cooling demand, it calls SimIceStorage
    // which in turn calls the appropriate Ice Thermal Storage model.

    // REFERENCES: Dion J. King, ASHRAE Transactions v104, pt1, 1998.

    std::string const cIceStorageSizing("ThermalStorage:Sizing");
    std::string const cIceStorageSimple("ThermalStorage:Ice:Simple");
    std::string const cIceStorageDetailed("ThermalStorage:Ice:Detailed");

    // ITS parameter
    constexpr Real64 FreezTemp(0.0);       // Water freezing Temperature, 0[C]
    constexpr Real64 FreezTempIP(32.0);    // Water freezing Temperature, 32[F]
    constexpr Real64 TimeInterval(3600.0); // Time Interval (1 hr) [s]

    // Conversion parameter
    constexpr Real64 EpsLimitForX(0.0);         // 0.02  ! See Dion's code as eps1
    constexpr Real64 EpsLimitForDisCharge(0.0); // 0.20  ! See Dion's code as eps2
    constexpr Real64 EpsLimitForCharge(0.0);    // 0.20  ! See Dion's code as eps3

    // Parameter used by the Detailed Ice Storage Model
    constexpr Real64 DeltaTofMin(0.5); // Minimum allowed outlet side temperature difference [C]
    // This is (Tout - Tfreezing)
    constexpr Real64 DeltaTifMin(1.0); // Minimum allowed inlet side temperature difference [C]
    // This is (Tin - Tfreezing)

    PlantComponent *SimpleIceStorageData::factory(EnergyPlusData &state, std::string const &objectName)
    {
        // Process the input data for boilers if it hasn't been done already
        if (state.dataIceThermalStorage->getITSInput) {
            GetIceStorageInput(state);
            state.dataIceThermalStorage->getITSInput = false;
        }

        // Now look for this particular pipe in the list
        for (auto &ITS : state.dataIceThermalStorage->SimpleIceStorage) {
            if (ITS.Name == objectName) {
                return &ITS;
            }
        }
        // If we didn't find it, fatal
        ShowFatalError(state,
                       format("LocalSimpleIceStorageFactory: Error getting inputs for simple ice storage named: {}", objectName)); // LCOV_EXCL_LINE
        // Shut up the compiler
        return nullptr; // LCOV_EXCL_LINE
    }

    PlantComponent *DetailedIceStorageData::factory(EnergyPlusData &state, std::string const &objectName)
    {
        // Process the input data for boilers if it hasn't been done already
        if (state.dataIceThermalStorage->getITSInput) {
            GetIceStorageInput(state);
            state.dataIceThermalStorage->getITSInput = false;
        }

        // Now look for this particular pipe in the list
        for (auto &ITS : state.dataIceThermalStorage->DetailedIceStorage) {
            if (ITS.Name == objectName) {
                return &ITS;
            }
        }
        // If we didn't find it, fatal
        ShowFatalError(
            state, format("LocalDetailedIceStorageFactory: Error getting inputs for detailed ice storage named: {}", objectName)); // LCOV_EXCL_LINE
        // Shut up the compiler
        return nullptr; // LCOV_EXCL_LINE
    }

    void SimpleIceStorageData::simulate(EnergyPlusData &state,
                                        const PlantLocation &calledFromLocation,
                                        [[maybe_unused]] bool FirstHVACIteration,
                                        [[maybe_unused]] Real64 &CurLoad,
                                        bool RunFlag)
    {
        static constexpr std::string_view RoutineName("SimpleIceStorageData::simulate");

        // this was happening in PlantLoopEquip before
        auto &thisComp(state.dataPlnt->PlantLoop(calledFromLocation.loopNum)
                           .LoopSide(calledFromLocation.loopSideNum)
                           .Branch(calledFromLocation.branchNum)
                           .Comp(calledFromLocation.compNum));

        // If component setpoint based control is active for this equipment
        // then reset CurLoad to original EquipDemand.
        // Allow negative CurLoad.  For cold storage this means the storage should
        // charge, for hot storage, this means the storage should discharge.
        if (thisComp.CurOpSchemeType == DataPlant::OpScheme::CompSetPtBased) {
            Real64 localCurLoad = thisComp.EquipDemand;
            if (localCurLoad != 0) {
                RunFlag = true;
            }
        }

        if (state.dataGlobal->BeginEnvrnFlag && this->MyEnvrnFlag) {
            this->ResetXForITSFlag = true;
            this->MyEnvrnFlag = false;
        }

        if (!state.dataGlobal->BeginEnvrnFlag) {
            this->MyEnvrnFlag = true;
        }

        this->initialize(state); // Initialize simple ice storage

        //------------------------------------------------------------------------
        // FIRST PROCESS (MyLoad = 0.0 as IN)
        // At this moment as first calling of ITS, ITS provide ONLY MaxCap/OptCap/MinCap.
        //------------------------------------------------------------------------
        // First process is in subroutine CalcIceStorageCapacity(MaxCap,MinCap,OptCap) shown bellow.

        //------------------------------------------------------------------------
        // SECOND PROCESS (MyLoad is provided by E+ based on MaxCap/OptCap/MinCap)
        //------------------------------------------------------------------------
        // Below routines are starting when second calling.
        // After previous return, MyLoad is calculated based on MaxCap, OptCap, and MinCap.
        // Then PlandSupplySideManager provides MyLoad to simulate Ice Thermal Storage.
        // The process will be decided based on sign(+,-,0) of input U.

        // MJW 19 Sep 2005 - New approach - calculate MyLoad locally from inlet node temp
        //                   and outlet node setpoint until MyLoad that is passed in behaves well

        Real64 TempSetPt(0.0);
        Real64 TempIn = state.dataLoopNodes->Node(this->PltInletNodeNum).Temp;
        switch (state.dataPlnt->PlantLoop(this->plantLoc.loopNum).LoopDemandCalcScheme) {
        case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
            TempSetPt = state.dataLoopNodes->Node(this->PltOutletNodeNum).TempSetPoint;
        } break;
        case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
            TempSetPt = state.dataLoopNodes->Node(this->PltOutletNodeNum).TempSetPointHi;
        } break;
        default: {
            assert(false);
        } break;
        }
        Real64 DemandMdot = this->DesignMassFlowRate;

        Real64 Cp = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, TempIn, RoutineName);

        Real64 MyLoad2 = (DemandMdot * Cp * (TempIn - TempSetPt));
        MyLoad = MyLoad2;

        //     Set fraction of ice remaining in storage
        this->XCurIceFrac = this->IceFracRemain;

        //***** Dormant Process for ITS *****************************************
        //************************************************************************
        //        IF( U .EQ. 0.0 ) THEN
        if ((MyLoad2 == 0.0) || (DemandMdot == 0.0)) {
            this->CalcIceStorageDormant(state);

            //***** Charging Process for ITS *****************************************
            //************************************************************************
            //        ELSE IF( U .GT. 0.0 ) THEN
        } else if (MyLoad2 < 0.0) {

            Real64 MaxCap;
            Real64 MinCap;
            Real64 OptCap;
            this->CalcIceStorageCapacity(state, MaxCap, MinCap, OptCap);
            this->CalcIceStorageCharge(state);

            //***** Discharging Process for ITS *****************************************
            //************************************************************************
            //        ELSE IF( U .LT. 0.0 ) THEN
        } else if (MyLoad2 > 0.0) {

            Real64 MaxCap;
            Real64 MinCap;
            Real64 OptCap;
            this->CalcIceStorageCapacity(state, MaxCap, MinCap, OptCap);
            this->CalcIceStorageDischarge(state, MyLoad, RunFlag, MaxCap);
        } // Based on input of U value, deciding Dormant/Charge/Discharge process

        // Update Node properties: mdot and Temperature
        this->UpdateNode(state, MyLoad2, RunFlag);

        // Update report variables.
        this->RecordOutput(MyLoad2, RunFlag);
    }

    void SimpleIceStorageData::onInitLoopEquip([[maybe_unused]] EnergyPlusData &state, [[maybe_unused]] const PlantLocation &calledFromLocation)
    {
        this->oneTimeInit(state); // Initialize detailed ice storage

        this->size(state);
    }

    void DetailedIceStorageData::simulate(EnergyPlusData &state,
                                          [[maybe_unused]] const PlantLocation &calledFromLocation,
                                          [[maybe_unused]] bool FirstHVACIteration,
                                          [[maybe_unused]] Real64 &CurLoad,
                                          [[maybe_unused]] bool RunFlag)
    {

        if (state.dataGlobal->BeginEnvrnFlag && this->MyEnvrnFlag) {
            this->ResetXForITSFlag = true;
            this->MyEnvrnFlag = false;
        }

        if (!state.dataGlobal->BeginEnvrnFlag) {
            this->MyEnvrnFlag = true;
        }

        this->initialize(state); // Initialize detailed ice storage

        this->SimDetailedIceStorage(state); // Simulate detailed ice storage

        this->UpdateDetailedIceStorage(state); // Update detailed ice storage

        this->ReportDetailedIceStorage(state); // Report detailed ice storage
    }

    void DetailedIceStorageData::onInitLoopEquip([[maybe_unused]] EnergyPlusData &state, [[maybe_unused]] const PlantLocation &calledFromLocation)
    {
        this->oneTimeInit(state); // Initialize detailed ice storage

        this->size(state);
    }

    void DetailedIceStorageData::SimDetailedIceStorage(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   February 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is the main simulation subroutine for the detailed
        // ice storage model.

        // METHODOLOGY EMPLOYED:
        // Based on whether the unit is dormant, in charging mode, or in discharging
        // mode, the code either passes the flow through the bypass, through the tank,
        // or both.  This depends on the temperature relative to the setpoint temperature
        // and other features of the model.  The model itself is a LMTD model that uses
        // performance curve fits that are quadratic in fraction charged/discharged and
        // linear in LMTD for the calculation of Q.  The equations are actually non-
        // dimensionalized.

        // REFERENCES:
        // Ice Storage Component Model Proposal (Revised).doc by Rick Strand (Dec 2005/Jan 2006)

        int constexpr MaxIterNum(100);                      // Maximum number of internal iterations for ice storage solution
        Real64 constexpr SmallestLoad(0.1);                 // Smallest load to actually run the ice storage unit [Watts]
        Real64 constexpr TankDischargeToler(0.001);         // Below this fraction, there is nothing left to discharge
        Real64 constexpr TankChargeToler(0.999);            // Above this fraction, we don't have anything left to charge
        Real64 constexpr TemperatureToler(0.1);             // Temperature difference between iterations that indicates convergence [C]
        Real64 constexpr SIEquiv100GPMinMassFlowRate(6.31); // Used to non-dimensionalize flow rate for use in CubicLinear charging equation
                                                            // Flow rate divided by nominal 100GPM used to non-dimensionalize volume flow rate
                                                            // Assumes approximate density of 1000 kg/m3 to get an estimate for mass flow rate
        static constexpr std::string_view RoutineName("DetailedIceStorageData::SimDetailedIceStorage");

        int NodeNumIn = this->PlantInNodeNum;                      // Plant loop inlet node number for component
        int NodeNumOut = this->PlantOutNodeNum;                    // Plant loop outlet node number for component
        Real64 TempIn = state.dataLoopNodes->Node(NodeNumIn).Temp; // Inlet temperature to component (from plant loop) [C]
        Real64 TempSetPt(0.0);                                     // Setpoint temperature defined by loop controls [C]
        switch (state.dataPlnt->PlantLoop(this->plantLoc.loopNum).LoopDemandCalcScheme) {
        case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
            TempSetPt = state.dataLoopNodes->Node(NodeNumOut).TempSetPoint;
        } break;
        case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
            TempSetPt = state.dataLoopNodes->Node(NodeNumOut).TempSetPointHi;
        } break;
        default: {
            assert(false);
        } break;
        }

        int IterNum = 0;

        // Set derived type variables
        this->InletTemp = TempIn;
        this->MassFlowRate = state.dataLoopNodes->Node(NodeNumIn).MassFlowRate;

        // if two-way common pipe and no mass flow and tank is not full, then use design flow rate
        if ((state.dataPlnt->PlantLoop(this->plantLoc.loopNum).CommonPipeType == DataPlant::CommonPipeType::TwoWay) &&
            (std::abs(this->MassFlowRate) < DataBranchAirLoopPlant::MassFlowTolerance) && (this->IceFracRemaining < TankChargeToler)) {
            this->MassFlowRate = this->DesignMassFlowRate;
        }

        // Calculate the current load on the ice storage unit
        Real64 Cp = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, TempIn, RoutineName);

        // Estimated load on the ice storage unit [W]
        Real64 LocalLoad = this->MassFlowRate * Cp * (TempIn - TempSetPt);

        // Determine what the status is regarding the ice storage unit and the loop level flow
        if ((std::abs(LocalLoad) <= SmallestLoad) || (this->availSched->getCurrentVal() <= 0)) {
            // No real load on the ice storage device or ice storage OFF--bypass all of the flow and leave the tank alone
            this->CompLoad = 0.0;
            this->OutletTemp = TempIn;
            this->TankOutletTemp = TempIn;
            Real64 mdot = 0.0;
            PlantUtilities::SetComponentFlowRate(state, mdot, this->PlantInNodeNum, this->PlantOutNodeNum, this->plantLoc);

            this->BypassMassFlowRate = mdot;
            this->TankMassFlowRate = 0.0;
            this->MassFlowRate = mdot;

        } else if (LocalLoad < 0.0) {
            // The load is less than zero so we should be charging
            // Before we do anything, we should check to make sure that we will actually be charging the unit

            if ((TempIn > (this->FreezingTemp - DeltaTifMin)) || (this->IceFracRemaining >= TankChargeToler)) {
                // If the inlet temperature is not below the freezing temperature of the
                // device, then we cannot actually do any charging.  Bypass all of the flow.
                // Also, if the tank is already sufficiently charged, we don't need to
                // do any further charging.  So, bypass all of the flow.
                this->CompLoad = 0.0;
                this->OutletTemp = TempIn;
                this->TankOutletTemp = TempIn;
                Real64 mdot = 0.0;
                PlantUtilities::SetComponentFlowRate(state, mdot, this->PlantInNodeNum, this->PlantOutNodeNum, this->plantLoc);

                this->BypassMassFlowRate = mdot;
                this->TankMassFlowRate = 0.0;
                this->MassFlowRate = mdot;

            } else {
                // make flow request so tank will get flow
                Real64 mdot = this->DesignMassFlowRate;
                PlantUtilities::SetComponentFlowRate(state, mdot, this->PlantInNodeNum, this->PlantOutNodeNum, this->plantLoc);

                // We are in charging mode, the temperatures are low enough to charge
                // the tank, and we have some charging left to do.
                // Make first guess at Qstar based on the current ice fraction remaining
                // and LMTDstar that is based on the freezing or TempSetPt temperature.
                if (TempSetPt > (this->FreezingTemp - DeltaTofMin)) {
                    // Outlet temperature cannot be above the freezing temperature so set
                    // the outlet temperature to the freezing temperature and calculate
                    // LMTDstar based on that assumption.
                    TempSetPt = this->FreezingTemp - DeltaTofMin;
                }

                // Tank outlet temperature from the last iteration [C]
                Real64 ToutOld = TempSetPt;
                // Non-dimensional log mean temperature difference of ice storage unit [non-dimensional]
                Real64 LMTDstar = CalcDetIceStorLMTDstar(TempIn, ToutOld, this->FreezingTemp);
                Real64 MassFlowstar = this->MassFlowRate / SIEquiv100GPMinMassFlowRate;

                // Find initial guess at average fraction charged during time step
                // Fraction of tank to be charged in the current time step
                Real64 ChargeFrac = LocalLoad * state.dataHVACGlobal->TimeStepSys / this->NomCapacity;
                if ((this->IceFracRemaining + ChargeFrac) > 1.0) {
                    ChargeFrac = 1.0 - this->IceFracRemaining;
                }

                Real64 AvgFracCharged; // Average fraction charged for the current time step
                if (this->ThawProcessIndex == DetIce::InsideMelt) {
                    AvgFracCharged = this->IceFracOnCoil + (ChargeFrac / 2.0);
                } else { // (DetailedIceStorage(IceNum)%ThawProcessIndex == DetIce::OutsideMelt)
                    AvgFracCharged = this->IceFracRemaining + (ChargeFrac / 2.0);
                }

                // Current load on the ice storage unit [non-dimensional]
                Real64 Qstar = std::abs(CalcQstar(state, this->ChargeCurveNum, this->ChargeCurveTypeNum, AvgFracCharged, LMTDstar, MassFlowstar));

                // Actual load on the ice storage unit [W]
                Real64 ActualLoad = Qstar * this->NomCapacity / this->CurveFitTimeStep;

                // Updated outlet temperature from the tank [C]
                Real64 ToutNew = TempIn + (ActualLoad / (this->MassFlowRate * Cp));
                // Again, the outlet temperature cannot be above the freezing temperature (factoring in the tolerance)
                if (ToutNew > (this->FreezingTemp - DeltaTofMin)) {
                    ToutNew = this->FreezingTemp - DeltaTofMin;
                }

                if (ActualLoad > std::abs(LocalLoad)) {
                    // We have more than enough capacity to meet the load so no need to iterate to find a solution
                    this->OutletTemp = TempSetPt;
                    this->TankOutletTemp = ToutNew;
                    this->CompLoad = this->MassFlowRate * Cp * std::abs(TempIn - TempSetPt);
                    this->TankMassFlowRate = this->CompLoad / Cp / std::abs(TempIn - ToutNew);
                    this->BypassMassFlowRate = this->MassFlowRate - this->TankMassFlowRate;

                } else {

                    while (IterNum < MaxIterNum) {
                        if (std::abs(ToutOld - ToutNew) > TemperatureToler) {
                            // Not converged yet so recalculated what is needed and keep iterating
                            // Calculate new values for LMTDstar and Qstar based on updated outlet temperature
                            ToutOld = ToutNew;
                            LMTDstar = CalcDetIceStorLMTDstar(TempIn, ToutOld, this->FreezingTemp);
                            MassFlowstar = this->MassFlowRate / SIEquiv100GPMinMassFlowRate;
                            Qstar =
                                std::abs(CalcQstar(state, this->ChargeCurveNum, this->ChargeCurveTypeNum, AvgFracCharged, LMTDstar, MassFlowstar));

                            // Now make sure that we don't go above 100% charged and calculate the new average fraction
                            ChargeFrac = Qstar * (state.dataHVACGlobal->TimeStepSys / this->CurveFitTimeStep);
                            if ((this->IceFracRemaining + ChargeFrac) > 1.0) {
                                ChargeFrac = 1.0 - this->IceFracRemaining;
                                Qstar = ChargeFrac;
                            }
                            if (this->ThawProcessIndex == DetIce::InsideMelt) {
                                AvgFracCharged = this->IceFracOnCoil + (ChargeFrac / 2.0);
                            } else { // (DetailedIceStorage(IceNum)%ThawProcessIndex == DetIce::OutsideMelt)
                                AvgFracCharged = this->IceFracRemaining + (ChargeFrac / 2.0);
                            }

                            // Finally, update the actual load and calculate the new outlet temperature; increment iteration counter
                            ActualLoad = Qstar * this->NomCapacity / this->CurveFitTimeStep;
                            ToutNew = TempIn + (ActualLoad / (this->MassFlowRate * Cp));
                            // Again, the outlet temperature cannot be above the freezing temperature (factoring in the tolerance)
                            if (ToutNew < (this->FreezingTemp - DeltaTofMin)) {
                                ToutNew = this->FreezingTemp - DeltaTofMin;
                            }
                            ++IterNum;

                        } else {
                            // Converged to acceptable tolerance so set output variables and exit DO WHILE loop
                            break;
                        }

                    } // ...loop iterating for the ice storage outlet temperature

                    // Keep track of times that the iterations got excessive and report if necessary
                    if (IterNum >= MaxIterNum) {
                        ++this->ChargeIterErrors;
                        if (this->ChargeIterErrors <= 25) {
                            ShowWarningError(state, "Detailed Ice Storage model exceeded its internal charging maximum iteration limit");
                            ShowContinueError(state, format("Detailed Ice Storage System Name = {}", this->Name));
                            ShowContinueErrorTimeStamp(state, "");
                        } else {
                            ShowRecurringWarningErrorAtEnd(state,
                                                           "Detailed Ice Storage system [" + this->Name +
                                                               "]  charging maximum iteration limit exceeded occurrence continues.",
                                                           this->ChargeErrorCount);
                        }
                    }

                    // Set the values for the key outlet parameters
                    // Note that in REAL(r64)ity the tank will probably bypass some flow when it
                    // gets close to full charge.  This is a simplification that assumes
                    // all flow through the tank during charging and a lower delta T near
                    // the full charge level.  From an energy perspective, this is a reasonable
                    // approximation.
                    this->OutletTemp = ToutNew;
                    this->TankOutletTemp = ToutNew;
                    this->BypassMassFlowRate = 0.0;
                    this->TankMassFlowRate = this->MassFlowRate;
                    this->CompLoad = this->MassFlowRate * Cp * std::abs(TempIn - ToutNew);
                }
            }

        } else if (LocalLoad > 0.0) {
            // The load is greater than zero so we should be discharging
            // Before we do anything, we should check to make sure that we will actually be discharging the unit

            if ((this->InletTemp < (this->FreezingTemp + DeltaTifMin)) || (this->IceFracRemaining <= TankDischargeToler)) {
                // If the inlet temperature is below the freezing temperature of the
                // device, then we cannot actually do any discharging.  Bypass all of the flow.
                // Also, if the tank is already discharged, we can't to do any further
                // discharging.  So, bypass all of the flow.
                this->CompLoad = 0.0;
                this->OutletTemp = this->InletTemp;
                this->TankOutletTemp = this->InletTemp;
                Real64 mdot = 0.0;
                PlantUtilities::SetComponentFlowRate(state, mdot, this->PlantInNodeNum, this->PlantOutNodeNum, this->plantLoc);

                this->BypassMassFlowRate = mdot;
                this->TankMassFlowRate = 0.0;
                this->MassFlowRate = mdot;

            } else {

                // make flow request so tank will get flow
                Real64 mdot = this->DesignMassFlowRate;
                PlantUtilities::SetComponentFlowRate(state, mdot, this->PlantInNodeNum, this->PlantOutNodeNum, this->plantLoc);

                // We are in discharging mode, the temperatures are high enough to discharge
                // the tank, and we have some discharging left to do.
                if (TempSetPt < (this->FreezingTemp + DeltaTofMin)) {
                    // Outlet temperature cannot be below the freezing temperature so set
                    // the outlet temperature to the freezing temperature and calculate
                    // LMTDstar based on that assumption.
                    TempSetPt = this->FreezingTemp + DeltaTofMin;
                }

                // Tank outlet temperature from the last iteration [C]
                Real64 ToutOld = TempSetPt;
                // Non-dimensional log mean temperature difference of ice storage unit [non-dimensional]
                Real64 LMTDstar = CalcDetIceStorLMTDstar(TempIn, ToutOld, this->FreezingTemp);
                Real64 MassFlowstar = this->MassFlowRate / SIEquiv100GPMinMassFlowRate;

                // Find initial guess at average fraction charged during time step
                Real64 ChargeFrac = LocalLoad * state.dataHVACGlobal->TimeStepSys / this->NomCapacity;
                if ((this->IceFracRemaining - ChargeFrac) < 0.0) {
                    ChargeFrac = this->IceFracRemaining;
                }
                Real64 AvgFracCharged = this->IceFracRemaining - (ChargeFrac / 2.0);

                // Current load on the ice storage unit [non-dimensional]
                Real64 Qstar =
                    std::abs(CalcQstar(state, this->DischargeCurveNum, this->DischargeCurveTypeNum, AvgFracCharged, LMTDstar, MassFlowstar));

                // Actual load on the ice storage unit [W]
                Real64 ActualLoad = Qstar * this->NomCapacity / this->CurveFitTimeStep;

                // Updated outlet temperature from the tank [C]
                Real64 ToutNew = TempIn - (ActualLoad / (this->MassFlowRate * Cp));
                // Again, the outlet temperature cannot be below the freezing temperature (factoring in the tolerance)
                if (ToutNew < (this->FreezingTemp + DeltaTofMin)) {
                    ToutNew = this->FreezingTemp + DeltaTofMin;
                }

                if (ActualLoad > LocalLoad) {
                    // We have more than enough storage to meet the load so no need to iterate to find a solution
                    this->OutletTemp = TempSetPt;
                    this->TankOutletTemp = ToutNew;
                    this->CompLoad = this->MassFlowRate * Cp * std::abs(TempIn - TempSetPt);
                    this->TankMassFlowRate = this->CompLoad / Cp / std::abs(TempIn - ToutNew);
                    this->BypassMassFlowRate = this->MassFlowRate - this->TankMassFlowRate;

                } else {

                    while (IterNum < MaxIterNum) {
                        if (std::abs(ToutOld - ToutNew) > TemperatureToler) {
                            // Not converged yet so recalculated what is needed and keep iterating
                            // Calculate new values for LMTDstar and Qstar based on updated outlet temperature
                            ToutOld = ToutNew;
                            LMTDstar = CalcDetIceStorLMTDstar(TempIn, ToutOld, this->FreezingTemp);

                            Qstar = std::abs(
                                CalcQstar(state, this->DischargeCurveNum, this->DischargeCurveTypeNum, AvgFracCharged, LMTDstar, MassFlowstar));

                            // Now make sure that we don't go below 100% discharged and calculate the new average fraction
                            ChargeFrac = Qstar * (state.dataHVACGlobal->TimeStepSys / this->CurveFitTimeStep);
                            if ((this->IceFracRemaining - ChargeFrac) < 0.0) {
                                ChargeFrac = this->IceFracRemaining;
                                Qstar = ChargeFrac;
                            }
                            AvgFracCharged = this->IceFracRemaining - (ChargeFrac / 2.0);

                            // Finally, update the actual load and calculate the new outlet temperature; increment iteration counter
                            ActualLoad = Qstar * this->NomCapacity / this->CurveFitTimeStep;
                            ToutNew = TempIn - (ActualLoad / (this->MassFlowRate * Cp));
                            // Again, the outlet temperature cannot be below the freezing temperature (factoring in the tolerance)
                            if (ToutNew < (this->FreezingTemp + DeltaTofMin)) {
                                ToutNew = this->FreezingTemp + DeltaTofMin;
                            }
                            ++IterNum;

                        } else {
                            // Converged to acceptable tolerance so set output variables and exit DO WHILE loop
                            break;
                        }

                    } // ...loop iterating for the ice storage outlet temperature

                    // Keep track of times that the iterations got excessive
                    if (IterNum >= MaxIterNum && (!state.dataGlobal->WarmupFlag)) {
                        ++this->DischargeIterErrors;
                        if (this->DischargeIterErrors <= 25) {
                            ShowWarningError(state, "Detailed Ice Storage model exceeded its internal discharging maximum iteration limit");
                            ShowContinueError(state, format("Detailed Ice Storage System Name = {}", this->Name));
                            ShowContinueErrorTimeStamp(state, "");
                        } else {
                            ShowRecurringWarningErrorAtEnd(state,
                                                           "Detailed Ice Storage system [" + this->Name +
                                                               "]  discharging maximum iteration limit exceeded occurrence continues.",
                                                           this->DischargeErrorCount);
                        }
                    }

                    // We are now done finding the outlet temperature of the tank.  We need
                    // to compare the outlet temperature to the setpoint temperature again
                    // to see where we are at and then we can set the values for the key
                    // outlet parameters.  If outlet temperature is greater than or equal
                    // to the setpoint temperature, then send all flow through the tank.
                    // Otherwise, we have more capacity than needed so let's bypass some
                    // flow and meet the setpoint temperature.
                    if (ToutNew >= TempSetPt) {
                        this->OutletTemp = ToutNew;
                        this->TankOutletTemp = ToutNew;
                        this->BypassMassFlowRate = 0.0;
                        this->TankMassFlowRate = this->MassFlowRate;
                        this->CompLoad = this->MassFlowRate * Cp * std::abs(TempIn - ToutNew);
                    } else {
                        this->OutletTemp = TempSetPt;
                        this->TankOutletTemp = ToutNew;
                        this->CompLoad = this->MassFlowRate * Cp * std::abs(TempIn - TempSetPt);
                        this->TankMassFlowRate = this->CompLoad / (Cp * std::abs(TempIn - ToutNew));
                        this->BypassMassFlowRate = this->MassFlowRate - this->TankMassFlowRate;
                    }
                }
            }

        } else { // Shouldn't get here ever (print error if we do)

            ShowFatalError(state, "Detailed Ice Storage systemic code error--contact EnergyPlus support");
        }
    }

    void GetIceStorageInput(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR:
        //       DATE WRITTEN:

        // PURPOSE OF THIS SUBROUTINE:!This routine will get the input
        // required by the PrimaryPlantLoopManager.  As such
        // it will interact with the Input Scanner to retrieve
        // information from the input file, count the number of
        // heating and cooling loops and begin to fill the
        // arrays associated with the type PlantLoopProps.

        static constexpr std::string_view routineName = "GetIceStorageInput";

        bool ErrorsFound;

        ErrorsFound = false; // Always need to reset this since there are multiple types of ice storage systems

        // LOAD ARRAYS WITH SimpleIceStorage DATA
        state.dataIceThermalStorage->NumThermalStorageSizing =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cIceStorageSizing);
        state.dataIceThermalStorage->NumSimpleIceStorage =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cIceStorageSimple); // by ZG
        state.dataIceThermalStorage->NumDetailedIceStorage =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cIceStorageDetailed);

        // Allocate ThermalStorage:Sizing based on NumThermalStorageSizing
        state.dataIceThermalStorage->ThermalStorageSizing.allocate(state.dataIceThermalStorage->NumThermalStorageSizing);

        state.dataIPShortCut->cCurrentModuleObject = cIceStorageSizing;
        for (int sizingNum = 1; sizingNum <= state.dataIceThermalStorage->NumThermalStorageSizing; ++sizingNum) {

            int NumAlphas;
            int NumNums;
            int IOStat;
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     state.dataIPShortCut->cCurrentModuleObject,
                                                                     sizingNum,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNums,
                                                                     IOStat,
                                                                     _,
                                                                     _,
                                                                     _,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            Util::IsNameEmpty(state, state.dataIPShortCut->cAlphaArgs(1), state.dataIPShortCut->cCurrentModuleObject, ErrorsFound);

            state.dataIceThermalStorage->ThermalStorageSizing(sizingNum).name = state.dataIPShortCut->cAlphaArgs(1);
            state.dataIceThermalStorage->ThermalStorageSizing(sizingNum).onPeakStart = state.dataIPShortCut->rNumericArgs(1);
            state.dataIceThermalStorage->ThermalStorageSizing(sizingNum).onPeakEnd = state.dataIPShortCut->rNumericArgs(2);
            state.dataIceThermalStorage->ThermalStorageSizing(sizingNum).sizingFactor = state.dataIPShortCut->rNumericArgs(3);
            if (state.dataIceThermalStorage->ThermalStorageSizing(sizingNum).onPeakEnd <=
                state.dataIceThermalStorage->ThermalStorageSizing(sizingNum).onPeakStart) {
                ShowSevereError(state,
                                format("{}{}=\"{}\"",
                                       routineName,
                                       state.dataIPShortCut->cCurrentModuleObject,
                                       state.dataIceThermalStorage->ThermalStorageSizing(sizingNum).name));
                ShowContinueError(state,
                                  format("Invalid start {} and end times {}. End time must be greater than start time.",
                                         state.dataIceThermalStorage->ThermalStorageSizing(sizingNum).onPeakStart,
                                         state.dataIceThermalStorage->ThermalStorageSizing(sizingNum).onPeakEnd));
            }
        }

        // Allocate SimpleIceStorage based on NumOfIceStorage
        state.dataIceThermalStorage->SimpleIceStorage.allocate(state.dataIceThermalStorage->NumSimpleIceStorage);

        state.dataIPShortCut->cCurrentModuleObject = cIceStorageSimple;
        for (int iceNum = 1; iceNum <= state.dataIceThermalStorage->NumSimpleIceStorage; ++iceNum) {

            int NumAlphas;
            int NumNums;
            int IOStat;
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     state.dataIPShortCut->cCurrentModuleObject,
                                                                     iceNum,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNums,
                                                                     IOStat,
                                                                     _,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            Util::IsNameEmpty(state, state.dataIPShortCut->cAlphaArgs(1), state.dataIPShortCut->cCurrentModuleObject, ErrorsFound);

            ++state.dataIceThermalStorage->TotalNumIceStorage;
            state.dataIceThermalStorage->SimpleIceStorage(iceNum).MapNum = state.dataIceThermalStorage->TotalNumIceStorage;

            // ITS name
            state.dataIceThermalStorage->SimpleIceStorage(iceNum).Name = state.dataIPShortCut->cAlphaArgs(1);

            // Get Ice Thermal Storage Type
            state.dataIceThermalStorage->SimpleIceStorage(iceNum).ITSType = state.dataIPShortCut->cAlphaArgs(2);
            if (Util::SameString(state.dataIceThermalStorage->SimpleIceStorage(iceNum).ITSType, "IceOnCoilInternal")) {
                state.dataIceThermalStorage->SimpleIceStorage(iceNum).ITSType_Num = ITSType::IceOnCoilInternal;
            } else if (Util::SameString(state.dataIceThermalStorage->SimpleIceStorage(iceNum).ITSType, "IceOnCoilExternal")) {
                state.dataIceThermalStorage->SimpleIceStorage(iceNum).ITSType_Num = ITSType::IceOnCoilExternal;
            } else {
                ShowSevereError(state, format("{}={}", state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state, format("Invalid {}={}", state.dataIPShortCut->cAlphaFieldNames(2), state.dataIPShortCut->cAlphaArgs(2)));
                ErrorsFound = true;
            }

            // Get and Verify ITS nominal Capacity (user input is in GJ, internal value in in J)
            state.dataIceThermalStorage->SimpleIceStorage(iceNum).ITSNomCap = state.dataIPShortCut->rNumericArgs(1) * 1.e+09;
            if (state.dataIPShortCut->rNumericArgs(1) == DataSizing::AutoSize) {
                state.dataIceThermalStorage->SimpleIceStorage(iceNum).NomCapacityWasAutoSized = true;
            } else if (state.dataIPShortCut->rNumericArgs(1) == 0.0) {
                ShowSevereError(state, format("{}={}", state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state,
                                  format("Invalid {}={:.2R}", state.dataIPShortCut->cNumericFieldNames(1), state.dataIPShortCut->rNumericArgs(1)));
                ErrorsFound = true;
            }

            // Get Plant Inlet Node Num
            state.dataIceThermalStorage->SimpleIceStorage(iceNum).PltInletNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    state.dataIPShortCut->cAlphaArgs(3),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::ThermalStorageIceSimple,
                                                    state.dataIPShortCut->cAlphaArgs(1),
                                                    DataLoopNode::NodeFluidType::Water,
                                                    DataLoopNode::ConnectionType::Inlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsNotParent);

            // Get Plant Outlet Node Num
            state.dataIceThermalStorage->SimpleIceStorage(iceNum).PltOutletNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    state.dataIPShortCut->cAlphaArgs(4),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::ThermalStorageIceSimple,
                                                    state.dataIPShortCut->cAlphaArgs(1),
                                                    DataLoopNode::NodeFluidType::Water,
                                                    DataLoopNode::ConnectionType::Outlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsNotParent);

            // Test InletNode and OutletNode
            BranchNodeConnections::TestCompSet(state,
                                               state.dataIPShortCut->cCurrentModuleObject,
                                               state.dataIPShortCut->cAlphaArgs(1),
                                               state.dataIPShortCut->cAlphaArgs(3),
                                               state.dataIPShortCut->cAlphaArgs(4),
                                               "Chilled Water Nodes");

            state.dataIceThermalStorage->SimpleIceStorage(iceNum).TESSizingIndex = Util::FindItemInList(
                state.dataIPShortCut->cAlphaArgs(5), state.dataIceThermalStorage->ThermalStorageSizing, &ThermalStorageSizingData::name);
            if (state.dataIceThermalStorage->SimpleIceStorage(iceNum).TESSizingIndex == 0 &&
                state.dataIceThermalStorage->SimpleIceStorage(iceNum).NomCapacityWasAutoSized) {
                ShowSevereError(state, format("Invalid {}={}", state.dataIPShortCut->cAlphaFieldNames(5), state.dataIPShortCut->cAlphaArgs(5)));
                ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state,
                                  format("Input field {} must be entered when input field {} is autosized",
                                         state.dataIPShortCut->cAlphaFieldNames(5),
                                         state.dataIPShortCut->cNumericFieldNames(1)));
                ErrorsFound = true;
            }

            // Initialize Report Variables
            state.dataIceThermalStorage->SimpleIceStorage(iceNum).MyLoad = 0.0;
            state.dataIceThermalStorage->SimpleIceStorage(iceNum).Urate = 0.0;
            state.dataIceThermalStorage->SimpleIceStorage(iceNum).IceFracRemain = 1.0;
            state.dataIceThermalStorage->SimpleIceStorage(iceNum).ITSCoolingRate_rep = 0.0;
            state.dataIceThermalStorage->SimpleIceStorage(iceNum).ITSCoolingEnergy_rep = 0.0;
            state.dataIceThermalStorage->SimpleIceStorage(iceNum).ITSChargingRate = 0.0;
            state.dataIceThermalStorage->SimpleIceStorage(iceNum).ITSChargingEnergy = 0.0;
            state.dataIceThermalStorage->SimpleIceStorage(iceNum).ITSmdot = 0.0;
            state.dataIceThermalStorage->SimpleIceStorage(iceNum).ITSInletTemp = 0.0;
            state.dataIceThermalStorage->SimpleIceStorage(iceNum).ITSOutletTemp = 0.0;

        } // IceNum

        if (ErrorsFound) {
            ShowFatalError(state, format("Errors found in processing input for {}", state.dataIPShortCut->cCurrentModuleObject));
        }

        ErrorsFound = false; // Always need to reset this since there are multiple types of ice storage systems

        // Determine the number of detailed ice storage devices are in the input file and allocate appropriately
        state.dataIPShortCut->cCurrentModuleObject = cIceStorageDetailed;

        state.dataIceThermalStorage->DetailedIceStorage.allocate(
            state.dataIceThermalStorage->NumDetailedIceStorage); // Allocate DetIceStorage based on NumDetIceStorages

        for (int iceNum = 1; iceNum <= state.dataIceThermalStorage->NumDetailedIceStorage; ++iceNum) {

            int NumAlphas;
            int NumNums;
            int IOStat;
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     state.dataIPShortCut->cCurrentModuleObject,
                                                                     iceNum,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNums,
                                                                     IOStat,
                                                                     _,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)};

            Util::IsNameEmpty(state, state.dataIPShortCut->cAlphaArgs(1), state.dataIPShortCut->cCurrentModuleObject, ErrorsFound);

            ++state.dataIceThermalStorage->TotalNumIceStorage;

            state.dataIceThermalStorage->DetailedIceStorage(iceNum).MapNum = state.dataIceThermalStorage->TotalNumIceStorage;
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).Name = state.dataIPShortCut->cAlphaArgs(1); // Detailed ice storage name

            // Get and verify availability schedule
            if (state.dataIPShortCut->lAlphaFieldBlanks(2)) {
                state.dataIceThermalStorage->DetailedIceStorage(iceNum).availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((state.dataIceThermalStorage->DetailedIceStorage(iceNum).availSched =
                            Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(2), state.dataIPShortCut->cAlphaArgs(2));
                ErrorsFound = true;
            }

            // Get and Verify ITS nominal Capacity (user input is in GJ, internal value is in W-hr)
            // Convert GJ to J by multiplying by 10^9
            // Convert J to W-hr by dividing by number of seconds in an hour (3600)
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).NomCapacity =
                state.dataIPShortCut->rNumericArgs(1) * (1.e+09) / Constant::rSecsInHour;
            if (state.dataIPShortCut->rNumericArgs(1) == DataSizing::AutoSize) {
                state.dataIceThermalStorage->DetailedIceStorage(iceNum).NomCapacityWasAutoSized = true;
            } else if (state.dataIPShortCut->rNumericArgs(1) <= 0.0) {
                ShowSevereError(state,
                                format("Invalid {}={:.2R}", state.dataIPShortCut->cNumericFieldNames(1), state.dataIPShortCut->rNumericArgs(1)));
                ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ErrorsFound = true;
            }

            // Get Plant Inlet Node Num
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).PlantInNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    state.dataIPShortCut->cAlphaArgs(3),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::ThermalStorageIceDetailed,
                                                    state.dataIPShortCut->cAlphaArgs(1),
                                                    DataLoopNode::NodeFluidType::Water,
                                                    DataLoopNode::ConnectionType::Inlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsNotParent);

            // Get Plant Outlet Node Num
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).PlantOutNodeNum =
                NodeInputManager::GetOnlySingleNode(state,
                                                    state.dataIPShortCut->cAlphaArgs(4),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::ThermalStorageIceDetailed,
                                                    state.dataIPShortCut->cAlphaArgs(1),
                                                    DataLoopNode::NodeFluidType::Water,
                                                    DataLoopNode::ConnectionType::Outlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsNotParent);

            // Test InletNode and OutletNode
            BranchNodeConnections::TestCompSet(state,
                                               state.dataIPShortCut->cCurrentModuleObject,
                                               state.dataIPShortCut->cAlphaArgs(1),
                                               state.dataIPShortCut->cAlphaArgs(3),
                                               state.dataIPShortCut->cAlphaArgs(4),
                                               "Chilled Water Nodes");

            // Obtain the Charging and Discharging Curve types and names
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).DischargeCurveName = state.dataIPShortCut->cAlphaArgs(6);
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).DischargeCurveNum =
                Curve::GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(6));
            if (state.dataIceThermalStorage->DetailedIceStorage(iceNum).DischargeCurveNum <= 0) {
                ShowSevereError(state, format("Invalid {}={}", state.dataIPShortCut->cAlphaFieldNames(6), state.dataIPShortCut->cAlphaArgs(6)));
                ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ErrorsFound = true;
            }

            int dischargeCurveDim =
                state.dataCurveManager->curves(state.dataIceThermalStorage->DetailedIceStorage(iceNum).DischargeCurveNum)->numDims;
            if (dischargeCurveDim != 2) {
                ShowSevereError(state, format("{}: Discharge curve must have 2 independent variables", state.dataIPShortCut->cCurrentModuleObject));
                ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state,
                                  format("{} does not have 2 independent variables and thus cannot be used for detailed ice storage",
                                         state.dataIPShortCut->cAlphaArgs(6)));
                ErrorsFound = true;
            } else {
                if (state.dataIPShortCut->cAlphaArgs(5) == "FRACTIONCHARGEDLMTD") {
                    state.dataIceThermalStorage->DetailedIceStorage(iceNum).DischargeCurveTypeNum = CurveVars::FracChargedLMTD;
                } else if (state.dataIPShortCut->cAlphaArgs(5) == "FRACTIONDISCHARGEDLMTD") {
                    state.dataIceThermalStorage->DetailedIceStorage(iceNum).DischargeCurveTypeNum = CurveVars::FracDischargedLMTD;
                } else if (state.dataIPShortCut->cAlphaArgs(5) == "LMTDMASSFLOW") {
                    state.dataIceThermalStorage->DetailedIceStorage(iceNum).DischargeCurveTypeNum = CurveVars::LMTDMassFlow;
                } else if (state.dataIPShortCut->cAlphaArgs(5) == "LMTDFRACTIONCHARGED") {
                    state.dataIceThermalStorage->DetailedIceStorage(iceNum).DischargeCurveTypeNum = CurveVars::LMTDFracCharged;
                } else {
                    ShowSevereError(state,
                                    format("{}: Discharge curve independent variable options not valid, option={}",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(5)));
                    ShowContinueError(state,
                                      format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                    ShowContinueError(state,
                                      "The valid options are: FractionChargedLMTD, FractionDischargedLMTD, LMTDMassFlow or LMTDFractionCharged");
                    ErrorsFound = true;
                }
            }

            ErrorsFound |= Curve::CheckCurveDims(state,
                                                 state.dataIceThermalStorage->DetailedIceStorage(iceNum).DischargeCurveNum, // Curve index
                                                 {2},                                                                       // Valid dimensions
                                                 "GetIceStorageInput: ",                                                    // Routine name
                                                 state.dataIPShortCut->cCurrentModuleObject,                                // Object Type
                                                 state.dataIceThermalStorage->DetailedIceStorage(iceNum).Name,              // Object Name
                                                 state.dataIPShortCut->cAlphaFieldNames(6));                                // Field Name

            state.dataIceThermalStorage->DetailedIceStorage(iceNum).ChargeCurveName = state.dataIPShortCut->cAlphaArgs(8);
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).ChargeCurveNum = Curve::GetCurveIndex(state, state.dataIPShortCut->cAlphaArgs(8));
            if (state.dataIceThermalStorage->DetailedIceStorage(iceNum).ChargeCurveNum <= 0) {
                ShowSevereError(state, format("Invalid {}={}", state.dataIPShortCut->cAlphaFieldNames(8), state.dataIPShortCut->cAlphaArgs(8)));
                ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ErrorsFound = true;
            }

            int chargeCurveDim = state.dataCurveManager->curves(state.dataIceThermalStorage->DetailedIceStorage(iceNum).ChargeCurveNum)->numDims;
            if (chargeCurveDim != 2) {
                ShowSevereError(state, format("{}: Charge curve must have 2 independent variables", state.dataIPShortCut->cCurrentModuleObject));
                ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state,
                                  format("{} does not have 2 independent variables and thus cannot be used for detailed ice storage",
                                         state.dataIPShortCut->cAlphaArgs(8)));
                ErrorsFound = true;
            } else {
                if (state.dataIPShortCut->cAlphaArgs(7) == "FRACTIONCHARGEDLMTD") {
                    state.dataIceThermalStorage->DetailedIceStorage(iceNum).ChargeCurveTypeNum = CurveVars::FracChargedLMTD;
                } else if (state.dataIPShortCut->cAlphaArgs(7) == "FRACTIONDISCHARGEDLMTD") {
                    state.dataIceThermalStorage->DetailedIceStorage(iceNum).ChargeCurveTypeNum = CurveVars::FracDischargedLMTD;
                } else if (state.dataIPShortCut->cAlphaArgs(7) == "LMTDMASSFLOW") {
                    state.dataIceThermalStorage->DetailedIceStorage(iceNum).ChargeCurveTypeNum = CurveVars::LMTDMassFlow;
                } else if (state.dataIPShortCut->cAlphaArgs(7) == "LMTDFRACTIONCHARGED") {
                    state.dataIceThermalStorage->DetailedIceStorage(iceNum).ChargeCurveTypeNum = CurveVars::LMTDFracCharged;
                } else {
                    ShowSevereError(state,
                                    format("{}: Charge curve independent variable options not valid, option={}",
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(7)));
                    ShowContinueError(state,
                                      format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                    ShowContinueError(state,
                                      "The valid options are: FractionChargedLMTD, FractionDischargedLMTD, LMTDMassFlow or LMTDFractionCharged");
                    ErrorsFound = true;
                }
            }

            ErrorsFound |= Curve::CheckCurveDims(state,
                                                 state.dataIceThermalStorage->DetailedIceStorage(iceNum).ChargeCurveNum, // Curve index
                                                 {2},                                                                    // Valid dimensions
                                                 "GetIceStorageInput: ",                                                 // Routine name
                                                 state.dataIPShortCut->cCurrentModuleObject,                             // Object Type
                                                 state.dataIceThermalStorage->DetailedIceStorage(iceNum).Name,           // Object Name
                                                 state.dataIPShortCut->cAlphaFieldNames(8));                             // Field Name

            state.dataIceThermalStorage->DetailedIceStorage(iceNum).CurveFitTimeStep = state.dataIPShortCut->rNumericArgs(2);
            if ((state.dataIceThermalStorage->DetailedIceStorage(iceNum).CurveFitTimeStep <= 0.0) ||
                (state.dataIceThermalStorage->DetailedIceStorage(iceNum).CurveFitTimeStep > 1.0)) {
                ShowSevereError(state,
                                format("Invalid {}={:.3R}", state.dataIPShortCut->cNumericFieldNames(2), state.dataIPShortCut->rNumericArgs(2)));
                ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(
                    state, format("Curve fit time step invalid, less than zero or greater than 1 for {}", state.dataIPShortCut->cAlphaArgs(1)));
                ErrorsFound = true;
            }

            state.dataIceThermalStorage->DetailedIceStorage(iceNum).ThawProcessIndicator = state.dataIPShortCut->cAlphaArgs(9);
            if (Util::SameString(state.dataIceThermalStorage->DetailedIceStorage(iceNum).ThawProcessIndicator, "INSIDEMELT")) {
                state.dataIceThermalStorage->DetailedIceStorage(iceNum).ThawProcessIndex = DetIce::InsideMelt;
            } else if ((Util::SameString(state.dataIceThermalStorage->DetailedIceStorage(iceNum).ThawProcessIndicator, "OUTSIDEMELT")) ||
                       (state.dataIceThermalStorage->DetailedIceStorage(iceNum).ThawProcessIndicator.empty())) {
                state.dataIceThermalStorage->DetailedIceStorage(iceNum).ThawProcessIndex = DetIce::OutsideMelt;
            } else {
                ShowSevereError(state, format("Invalid thaw process indicator of {} was entered", state.dataIPShortCut->cAlphaArgs(9)));
                ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state, R"(Value should either be "InsideMelt" or "OutsideMelt")");
                state.dataIceThermalStorage->DetailedIceStorage(iceNum).ThawProcessIndex =
                    DetIce::InsideMelt; // Severe error will end simulation, but just in case...
                ErrorsFound = true;
            }
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).TESSizingIndex = Util::FindItemInList(
                state.dataIPShortCut->cAlphaArgs(10), state.dataIceThermalStorage->ThermalStorageSizing, &ThermalStorageSizingData::name);
            if (state.dataIceThermalStorage->DetailedIceStorage(iceNum).TESSizingIndex == 0 &&
                state.dataIceThermalStorage->DetailedIceStorage(iceNum).NomCapacityWasAutoSized) {
                ShowSevereError(state, format("Invalid {}={}", state.dataIPShortCut->cAlphaFieldNames(10), state.dataIPShortCut->cAlphaArgs(10)));
                ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state,
                                  format("Input field {} must be entered when input field {} is autosized",
                                         state.dataIPShortCut->cAlphaFieldNames(10),
                                         state.dataIPShortCut->cNumericFieldNames(1)));
                ErrorsFound = true;
            }

            // Get the other ice storage parameters (electric, heat loss, freezing temperature) and stupidity check each one
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).DischargeParaElecLoad = state.dataIPShortCut->rNumericArgs(3);
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).ChargeParaElecLoad = state.dataIPShortCut->rNumericArgs(4);
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).TankLossCoeff = state.dataIPShortCut->rNumericArgs(5);
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).FreezingTemp = state.dataIPShortCut->rNumericArgs(6);

            if ((state.dataIceThermalStorage->DetailedIceStorage(iceNum).DischargeParaElecLoad < 0.0) ||
                (state.dataIceThermalStorage->DetailedIceStorage(iceNum).DischargeParaElecLoad > 1.0)) {
                ShowSevereError(state,
                                format("Invalid {}={:.3R}", state.dataIPShortCut->cNumericFieldNames(3), state.dataIPShortCut->rNumericArgs(3)));
                ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state, "Value is either less than/equal to zero or greater than 1");
                ErrorsFound = true;
            }

            if ((state.dataIceThermalStorage->DetailedIceStorage(iceNum).ChargeParaElecLoad < 0.0) ||
                (state.dataIceThermalStorage->DetailedIceStorage(iceNum).ChargeParaElecLoad > 1.0)) {
                ShowSevereError(state,
                                format("Invalid {}={:.3R}", state.dataIPShortCut->cNumericFieldNames(4), state.dataIPShortCut->rNumericArgs(4)));
                ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state, "Value is either less than/equal to zero or greater than 1");
                ErrorsFound = true;
            }

            if ((state.dataIceThermalStorage->DetailedIceStorage(iceNum).TankLossCoeff < 0.0) ||
                (state.dataIceThermalStorage->DetailedIceStorage(iceNum).TankLossCoeff > 0.1)) {
                ShowSevereError(state,
                                format("Invalid {}={:.3R}", state.dataIPShortCut->cNumericFieldNames(5), state.dataIPShortCut->rNumericArgs(5)));
                ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state, "Value is either less than/equal to zero or greater than 0.1 (10%)");
                ErrorsFound = true;
            }

            if ((state.dataIceThermalStorage->DetailedIceStorage(iceNum).FreezingTemp < -10.0) ||
                (state.dataIceThermalStorage->DetailedIceStorage(iceNum).FreezingTemp > 10.0)) {
                ShowWarningError(
                    state,
                    format("Potentially invalid {}={:.3R}", state.dataIPShortCut->cNumericFieldNames(6), state.dataIPShortCut->rNumericArgs(6)));
                ShowContinueError(state, format("Entered in {}={}", state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state, "Value is either less than -10.0C or greater than 10.0C");
                ShowContinueError(state, "This value will be allowed but the user should verify that this temperature is correct");
            }

            // Initialize Report Variables
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).CompLoad = 0.0;
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).IceFracChange = 0.0;
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).IceFracRemaining = 1.0;
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).IceFracOnCoil = 1.0;
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).DischargingRate = 0.0;
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).DischargingEnergy = 0.0;
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).ChargingRate = 0.0;
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).ChargingEnergy = 0.0;
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).MassFlowRate = 0.0;
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).BypassMassFlowRate = 0.0;
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).TankMassFlowRate = 0.0;
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).InletTemp = 0.0;
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).OutletTemp = 0.0;
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).TankOutletTemp = 0.0;
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).ParasiticElecRate = 0.0;
            state.dataIceThermalStorage->DetailedIceStorage(iceNum).ParasiticElecEnergy = 0.0;

        } // ...over detailed ice storage units

        if ((state.dataIceThermalStorage->NumSimpleIceStorage + state.dataIceThermalStorage->NumDetailedIceStorage) <= 0) {
            ShowSevereError(state, "No Ice Storage Equipment found in GetIceStorage");
            ErrorsFound = true;
        }

        if (ErrorsFound) {
            ShowFatalError(state, format("Errors found in processing input for {}", state.dataIPShortCut->cCurrentModuleObject));
        }
    }

    void SimpleIceStorageData::setupOutputVars(EnergyPlusData &state)
    {
        SetupOutputVariable(state,
                            "Ice Thermal Storage Requested Load",
                            Constant::Units::W,
                            this->MyLoad,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Ice Thermal Storage End Fraction",
                            Constant::Units::None,
                            this->IceFracRemain,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Ice Thermal Storage Mass Flow Rate",
                            Constant::Units::kg_s,
                            this->ITSmdot,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Ice Thermal Storage Inlet Temperature",
                            Constant::Units::C,
                            this->ITSInletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Ice Thermal Storage Outlet Temperature",
                            Constant::Units::C,
                            this->ITSOutletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Ice Thermal Storage Cooling Discharge Rate",
                            Constant::Units::W,
                            this->ITSCoolingRate_rep,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Ice Thermal Storage Cooling Discharge Energy",
                            Constant::Units::J,
                            this->ITSCoolingEnergy_rep,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name);

        SetupOutputVariable(state,
                            "Ice Thermal Storage Cooling Charge Rate",
                            Constant::Units::W,
                            this->ITSChargingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Ice Thermal Storage Cooling Charge Energy",
                            Constant::Units::J,
                            this->ITSChargingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name);
    }

    void DetailedIceStorageData::setupOutputVars(EnergyPlusData &state)
    {
        SetupOutputVariable(state,
                            "Ice Thermal Storage Cooling Rate",
                            Constant::Units::W,
                            this->CompLoad,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Ice Thermal Storage Change Fraction",
                            Constant::Units::None,
                            this->IceFracChange,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Ice Thermal Storage End Fraction",
                            Constant::Units::None,
                            this->IceFracRemaining,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Ice Thermal Storage On Coil Fraction",
                            Constant::Units::None,
                            this->IceFracOnCoil,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Ice Thermal Storage Mass Flow Rate",
                            Constant::Units::kg_s,
                            this->MassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Ice Thermal Storage Bypass Mass Flow Rate",
                            Constant::Units::kg_s,
                            this->BypassMassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Ice Thermal Storage Tank Mass Flow Rate",
                            Constant::Units::kg_s,
                            this->TankMassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Ice Thermal Storage Fluid Inlet Temperature",
                            Constant::Units::C,
                            this->InletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Ice Thermal Storage Blended Outlet Temperature",
                            Constant::Units::C,
                            this->OutletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Ice Thermal Storage Tank Outlet Temperature",
                            Constant::Units::C,
                            this->TankOutletTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Ice Thermal Storage Cooling Discharge Rate",
                            Constant::Units::W,
                            this->DischargingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Ice Thermal Storage Cooling Discharge Energy",
                            Constant::Units::J,
                            this->DischargingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name);

        SetupOutputVariable(state,
                            "Ice Thermal Storage Cooling Charge Rate",
                            Constant::Units::W,
                            this->ChargingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Ice Thermal Storage Cooling Charge Energy",
                            Constant::Units::J,
                            this->ChargingEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name);

        SetupOutputVariable(state,
                            "Ice Thermal Storage Ancillary Electricity Rate",
                            Constant::Units::W,
                            this->ParasiticElecRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Ice Thermal Storage Ancillary Electricity Energy",
                            Constant::Units::J,
                            this->ParasiticElecEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name,
                            Constant::eResource::Electricity,
                            OutputProcessor::Group::HVAC,
                            OutputProcessor::EndUseCat::Invalid);
    }

    void DetailedIceStorageData::oneTimeInit(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   February 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine initializes variables for the detailed ice storage model.

        // METHODOLOGY EMPLOYED:
        // Initializes parameters based on current status flag values.

        if (this->MyPlantScanFlag) {
            bool errFlag = false;
            PlantUtilities::ScanPlantLoopsForObject(state, this->Name, DataPlant::PlantEquipmentType::TS_IceDetailed, this->plantLoc, errFlag);

            if (errFlag) {
                ShowFatalError(state, "DetailedIceStorageData: oneTimeInit: Program terminated due to previous condition(s).");
            }

            this->setupOutputVars(state);
            this->MyPlantScanFlag = false;
        }
    }

    void DetailedIceStorageData::initialize(EnergyPlusData &state)
    {

        if (state.dataGlobal->BeginEnvrnFlag && this->MyEnvrnFlag2) { // Beginning of environment initializations
            // Make sure all state variables are reset at the beginning of every environment to avoid problems.
            // The storage unit is assumed to be fully charged at the start of any environment.
            // The IceNum variable is a module level variable that is already set before this subroutine is called.
            this->IceFracChange = 0.0;
            this->IceFracRemaining = 1.0;
            this->IceFracOnCoil = 1.0;
            this->InletTemp = 0.0;
            this->OutletTemp = 0.0;
            this->TankOutletTemp = 0.0;
            this->DischargeIterErrors = 0;
            this->ChargeIterErrors = 0;
            this->DesignMassFlowRate = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).MaxMassFlowRate;
            // no design flow rates for model, assume min is zero and max is plant loop's max
            PlantUtilities::InitComponentNodes(state, 0.0, this->DesignMassFlowRate, this->PlantInNodeNum, this->PlantOutNodeNum);

            if ((state.dataPlnt->PlantLoop(this->plantLoc.loopNum).CommonPipeType == DataPlant::CommonPipeType::TwoWay) &&
                (this->plantLoc.loopSideNum == DataPlant::LoopSideLocation::Supply)) {
                // up flow priority of other components on the same branch as the Ice tank
                for (int CompNum = 1; CompNum <= state.dataPlnt->PlantLoop(this->plantLoc.loopNum)
                                                     .LoopSide(DataPlant::LoopSideLocation::Supply)
                                                     .Branch(this->plantLoc.branchNum)
                                                     .TotalComponents;
                     ++CompNum) {
                    state.dataPlnt->PlantLoop(this->plantLoc.loopNum)
                        .LoopSide(DataPlant::LoopSideLocation::Supply)
                        .Branch(this->plantLoc.branchNum)
                        .Comp(CompNum)
                        .FlowPriority = DataPlant::LoopFlowStatus::NeedyAndTurnsLoopOn;
                }
            }

            this->MyEnvrnFlag2 = false;
        }
        if (!state.dataGlobal->BeginEnvrnFlag) {
            this->MyEnvrnFlag2 = true;
        }

        // Initializations that are done every iteration
        // Make sure all of the reporting variables are always reset at the start of any iteration
        this->CompLoad = 0.0;
        this->IceFracChange = 0.0;
        this->DischargingRate = 0.0;
        this->DischargingEnergy = 0.0;
        this->ChargingRate = 0.0;
        this->ChargingEnergy = 0.0;
        this->MassFlowRate = 0.0;
        this->BypassMassFlowRate = 0.0;
        this->TankMassFlowRate = 0.0;
        this->ParasiticElecRate = 0.0;
        this->ParasiticElecEnergy = 0.0;
    }

    void DetailedIceStorageData::size(EnergyPlusData &state)
    {
        int const TESTankIndex = Util::FindItemInList(this->Name, state.dataIceThermalStorage->DetailedIceStorage, &DetailedIceStorageData::Name);
        int const TESSizingIndex = state.dataIceThermalStorage->DetailedIceStorage(TESTankIndex).TESSizingIndex;
        if (TESSizingIndex == 0) {
            return;
        }
        std::string_view const tankType = "ThermalStorage:Ice:Detailed";
        std::string_view const callingRoutine = "DetailedIceStorageData::size";
        // Real64 constexpr tankHeatOfFusion = 334000.0; // J/Kg
        auto &plntLoop = state.dataPlnt->PlantLoop(this->plantLoc.loopNum);
        int PltSizNum = plntLoop.PlantSizNum;
        auto &plntSizData = state.dataSize->PlantSizData(PltSizNum);

        int startPeak = state.dataIceThermalStorage->ThermalStorageSizing(TESSizingIndex).onPeakStart * state.dataGlobal->TimeStepsInHour;
        int endPeak = state.dataIceThermalStorage->ThermalStorageSizing(TESSizingIndex).onPeakEnd * state.dataGlobal->TimeStepsInHour;
        Real64 onPeakTimeSteps = endPeak - startPeak;
        Real64 onPeakHours = onPeakTimeSteps / state.dataGlobal->TimeStepsInHour;
        Real64 sizingFactor = state.dataIceThermalStorage->ThermalStorageSizing(TESSizingIndex).sizingFactor;
        Real64 onPeakSumWaterFlow = 0.0;
        if (!plntLoop.plantDesWaterFlowRate.empty()) {
            for (int ts = 0; ts < 24 * state.dataGlobal->TimeStepsInHour; ++ts) {
                if (ts > startPeak && ts <= endPeak) {
                    onPeakSumWaterFlow += plntLoop.plantDesWaterFlowRate[ts];
                }
            }
            onPeakSumWaterFlow /= onPeakTimeSteps;
        }
        Real64 Cp = plntLoop.glycol->getSpecificHeat(state, plntSizData.ExitTemp, callingRoutine);
        Real64 rho = plntLoop.glycol->getDensity(state, plntSizData.ExitTemp, callingRoutine);
        Real64 onPeakEnergy = onPeakSumWaterFlow * rho * Cp * plntSizData.DeltaT * Constant::rSecsInHour *
                              onPeakHours; // need Joules here, J = m3/s * kg/m3 * J/kg-C * C * sec/hr * hr

        // now apply the heat of fusion J/Kg and size tank
        // Real64 tankCapacityKg = onPeakEnergy * sizingFactor / tankHeatOfFusion;    // kg
        // Real64 tankCapacityM3 = tankCapacityKg / rho;                              // m3
        Real64 tankCapacity = onPeakEnergy * sizingFactor / Constant::rSecsInHour; // kWh for detailed model

        if (this->NomCapacityWasAutoSized) {
            this->NomCapacity = std::max(1.0, tankCapacity);
        }
        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            if (!this->NomCapacityWasAutoSized) {
                BaseSizer::reportSizerOutput(
                    state, tankType, this->Name, "User-Specified Capacity [GJ]", this->NomCapacity * Constant::rSecsInHour / 1.0E9);
            }
            BaseSizer::reportSizerOutput(state, tankType, this->Name, "Design Size Capacity [GJ]", tankCapacity * Constant::rSecsInHour / 1.0E9);
        }
    }

    void SimpleIceStorageData::size(EnergyPlusData &state)
    {
        int const TESTankIndex = Util::FindItemInList(this->Name, state.dataIceThermalStorage->SimpleIceStorage, &SimpleIceStorageData::Name);
        int const TESSizingIndex = state.dataIceThermalStorage->SimpleIceStorage(TESTankIndex).TESSizingIndex;
        if (TESSizingIndex == 0) {
            return;
        }
        std::string_view const tankType = "ThermalStorage:Ice:Simple";
        std::string_view const callingRoutine = "SimpleIceStorageData::size";
        // Real64 constexpr tankHeatOfFusion = 334000.0; // J/Kg
        auto &plntLoop = state.dataPlnt->PlantLoop(this->plantLoc.loopNum);
        int PltSizNum = plntLoop.PlantSizNum;
        auto &plntSizData = state.dataSize->PlantSizData(PltSizNum);

        Real64 startPeak = state.dataIceThermalStorage->ThermalStorageSizing(TESSizingIndex).onPeakStart * state.dataGlobal->TimeStepsInHour;
        Real64 endPeak = state.dataIceThermalStorage->ThermalStorageSizing(TESSizingIndex).onPeakEnd * state.dataGlobal->TimeStepsInHour;
        Real64 onPeakTimeSteps = endPeak - startPeak;
        Real64 onPeakHours = onPeakTimeSteps / state.dataGlobal->TimeStepsInHour;
        Real64 sizingFactor = state.dataIceThermalStorage->ThermalStorageSizing(TESSizingIndex).sizingFactor;
        Real64 onPeakSumWaterFlow = 0.0;
        if (!plntLoop.plantDesWaterFlowRate.empty()) {
            for (int ts = 0; ts < 24 * state.dataGlobal->TimeStepsInHour; ++ts) {
                if (ts > startPeak && ts <= endPeak) {
                    onPeakSumWaterFlow += plntLoop.plantDesWaterFlowRate[ts];
                }
            }
            onPeakSumWaterFlow /= onPeakTimeSteps; // average m3/s
        }
        Real64 Cp = plntLoop.glycol->getSpecificHeat(state, plntSizData.ExitTemp, callingRoutine);
        Real64 rho = plntLoop.glycol->getDensity(state, plntSizData.ExitTemp, callingRoutine);
        // Real64 onPeakEnergy2 = onPeakSumWaterFlow * rho * Constant::rSecsInHour * onPeakHours *
        //                        tankHeatOfFusion; // need Joules here, J = m3/s * kg/m3 * sec/hr * hr * J/kg
        Real64 onPeakEnergy = onPeakSumWaterFlow * rho * Cp * plntSizData.DeltaT * Constant::rSecsInHour *
                              onPeakHours; // need Joules here, J = m3/s * kg/m3 * J/kg-C * C * sec/hr * hr

        // now apply the heat of fusion J/Kg and size tank
        // Real64 tankCapacityKg = onPeakEnergy * sizingFactor / tankHeatOfFusion; // kg
        // Real64 tankCapacityM3 = tankCapacityKg / rho;                           // m3
        Real64 tankCapacity = onPeakEnergy * sizingFactor; // J for simple model

        if (this->NomCapacityWasAutoSized) {
            this->ITSNomCap = std::max(1.0, tankCapacity);
        }
        if (state.dataPlnt->PlantFirstSizesOkayToFinalize) {
            if (!this->NomCapacityWasAutoSized) {
                BaseSizer::reportSizerOutput(state, tankType, this->Name, "User-Specified Capacity [GJ]", this->ITSNomCap / 1.0E9);
            }
            BaseSizer::reportSizerOutput(state, tankType, this->Name, "Design Size Capacity [GJ]", tankCapacity / 1.0E9);
        }
    }

    void SimpleIceStorageData::oneTimeInit(EnergyPlusData &state)
    {

        if (this->MyPlantScanFlag) {
            // Locate the storage on the plant loops for later usage
            bool errFlag = false;
            PlantUtilities::ScanPlantLoopsForObject(
                state, this->Name, DataPlant::PlantEquipmentType::TS_IceSimple, this->plantLoc, errFlag, _, _, _, _, _);
            if (errFlag) {
                ShowFatalError(state, "SimpleIceStorageData:oneTimeInit: Program terminated due to previous condition(s).");
            }

            this->setupOutputVars(state);
            this->MyPlantScanFlag = false;
        }
    }

    void SimpleIceStorageData::initialize(EnergyPlusData &state)
    {

        if (state.dataGlobal->BeginEnvrnFlag && this->MyEnvrnFlag2) {
            this->DesignMassFlowRate = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).MaxMassFlowRate;
            // no design flow rates for model, assume min is zero and max is plant loop's max
            PlantUtilities::InitComponentNodes(state, 0.0, this->DesignMassFlowRate, this->PltInletNodeNum, this->PltOutletNodeNum);
            if ((state.dataPlnt->PlantLoop(this->plantLoc.loopNum).CommonPipeType == DataPlant::CommonPipeType::TwoWay) &&
                (this->plantLoc.loopSideNum == DataPlant::LoopSideLocation::Supply)) {
                // up flow priority of other components on the same branch as the Ice tank
                for (int compNum = 1; compNum <= state.dataPlnt->PlantLoop(this->plantLoc.loopNum)
                                                     .LoopSide(DataPlant::LoopSideLocation::Supply)
                                                     .Branch(this->plantLoc.branchNum)
                                                     .TotalComponents;
                     ++compNum) {
                    state.dataPlnt->PlantLoop(this->plantLoc.loopNum)
                        .LoopSide(DataPlant::LoopSideLocation::Supply)
                        .Branch(this->plantLoc.branchNum)
                        .Comp(compNum)
                        .FlowPriority = DataPlant::LoopFlowStatus::NeedyAndTurnsLoopOn;
                }
            }
            this->MyLoad = 0.0;
            this->Urate = 0.0;
            this->IceFracRemain = 1.0;
            this->ITSCoolingRate = 0.0;
            this->ITSCoolingEnergy_rep = 0.0;
            this->ITSChargingRate = 0.0;
            this->ITSChargingEnergy = 0.0;
            this->ITSmdot = 0.0;
            this->ITSInletTemp = 0.0;
            this->ITSOutletTemp = 0.0;

            this->MyEnvrnFlag2 = false;
        }

        if (!state.dataGlobal->BeginEnvrnFlag) {
            this->MyEnvrnFlag2 = true;
        }
    }

    //******************************************************************************

    void SimpleIceStorageData::CalcIceStorageCapacity(EnergyPlusData &state, Real64 &MaxCap, Real64 &MinCap, Real64 &OptCap)
    {
        //------------------------------------------------------------------------
        // FIRST PROCESS (MyLoad = 0.0 as IN)
        // At this moment as first calling of ITS, ITS provide ONLY MaxCap/OptCap/MinCap.
        //------------------------------------------------------------------------

        // Initialize Capacity
        MaxCap = 0.0;
        MinCap = 0.0;
        OptCap = 0.0;

        // XCurIceFrac is reset to 1.0 when first hour of day.
        // Starting full is assumed, because most ice systems are fully charged overnight
        if (this->ResetXForITSFlag) {
            this->XCurIceFrac = 1.0;
            this->IceFracRemain = 1.0;
            this->Urate = 0.0;
            this->ResetXForITSFlag = false;
        }

        // Calculate UAIceDisch[W/C] and UAIceCh[W/F] based on ONLY XCurIceFrac
        this->CalcUAIce(this->XCurIceFrac, this->UAIceCh, this->UAIceDisCh, this->HLoss);

        // Calculate QiceMin by UAIceDisCh*deltaTlm
        //   with UAIceDisCh(function of XCurIceFrac), ITSInletTemp and ITSOutletTemp(=Node(OutletNodeNum)%TempSetPoint by E+[C])
        // QiceMin is REAL(r64) ITS capacity.
        Real64 QiceMin;
        this->CalcQiceDischageMax(state, QiceMin);

        // At the first call of ITS model, MyLoad is 0. After that proper MyLoad will be provided by E+.
        // Therefore, Umin is decided between input U and ITS REAL(r64) capacity.
        Real64 Umin = min(max((-(1.0 - EpsLimitForDisCharge) * QiceMin * TimeInterval / this->ITSNomCap), (-this->XCurIceFrac + EpsLimitForX)), 0.0);

        // Calculate CoolingRate with Uact to provide E+.
        Real64 Uact = Umin;
        Real64 ITSCoolingRateMax = std::abs(Uact * this->ITSNomCap / TimeInterval);
        Real64 ITSCoolingRateOpt = ITSCoolingRateMax;
        Real64 ITSCoolingRateMin = 0.0;

        // Define MaxCap, OptCap, and MinCap
        MaxCap = ITSCoolingRateMax;
        OptCap = ITSCoolingRateOpt;
        MinCap = ITSCoolingRateMin;
    }

    //******************************************************************************

    void SimpleIceStorageData::CalcIceStorageDormant(EnergyPlusData &state)
    {
        // Provide output results for ITS.
        this->ITSMassFlowRate = 0.0; //[kg/s]

        PlantUtilities::SetComponentFlowRate(state, this->ITSMassFlowRate, this->PltInletNodeNum, this->PltOutletNodeNum, this->plantLoc);

        this->ITSInletTemp = state.dataLoopNodes->Node(this->PltInletNodeNum).Temp; //[C]
        this->ITSOutletTemp = this->ITSInletTemp;                                   //[C]
        switch (state.dataPlnt->PlantLoop(this->plantLoc.loopNum).LoopDemandCalcScheme) {
        case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
            this->ITSOutletSetPointTemp = state.dataLoopNodes->Node(this->PltOutletNodeNum).TempSetPoint;
        } break;
        case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
            this->ITSOutletSetPointTemp = state.dataLoopNodes->Node(this->PltOutletNodeNum).TempSetPointHi;
        } break;
        default:
            break;
        }
        this->ITSCoolingRate = 0.0;   //[W]
        this->ITSCoolingEnergy = 0.0; //[J]

        this->Urate = 0.0; //[n/a]
    }

    //******************************************************************************

    void SimpleIceStorageData::CalcIceStorageCharge(EnergyPlusData &state)
    {
        //--------------------------------------------------------
        // Initialize
        //--------------------------------------------------------
        // Below values for ITS are reported forCharging process.
        this->ITSMassFlowRate = this->DesignMassFlowRate; //[kg/s]

        PlantUtilities::SetComponentFlowRate(state, this->ITSMassFlowRate, this->PltInletNodeNum, this->PltOutletNodeNum, this->plantLoc);

        this->ITSInletTemp = state.dataLoopNodes->Node(this->PltInletNodeNum).Temp; //[C]
        this->ITSOutletTemp = this->ITSInletTemp;                                   //[C]
        switch (state.dataPlnt->PlantLoop(this->plantLoc.loopNum).LoopDemandCalcScheme) {
        case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
            this->ITSOutletSetPointTemp = state.dataLoopNodes->Node(this->PltOutletNodeNum).TempSetPoint;
        } break;
        case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
            this->ITSOutletSetPointTemp = state.dataLoopNodes->Node(this->PltOutletNodeNum).TempSetPointHi;
        } break;
        default:
            break;
        }
        this->ITSCoolingRate = 0.0;   //[W]
        this->ITSCoolingEnergy = 0.0; //[J]

        // Initialize processed U values
        this->Urate = 0.0;

        // Calculate QiceMax which is REAL(r64) ITS capacity.
        // There are three possible to calculate QiceMax
        //   with ChillerCapacity(Chiller+ITS), ITS capacity(ITS), and QchillerMax(Chiller).
        //--------------------------------------------------------
        // Calculate QiceMax with QiceMaxByChiller, QiceMaxByITS, QchillerMax
        //--------------------------------------------------------
        // Calculate Qice charge max by Chiller with Twb and UAIceCh
        Real64 QiceMaxByChiller;
        this->CalcQiceChargeMaxByChiller(state, QiceMaxByChiller); //[W]

        // Chiller is remote now, so chiller out is inlet node temp
        Real64 chillerOutletTemp = state.dataLoopNodes->Node(this->PltInletNodeNum).Temp;
        // Calculate Qice charge max by ITS with ChillerOutletTemp
        Real64 QiceMaxByITS;
        this->CalcQiceChargeMaxByITS(chillerOutletTemp, QiceMaxByITS); //[W]

        // Select minimum as QiceMax
        // Because It is uncertain that QiceMax by chiller is same as QiceMax by ITS.
        Real64 QiceMax = min(QiceMaxByChiller, QiceMaxByITS);

        //--------------------------------------------------------
        // Calculate Umin,Umax,Uact
        //--------------------------------------------------------
        // Set Umin
        // Calculate Umax based on real ITS Max Capacity and remained XCurIceFrac.
        // Umax should be equal or larger than 0.02 for realistic purpose by Dion.
        Real64 Umax = max(min(((1.0 - EpsLimitForCharge) * QiceMax * TimeInterval / this->ITSNomCap), (1.0 - this->XCurIceFrac - EpsLimitForX)), 0.0);

        // Cannot charge more than the fraction that is left uncharged
        Umax = min(Umax, (1.0 - this->IceFracRemain) / state.dataHVACGlobal->TimeStepSys);
        // First, check input U value.
        // Based on Umax and Umin, if necessary to run E+, calculate proper Uact.
        Real64 Uact;
        if (Umax == 0.0) { //(No Capacity of ITS), ITS is OFF.
            Uact = 0.0;

        } else { // Umax non-zero
            Uact = Umax;
        } // Check Uact for Discharging Process

        //--------------------------------------------------------
        // Calculate possible ITSChargingRate with Uact, Then error check
        //--------------------------------------------------------
        // Calculate possible ITSChargingRate with Uact
        Real64 Qice = Uact * this->ITSNomCap / TimeInterval; //[W]
        // If Qice is equal or less than 0.0, no need to calculate anymore.
        if (Qice <= 0.0) {
            this->Urate = 0.0; //[ratio]
        }

        // Calculate leaving water temperature
        if ((Qice <= 0.0) || (this->XCurIceFrac >= 1.0)) {
            this->ITSOutletTemp = this->ITSInletTemp;
            Qice = 0.0;
            Uact = 0.0;
        } else {
            Real64 DeltaTemp = Qice / Psychrometrics::CPCW(this->ITSInletTemp) / this->ITSMassFlowRate;
            this->ITSOutletTemp = this->ITSInletTemp + DeltaTemp;
            // Limit leaving temp to be no greater than setpoint or freezing temp minus 1C
            this->ITSOutletTemp = min(this->ITSOutletTemp, this->ITSOutletSetPointTemp, (FreezTemp - 1));
            // Limit leaving temp to be no less than inlet temp
            this->ITSOutletTemp = max(this->ITSOutletTemp, this->ITSInletTemp);
            DeltaTemp = this->ITSOutletTemp - this->ITSInletTemp;
            Qice = DeltaTemp * Psychrometrics::CPCW(this->ITSInletTemp) * this->ITSMassFlowRate;
            Uact = Qice / (this->ITSNomCap / TimeInterval);
        } // End of leaving temp checks

        this->Urate = Uact;
        this->ITSCoolingRate = -Qice;
        this->ITSCoolingEnergy = this->ITSCoolingRate * state.dataHVACGlobal->TimeStepSysSec;
    }

    //******************************************************************************

    void SimpleIceStorageData::CalcQiceChargeMaxByChiller(EnergyPlusData &state, Real64 &QiceMaxByChiller)
    {
        // METHODOLOGY EMPLOYED:
        // Calculation inside is IP unit, then return QiceMaxByChiller as SI [W] unit.

        // Chiller is remote now, so chiller out is inlet node temp
        Real64 TchillerOut = state.dataLoopNodes->Node(this->PltInletNodeNum).Temp;
        QiceMaxByChiller = this->UAIceCh * (FreezTemp - TchillerOut); //[W] = [W/degC]*[degC]

        // If it happened, it is occurred at the Discharging or Dormant process.
        if (QiceMaxByChiller <= 0.0) {
            QiceMaxByChiller = 0.0;
        }
    }

    void SimpleIceStorageData::CalcQiceChargeMaxByITS(Real64 const chillerOutletTemp, // [degC]
                                                      Real64 &QiceMaxByITS            // [W]
    )
    {
        // Qice is maximized when ChillerInletTemp and ChillerOutletTemp(input data) is almost same due to LMTD method.
        // Qice is minimized(=0) when ChillerInletTemp is almost same as FreezTemp(=0).

        // Initialize
        Real64 Tfr = FreezTempIP;
        Real64 ChOutletTemp = TempSItoIP(chillerOutletTemp); //[degF] = ConvertSItoIP[degC]
        // Chiller outlet temp must be below freeze temp, or else no charge
        if (ChOutletTemp >= Tfr) {
            QiceMaxByITS = 0.0;
        } else {
            // Make ChillerInletTemp as almost same as ChillerOutletTemp(input data)
            Real64 ChillerInletTemp = ChOutletTemp + 0.01;
            // ChillerInletTemp cannot be greater than or equal to freeze temp
            if (ChillerInletTemp >= Tfr) {
                ChillerInletTemp = ChOutletTemp + (Tfr - ChOutletTemp) / 2;
            }

            Real64 LogTerm = (Tfr - ChOutletTemp) / (Tfr - ChillerInletTemp);
            // Need to protect this from LogTerm <= 0 - not sure what it should do then
            if (LogTerm <= 0.0) {
                ChillerInletTemp = ChOutletTemp;
                QiceMaxByITS = 0.0;
            }
            QiceMaxByITS = this->UAIceCh * (TempIPtoSI(ChillerInletTemp) - TempIPtoSI(ChOutletTemp)) / std::log(LogTerm);
        }
    }

    void SimpleIceStorageData::CalcIceStorageDischarge(EnergyPlusData &state,
                                                       Real64 const myLoad, // operating load
                                                       bool const RunFlag,  // TRUE when ice storage operating
                                                       Real64 const MaxCap  // Max possible discharge rate (positive value)
    )
    {
        static constexpr std::string_view RoutineName("SimpleIceStorageData::CalcIceStorageDischarge");

        // Initialize processed Rate and Energy
        this->ITSMassFlowRate = 0.0;
        this->ITSCoolingRate = 0.0;
        this->ITSCoolingEnergy = 0.0;

        switch (state.dataPlnt->PlantLoop(this->plantLoc.loopNum).LoopDemandCalcScheme) {
        case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
            this->ITSOutletSetPointTemp = state.dataLoopNodes->Node(this->PltOutletNodeNum).TempSetPoint;
        } break;
        case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
            this->ITSOutletSetPointTemp = state.dataLoopNodes->Node(this->PltOutletNodeNum).TempSetPointHi;
        } break;
        default:
            break;
        }

        // Initialize processed U values
        this->Urate = 0.0;

        // If no component demand or ITS OFF, then RETURN.
        if (myLoad == 0 || !RunFlag) {
            this->ITSMassFlowRate = 0.0;
            this->ITSInletTemp = state.dataLoopNodes->Node(this->PltInletNodeNum).Temp;
            this->ITSOutletTemp = this->ITSInletTemp;
            this->ITSCoolingRate = 0.0;
            this->ITSCoolingEnergy = 0.0;
            return;
        }

        // If FlowLock(provided by PlantSupplyManager) is False(=0), that is, MyLoad is not changed.
        // then based on MyLoad, new ITSMassFlowRate will be calculated.

        //----------------------------
        int loopNum = this->plantLoc.loopNum;

        Real64 CpFluid =
            state.dataPlnt->PlantLoop(loopNum).glycol->getSpecificHeat(state, state.dataLoopNodes->Node(this->PltInletNodeNum).Temp, RoutineName);

        // Calculate Umyload based on MyLoad from E+
        Real64 Umyload = -myLoad * TimeInterval / this->ITSNomCap;
        // Calculate Umax and Umin
        // Cannot discharge more than the fraction that is left
        Real64 Umax = -this->IceFracRemain / state.dataHVACGlobal->TimeStepSys;
        // Calculate Umin based on returned MyLoad from E+.
        Real64 Umin = min(Umyload, 0.0);
        // Based on Umax and Umin, if necessary to run E+, calculate proper Uact
        // U is negative here.
        Real64 Uact = max(Umin, Umax);

        // Set ITSInletTemp provided by E+
        this->ITSInletTemp = state.dataLoopNodes->Node(this->PltInletNodeNum).Temp;
        // The first thing is to set the ITSMassFlowRate
        this->ITSMassFlowRate = this->DesignMassFlowRate; //[kg/s]

        PlantUtilities::SetComponentFlowRate(state, this->ITSMassFlowRate, this->PltInletNodeNum, this->PltOutletNodeNum, this->plantLoc);

        // Qice is calculate input U which is within boundary between Umin and Umax.
        Real64 Qice = Uact * this->ITSNomCap / TimeInterval;
        // Qice cannot exceed MaxCap calculated by CalcIceStorageCapacity
        // Note Qice is negative here, MaxCap is positive
        Qice = max(Qice, -MaxCap);

        // Calculate leaving water temperature
        if ((Qice >= 0.0) || (this->XCurIceFrac <= 0.0) || (this->ITSMassFlowRate < DataBranchAirLoopPlant::MassFlowTolerance)) {
            this->ITSOutletTemp = this->ITSInletTemp;
            Qice = 0.0;
            Uact = 0.0;
        } else {
            Real64 DeltaTemp = Qice / CpFluid / this->ITSMassFlowRate;
            this->ITSOutletTemp = this->ITSInletTemp + DeltaTemp;
            // Limit leaving temp to be no less than setpoint or freezing temp plus 1C
            this->ITSOutletTemp = max(this->ITSOutletTemp, this->ITSOutletSetPointTemp, (FreezTemp + 1));
            // Limit leaving temp to be no greater than inlet temp
            this->ITSOutletTemp = min(this->ITSOutletTemp, this->ITSInletTemp);
            DeltaTemp = this->ITSOutletTemp - this->ITSInletTemp;
            Qice = DeltaTemp * CpFluid * this->ITSMassFlowRate;
            Uact = Qice / (this->ITSNomCap / TimeInterval);
        } // End of leaving temp checks

        // Calculate reported U value
        this->Urate = Uact;
        // Calculate ITSCoolingEnergy [J]
        this->ITSCoolingRate = -Qice;
        this->ITSCoolingEnergy = this->ITSCoolingRate * state.dataHVACGlobal->TimeStepSysSec;
    }

    void SimpleIceStorageData::CalcQiceDischageMax(EnergyPlusData &state, Real64 &QiceMin)
    {

        // Qice is minimized when ITSInletTemp and ITSOutletTemp is almost same due to LMTD method.
        // Qice is maximized(=0) when ITSOutletTemp is almost same as FreezTemp(=0).

        Real64 ITSInletTemp_loc = state.dataLoopNodes->Node(this->PltInletNodeNum).Temp;
        Real64 ITSOutletTemp_loc = 0.0;
        switch (state.dataPlnt->PlantLoop(this->plantLoc.loopNum).LoopDemandCalcScheme) {
        case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
            ITSOutletTemp_loc = state.dataLoopNodes->Node(this->PltOutletNodeNum).TempSetPoint;
        } break;
        case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
            ITSOutletTemp_loc = state.dataLoopNodes->Node(this->PltOutletNodeNum).TempSetPointHi;
        } break;
        default: {
            assert(false);
        } break;
        }

        Real64 LogTerm = (ITSInletTemp_loc - FreezTemp) / (ITSOutletTemp_loc - FreezTemp);

        if (LogTerm <= 1) {
            QiceMin = 0.0;
        } else {
            QiceMin = this->UAIceDisCh * (ITSInletTemp_loc - ITSOutletTemp_loc) / std::log(LogTerm);
        }
    }

    void SimpleIceStorageData::CalcUAIce(Real64 const XCurIceFrac_loc, Real64 &UAIceCh_loc, Real64 &UAIceDisCh_loc, Real64 &HLoss_loc)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR
        //       DATE WRITTEN
        //       MODIFIED
        //       RE-ENGINEERED

        // PURPOSE OF THIS SUBROUTINE:

        // METHODOLOGY EMPLOYED:
        // This routine is function of XCurIceFrac, and UA value is based on 1 hour.

        switch (this->ITSType_Num) {
        case ITSType::IceOnCoilInternal: {
            Real64 y = XCurIceFrac_loc;
            UAIceCh_loc = (1.3879 - 7.6333 * y + 26.3423 * pow_2(y) - 47.6084 * pow_3(y) + 41.8498 * pow_4(y) - 14.2948 * pow_5(y)) *
                          this->ITSNomCap / TimeInterval / 10.0; // [W/C]
            y = 1.0 - XCurIceFrac_loc;
            UAIceDisCh_loc = (1.3879 - 7.6333 * y + 26.3423 * pow_2(y) - 47.6084 * pow_3(y) + 41.8498 * pow_4(y) - 14.2948 * pow_5(y)) *
                             this->ITSNomCap / TimeInterval / 10.0; // [W/C]
            HLoss_loc = 0.0;
        } break;
        case ITSType::IceOnCoilExternal: {
            Real64 y = XCurIceFrac_loc;
            UAIceCh_loc = (1.3879 - 7.6333 * y + 26.3423 * pow_2(y) - 47.6084 * pow_3(y) + 41.8498 * pow_4(y) - 14.2948 * pow_5(y)) *
                          this->ITSNomCap / TimeInterval / 10.0; // [W/C]
            y = 1.0 - XCurIceFrac_loc;
            UAIceDisCh_loc = (1.1756 - 5.3689 * y + 17.3602 * pow_2(y) - 30.1077 * pow_3(y) + 25.6387 * pow_4(y) - 8.5102 * pow_5(y)) *
                             this->ITSNomCap / TimeInterval / 10.0; // [W/C]
            HLoss_loc = 0.0;
        } break;
        default:
            break;
        }
    }

    Real64 CalcDetIceStorLMTDstar(Real64 const Tin,  // ice storage unit inlet temperature
                                  Real64 const Tout, // ice storage unit outlet (setpoint) temperature
                                  Real64 const Tfr   // freezing temperature
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   February 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine calculates the log mean temperature difference for
        // the detailed ice storage unit.  The temperature difference is non-
        // dimensionalized using a nominal temperature difference of 10C.
        // This value must be used when obtaining the curve fit coefficients.

        // METHODOLOGY EMPLOYED:
        // Straight-forward calculation where:
        // LMTD* = LMTD/Tnom
        // LMTD = (Tin-Tout)/ln((Tin-Tfr)/(Tout-Tfr))

        Real64 CalcDetIceStorLMTDstar;
        Real64 constexpr Tnom(10.0); // Nominal temperature difference across the ice storage unit [C]

        // First set the temperature differences and avoid problems with the LOG
        // term by setting some reasonable minimums
        Real64 DeltaTio = std::abs(Tin - Tout); // Inlet to outlet temperature difference
        Real64 DeltaTif = std::abs(Tin - Tfr);  // Inlet to freezing temperature difference
        Real64 DeltaTof = std::abs(Tout - Tfr); // Outlet to freezing temperature difference

        if (DeltaTif < DeltaTifMin) {
            DeltaTif = DeltaTifMin;
        }
        if (DeltaTof < DeltaTofMin) {
            DeltaTof = DeltaTofMin;
        }

        CalcDetIceStorLMTDstar = (DeltaTio / std::log(DeltaTif / DeltaTof)) / Tnom;

        return CalcDetIceStorLMTDstar;
    }

    Real64 CalcQstar(EnergyPlusData &state,
                     int const CurveIndex,           // curve index
                     enum CurveVars CurveIndVarType, // independent variable type for ice storage
                     Real64 const FracCharged,       // fraction charged for ice storage unit
                     Real64 const LMTDstar,          // normalized log mean temperature difference across the ice storage unit
                     Real64 const MassFlowstar       // normalized mass flow rate through the ice storage unit
    )
    {

        Real64 CalcQstar;

        if (CurveIndVarType == CurveVars::FracChargedLMTD) {
            CalcQstar = std::abs(Curve::CurveValue(state, CurveIndex, FracCharged, LMTDstar));
        } else if (CurveIndVarType == CurveVars::FracDischargedLMTD) {
            CalcQstar = std::abs(Curve::CurveValue(state, CurveIndex, (1.0 - FracCharged), LMTDstar));
        } else if (CurveIndVarType == CurveVars::LMTDMassFlow) {
            CalcQstar = std::abs(Curve::CurveValue(state, CurveIndex, LMTDstar, MassFlowstar));
        } else if (CurveIndVarType == CurveVars::LMTDFracCharged) {
            CalcQstar = std::abs(Curve::CurveValue(state, CurveIndex, LMTDstar, FracCharged));
        } else { // should never get here as this is checked on input
            CalcQstar = 0.0;
        }

        return CalcQstar;
    }

    Real64 TempSItoIP(Real64 const Temp)
    {
        return (Temp * 9.0 / 5.0) + 32.0;
    }

    Real64 TempIPtoSI(Real64 const Temp)
    {
        return (Temp - 32.0) * 5.0 / 9.0;
    }

    void SimpleIceStorageData::UpdateNode(EnergyPlusData &state, Real64 const myLoad, bool const RunFlag)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR:          Dan Fisher
        //       DATE WRITTEN:    October 1998

        // Update Node Inlet & Outlet MassFlowRat
        PlantUtilities::SafeCopyPlantNode(state, this->PltInletNodeNum, this->PltOutletNodeNum);
        if (myLoad == 0 || !RunFlag) {
            // Update Outlet Conditions so that same as Inlet, so component can be bypassed if necessary
            state.dataLoopNodes->Node(this->PltOutletNodeNum).Temp = state.dataLoopNodes->Node(this->PltInletNodeNum).Temp;
        } else {
            state.dataLoopNodes->Node(this->PltOutletNodeNum).Temp = this->ITSOutletTemp;
        }
    }

    void SimpleIceStorageData::RecordOutput(Real64 const myLoad, bool const RunFlag)
    {
        if (myLoad == 0 || !RunFlag) {
            this->MyLoad = myLoad;
            this->ITSCoolingRate_rep = 0.0;
            this->ITSCoolingEnergy_rep = 0.0;
            this->ITSChargingRate = 0.0;
            this->ITSChargingEnergy = 0.0;
            this->ITSmdot = 0.0;

        } else {
            this->MyLoad = myLoad;
            if (this->ITSCoolingRate > 0.0) {
                this->ITSCoolingRate_rep = this->ITSCoolingRate;
                this->ITSCoolingEnergy_rep = this->ITSCoolingEnergy;
                this->ITSChargingRate = 0.0;
                this->ITSChargingEnergy = 0.0;
            } else {
                this->ITSCoolingRate_rep = 0.0;
                this->ITSCoolingEnergy_rep = 0.0;
                this->ITSChargingRate = -this->ITSCoolingRate;
                this->ITSChargingEnergy = -this->ITSCoolingEnergy;
            }
            this->ITSmdot = this->ITSMassFlowRate;
        }
    }

    void UpdateIceFractions(EnergyPlusData const &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Mike Witte
        //       DATE WRITTEN   September 2005
        //       MODIFIED       Rick Strand (Feb 2006, for detailed ice storage model)
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Update all ice fractions at end of system time step.

        // METHODOLOGY EMPLOYED:
        // This is called from HVACManager once we have actually stepped forward
        // a system time step.

        for (auto &thisITS : state.dataIceThermalStorage->SimpleIceStorage) {
            thisITS.IceFracRemain += thisITS.Urate * state.dataHVACGlobal->TimeStepSys;
            if (thisITS.IceFracRemain <= 0.001) {
                thisITS.IceFracRemain = 0.0;
            }
            if (thisITS.IceFracRemain > 1.0) {
                thisITS.IceFracRemain = 1.0;
            }
        }

        for (auto &thisITS : state.dataIceThermalStorage->DetailedIceStorage) {
            thisITS.IceFracRemaining += thisITS.IceFracChange - (thisITS.TankLossCoeff * state.dataHVACGlobal->TimeStepSys);
            if (thisITS.IceFracRemaining < 0.001) {
                thisITS.IceFracRemaining = 0.0;
            }
            if (thisITS.IceFracRemaining > 1.000) {
                thisITS.IceFracRemaining = 1.0;
            }
            // Reset the ice on the coil to zero for inside melt whenever discharging takes place.
            // This assumes that any remaining ice floats away from the coil and resettles perfectly.
            // While this is not exactly what happens and it is possible theoretically to have multiple
            // freeze thaw cycles that are not complete, this is the best we can do.
            if (thisITS.ThawProcessIndex == DetIce::InsideMelt) {
                if (thisITS.IceFracChange < 0.0) {
                    thisITS.IceFracOnCoil = 0.0;
                } else {
                    // Assume loss term does not impact ice on the coil but what is remaining
                    thisITS.IceFracOnCoil += thisITS.IceFracChange;
                    // If the ice remaining has run out because of tank losses, reset ice fraction on coil so that it keeps track of losses
                    if (thisITS.IceFracOnCoil > thisITS.IceFracRemaining) {
                        thisITS.IceFracOnCoil = thisITS.IceFracRemaining;
                    }
                }
            } else { // Outside melt system so IceFracOnCoil is always the same as IceFracRemaining (needs to be done for reporting only)
                thisITS.IceFracOnCoil = thisITS.IceFracRemaining;
            }
        }
    }

    void DetailedIceStorageData::UpdateDetailedIceStorage(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   February 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine takes the necessary information from the local data
        // structure and moves it back to the loop node data structure.

        // METHODOLOGY EMPLOYED:
        // Not much mystery here--just move the data to the appropriate place
        // for the detailed ice storage system in question.

        // Set the temperature and flow rate for the component outlet node
        int InNodeNum = this->PlantInNodeNum;
        int OutNodeNum = this->PlantOutNodeNum;

        PlantUtilities::SafeCopyPlantNode(state, InNodeNum, OutNodeNum);

        state.dataLoopNodes->Node(OutNodeNum).Temp = this->OutletTemp;
    }

    void DetailedIceStorageData::ReportDetailedIceStorage(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   February 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine reports all of the output necessary for the model.

        // METHODOLOGY EMPLOYED:
        // Just take what has already been calculated or calculate the appropriate
        // output value based on simulation data.

        Real64 constexpr LowLoadLimit(0.1); // Load below which device can be assumed off [W]

        if (this->CompLoad < LowLoadLimit) { // No load condition

            this->IceFracChange = 0.0;
            this->DischargingRate = 0.0;
            this->DischargingEnergy = 0.0;
            this->ChargingRate = 0.0;
            this->ChargingEnergy = 0.0;
            this->ParasiticElecRate = 0.0;
            this->ParasiticElecEnergy = 0.0;

        } else { // There is a load, determine whether we are charging or discharging based on inlet and outlet temperature

            if (this->InletTemp < this->OutletTemp) { // Charging Mode

                this->ChargingRate = this->CompLoad;
                this->ChargingEnergy = this->CompLoad * (state.dataHVACGlobal->TimeStepSysSec);
                this->IceFracChange = this->CompLoad * state.dataHVACGlobal->TimeStepSys / this->NomCapacity;
                this->DischargingRate = 0.0;
                this->DischargingEnergy = 0.0;
                this->ParasiticElecRate = this->ChargeParaElecLoad * this->CompLoad;
                this->ParasiticElecEnergy = this->ChargeParaElecLoad * this->ChargingEnergy;

            } else { // (DetailedIceStorage(IceNum)%InletTemp < DetailedIceStorage(IceNum)%OutletTemp) Discharging Mode

                this->DischargingRate = this->CompLoad;
                this->DischargingEnergy = this->CompLoad * (state.dataHVACGlobal->TimeStepSysSec);
                this->IceFracChange = -this->CompLoad * state.dataHVACGlobal->TimeStepSys / this->NomCapacity;
                this->ChargingRate = 0.0;
                this->ChargingEnergy = 0.0;
                this->ParasiticElecRate = this->DischargeParaElecLoad * this->CompLoad;
                this->ParasiticElecEnergy = this->DischargeParaElecLoad * this->ChargingEnergy;
            }
        }
    }

} // namespace IceThermalStorage

} // namespace EnergyPlus
