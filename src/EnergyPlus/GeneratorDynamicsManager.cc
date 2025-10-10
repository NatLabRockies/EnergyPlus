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
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataGenerators.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/GeneratorDynamicsManager.hh>
#include <EnergyPlus/MicroCHPElectricGenerator.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/ScheduleManager.hh>

namespace EnergyPlus {

namespace GeneratorDynamicsManager {

    //_______________________________________________
    // Utility modules used by other generators.
    //
    // GeneratorDynamicsManager
    //   reused among some generators to on/off state, transient limits, control implications etc.

    // Module containing the routines dealing with the management of dynamic constraints on Generator response

    // MODULE INFORMATION:
    //       AUTHOR        B. Griffith
    //       DATE WRITTEN   July 2006

    // PURPOSE OF THIS MODULE:
    // collect routines for managing generator states
    // reused by different generator models
    //  determine response that generator is capable of providing
    //  given load request data
    //   models requiring calculations across timesteps

    void SetupGeneratorControlStateManager(EnergyPlusData &state, int const GenNum) // index of generator to setup
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   July 2006

        // PURPOSE OF THIS SUBROUTINE:
        // sets up data structures

        // METHODOLOGY EMPLOYED:
        // like a get input routine but feeds from
        //  parent objects, could have its own input object someday

        // get the number of generators that might use this module
        int const NumGensWDynamics = state.dataCHPElectGen->NumMicroCHPs; // TODO  + NumFuelCellCGenerators

        if (!allocated(state.dataGenerator->GeneratorDynamics)) {
            state.dataGenerator->GeneratorDynamics.allocate(NumGensWDynamics);
        }

        // first populate with Micro CHP data
        auto &thisGen = state.dataGenerator->GeneratorDynamics(GenNum);
        auto &thisMicroCHP = state.dataCHPElectGen->MicroCHP(GenNum);
        thisGen.Name = thisMicroCHP.Name;
        thisGen.PelMin = thisMicroCHP.A42Model.MinElecPower;
        thisGen.PelMax = thisMicroCHP.A42Model.MaxElecPower;
        thisGen.UpTranLimit = thisMicroCHP.A42Model.DeltaPelMax;
        thisGen.DownTranLimit = thisMicroCHP.A42Model.DeltaPelMax;
        thisGen.UpTranLimitFuel = thisMicroCHP.A42Model.DeltaFuelMdotMax;
        thisGen.DownTranLimitFuel = thisMicroCHP.A42Model.DeltaFuelMdotMax;
        thisGen.WarmUpByTimeDelay = thisMicroCHP.A42Model.WarmUpByTimeDelay;
        thisGen.WarmUpByEngineTemp = thisMicroCHP.A42Model.WarmUpByEngineTemp;
        thisGen.MandatoryFullCoolDown = thisMicroCHP.A42Model.MandatoryFullCoolDown;
        thisGen.WarmRestartOkay = thisMicroCHP.A42Model.WarmRestartOkay;
        thisGen.WarmUpDelay = thisMicroCHP.A42Model.WarmUpDelay;
        thisGen.CoolDownDelay = thisMicroCHP.A42Model.CoolDownDelay / Constant::rSecsInHour; // seconds to hours
        thisGen.PcoolDown = thisMicroCHP.A42Model.PcoolDown;
        thisGen.Pstandby = thisMicroCHP.A42Model.Pstandby;
        thisGen.MCeng = thisMicroCHP.A42Model.MCeng;
        thisGen.MCcw = thisMicroCHP.A42Model.MCcw;
        thisGen.kf = thisMicroCHP.A42Model.kf;
        thisGen.TnomEngOp = thisMicroCHP.A42Model.TnomEngOp;
        thisGen.kp = thisMicroCHP.A42Model.kp;
        thisGen.availSched = thisMicroCHP.availSched;
        thisGen.StartUpTimeDelay = thisMicroCHP.A42Model.WarmUpDelay / Constant::rSecsInHour; // seconds to hours

        thisGen.ElectEffNom = thisMicroCHP.A42Model.ElecEff;
        thisGen.ThermEffNom = thisMicroCHP.A42Model.ThermEff;
        thisGen.QdotHXMax = thisMicroCHP.A42Model.ThermEff * thisMicroCHP.A42Model.MaxElecPower / thisMicroCHP.A42Model.ElecEff;
        thisGen.QdotHXMin = thisMicroCHP.A42Model.ThermEff * thisMicroCHP.A42Model.MinElecPower / thisMicroCHP.A42Model.ElecEff;
        thisGen.QdotHXOpt = thisGen.QdotHXMax;
        thisMicroCHP.DynamicsControlID = GenNum;
    }

