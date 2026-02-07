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
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/GroundHeatExchangers/Base.hh>
#include <EnergyPlus/GroundHeatExchangers/State.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>

namespace EnergyPlus::GroundHeatExchangers {

void GLHEBase::calcGroundHeatExchanger(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Dan Fisher
    //       DATE WRITTEN:    August, 2000
    //       MODIFIED         Arun Murugappan

    // PURPOSE OF THIS SUBROUTINE:
    // This is the main routine to simulate the operation of vertical
    // closed-loop ground heat exchangers (GLHE).

    // METHODOLOGY EMPLOYED:
    // The borehole and fluid temperatures are calculated from the response to
    // the current heat transfer rate and the response to the history of past
    // applied heat pulses. The response to each pulse is calculated from a non-
    // dimensionalized response function, or G-function, that is specific to the
    // given borehole field arrangement, depth and spacing. The data defining
    // this function is read from input.
    // The heat pulse histories need to be recorded over an extended period (months).
    // To aid computational efficiency past pulses are continuously aggregated into
    // equivalent heat pulses of longer duration, as each pulse becomes less recent.

    // REFERENCES:
    // Eskilson, P. 'Thermal Analysis of Heat Extraction Boreholes' Ph.D. Thesis:
    //   Dept. of Mathematical Physics, University of Lund, Sweden, June 1987.
    // Yavuzturk, C., J.D. Spitler. 1999. 'A Short Time Step Response Factor Model
    //   for Vertical Ground Loop Heat Exchangers.' ASHRAE Transactions. 105(2): 475-485.

    // SUBROUTINE ARGUMENT DEFINITIONS
    constexpr std::string_view RoutineName = "CalcGroundHeatExchanger";

    // LOCAL PARAMETERS
    Real64 fluidAveTemp;
    Real64 tmpQnSubHourly; // current Qn sub-hourly value
    Real64 sumTotal(0.0);  // sum of all the Qn (load) blocks

    // Calculate G-Functions
    if (this->firstTime) {
        if (!gFunctionsExist) {
            calcGFunctions(state);
            gFunctionsExist = true;
        }
        this->firstTime = false;
    }

    this->inletTemp = state.dataLoopNodes->Node(this->inletNodeNum).Temp;

    Real64 cpFluid = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, this->inletTemp, RoutineName);

    Real64 kGroundFactor = 2.0 * Constant::Pi * this->soil.k;

    // Get time constants
    getAnnualTimeConstant();

    if (triggerDesignDayReset && state.dataGlobal->WarmupFlag) {
        this->updateCurSimTime = true;
    }
    if (state.dataGlobal->DayOfSim == 1 && updateCurSimTime) {
        this->currentSimTime = 0.0;
        this->prevTimeSteps = 0.0;
        this->QnHr = 0.0;
        this->QnMonthlyAgg = 0.0;
        this->QnSubHr = 0.0;
        this->LastHourN = 1;
        this->N = 1;
        this->updateCurSimTime = false;
        this->triggerDesignDayReset = false;
    }

    this->currentSimTime = (state.dataGlobal->DayOfSim - 1) * 24 + state.dataGlobal->HourOfDay - 1 +
                           (state.dataGlobal->TimeStep - 1) * state.dataGlobal->TimeStepZone + state.dataHVACGlobal->SysTimeElapsed; //+ TimeStepSys
    this->locHourOfDay = static_cast<int>(mod(this->currentSimTime, Constant::iHoursInDay) + 1);
    this->locDayOfSim = static_cast<int>(this->currentSimTime / 24 + 1);

    if (state.dataGlobal->DayOfSim > 1) {
        updateCurSimTime = true;
    }

    if (!state.dataGlobal->WarmupFlag) {
        triggerDesignDayReset = true;
    }

    if (this->currentSimTime <= 0.0) {
        this->prevTimeSteps = 0.0; // This resets history when rounding 24:00 hours during warmup avoids hard crash later
        calcAggregateLoad(state);  // Just allocates and initializes prevHour array
        return;
    }