    void ManageGeneratorControlState(EnergyPlusData &state,
                                     int const GeneratorNum,                       // Generator number
                                     bool const RunFlagElectCenter,                // TRUE when Generator operating per electric load center request
                                     bool const RunFlagPlant,                      // TRUE when generator operating per Plant request (always false)
                                     Real64 const ElecLoadRequest,                 // Generator Electrical power demand
                                     Real64 const ThermalLoadRequest,              // cogenerator Thermal power demand
                                     Real64 &ElecLoadProvided,                     // power allowed
                                     DataGenerators::OperatingMode &OperatingMode, // operating mode
                                     Real64 &PLRforSubtimestepStartUp,             // part load ratio for switch to normal from start up
                                     Real64 &PLRforSubtimestepShutDown             // part load ratio for switch from cool down to other
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B Griffith
        //       DATE WRITTEN   February-March 2007  (replaced July 2006 attempt)
        //       MODIFIED       Dec 2009, check and constrain with flow available from plant

        // PURPOSE OF THIS SUBROUTINE:
        // provide a service to other generators to make decisions, mostly temporal, or cross-timestep issues
        //  used to model internal controlling issues within an individual generator model
        //  This subroutine determines the current operating mode and returns the allowed power and
        // and part load ratio for certain sub-time step switching e.g. in and out of normal mode or cool down mode

        // METHODOLOGY EMPLOYED:
        // model controls-related issues, rules based algorithm
        // Control decision results include:
        //     -- electrical load allowed/resulting/provided
        //     -- new operating mode
        //     -- part load this timestep for shift to normal mode occurring midway in timestep
        //     -- part load this timestep for shift out of cool down mode

        // Input data used to make control decisions include:
        //     -- Electrical load request
        //     -- Thermal Load request
        //     -- RunFlagElectricCenter
        //     -- RunFlagPlant
        //     -- previous timestep operating mode
        //     -- previous timestep Power generated
        //     -- availability schedule (off if not available)
        //     -- Generator control parameter constants including
        //           ** Start Up Time Delay  (in hours)
        //           ** Cool-down time delay (in hours)
        //     -- Expected Plant flow rate
        //     -- minimum cooling water flow rate

        // Algorithm summary
        //   1.  examine calling run flags and refine electric load request to account for
        //       thermal load requests (not yet ready for prime time)
        //   2.  Determine states of various control inputs that change during simulation
        //   3.  enter case statement based on previous operating mode.
        //       --  decide on current operating mode
        //       --  calculate part loads

        //   4.  based on current operating mode determine allowed/provided electrical load
        //        a. set allowed elec load by mode
        //        b. set allowed elec load by constraints on rate of change
        //        c. set allowed elec load by min and max

        //   5.  Calculated part load ratios for special cases.
        // REFERENCES:
        // controls specifications in Annex 42 model specs.
        // Using/Aliasing
        Real64 const SysTimeElapsed = state.dataHVACGlobal->SysTimeElapsed;
        Real64 const TimeStepSys = state.dataHVACGlobal->TimeStepSys;
        Real64 const TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool RunFlag; // true if generator supposed to run
        DataGenerators::OperatingMode newOpMode(DataGenerators::OperatingMode::Invalid);

        // inits
        PLRforSubtimestepStartUp = 1.0;
        PLRforSubtimestepShutDown = 0.0;
        bool PLRStartUp = false; // true if subtimestep issue involving startup
        bool PLRShutDown = false;
        state.dataGenerator->InternalFlowControl = false;

        // get index for this generator in dynamics control structure
        int DynaCntrlNum = state.dataCHPElectGen->MicroCHP(GeneratorNum).DynamicsControlID;
        // OutletCWnode = MicroCHPElectricGenerator::MicroCHP(GeneratorNum)%PlantOutletNodeID
        state.dataGenerator->InletCWnode = state.dataCHPElectGen->MicroCHP(GeneratorNum).PlantInletNodeID;
        state.dataGenerator->TcwIn = state.dataLoopNodes->Node(state.dataCHPElectGen->MicroCHP(GeneratorNum).PlantInletNodeID).Temp;
        if (state.dataCHPElectGen->MicroCHP(GeneratorNum).A42Model.InternalFlowControl) {
            state.dataGenerator->InternalFlowControl = true;
        }
        state.dataGenerator->LimitMinMdotcw = state.dataCHPElectGen->MicroCHP(GeneratorNum).A42Model.MinWaterMdot;

        auto &thisGen = state.dataGenerator->GeneratorDynamics(DynaCntrlNum);
        Real64 PelInput = ElecLoadRequest; // holds initial value of IN var
        Real64 ElectLoadForThermalRequest = 0.0;
        if ((ThermalLoadRequest > 0.0) && RunFlagPlant) { // deal with possible thermal load following
            // Modify electric load request based on thermal load following signal using nominal efficiencies
            ElectLoadForThermalRequest = thisGen.ThermEffNom * ThermalLoadRequest / thisGen.ElectEffNom;
            PelInput = max(PelInput, ElectLoadForThermalRequest);
        }

        if ((RunFlagElectCenter) || (RunFlagPlant)) {
            RunFlag = true;
        } else {
            RunFlag = false;
        }

        // check availability schedule
        Real64 SchedVal = thisGen.availSched->getCurrentVal();
        Real64 Pel = PelInput;

        // get data to check if sufficient flow available from Plant
        if (state.dataGenerator->InternalFlowControl && (SchedVal > 0.0)) {
            state.dataGenerator->TrialMdotcw = FuncDetermineCWMdotForInternalFlowControl(state, GeneratorNum, Pel, state.dataGenerator->TcwIn);
        } else {
            state.dataGenerator->TrialMdotcw = state.dataLoopNodes->Node(state.dataGenerator->InletCWnode).MassFlowRate;
        }

        // determine current operating mode.
        switch (thisGen.LastOpMode) {
        case DataGenerators::OperatingMode::Off:
        case DataGenerators::OperatingMode::Standby: {
            // possible future states {Off, Standby, WarmUp,Normal }
            if (SchedVal == 0.0) {
                newOpMode = DataGenerators::OperatingMode::Off;

            } else if (((SchedVal != 0.0) && (!RunFlag)) || (state.dataGenerator->TrialMdotcw < state.dataGenerator->LimitMinMdotcw)) {
                newOpMode = DataGenerators::OperatingMode::Standby;
            } else if ((SchedVal != 0.0) && (RunFlag)) {

                if (thisGen.WarmUpByTimeDelay) {

                    if (thisGen.StartUpTimeDelay == 0.0) {
                        newOpMode = DataGenerators::OperatingMode::Normal;

                        // is startUp time delay longer than timestep?
                    } else if (thisGen.StartUpTimeDelay >= TimeStepSys) {
                        newOpMode = DataGenerators::OperatingMode::WarmUp;
                        // generator just started so set start time
                        thisGen.FractionalDayofLastStartUp =
                            double(state.dataGlobal->DayOfSim) +
                            (int(state.dataGlobal->CurrentTime) +
                             (SysTimeElapsed + (state.dataGlobal->CurrentTime - int(state.dataGlobal->CurrentTime) - TimeStepSys))) /
                                Constant::rHoursInDay;

                    } else { // warm up period is less than a single system time step
                        newOpMode = DataGenerators::OperatingMode::Normal;
                        PLRStartUp = true;
                        PLRforSubtimestepStartUp = (TimeStepSys - thisGen.StartUpTimeDelay) / TimeStepSys;
                    }
                }
                if (thisGen.WarmUpByEngineTemp) {
                    if (state.dataCHPElectGen->MicroCHP(GeneratorNum).A42Model.Teng >= thisGen.TnomEngOp) {
                        auto const &thisMicroCHP = state.dataCHPElectGen->MicroCHP(GeneratorNum);
                        newOpMode = DataGenerators::OperatingMode::Normal;
                        // assume linear interpolation for PLR
                        PLRStartUp = true;
                        if ((thisMicroCHP.A42Model.Teng - thisMicroCHP.A42Model.TengLast) > 0.0) {
                            // protect divide by zero or neg
                            PLRforSubtimestepStartUp =
                                (thisMicroCHP.A42Model.Teng - thisGen.TnomEngOp) / (thisMicroCHP.A42Model.Teng - thisMicroCHP.A42Model.TengLast);
                        } else {
                            PLRforSubtimestepStartUp = 1.0;
                        }
                    } else {
                        newOpMode = DataGenerators::OperatingMode::WarmUp;
                    }
                }
            }

        } break;
        case DataGenerators::OperatingMode::WarmUp: {
            // possible Future states {OFF, WarmUp, Normal, CoolDown }
            // check availability manager
            if (SchedVal == 0.0) {
                // to off unless cool down time period is needed
                if (thisGen.CoolDownDelay == 0.0) {
                    newOpMode = DataGenerators::OperatingMode::Off;
                } else {
                    if (thisGen.CoolDownDelay > TimeStepSys) {
                        newOpMode = DataGenerators::OperatingMode::CoolDown;
                        // need to reset time of last shut down here
                        thisGen.FractionalDayofLastShutDown =
                            double(state.dataGlobal->DayOfSim) +
                            (int(state.dataGlobal->CurrentTime) +
                             (SysTimeElapsed + (state.dataGlobal->CurrentTime - int(state.dataGlobal->CurrentTime)))) /
                                Constant::rHoursInDay;
                    } else {
                        newOpMode = DataGenerators::OperatingMode::Off;
                    }
                }
            } else if (((SchedVal != 0.0) && (!RunFlag)) || (state.dataGenerator->TrialMdotcw < state.dataGenerator->LimitMinMdotcw)) {
                // to standby unless cool down time period is needed
                if (thisGen.CoolDownDelay == 0.0) {
                    newOpMode = DataGenerators::OperatingMode::Standby;
                } else {
                    if (thisGen.CoolDownDelay > TimeStepSys) {
                        newOpMode = DataGenerators::OperatingMode::CoolDown;
                        // need to reset time of last shut down here
                        thisGen.FractionalDayofLastShutDown =
                            double(state.dataGlobal->DayOfSim) +
                            (int(state.dataGlobal->CurrentTime) +
                             (SysTimeElapsed + (state.dataGlobal->CurrentTime - int(state.dataGlobal->CurrentTime)))) /
                                Constant::rHoursInDay;

                    } else {
                        newOpMode = DataGenerators::OperatingMode::Standby;
                        // assuming no PLR situation unless engine made to normal operation.
                    }
                }
            } else if ((SchedVal != 0.0) && (RunFlag)) {
                // either warm up or normal
                // check if warm up completed, depends on type of warm up control time delay or reach nominal temperature
                if (thisGen.WarmUpByTimeDelay) {
                    // compare current time to when warm up is over
                    // calculate time for end of warmup period
                    Real64 CurrentFractionalDay = double(state.dataGlobal->DayOfSim) +
                                                  (int(state.dataGlobal->CurrentTime) +
                                                   (SysTimeElapsed + (state.dataGlobal->CurrentTime - int(state.dataGlobal->CurrentTime)))) /
                                                      Constant::rHoursInDay;
                    Real64 EndingFractionalDay = thisGen.FractionalDayofLastStartUp + thisGen.StartUpTimeDelay / Constant::rHoursInDay;
                    if ((std::abs(CurrentFractionalDay - EndingFractionalDay) < 0.000001) || (CurrentFractionalDay > EndingFractionalDay)) {
                        newOpMode = DataGenerators::OperatingMode::Normal;
                        PLRStartUp = true;
                        Real64 LastSystemTimeStepFractionalDay = CurrentFractionalDay - (TimeStepSys / Constant::rHoursInDay);
                        PLRforSubtimestepStartUp =
                            ((CurrentFractionalDay - EndingFractionalDay) / (CurrentFractionalDay - LastSystemTimeStepFractionalDay));
                    } else {
                        newOpMode = DataGenerators::OperatingMode::WarmUp;
                    }

                } else if (thisGen.WarmUpByEngineTemp) {
                    // only change to normal if this is result from completed timestep, not just an interation
                    if (state.dataCHPElectGen->MicroCHP(GeneratorNum).A42Model.TengLast >= thisGen.TnomEngOp) {
                        auto const &thisMicroCHP = state.dataCHPElectGen->MicroCHP(GeneratorNum);
                        newOpMode = DataGenerators::OperatingMode::Normal;
                        // assume linear interpolation for PLR
                        PLRStartUp = true;
                        if ((thisMicroCHP.A42Model.Teng - thisMicroCHP.A42Model.TengLast) > 0.0) {
                            // protect divide by zero or neg
                            PLRforSubtimestepStartUp =
                                (thisMicroCHP.A42Model.Teng - thisGen.TnomEngOp) / (thisMicroCHP.A42Model.Teng - thisMicroCHP.A42Model.TengLast);
                        } else {
                            PLRforSubtimestepStartUp = 1.0;
                        }
                    } else {
                        newOpMode = DataGenerators::OperatingMode::WarmUp;
                    }
                } else {
                    // shouldn't come here
                    // Write(*,*) 'problem with warm up type of control logical flags'
                }
            }
        } break;
        case DataGenerators::OperatingMode::Normal: {
            // possible Future states {CoolDown, standby, off}
            if (((SchedVal == 0.0) || (!RunFlag)) || (state.dataGenerator->TrialMdotcw < state.dataGenerator->LimitMinMdotcw)) {
                // is cool down time delay longer than timestep?
                if (thisGen.CoolDownDelay == 0.0) {
                    if (SchedVal != 0.0) {
                        newOpMode = DataGenerators::OperatingMode::Standby;
                    } else {
                        newOpMode = DataGenerators::OperatingMode::Off;
                    }
                } else if (thisGen.CoolDownDelay >= TimeStepSys) {
                    newOpMode = DataGenerators::OperatingMode::CoolDown;
                    // also, generator just shut down so record shut down time
                    thisGen.FractionalDayofLastShutDown = double(state.dataGlobal->DayOfSim) +
                                                          (int(state.dataGlobal->CurrentTime) +
                                                           (SysTimeElapsed + (state.dataGlobal->CurrentTime - int(state.dataGlobal->CurrentTime)))) /
                                                              Constant::rHoursInDay;
                } else { // cool down period is less than a single system time step
                    if (SchedVal != 0.0) {
                        newOpMode = DataGenerators::OperatingMode::Standby;
                    } else {
                        newOpMode = DataGenerators::OperatingMode::Off;
                    }
                    PLRShutDown = true;
                    PLRforSubtimestepShutDown = (thisGen.CoolDownDelay) / TimeStepSys;

                    // also, generator just shut down so record shut down time
                    thisGen.FractionalDayofLastShutDown = double(state.dataGlobal->DayOfSim) +
                                                          (int(state.dataGlobal->CurrentTime) +
                                                           (SysTimeElapsed + (state.dataGlobal->CurrentTime - int(state.dataGlobal->CurrentTime)))) /
                                                              Constant::rHoursInDay;
                }
            } else {

                newOpMode = DataGenerators::OperatingMode::Normal;
            }
        } break;
        case DataGenerators::OperatingMode::CoolDown: {
            // possible Future States {Standby, OFF, WarmUp, Normal}

            if (SchedVal == 0.0) { // no longer available.
                // probably goes to off but could be stuck in cool down for awhile
                if (thisGen.CoolDownDelay > 0.0) {
                    // calculate time for end of cool down period
                    Real64 CurrentFractionalDay = double(state.dataGlobal->DayOfSim) +
                                                  (int(state.dataGlobal->CurrentTime) +
                                                   (SysTimeElapsed + (state.dataGlobal->CurrentTime - int(state.dataGlobal->CurrentTime)))) /
                                                      Constant::rHoursInDay;
                    Real64 EndingFractionalDay =
                        thisGen.FractionalDayofLastShutDown + thisGen.CoolDownDelay / Constant::rHoursInDay - (TimeStepSys / Constant::rHoursInDay);
                    if ((std::abs(CurrentFractionalDay - EndingFractionalDay) < 0.000001) ||
                        (CurrentFractionalDay > EndingFractionalDay)) { // CurrentFractionalDay == EndingFractionalDay
                        newOpMode = DataGenerators::OperatingMode::Off;
                        PLRShutDown = true;
                        Real64 LastSystemTimeStepFractionalDay = CurrentFractionalDay - (TimeStepSys / Constant::rHoursInDay);
                        PLRforSubtimestepShutDown = (EndingFractionalDay - LastSystemTimeStepFractionalDay) * Constant::rHoursInDay / TimeStepSys;
                    } else { // CurrentFractionalDay > EndingFractionalDay
                        newOpMode = DataGenerators::OperatingMode::CoolDown;
                    }
                } else {
                    newOpMode = DataGenerators::OperatingMode::Off;
                }
            } else if (((SchedVal != 0.0) && (!RunFlag)) || (state.dataGenerator->TrialMdotcw < state.dataGenerator->LimitMinMdotcw)) {
                // probably goes to standby but could be stuck in cool down for awhile
                if (thisGen.CoolDownDelay > 0.0) {
                    // calculate time for end of cool down period
                    Real64 CurrentFractionalDay = double(state.dataGlobal->DayOfSim) +
                                                  (int(state.dataGlobal->CurrentTime) +
                                                   (SysTimeElapsed + (state.dataGlobal->CurrentTime - int(state.dataGlobal->CurrentTime)))) /
                                                      Constant::rHoursInDay;
                    Real64 EndingFractionalDay =
                        thisGen.FractionalDayofLastShutDown + thisGen.CoolDownDelay / Constant::rHoursInDay - (TimeStepSys / Constant::rHoursInDay);
                    if ((std::abs(CurrentFractionalDay - EndingFractionalDay) < 0.000001) ||
                        (CurrentFractionalDay > EndingFractionalDay)) { // CurrentFractionalDay == EndingFractionalDay
                        newOpMode = DataGenerators::OperatingMode::Standby;
                        PLRShutDown = true;
                        Real64 LastSystemTimeStepFractionalDay = CurrentFractionalDay - (TimeStepSys / Constant::rHoursInDay);
                        PLRforSubtimestepShutDown = (EndingFractionalDay - LastSystemTimeStepFractionalDay) * Constant::rHoursInDay / TimeStepSys;
                    } else { // CurrentFractionalDay < EndingFractionalDay
                        newOpMode = DataGenerators::OperatingMode::CoolDown;
                    }
                } else {
                    newOpMode = DataGenerators::OperatingMode::Standby;
                }
            } else if ((SchedVal != 0.0) && (RunFlag)) {
                // was in cool down mode but is now being asked to restart
                // probably goes to warm up but could be stuck in cool down or jump to normal
                if (thisGen.MandatoryFullCoolDown) {
                    // is cool down done or not?
                    if (thisGen.CoolDownDelay > 0.0) {
                        // calculate time for end of cool down period
                        Real64 CurrentFractionalDay = double(state.dataGlobal->DayOfSim) +
                                                      (int(state.dataGlobal->CurrentTime) +
                                                       (SysTimeElapsed + (state.dataGlobal->CurrentTime - int(state.dataGlobal->CurrentTime)))) /
                                                          Constant::rHoursInDay;
                        Real64 EndingFractionalDay = thisGen.FractionalDayofLastShutDown + thisGen.CoolDownDelay / Constant::rHoursInDay -
                                                     (TimeStepSys / Constant::rHoursInDay);
                        if ((std::abs(CurrentFractionalDay - EndingFractionalDay) < 0.000001) ||
                            (CurrentFractionalDay < EndingFractionalDay)) { // CurrentFractionalDay == EndingFractionalDay

                            newOpMode = DataGenerators::OperatingMode::CoolDown;
                        } else { // CurrentFractionalDay > EndingFractionalDay
                            // could go to warm up or normal now
                            PLRShutDown = true;
                            Real64 LastSystemTimeStepFractionalDay = CurrentFractionalDay - (TimeStepSys / Constant::rHoursInDay);
                            PLRforSubtimestepShutDown = (EndingFractionalDay - LastSystemTimeStepFractionalDay) * Constant::rHoursInDay / TimeStepSys;
                            if (thisGen.StartUpTimeDelay == 0.0) {
                                newOpMode = DataGenerators::OperatingMode::Normal;
                                // possible PLR on start up.
                                PLRStartUp = true;
                                PLRforSubtimestepStartUp =
                                    ((CurrentFractionalDay - EndingFractionalDay) / (CurrentFractionalDay - LastSystemTimeStepFractionalDay));

                            } else if (thisGen.StartUpTimeDelay > 0.0) {
                                // is remaining time enough?
                                if ((CurrentFractionalDay - EndingFractionalDay) > thisGen.StartUpTimeDelay) {
                                    newOpMode = DataGenerators::OperatingMode::Normal;
                                    // possible PLR on start up.
                                    PLRStartUp = true;
                                    PLRforSubtimestepStartUp =
                                        ((CurrentFractionalDay - EndingFractionalDay) / (CurrentFractionalDay - LastSystemTimeStepFractionalDay));
                                } else {
                                    newOpMode = DataGenerators::OperatingMode::WarmUp;
                                    // generator just started so set start time
                                    thisGen.FractionalDayofLastStartUp =
                                        double(state.dataGlobal->DayOfSim) +
                                        (int(state.dataGlobal->CurrentTime) +
                                         (SysTimeElapsed + (state.dataGlobal->CurrentTime - int(state.dataGlobal->CurrentTime) - TimeStepSys))) /
                                            Constant::rHoursInDay;
                                }
                            }
                        }
                    } else {

                        newOpMode = DataGenerators::OperatingMode::Standby;
                    }
                } else { // not mandatory cool down
                    // likely to go into warm up but if no warm up then back to normal
                    if (thisGen.WarmUpByTimeDelay) {
                        if (thisGen.StartUpTimeDelay == 0.0) {
                            newOpMode = DataGenerators::OperatingMode::Normal;

                        } else if (thisGen.StartUpTimeDelay > 0.0) {
                            Real64 CurrentFractionalDay = double(state.dataGlobal->DayOfSim) +
                                                          (int(state.dataGlobal->CurrentTime) +
                                                           (SysTimeElapsed + (state.dataGlobal->CurrentTime - int(state.dataGlobal->CurrentTime)))) /
                                                              Constant::rHoursInDay;
                            Real64 EndingFractionalDay = thisGen.FractionalDayofLastShutDown + thisGen.CoolDownDelay / Constant::rHoursInDay;
                            if ((std::abs(CurrentFractionalDay - EndingFractionalDay) < 0.000001) ||
                                (CurrentFractionalDay > EndingFractionalDay)) { // CurrentFractionalDay == EndingFractionalDay
                                newOpMode = DataGenerators::OperatingMode::Normal;
                                // possible PLR on start up.
                                PLRStartUp = true;
                                Real64 LastSystemTimeStepFractionalDay = CurrentFractionalDay - (TimeStepSys / Constant::rHoursInDay);
                                PLRforSubtimestepStartUp =
                                    ((CurrentFractionalDay - EndingFractionalDay) / (CurrentFractionalDay - LastSystemTimeStepFractionalDay));
                            } else {
                                newOpMode = DataGenerators::OperatingMode::WarmUp;
                                // set start up time
                                // generator just started so set start time
                                thisGen.FractionalDayofLastStartUp =
                                    double(state.dataGlobal->DayOfSim) +
                                    (int(state.dataGlobal->CurrentTime) +
                                     (SysTimeElapsed + (state.dataGlobal->CurrentTime - int(state.dataGlobal->CurrentTime) - TimeStepSys))) /
                                        Constant::rHoursInDay;
                            }
                        }
                    }
                }
            }
        } break;
        default:
            break;
        } // previous case

        if (PLRforSubtimestepStartUp < 0.0) {
            PLRforSubtimestepStartUp = 0.0;
        }
        if (PLRforSubtimestepStartUp > 1.0) {
            PLRforSubtimestepStartUp = 1.0;
        }

        if (PLRforSubtimestepShutDown < 0.0) {
            PLRforSubtimestepShutDown = 0.0;
        }
        if (PLRforSubtimestepShutDown > 1.0) {
            PLRforSubtimestepShutDown = 1.0;
        }

        if (newOpMode == DataGenerators::OperatingMode::WarmUp) {
            Pel = PelInput * PLRforSubtimestepStartUp;
        }

        if (newOpMode == DataGenerators::OperatingMode::Normal) {
            // correct if switched to normal at sub timestep
            Pel *= PLRforSubtimestepStartUp;
            // unit may have constraints from transient limits or operating ranges.
            if (Pel > thisGen.PelLastTimeStep) { // powering up
                Real64 MaxPel = thisGen.PelLastTimeStep + thisGen.UpTranLimit * TimeStepSysSec;
                if (MaxPel < Pel) {
                    Pel = MaxPel;
                }
            } else if (Pel < thisGen.PelLastTimeStep) { // powering down
                Real64 MinPel = thisGen.PelLastTimeStep - thisGen.DownTranLimit * TimeStepSysSec;
                if (Pel < MinPel) {
                    Pel = MinPel;
                }
            }
        }

        if (newOpMode == DataGenerators::OperatingMode::CoolDown) {
            Pel = 0.0; // assumes no power generated during shut down
        }

        if (newOpMode == DataGenerators::OperatingMode::Off) {
            Pel = 0.0; // assumes no power generated during OFF mode
        }

        if (newOpMode == DataGenerators::OperatingMode::Standby) {
            Pel = 0.0; // assumes no power generated during standby mode
        }

        // Control step 3: adjust for max and min limits on Pel

        if (Pel < thisGen.PelMin) {
            Pel = thisGen.PelMin;
        }
        if (Pel > thisGen.PelMax) {
            Pel = thisGen.PelMax;
        }

        auto &thisMicroCHP = state.dataCHPElectGen->MicroCHP(GeneratorNum);
        // now do record keeping for amount of time spent in various operating modes
        // first clear out values
        thisMicroCHP.A42Model.OffModeTime = 0.0;
        thisMicroCHP.A42Model.StandyByModeTime = 0.0;
        thisMicroCHP.A42Model.WarmUpModeTime = 0.0;
        thisMicroCHP.A42Model.NormalModeTime = 0.0;
        thisMicroCHP.A42Model.CoolDownModeTime = 0.0;
        switch (newOpMode) {
        case DataGenerators::OperatingMode::Off: {
            if (PLRforSubtimestepShutDown == 0.0) {
                thisMicroCHP.A42Model.OffModeTime = TimeStepSysSec;
            } else if ((PLRforSubtimestepShutDown > 0.0) && (PLRforSubtimestepShutDown < 1.0)) {
                thisMicroCHP.A42Model.CoolDownModeTime = TimeStepSysSec * (PLRforSubtimestepShutDown);
                thisMicroCHP.A42Model.OffModeTime = TimeStepSysSec * (1.0 - PLRforSubtimestepShutDown);
            } else {
                thisMicroCHP.A42Model.OffModeTime = TimeStepSysSec;
            }
        } break;
        case DataGenerators::OperatingMode::Standby: {
            if (PLRforSubtimestepShutDown == 0.0) {
                thisMicroCHP.A42Model.StandyByModeTime = TimeStepSysSec;
            } else if ((PLRforSubtimestepShutDown > 0.0) && (PLRforSubtimestepShutDown < 1.0)) {
                thisMicroCHP.A42Model.CoolDownModeTime = TimeStepSysSec * (PLRforSubtimestepShutDown);
                thisMicroCHP.A42Model.StandyByModeTime = TimeStepSysSec * (1.0 - PLRforSubtimestepShutDown);
            } else {
                thisMicroCHP.A42Model.StandyByModeTime = TimeStepSysSec;
            }
        } break;
        case DataGenerators::OperatingMode::WarmUp: {
            if (PLRforSubtimestepShutDown == 0.0) {
                thisMicroCHP.A42Model.WarmUpModeTime = TimeStepSysSec;
            } else if ((PLRforSubtimestepShutDown > 0.0) && (PLRforSubtimestepShutDown < 1.0)) {
                thisMicroCHP.A42Model.CoolDownModeTime = TimeStepSysSec * (PLRforSubtimestepShutDown);
                thisMicroCHP.A42Model.WarmUpModeTime = TimeStepSysSec * (1.0 - PLRforSubtimestepShutDown);
            } else {
                thisMicroCHP.A42Model.WarmUpModeTime = TimeStepSysSec;
            }
        } break;
        case DataGenerators::OperatingMode::Normal: {
            if (PLRforSubtimestepStartUp == 0.0) {
                thisMicroCHP.A42Model.WarmUpModeTime = TimeStepSysSec;

            } else if ((PLRforSubtimestepStartUp > 0.0) && (PLRforSubtimestepStartUp < 1.0)) {
                thisMicroCHP.A42Model.WarmUpModeTime = TimeStepSysSec * (1.0 - PLRforSubtimestepStartUp);
                thisMicroCHP.A42Model.NormalModeTime = TimeStepSysSec * (PLRforSubtimestepStartUp);
            } else {
                if (PLRforSubtimestepShutDown == 0.0) {
                    thisMicroCHP.A42Model.NormalModeTime = TimeStepSysSec;
                } else if ((PLRforSubtimestepShutDown > 0.0) && (PLRforSubtimestepShutDown < 1.0)) {
                    thisMicroCHP.A42Model.CoolDownModeTime = TimeStepSysSec * (PLRforSubtimestepShutDown);
                    thisMicroCHP.A42Model.NormalModeTime = TimeStepSysSec * (1.0 - PLRforSubtimestepShutDown);
                } else {
                    thisMicroCHP.A42Model.NormalModeTime = TimeStepSysSec;
                }
            }
        } break;
        case DataGenerators::OperatingMode::CoolDown: {
            thisMicroCHP.A42Model.CoolDownModeTime = TimeStepSysSec;
        } break;
        default:
            break;
        }

        ElecLoadProvided = Pel;

        thisGen.CurrentOpMode = newOpMode;
        OperatingMode = newOpMode;
    }

    void ManageGeneratorFuelFlow(EnergyPlusData &state,
                                 int const GeneratorNum,          // Generator number
                                 Real64 const FuelFlowRequest,    // Generator demand mdot kg/ s
                                 Real64 &FuelFlowProvided,        // allowed after constraints kg/s
                                 bool &ConstrainedIncreasingMdot, // true if request was altered because of fuel rate of change up
                                 bool &ConstrainedDecreasingMdot  // true if request was altered because of fuel rate of change down
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   July 2006

        // PURPOSE OF THIS SUBROUTINE:
        // check if change in fuel flow rate is okay

        // Using/Aliasing
        Real64 const TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

        ConstrainedIncreasingMdot = false;
        ConstrainedDecreasingMdot = false;
        Real64 MdotFuel = FuelFlowRequest;

        // get index from GeneratorNum
        int DynaCntrlNum = state.dataCHPElectGen->MicroCHP(GeneratorNum).DynamicsControlID;
        auto const &thisGen = state.dataGenerator->GeneratorDynamics(DynaCntrlNum);

        if (FuelFlowRequest > thisGen.FuelMdotLastTimestep) { // fuel flow is up
            Real64 MaxMdot = thisGen.FuelMdotLastTimestep + thisGen.UpTranLimitFuel * TimeStepSysSec;
            if (MaxMdot < FuelFlowRequest) {
                MdotFuel = MaxMdot;
                ConstrainedIncreasingMdot = true;
            }

        } else if (FuelFlowRequest < thisGen.FuelMdotLastTimestep) { // fuel flow is down
            Real64 MinMdot = thisGen.FuelMdotLastTimestep - thisGen.DownTranLimitFuel * TimeStepSysSec;
            if (FuelFlowRequest < MinMdot) {
                MdotFuel = MinMdot;
                ConstrainedDecreasingMdot = true;
            }
        } else {
            // do nothing
        }

        FuelFlowProvided = MdotFuel;
    }