    // Store currentSimTime in prevTimeSteps only if a time step occurs
    if (this->prevTimeSteps(1) != this->currentSimTime) {
        this->prevTimeSteps = eoshift(this->prevTimeSteps, -1, this->currentSimTime);
        ++this->N;
    }

    if (this->N != PrevN) {
        PrevN = this->N;
        this->QnSubHr = eoshift(this->QnSubHr, -1, this->lastQnSubHr);
    }

    calcAggregateLoad(state);

    // Update the heat exchanger resistance each time
    this->HXResistance = calcHXResistance(state);

    if (this->N == 1) {
        if (this->massFlowRate <= 0.0) {
            tmpQnSubHourly = 0.0;
            fluidAveTemp = this->tempGround;
            this->ToutNew = this->inletTemp;
        } else {
            Real64 gFuncVal = getGFunc(this->currentSimTime / (this->timeSSFactor));

            Real64 C_1 = (this->totalTubeLength) / (2.0 * this->massFlowRate * cpFluid);
            tmpQnSubHourly = (this->tempGround - this->inletTemp) / (gFuncVal / (kGroundFactor) + this->HXResistance + C_1);
            fluidAveTemp = this->tempGround - tmpQnSubHourly * this->HXResistance;
            this->ToutNew = this->tempGround - tmpQnSubHourly * (gFuncVal / (kGroundFactor) + this->HXResistance - C_1);
        }
    } else {
        // no monthly super position
        if (this->currentSimTime < (hrsPerMonth + this->AGG + this->SubAGG)) {

            // Calculate the Sub Hourly Superposition

            // same as above for sub-hourly (with no aggregation)
            Real64 sumQnSubHourly = 0.0;
            int IndexN;
            if (static_cast<int>(this->currentSimTime) < this->SubAGG) {
                IndexN = static_cast<int>(this->currentSimTime) + 1;
            } else {
                IndexN = this->SubAGG + 1;
            }

            int subHourlyLimit = this->N - this->LastHourN(IndexN); // Check this when running simulation
            for (int I = 1; I <= subHourlyLimit; ++I) {
                if (I == subHourlyLimit) {
                    if (static_cast<int>(this->currentSimTime) >= this->SubAGG) {
                        Real64 gFuncVal = getGFunc((this->currentSimTime - this->prevTimeSteps(I + 1)) / (this->timeSSFactor));
                        Real64 RQSubHr = gFuncVal / (kGroundFactor);
                        sumQnSubHourly += (this->QnSubHr(I) - this->QnHr(IndexN)) * RQSubHr;
                    } else {
                        Real64 gFuncVal = getGFunc((this->currentSimTime - this->prevTimeSteps(I + 1)) / (this->timeSSFactor));
                        Real64 RQSubHr = gFuncVal / (kGroundFactor);
                        sumQnSubHourly += this->QnSubHr(I) * RQSubHr;
                    }
                    break;
                }
                // prevTimeSteps(I+1) This is "I+1" because prevTimeSteps(1) = CurrentTimestep
                Real64 gFuncVal = getGFunc((this->currentSimTime - this->prevTimeSteps(I + 1)) / (this->timeSSFactor));
                Real64 RQSubHr = gFuncVal / (kGroundFactor);
                sumQnSubHourly += (this->QnSubHr(I) - this->QnSubHr(I + 1)) * RQSubHr;
            }

            // Calculate the Hourly Superposition
            // same as above for hourly
            Real64 sumQnHourly = 0.0;

            int hourlyLimit = static_cast<int>(this->currentSimTime);
            for (int I = this->SubAGG + 1; I <= hourlyLimit; ++I) {
                if (I == hourlyLimit) {
                    Real64 gFuncVal = getGFunc(this->currentSimTime / (this->timeSSFactor));
                    Real64 RQHour = gFuncVal / (kGroundFactor);
                    sumQnHourly += this->QnHr(I) * RQHour;
                    break;
                }
                Real64 gFuncVal = getGFunc((this->currentSimTime - static_cast<int>(this->currentSimTime) + I) / (this->timeSSFactor));
                Real64 RQHour = gFuncVal / (kGroundFactor);
                sumQnHourly += (this->QnHr(I) - this->QnHr(I + 1)) * RQHour;
            }

            // Find the total Sum of the Temperature difference due to all load blocks
            sumTotal = sumQnSubHourly + sumQnHourly;

            // Calculate the sub-hourly temperature due the Last Time steps Load
            Real64 gFuncVal = getGFunc((this->currentSimTime - this->prevTimeSteps(2)) / (this->timeSSFactor));
            Real64 RQSubHr = gFuncVal / kGroundFactor;

            if (this->massFlowRate <= 0.0) {
                tmpQnSubHourly = 0.0;
                fluidAveTemp = this->tempGround - sumTotal; // Q(N)*RB = 0
                this->ToutNew = this->inletTemp;
            } else {
                // Dr.Sitler's Explicit set of equations to calculate the New Outlet Temperature of the U-Tube
                Real64 C0 = RQSubHr;
                Real64 C1 = this->tempGround - (sumTotal - this->QnSubHr(1) * RQSubHr);
                Real64 C2 = this->totalTubeLength / (2.0 * this->massFlowRate * cpFluid);
                Real64 C3 = this->massFlowRate * cpFluid / (this->totalTubeLength);
                tmpQnSubHourly = (C1 - this->inletTemp) / (this->HXResistance + C0 - C2 + (1 / C3));
                fluidAveTemp = C1 - (C0 + this->HXResistance) * tmpQnSubHourly;
                this->ToutNew = C1 + (C2 - C0 - this->HXResistance) * tmpQnSubHourly;
            }

        } else { // Monthly Aggregation and super position

            // the number of months of simulation elapsed
            int numOfMonths = static_cast<int>((this->currentSimTime + 1) / hrsPerMonth);

            // The Month up to which the monthly blocks are superposed
            int currentMonth;

            if (this->currentSimTime < ((numOfMonths)*hrsPerMonth) + this->AGG + this->SubAGG) {
                currentMonth = numOfMonths - 1;
            } else {
                currentMonth = numOfMonths;
            }

            // Monthly superposition
            // tmp variable which holds the sum of the Temperature difference due to Aggregated heat extraction/rejection step
            Real64 sumQnMonthly = 0.0;

            for (int I = 1; I <= currentMonth; ++I) {
                if (I == 1) {
                    Real64 gFuncVal = getGFunc(this->currentSimTime / (this->timeSSFactor));
                    Real64 RQMonth = gFuncVal / (kGroundFactor);
                    sumQnMonthly += this->QnMonthlyAgg(I) * RQMonth;
                    continue;
                }
                Real64 gFuncVal = getGFunc((this->currentSimTime - (I - 1) * hrsPerMonth) / (this->timeSSFactor));
                Real64 RQMonth = gFuncVal / (kGroundFactor);
                sumQnMonthly += (this->QnMonthlyAgg(I) - this->QnMonthlyAgg(I - 1)) * RQMonth;
            }

            // Hourly Superposition
            Real64 sumQnHourly = 0.0;
            int hourlyLimit = static_cast<int>(this->currentSimTime - currentMonth * hrsPerMonth);
            for (int I = 1 + this->SubAGG; I <= hourlyLimit; ++I) {
                if (I == hourlyLimit) {
                    Real64 gFuncVal = getGFunc((this->currentSimTime - static_cast<int>(this->currentSimTime) + I) / (this->timeSSFactor));
                    Real64 RQHour = gFuncVal / (kGroundFactor);
                    sumQnHourly += (this->QnHr(I) - this->QnMonthlyAgg(currentMonth)) * RQHour;
                    break;
                }
                Real64 gFuncVal = getGFunc((this->currentSimTime - static_cast<int>(this->currentSimTime) + I) / (this->timeSSFactor));
                Real64 RQHour = gFuncVal / (kGroundFactor);
                sumQnHourly += (this->QnHr(I) - this->QnHr(I + 1)) * RQHour;
            }

            // sub-hourly Superposition
            int subHourlyLimit = this->N - this->LastHourN(this->SubAGG + 1);
            Real64 sumQnSubHourly = 0.0;
            for (int I = 1; I <= subHourlyLimit; ++I) {
                if (I == subHourlyLimit) {
                    Real64 gFuncVal = getGFunc((this->currentSimTime - this->prevTimeSteps(I + 1)) / (this->timeSSFactor));
                    Real64 RQSubHr = gFuncVal / (kGroundFactor);
                    sumQnSubHourly += (this->QnSubHr(I) - this->QnHr(this->SubAGG + 1)) * RQSubHr;
                    break;
                }
                Real64 gFuncVal = getGFunc((this->currentSimTime - this->prevTimeSteps(I + 1)) / (this->timeSSFactor));
                Real64 RQSubHr = gFuncVal / (kGroundFactor);
                sumQnSubHourly += (this->QnSubHr(I) - this->QnSubHr(I + 1)) * RQSubHr;
            }

            sumTotal = sumQnMonthly + sumQnHourly + sumQnSubHourly;

            // Calculate the sub-hourly temperature due the Last Time steps Load
            Real64 gFuncVal = getGFunc((this->currentSimTime - this->prevTimeSteps(2)) / (this->timeSSFactor));
            Real64 RQSubHr = gFuncVal / (kGroundFactor);

            if (this->massFlowRate <= 0.0) {
                tmpQnSubHourly = 0.0;
                fluidAveTemp = this->tempGround - sumTotal; // Q(N)*RB = 0
                this->ToutNew = this->inletTemp;
            } else {
                // Explicit set of equations to calculate the New Outlet Temperature of the U-Tube
                Real64 C0 = RQSubHr;
                Real64 C1 = this->tempGround - (sumTotal - this->QnSubHr(1) * RQSubHr);
                Real64 C2 = this->totalTubeLength / (2 * this->massFlowRate * cpFluid);
                Real64 C3 = this->massFlowRate * cpFluid / (this->totalTubeLength);
                tmpQnSubHourly = (C1 - this->inletTemp) / (this->HXResistance + C0 - C2 + (1 / C3));
                fluidAveTemp = C1 - (C0 + this->HXResistance) * tmpQnSubHourly;
                this->ToutNew = C1 + (C2 - C0 - this->HXResistance) * tmpQnSubHourly;
            }
        } //  end of AGG OR NO AGG
    } // end of N  = 1 branch
    this->bhTemp = this->tempGround - sumTotal;

    // Load the QnSubHourly Array with a new value at end of every timestep
    this->lastQnSubHr = tmpQnSubHourly;
    this->outletTemp = this->ToutNew;
    this->QGLHE = tmpQnSubHourly * this->totalTubeLength;
    this->aveFluidTemp = fluidAveTemp;
}

void GLHEBase::updateGHX(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Matt Mitchell
    //       DATE WRITTEN:    February 2015

    // PURPOSE OF THIS SUBROUTINE:
    // Updates the outlet node and check for out of bounds temperatures

    // SUBROUTINE ARGUMENT DEFINITIONS
    constexpr std::string_view RoutineName = "UpdateGroundHeatExchanger";
    constexpr Real64 deltaTempLimit = 100.0; // temp limit for warnings

    PlantUtilities::SafeCopyPlantNode(state, this->inletNodeNum, this->outletNodeNum);

    state.dataLoopNodes->Node(this->outletNodeNum).Temp = this->outletTemp;
    state.dataLoopNodes->Node(this->outletNodeNum).Enthalpy =
        this->outletTemp * state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, this->outletTemp, RoutineName);

    const Real64 GLHEDeltaTemp = std::abs(this->outletTemp - this->inletTemp);

    if (GLHEDeltaTemp > deltaTempLimit && this->numErrorCalls < state.dataGroundHeatExchanger->numVerticalGLHEs && !state.dataGlobal->WarmupFlag) {
        Real64 fluidDensity = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getDensity(state, this->inletTemp, RoutineName);
        this->designMassFlow = this->designFlow * fluidDensity;
        ShowWarningError(state, "Check GLHE design inputs & g-functions for consistency");
        ShowContinueError(state, format("For GroundHeatExchanger: {}GLHE delta Temp > 100C.", this->name));
        ShowContinueError(state, "This can be encountered in cases where the GLHE mass flow rate is either significantly");
        ShowContinueError(state, " lower than the design value, or cases where the mass flow rate rapidly changes.");
        ShowContinueError(state, format("GLHE Current Flow Rate={:.3T}; GLHE Design Flow Rate={:.3T}", this->massFlowRate, this->designMassFlow));
        ++this->numErrorCalls;
    }
}