    Real64 FuncDetermineCWMdotForInternalFlowControl(EnergyPlusData &state,
                                                     int const GeneratorNum, // ID of generator
                                                     Real64 const Pnetss,    // power net steady state
                                                     Real64 const TcwIn      // temperature of cooling water at inlet
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Dec 2009
        //       RE-ENGINEERED  B. Griffith, Sept 2010, plant upgrade

        // PURPOSE OF THIS FUNCTION:
        // common place to figure flow rates with internal flow control

        // METHODOLOGY EMPLOYED:
        // apply constraints imposed by plant according to flow lock, first HVAC iteration etc.

        // Return value
        Real64 FuncDetermineCWMdotForInternalFlowControl;
        auto const &thisMicroCHP = state.dataCHPElectGen->MicroCHP(GeneratorNum);
        int const InletNode = thisMicroCHP.PlantInletNodeID;
        int const OutletNode = thisMicroCHP.PlantOutletNodeID;

        // first evaluate curve
        Real64 MdotCW = thisMicroCHP.A42Model.WaterFlowCurve->value(state, Pnetss, TcwIn);

        // now apply constraints
        MdotCW = max(0.0, MdotCW);

        // make sure plant can provide, utility call may change flow
        if (thisMicroCHP.CWPlantLoc.loopNum > 0) { // protect early calls
            PlantUtilities::SetComponentFlowRate(state, MdotCW, InletNode, OutletNode, thisMicroCHP.CWPlantLoc);
        }

        FuncDetermineCWMdotForInternalFlowControl = MdotCW;
        return FuncDetermineCWMdotForInternalFlowControl;
    }

} // namespace GeneratorDynamicsManager

} // namespace EnergyPlus