void GLHEBase::calcAggregateLoad([[maybe_unused]] const EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Arun Murugappan
    //       DATE WRITTEN:    August, 2000

    // PURPOSE OF THIS SUBROUTINE:
    // Manages the heat transfer history.

    // METHODOLOGY EMPLOYED:
    // The heat pulse histories need to be recorded over an extended period (months).
    // To aid computational efficiency past pulses are continuously aggregated into
    // equivalent heat pulses of longer duration, as each pulse becomes less recent.
    // Past sub-hourly loads are re-aggregated into equivalent hourly and monthly loads.

    // REFERENCES:
    // Eskilson, P. 'Thermal Analysis of Heat Extraction Boreholes' Ph.D. Thesis:
    //   Dept. of Mathematical Physics, University of Lund, Sweden, June 1987.
    // Yavuzturk, C., J.D. Spitler. 1999. 'A Short Time Step Response Factor Model
    //   for Vertical Ground Loop Heat Exchangers'. ASHRAE Transactions. 105(2): 475-485.

    if (this->currentSimTime <= 0.0) {
        return;
    }

    // FOR EVERY HOUR UPDATE THE HOURLY QN this->QnHr(J)
    // THIS IS DONE BY AGGREGATING THE sub-hourly QN FROM THE PREVIOUS HOUR TO UNTIL THE CURRENT HOUR
    // AND STORING IT IN  verticalGLHE(GLHENum)%QnHr(J)

    // sub-hourly Qn IS NOT AGGREGATED . IT IS THE BASIC LOAD
    if (this->prevHour != this->locHourOfDay) {
        Real64 SumQnHr = 0.0;
        int J;
        for (J = 1; J <= (this->N - this->LastHourN(1)); ++J) {
            SumQnHr += this->QnSubHr(J) * std::abs(this->prevTimeSteps(J) - this->prevTimeSteps(J + 1));
        }
        if (this->prevTimeSteps(1) != this->prevTimeSteps(J)) {
            SumQnHr /= std::abs(this->prevTimeSteps(1) - this->prevTimeSteps(J));
        } else {
            SumQnHr /= 0.05; // estimated small timestep
        }
        this->QnHr = eoshift(this->QnHr, -1, SumQnHr);
        this->LastHourN = eoshift(this->LastHourN, -1, this->N);
    }

    // CHECK IF A MONTH PASSES...
    if (mod(((this->locDayOfSim - 1) * Constant::iHoursInDay + (this->locHourOfDay)), hrsPerMonth) == 0 && this->prevHour != this->locHourOfDay) {
        int const MonthNum = static_cast<int>((this->locDayOfSim * Constant::iHoursInDay + this->locHourOfDay) / hrsPerMonth);
        Real64 SumQnMonth = 0.0;
        for (int J = 1; J <= static_cast<int>(hrsPerMonth); ++J) {
            SumQnMonth += this->QnHr(J);
        }
        SumQnMonth /= hrsPerMonth;
        this->QnMonthlyAgg(MonthNum) = SumQnMonth;
    }
    this->prevHour = this->locHourOfDay;
}

void GLHEBase::onInitLoopEquip(EnergyPlusData &state, [[maybe_unused]] const PlantLocation &calledFromLocation)
{
    this->initGLHESimVars(state);
}

void GLHEBase::simulate(EnergyPlusData &state,
                        [[maybe_unused]] const PlantLocation &calledFromLocation,
                        [[maybe_unused]] bool const FirstHVACIteration,
                        [[maybe_unused]] Real64 &CurLoad,
                        [[maybe_unused]] bool const RunFlag)
{

    if (this->needToSetupOutputVars) {
        this->setupOutput(state);
        this->needToSetupOutputVars = false;
    }

    this->initGLHESimVars(state);
    if (state.dataGlobal->KickOffSimulation) {
        return;
    }

    this->calcGroundHeatExchanger(state);
    this->updateGHX(state);
}

GLHEBase *GLHEBase::factory(EnergyPlusData &state, DataPlant::PlantEquipmentType objectType, std::string const &objectName)
{
    if (state.dataGroundHeatExchanger->GetInput) {
        GetGroundHeatExchangerInput(state);
        state.dataGroundHeatExchanger->GetInput = false;
    }
    if (objectType == DataPlant::PlantEquipmentType::GrndHtExchgSystem) {
        auto thisObj = std::find_if(state.dataGroundHeatExchanger->verticalGLHE.begin(),
                                    state.dataGroundHeatExchanger->verticalGLHE.end(),
                                    [&objectName](const GLHEBase &myObj) { return myObj.name == objectName; });
        if (thisObj != state.dataGroundHeatExchanger->verticalGLHE.end()) {
            return &(*thisObj);
        }
    } else if (objectType == DataPlant::PlantEquipmentType::GrndHtExchgSlinky) {
        auto thisObj = std::find_if(state.dataGroundHeatExchanger->slinkyGLHE.begin(),
                                    state.dataGroundHeatExchanger->slinkyGLHE.end(),
                                    [&objectName](const GLHEBase &myObj) { return myObj.name == objectName; });
        if (thisObj != state.dataGroundHeatExchanger->slinkyGLHE.end()) {
            return &(*thisObj);
        }
    }

    // If we didn't find it, fatal
    ShowFatalError(state, format("Ground Heat Exchanger Factory: Error getting inputs for GHX named: {}", objectName));
}

void GLHEBase::setupOutput(EnergyPlusData &state)
{
    SetupOutputVariable(state,
                        "Ground Heat Exchanger Average Borehole Temperature",
                        Constant::Units::C,
                        this->bhTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Ground Heat Exchanger Heat Transfer Rate",
                        Constant::Units::W,
                        this->QGLHE,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Ground Heat Exchanger Inlet Temperature",
                        Constant::Units::C,
                        this->inletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Ground Heat Exchanger Outlet Temperature",
                        Constant::Units::C,
                        this->outletTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Ground Heat Exchanger Mass Flow Rate",
                        Constant::Units::kg_s,
                        this->massFlowRate,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Ground Heat Exchanger Average Fluid Temperature",
                        Constant::Units::C,
                        this->aveFluidTemp,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
    SetupOutputVariable(state,
                        "Ground Heat Exchanger Farfield Ground Temperature",
                        Constant::Units::C,
                        this->tempGround,
                        OutputProcessor::TimeStepType::System,
                        OutputProcessor::StoreType::Average,
                        this->name);
}

Real64 GLHEBase::interpGFunc(Real64 const x_val) const
{
    // Purpose: interpolate between g-function values, with linear extrapolation above and below range

    auto const &x = this->myRespFactors->LNTTS;
    auto const &y = this->myRespFactors->GFNC;
    auto const &upper_it = std::upper_bound(x.begin(), x.end(), x_val);

    unsigned int l_idx = 0;
    unsigned int u_idx = 0;

    if (upper_it == x.begin()) {
        // Linear extrapolation beyond the lower bound
        l_idx = 0;
        u_idx = 1;
    } else if (upper_it == x.end()) {
        // Linear extrapolation beyond the upper bound
        u_idx = x.size() - 1;
        l_idx = u_idx - 1;
    } else {
        // In the middle of the range
        u_idx = std::distance(x.begin(), upper_it);
        l_idx = u_idx - 1;
    }

    Real64 const x_low = x[l_idx];
    Real64 const x_high = x[u_idx];
    Real64 const y_low = y[l_idx];
    Real64 const y_high = y[u_idx];

    return (x_val - x_low) / (x_high - x_low) * (y_high - y_low) + y_low;
}

std::vector<Real64> TDMA(std::vector<Real64> const &a, std::vector<Real64> const &b, std::vector<Real64> &c, std::vector<Real64> &d)
{
    // from: https://en.wikibooks.org/wiki/Algorithm_Implementation/Linear_Algebra/Tridiagonal_matrix_algorithm#C.2B.2B

    int n = static_cast<int>(d.size() - 1u);

    c[0] /= b[0];
    d[0] /= b[0];

    for (int i = 1; i < n; ++i) {
        c[i] /= b[i] - a[i] * c[i - 1];
        d[i] = (d[i] - a[i] * d[i - 1]) / (b[i] - a[i] * c[i - 1]);
    }

    d[n] = (d[n] - a[n] * d[n - 1]) / (b[n] - a[n] * c[n - 1]);

    for (int i = n; i-- > 0;) {
        d[i] -= c[i] * d[i + 1];
    }

    return d;
}
void GetGroundHeatExchangerInput(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Dan Fisher
    //       DATE WRITTEN:    August, 2000
    //       MODIFIED         Arun Murugappan

    // GET NUMBER OF ALL EQUIPMENT TYPES
    state.dataGroundHeatExchanger->numVerticalGLHEs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, GLHEVert::moduleName);
    state.dataGroundHeatExchanger->numSlinkyGLHEs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, GLHESlinky::moduleName);
    state.dataGroundHeatExchanger->numVertArray = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, GLHEVertArray::moduleName);
    state.dataGroundHeatExchanger->numVertProps = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, GLHEVertProps::moduleName);
    state.dataGroundHeatExchanger->numResponseFactors =
        state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, GLHEResponseFactors::moduleName);
    state.dataGroundHeatExchanger->numSingleBorehole =
        state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, GLHEVertSingle::moduleName);

    if (state.dataGroundHeatExchanger->numVerticalGLHEs <= 0 && state.dataGroundHeatExchanger->numSlinkyGLHEs <= 0) {
        ShowSevereError(state, "Error processing inputs for GLHE objects");
        ShowContinueError(state, "Simulation indicated these objects were found, but input processor doesn't find any");
        ShowContinueError(state, "Check inputs for GroundHeatExchanger:System and GroundHeatExchanger:Slinky");
        ShowContinueError(state, "Also check plant/branch inputs for references to invalid/deleted objects");
    }

    if (state.dataGroundHeatExchanger->numVertProps > 0) {
        auto const instances = state.dataInputProcessing->inputProcessor->epJSON.find(GLHEVertProps::moduleName);
        if (instances == state.dataInputProcessing->inputProcessor->epJSON.end()) {
            ShowSevereError(state,
                            format("{}: Somehow getNumObjectsFound was > 0 but epJSON.find found 0", GLHEVertProps::moduleName)); // LCOV_EXCL_LINE
        }
        auto &instancesValue = instances.value();
        for (auto it = instancesValue.begin(); it != instancesValue.end(); ++it) {
            auto const &instance = it.value();
            std::string const &objName = it.key();
            std::string const &objNameUC = Util::makeUPPER(objName);
            state.dataInputProcessing->inputProcessor->markObjectAsUsed(GLHEVertProps::moduleName, objName);
            std::shared_ptr<GLHEVertProps> thisObj(new GLHEVertProps(state, objNameUC, instance));
            state.dataGroundHeatExchanger->vertPropsVector.push_back(thisObj);
        }
    }

    if (state.dataGroundHeatExchanger->numResponseFactors > 0) {
        auto const instances = state.dataInputProcessing->inputProcessor->epJSON.find(GLHEResponseFactors::moduleName);
        if (instances == state.dataInputProcessing->inputProcessor->epJSON.end()) {
            ShowSevereError(
                state, format("{}: Somehow getNumObjectsFound was > 0 but epJSON.find found 0", GLHEResponseFactors::moduleName)); // LCOV_EXCL_LINE
        }
        auto &instancesValue = instances.value();
        for (auto it = instancesValue.begin(); it != instancesValue.end(); ++it) {
            auto const &instance = it.value();
            std::string const &objName = it.key();
            std::string const &objNameUC = Util::makeUPPER(objName);
            state.dataInputProcessing->inputProcessor->markObjectAsUsed(GLHEResponseFactors::moduleName, objName);
            std::shared_ptr<GLHEResponseFactors> thisObj(new GLHEResponseFactors(state, objNameUC, instance));
            state.dataGroundHeatExchanger->responseFactorsVector.push_back(thisObj);
        }
    }

    if (state.dataGroundHeatExchanger->numVertArray > 0) {
        auto const instances = state.dataInputProcessing->inputProcessor->epJSON.find(GLHEVertArray::moduleName);
        if (instances == state.dataInputProcessing->inputProcessor->epJSON.end()) {
            ShowSevereError(state,
                            format("{}: Somehow getNumObjectsFound was > 0 but epJSON.find found 0", GLHEVertArray::moduleName)); // LCOV_EXCL_LINE
        }
        auto &instancesValue = instances.value();
        for (auto it = instancesValue.begin(); it != instancesValue.end(); ++it) {
            auto const &instance = it.value();
            std::string const &objName = it.key();
            std::string const &objNameUC = Util::makeUPPER(objName);
            state.dataInputProcessing->inputProcessor->markObjectAsUsed(GLHEVertArray::moduleName, objName);
            std::shared_ptr<GLHEVertArray> thisObj(new GLHEVertArray(state, objNameUC, instance));
            state.dataGroundHeatExchanger->vertArraysVector.push_back(thisObj);
        }
    }

    if (state.dataGroundHeatExchanger->numSingleBorehole > 0) {
        auto const instances = state.dataInputProcessing->inputProcessor->epJSON.find(GLHEVertSingle::moduleName);
        if (instances == state.dataInputProcessing->inputProcessor->epJSON.end()) {
            ShowSevereError(state,
                            format("{}: Somehow getNumObjectsFound was > 0 but epJSON.find found 0", GLHEVertSingle::moduleName)); // LCOV_EXCL_LINE
        }
        auto &instancesValue = instances.value();
        for (auto it = instancesValue.begin(); it != instancesValue.end(); ++it) {
            auto const &instance = it.value();
            std::string const &objName = it.key();
            std::string const &objNameUC = Util::makeUPPER(objName);
            state.dataInputProcessing->inputProcessor->markObjectAsUsed(GLHEVertSingle::moduleName, objName);
            std::shared_ptr<GLHEVertSingle> thisObj(new GLHEVertSingle(state, objNameUC, instance));
            state.dataGroundHeatExchanger->singleBoreholesVector.push_back(thisObj);
        }
    }

    if (state.dataGroundHeatExchanger->numVerticalGLHEs > 0) {
        auto const instances = state.dataInputProcessing->inputProcessor->epJSON.find(GLHEVert::moduleName);
        if (instances == state.dataInputProcessing->inputProcessor->epJSON.end()) {
            ShowSevereError(state, format("{}: Somehow getNumObjectsFound was > 0 but epJSON.find found 0", GLHEVert::moduleName)); // LCOV_EXCL_LINE
        }
        auto &instancesValue = instances.value();
        for (auto it = instancesValue.begin(); it != instancesValue.end(); ++it) {
            auto const &instance = it.value();
            std::string const &objName = it.key();
            std::string const &objNameUC = Util::makeUPPER(objName);
            state.dataInputProcessing->inputProcessor->markObjectAsUsed(GLHEVert::moduleName, objName);
            state.dataGroundHeatExchanger->verticalGLHE.emplace_back(state, objNameUC, instance);
        }
    }

    if (state.dataGroundHeatExchanger->numSlinkyGLHEs > 0) {
        auto const instances = state.dataInputProcessing->inputProcessor->epJSON.find(GLHESlinky::moduleName);
        if (instances == state.dataInputProcessing->inputProcessor->epJSON.end()) {
            ShowSevereError(state,
                            format("{}: Somehow getNumObjectsFound was > 0 but epJSON.find found 0", GLHESlinky::moduleName)); // LCOV_EXCL_LINE
        }
        auto &instancesValue = instances.value();
        for (auto it = instancesValue.begin(); it != instancesValue.end(); ++it) {
            auto const &instance = it.value();
            std::string const &objName = it.key();
            std::string const &objNameUC = Util::makeUPPER(objName);
            state.dataInputProcessing->inputProcessor->markObjectAsUsed(GLHESlinky::moduleName, objName);
            state.dataGroundHeatExchanger->slinkyGLHE.emplace_back(state, objNameUC, instance);
        }
    }
}

} // namespace EnergyPlus::GroundHeatExchangers
