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
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <string>
#include <string_view>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// Third-party Headers
#include <fast_float/fast_float.h>

// EnergyPlus Headers
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/FileSystem.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace Curve {
    // Module containing the Curve Manager routines

    // MODULE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   May 2000
    //       MODIFIED       January 2006, Rick Strand, added a curve type (quadratic-linear)
    //                      July 2006, L. Gu, added a new curve type (bicubic)

    //                      July 2006, Brent Griffith, added triquadratic curve
    //                       RR added exponential curve
    //                      May 2009 Brent griffith add EMS actuator registry and override (for custom equations)
    //                      August 2010, Richard Raustad, FSEC, added Table:* objects
    //                      August 2014, Rick Strand, added a curve type (cubic-linear)
    //                      Future Improvements:
    //                          Subroutine PerformanceTableObject is not really needed (and is probably slower)
    //                          since Subroutine TableLookupObject can do the same thing. The difference
    //                          is that Sub PerformanceTableObject does a linear interpolation without extrapolation.
    //                          More math is also involved. Sub TableLookupObject can also do this if a) the limits
    //                          of the input data use the boundaries of the tabular data, b) the arrays are corrected
    //                          to use this other subroutine, and c) the Number of Interpolation Points is set to 2.
    //                      22Aug2010 Craig Wray, added new curves for fan component model:
    //                          FanPressureRise, ExponentialSkewNormal, Sigmoid, RectangularHyperbola1,
    //                          RectangularHyperbola2, ExponentialDecay
    //                      March 2012, Atefe Makhmalbaf and Heejin Cho, added a new curve type (QuadLinear)
    //                      Jan 2021 Yueyue, added a new curve type (QuintLinear)
    //                      Aug.  2014, Rongpeng Zhang, added a new curve type (ChillerPartLoadWithLift)
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // To provide the capabilities of getting the curve data from the input,
    // validating it, and storing it in such a manner that the curve manager
    // can provide the simulation with performance curve output.

    std::shared_ptr<EnergyPlusLogger> BtwxtManager::btwxt_logger{{std::make_shared<EnergyPlusLogger>()}};

    // Functions
    void commonEnvironInit(EnergyPlusData &state)
    {
        // need to be careful on where and how resetting curve outputs to some "inactive value" is done
        // EMS can intercept curves and modify output
        if (state.dataGlobal->BeginEnvrnFlag && state.dataCurveManager->CurveValueMyBeginTimeStepFlag) {
            ResetPerformanceCurveOutput(state);
            state.dataCurveManager->CurveValueMyBeginTimeStepFlag = false;
        }
        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataCurveManager->CurveValueMyBeginTimeStepFlag = true;
        }
    }

    void ResetPerformanceCurveOutput(const EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   August 2010
        // PURPOSE OF THIS SUBROUTINE:
        // Reset curve outputs prior to simulating air loops, plant loops, etc.
        // This allows the report variable for curve/table objects to show an inactive state.

        for (auto *c : state.dataCurveManager->curves) {
            c->output = DataLoopNode::SensedNodeFlagValue;
            for (auto &i : c->inputs) { // Not a good pattern
                i = DataLoopNode::SensedNodeFlagValue;
            }
        }
    }

    Real64 Curve::value(EnergyPlusData &state, Real64 V1)
    {
        commonEnvironInit(state);
        this->inputs[0] = V1;

        V1 = max(min(V1, this->inputLimits[0].max), this->inputLimits[0].min);

        Real64 Val = 0.0;

        switch (this->curveType) {

        case CurveType::Linear: {
            Val = this->coeff[0] + V1 * this->coeff[1];
        } break;

        case CurveType::Quadratic: {
            Val = this->coeff[0] + V1 * (this->coeff[1] + V1 * this->coeff[2]);
        } break;

        case CurveType::Cubic: {
            Val = this->coeff[0] + V1 * (this->coeff[1] + V1 * (this->coeff[2] + V1 * this->coeff[3]));
        } break;

        case CurveType::Quartic: {
            Val = this->coeff[0] + V1 * (this->coeff[1] + V1 * (this->coeff[2] + V1 * (this->coeff[3] + V1 * this->coeff[4])));
        } break;

        case CurveType::Exponent: {
            Val = this->coeff[0] + this->coeff[1] * std::pow(V1, this->coeff[2]);
        } break;

        case CurveType::ExponentialSkewNormal: {
            Real64 CoeffZ1 = (V1 - this->coeff[0]) / this->coeff[1];
            Real64 CoeffZ2 = (this->coeff[3] * V1 * std::exp(this->coeff[2] * V1) - this->coeff[0]) / this->coeff[1];
            Real64 CoeffZ3 = -this->coeff[0] / this->coeff[1];
            static Real64 const sqrt_2_inv(1.0 / std::sqrt(2.0)); // would be constexpr-able if std::sqrt was constexpr, but not yet
            Real64 Numer = std::exp(-0.5 * (CoeffZ1 * CoeffZ1)) * (1.0 + sign(1.0, CoeffZ2) * std::erf(std::abs(CoeffZ2) * sqrt_2_inv));
            Real64 Denom = std::exp(-0.5 * (CoeffZ3 * CoeffZ3)) * (1.0 + sign(1.0, CoeffZ3) * std::erf(std::abs(CoeffZ3) * sqrt_2_inv));
            Val = Numer / Denom;
        } break;

        case CurveType::Sigmoid: {
            Real64 CurveValueExp = std::exp((this->coeff[2] - V1) / this->coeff[3]);
            Val = this->coeff[0] + this->coeff[1] / std::pow(1.0 + CurveValueExp, this->coeff[4]);
        } break;

        case CurveType::RectangularHyperbola1: {
            Real64 Numer = this->coeff[0] * V1;
            Real64 Denom = this->coeff[1] + V1;
            Val = (Numer / Denom) + this->coeff[2];
        } break;

        case CurveType::RectangularHyperbola2: {
            Real64 Numer = this->coeff[0] * V1;
            Real64 Denom = this->coeff[1] + V1;
            Val = (Numer / Denom) + (this->coeff[2] * V1);
        } break;

        case CurveType::ExponentialDecay: {
            Val = this->coeff[0] + this->coeff[1] * std::exp(this->coeff[2] * V1);
        } break;

        case CurveType::DoubleExponentialDecay: {
            Val = this->coeff[0] + this->coeff[1] * std::exp(this->coeff[2] * V1) + this->coeff[3] * std::exp(this->coeff[4] * V1);
        } break;

        case CurveType::BtwxtTableLookup: {
            Val = BtwxtTableInterpolation(state, V1);
        } break;

        default: {
            Val = this->valueFallback(state, V1, 0.0, 0.0, 0.0, 0.0);
        } break;
        }

        if (this->outputLimits.minPresent) {
            Val = max(Val, this->outputLimits.min);
        }
        if (this->outputLimits.maxPresent) {
            Val = min(Val, this->outputLimits.max);
        }
        if (this->EMSOverrideOn) {
            Val = this->EMSOverrideCurveValue;
        }

        this->output = Val;
        return Val;
    }

    Real64 Curve::value(EnergyPlusData &state, Real64 V1, Real64 V2)
    {
        commonEnvironInit(state);
        this->inputs[0] = V1;
        this->inputs[1] = V2;

        V1 = max(min(V1, this->inputLimits[0].max), this->inputLimits[0].min);
        V2 = max(min(V2, this->inputLimits[1].max), this->inputLimits[1].min);

        Real64 Val = 0.0;

        switch (this->curveType) {
        case CurveType::FanPressureRise: {
            return V1 * (this->coeff[0] * V1 + this->coeff[1] + this->coeff[2] * std::sqrt(V2)) + this->coeff[3] * V2;
        } break;

        case CurveType::BiQuadratic: {
            Val =
                this->coeff[0] + V1 * (this->coeff[1] + V1 * this->coeff[2]) + V2 * (this->coeff[3] + V2 * this->coeff[4]) + V1 * V2 * this->coeff[5];
        } break;

        case CurveType::QuadraticLinear: {
            Val =
                (this->coeff[0] + V1 * (this->coeff[1] + V1 * this->coeff[2])) + (this->coeff[3] + V1 * (this->coeff[4] + V1 * this->coeff[5])) * V2;
        } break;

        case CurveType::CubicLinear: {
            Val =
                (this->coeff[0] + V1 * (this->coeff[1] + V1 * (this->coeff[2] + V1 * this->coeff[3]))) + (this->coeff[4] + V1 * this->coeff[5]) * V2;
        } break;

        case CurveType::BiCubic: {
            Val = this->coeff[0] + V1 * this->coeff[1] + V1 * V1 * this->coeff[2] + V2 * this->coeff[3] + V2 * V2 * this->coeff[4] +
                  V1 * V2 * this->coeff[5] + V1 * V1 * V1 * this->coeff[6] + V2 * V2 * V2 * this->coeff[7] + V1 * V1 * V2 * this->coeff[8] +
                  V1 * V2 * V2 * this->coeff[9];
        } break;

        case CurveType::BtwxtTableLookup: {
            Val = BtwxtTableInterpolation(state, V1, V2);
        } break;

        default: {
            Val = this->valueFallback(state, V1, V2, 0.0, 0.0, 0.0);
        } break;
        }

        if (this->outputLimits.minPresent) {
            Val = max(Val, this->outputLimits.min);
        }
        if (this->outputLimits.maxPresent) {
            Val = min(Val, this->outputLimits.max);
        }
        if (this->EMSOverrideOn) {
            Val = this->EMSOverrideCurveValue;
        }

        this->output = Val;
        return Val;
    }

    Real64 Curve::value(EnergyPlusData &state, Real64 V1, Real64 V2, Real64 V3)
    {
        commonEnvironInit(state);
        this->inputs[0] = V1;
        this->inputs[1] = V2;
        this->inputs[2] = V3;

        V1 = max(min(V1, this->inputLimits[0].max), this->inputLimits[0].min);
        V2 = max(min(V2, this->inputLimits[1].max), this->inputLimits[1].min);
        V3 = max(min(V3, this->inputLimits[2].max), this->inputLimits[2].min);

        Real64 Val = 0.0;

        switch (this->curveType) {
        case CurveType::ChillerPartLoadWithLift: {
            Val = this->coeff[0] + this->coeff[1] * V1 + this->coeff[2] * V1 * V1 + this->coeff[3] * V2 + this->coeff[4] * V2 * V2 +
                  this->coeff[5] * V1 * V2 + this->coeff[6] * V1 * V1 * V1 + this->coeff[7] * V2 * V2 * V2 + this->coeff[8] * V1 * V1 * V2 +
                  this->coeff[9] * V1 * V2 * V2 + this->coeff[10] * V1 * V1 * V2 * V2 + this->coeff[11] * V3 * V2 * V2 * V2;
        } break;

        case CurveType::TriQuadratic: {
            auto const &c = this->coeff;
            Real64 const V1s = V1 * V1;
            Real64 const V2s = V2 * V2;
            Real64 const V3s = V3 * V3;
            Val = c[0] + c[1] * V1s + c[2] * V1 + c[3] * V2s + c[4] * V2 + c[5] * V3s + c[6] * V3 + c[7] * V1s * V2s + c[8] * V1 * V2 +
                  c[9] * V1 * V2s + c[10] * V1s * V2 + c[11] * V1s * V3s + c[12] * V1 * V3 + c[13] * V1 * V3s + c[14] * V1s * V3 + c[15] * V2s * V3s +
                  c[16] * V2 * V3 + c[17] * V2 * V3s + c[18] * V2s * V3 + c[19] * V1s * V2s * V3s + c[20] * V1s * V2s * V3 + c[21] * V1s * V2 * V3s +
                  c[22] * V1 * V2s * V3s + c[23] * V1s * V2 * V3 + c[24] * V1 * V2s * V3 + c[25] * V1 * V2 * V3s + c[26] * V1 * V2 * V3;
        } break;

        case CurveType::BtwxtTableLookup: {
            Val = BtwxtTableInterpolation(state, V1, V2, V3);
        } break;

        default: {
            Val = this->valueFallback(state, V1, V2, V3, 0.0, 0.0);
        } break;
        }

        if (this->outputLimits.minPresent) {
            Val = max(Val, this->outputLimits.min);
        }
        if (this->outputLimits.maxPresent) {
            Val = min(Val, this->outputLimits.max);
        }
        if (this->EMSOverrideOn) {
            Val = this->EMSOverrideCurveValue;
        }

        this->output = Val;
        return Val;
    }

    Real64 Curve::value(EnergyPlusData &state, Real64 V1, Real64 V2, Real64 V3, Real64 V4)
    {
        commonEnvironInit(state);
        this->inputs[0] = V1;
        this->inputs[1] = V2;
        this->inputs[2] = V3;
        this->inputs[3] = V4;

        V1 = max(min(V1, this->inputLimits[0].max), this->inputLimits[0].min);
        V2 = max(min(V2, this->inputLimits[1].max), this->inputLimits[1].min);
        V3 = max(min(V3, this->inputLimits[2].max), this->inputLimits[2].min);
        V4 = max(min(V4, this->inputLimits[3].max), this->inputLimits[3].min);

        Real64 Val = 0.0;

        switch (this->curveType) {
        case CurveType::QuadLinear: {
            Val = this->coeff[0] + V1 * this->coeff[1] + V2 * this->coeff[2] + V3 * this->coeff[3] + V4 * this->coeff[4];
        } break;

        case CurveType::BtwxtTableLookup: {
            Val = BtwxtTableInterpolation(state, V1, V2, V3, V4);
        } break;

        default: {
            Val = this->valueFallback(state, V1, V2, V3, V4, 0.0);
        } break;
        }

        if (this->outputLimits.minPresent) {
            Val = max(Val, this->outputLimits.min);
        }
        if (this->outputLimits.maxPresent) {
            Val = min(Val, this->outputLimits.max);
        }
        if (this->EMSOverrideOn) {
            Val = this->EMSOverrideCurveValue;
        }

        this->output = Val;
        return Val;
    }

    Real64 Curve::value(EnergyPlusData &state, Real64 V1, Real64 V2, Real64 V3, Real64 V4, Real64 V5)
    {
        commonEnvironInit(state);
        this->inputs[0] = V1;
        this->inputs[1] = V2;
        this->inputs[2] = V3;
        this->inputs[3] = V4;
        this->inputs[4] = V5;

        V1 = max(min(V1, this->inputLimits[0].max), this->inputLimits[0].min);
        V2 = max(min(V2, this->inputLimits[1].max), this->inputLimits[1].min);
        V3 = max(min(V3, this->inputLimits[2].max), this->inputLimits[2].min);
        V4 = max(min(V4, this->inputLimits[3].max), this->inputLimits[3].min);
        V5 = max(min(V5, this->inputLimits[4].max), this->inputLimits[4].min);

        Real64 Val = 0.0;

        switch (this->curveType) {
        case CurveType::QuintLinear: {
            Val = this->coeff[0] + V1 * this->coeff[1] + V2 * this->coeff[2] + V3 * this->coeff[3] + V4 * this->coeff[4] + V5 * this->coeff[5];
        } break;

        case CurveType::BtwxtTableLookup: {
            Val = BtwxtTableInterpolation(state, V1, V2, V3, V4, V5);
        } break;

        default: {
            Val = this->valueFallback(state, V1, V2, V3, V4, V5);
        }
        }

        if (this->outputLimits.minPresent) {
            Val = max(Val, this->outputLimits.min);
        }
        if (this->outputLimits.maxPresent) {
            Val = min(Val, this->outputLimits.max);
        }
        if (this->EMSOverrideOn) {
            Val = this->EMSOverrideCurveValue;
        }

        this->output = Val;
        return Val;
    }

    Real64 Curve::value(EnergyPlusData &state, Real64 V1, Real64 V2, Real64 V3, Real64 V4, Real64 V5, Real64 V6)
    {
        commonEnvironInit(state);
        this->inputs[0] = V1;
        this->inputs[1] = V2;
        this->inputs[2] = V3;
        this->inputs[3] = V4;
        this->inputs[4] = V5;
        this->inputs[5] = V6;

        V1 = max(min(V1, this->inputLimits[0].max), this->inputLimits[0].min);
        V2 = max(min(V2, this->inputLimits[1].max), this->inputLimits[1].min);
        V3 = max(min(V3, this->inputLimits[2].max), this->inputLimits[2].min);
        V4 = max(min(V4, this->inputLimits[3].max), this->inputLimits[3].min);
        V5 = max(min(V5, this->inputLimits[4].max), this->inputLimits[4].min);
        V6 = max(min(V6, this->inputLimits[5].max), this->inputLimits[5].min);

        // tables are the only 6-D curves, for now at least
        Real64 Val = BtwxtTableInterpolation(state, V1, V2, V3, V4, V5, V6);

        if (this->outputLimits.minPresent) {
            Val = max(Val, this->outputLimits.min);
        }
        if (this->outputLimits.maxPresent) {
            Val = min(Val, this->outputLimits.max);
        }
        if (this->EMSOverrideOn) {
            Val = this->EMSOverrideCurveValue;
        }

        this->output = Val;
        return Val;
    }

    Real64 CurveValue(EnergyPlusData &state,
                      int const CurveIndex, // index of curve in curve array
                      Real64 const Var1     // 1st independent variable
    )
    {
        return state.dataCurveManager->curves(CurveIndex)->value(state, Var1);
    }

    Real64 CurveValue(EnergyPlusData &state,
                      int const CurveIndex, // index of curve in curve array
                      Real64 const Var1,    // 1st independent variable
                      Real64 const Var2     // 2nd independent variable
    )
    {
        return state.dataCurveManager->curves(CurveIndex)->value(state, Var1, Var2);
    }

    Real64 CurveValue(EnergyPlusData &state,
                      int const CurveIndex, // index of curve in curve array
                      Real64 const Var1,    // 1st independent variable
                      Real64 const Var2,    // 2nd independent variable
                      Real64 const Var3     // 3rd independent variable
    )
    {
        return state.dataCurveManager->curves(CurveIndex)->value(state, Var1, Var2, Var3);
    }

    Real64 CurveValue(EnergyPlusData &state,
                      int const CurveIndex, // index of curve in curve array
                      Real64 const Var1,    // 1st independent variable
                      Real64 const Var2,    // 2nd independent variable
                      Real64 const Var3,    // 3rd independent variable
                      Real64 const Var4     // 4th independent variable
    )
    {
        return state.dataCurveManager->curves(CurveIndex)->value(state, Var1, Var2, Var3, Var4);
    }

    Real64 CurveValue(EnergyPlusData &state,
                      int const CurveIndex, // index of curve in curve array
                      Real64 const Var1,    // 1st independent variable
                      Real64 const Var2,    // 2nd independent variable
                      Real64 const Var3,    // 3rd independent variable
                      Real64 const Var4,    // 4th independent variable
                      Real64 const Var5     // 5th independent variable
    )
    {
        return state.dataCurveManager->curves(CurveIndex)->value(state, Var1, Var2, Var3, Var4, Var5);
    }

    Real64 CurveValue(EnergyPlusData &state,
                      int const CurveIndex, // index of curve in curve array
                      Real64 const Var1,    // 1st independent variable
                      Real64 const Var2,    // 2nd independent variable
                      Real64 const Var3,    // 3rd independent variable
                      Real64 const Var4,    // 4th independent variable
                      Real64 const Var5,    // 5th independent variable
                      Real64 const Var6     // 6th independent variable
    )
    {
        return state.dataCurveManager->curves(CurveIndex)->value(state, Var1, Var2, Var3, Var4, Var5, Var6);
    }

    Real64 Curve::valueFallback(EnergyPlusData &state, Real64 V1, Real64 V2, Real64 V3, Real64 V4, Real64 V5)
    {
        if (state.dataCurveManager->showFallbackMessage) {
            ShowMessage(state, "Note: You have encountered a corner case in the EnergyPlus Curve:* evaluation code.");
            ShowMessage(state, "The code was refactored for version 23.1, but there were a few corner cases that could not be found automatically");
            ShowMessage(state,
                        "If you are able, please provide your input file to the EnergyPlus helpdesk or repository so a developer can patch for your "
                        "use case");
            ShowMessage(state, "Your simulation continues as normal, thanks!");
            state.dataCurveManager->showFallbackMessage = false;
        }
        switch (this->curveType) {
        case CurveType::Linear: {
            return this->coeff[0] + V1 * this->coeff[1];
        } break;
        case CurveType::Quadratic: {
            return this->coeff[0] + V1 * (this->coeff[1] + V1 * this->coeff[2]);
        } break;
        case CurveType::QuadLinear: {
            return this->coeff[0] + V1 * this->coeff[1] + V2 * this->coeff[2] + V3 * this->coeff[3] + V4 * this->coeff[4];
        } break;
        case CurveType::QuintLinear: {
            return this->coeff[0] + V1 * this->coeff[1] + V2 * this->coeff[2] + V3 * this->coeff[3] + V4 * this->coeff[4] + V5 * this->coeff[5];
        } break;
        case CurveType::Cubic: {
            return this->coeff[0] + V1 * (this->coeff[1] + V1 * (this->coeff[2] + V1 * this->coeff[3]));
        } break;
        case CurveType::Quartic: {
            return this->coeff[0] + V1 * (this->coeff[1] + V1 * (this->coeff[2] + V1 * (this->coeff[3] + V1 * this->coeff[4])));
        } break;
        case CurveType::BiQuadratic: {
            return this->coeff[0] + V1 * (this->coeff[1] + V1 * this->coeff[2]) + V2 * (this->coeff[3] + V2 * this->coeff[4]) +
                   V1 * V2 * this->coeff[5];
        } break;
        case CurveType::QuadraticLinear: {
            return (this->coeff[0] + V1 * (this->coeff[1] + V1 * this->coeff[2])) +
                   (this->coeff[3] + V1 * (this->coeff[4] + V1 * this->coeff[5])) * V2;
        } break;
        case CurveType::CubicLinear: {
            return (this->coeff[0] + V1 * (this->coeff[1] + V1 * (this->coeff[2] + V1 * this->coeff[3]))) +
                   (this->coeff[4] + V1 * this->coeff[5]) * V2;
        } break;
        case CurveType::BiCubic: {
            return this->coeff[0] + V1 * this->coeff[1] + V1 * V1 * this->coeff[2] + V2 * this->coeff[3] + V2 * V2 * this->coeff[4] +
                   V1 * V2 * this->coeff[5] + V1 * V1 * V1 * this->coeff[6] + V2 * V2 * V2 * this->coeff[7] + V1 * V1 * V2 * this->coeff[8] +
                   V1 * V2 * V2 * this->coeff[9];
        } break;
        case CurveType::ChillerPartLoadWithLift: {
            return this->coeff[0] + this->coeff[1] * V1 + this->coeff[2] * V1 * V1 + this->coeff[3] * V2 + this->coeff[4] * V2 * V2 +
                   this->coeff[5] * V1 * V2 + this->coeff[6] * V1 * V1 * V1 + this->coeff[7] * V2 * V2 * V2 + this->coeff[8] * V1 * V1 * V2 +
                   this->coeff[9] * V1 * V2 * V2 + this->coeff[10] * V1 * V1 * V2 * V2 + this->coeff[11] * V3 * V2 * V2 * V2;
        } break;
        case CurveType::TriQuadratic: {
            auto const &c = this->coeff;
            Real64 const V1s = V1 * V1;
            Real64 const V2s = V2 * V2;
            Real64 const V3s = V3 * V3;
            return c[0] + c[1] * V1s + c[2] * V1 + c[3] * V2s + c[4] * V2 + c[5] * V3s + c[6] * V3 + c[7] * V1s * V2s + c[8] * V1 * V2 +
                   c[9] * V1 * V2s + c[10] * V1s * V2 + c[11] * V1s * V3s + c[12] * V1 * V3 + c[13] * V1 * V3s + c[14] * V1s * V3 +
                   c[15] * V2s * V3s + c[16] * V2 * V3 + c[17] * V2 * V3s + c[18] * V2s * V3 + c[19] * V1s * V2s * V3s + c[20] * V1s * V2s * V3 +
                   c[21] * V1s * V2 * V3s + c[22] * V1 * V2s * V3s + c[23] * V1s * V2 * V3 + c[24] * V1 * V2s * V3 + c[25] * V1 * V2 * V3s +
                   c[26] * V1 * V2 * V3;
        } break;
        case CurveType::Exponent: {
            return this->coeff[0] + this->coeff[1] * std::pow(V1, this->coeff[2]);
        } break;
        case CurveType::FanPressureRise: {
            return V1 * (this->coeff[0] * V1 + this->coeff[1] + this->coeff[2] * std::sqrt(V2)) + this->coeff[3] * V2;
        } break;
        case CurveType::ExponentialSkewNormal: {
            Real64 const CoeffZ1 = (V1 - this->coeff[0]) / this->coeff[1];
            Real64 const CoeffZ2 = (this->coeff[3] * V1 * std::exp(this->coeff[2] * V1) - this->coeff[0]) / this->coeff[1];
            Real64 const CoeffZ3 = -this->coeff[0] / this->coeff[1];
            Real64 const sqrt_2_inv(1.0 / std::sqrt(2.0));
            Real64 const CurveValueNumer =
                std::exp(-0.5 * (CoeffZ1 * CoeffZ1)) * (1.0 + sign(1.0, CoeffZ2) * std::erf(std::abs(CoeffZ2) * sqrt_2_inv));
            Real64 const CurveValueDenom =
                std::exp(-0.5 * (CoeffZ3 * CoeffZ3)) * (1.0 + sign(1.0, CoeffZ3) * std::erf(std::abs(CoeffZ3) * sqrt_2_inv));
            return CurveValueNumer / CurveValueDenom;
        } break;
        case CurveType::Sigmoid: {
            Real64 const CurveValueExp = std::exp((this->coeff[2] - V1) / this->coeff[3]);
            return this->coeff[0] + this->coeff[1] / std::pow(1.0 + CurveValueExp, this->coeff[4]);
        } break;
        case CurveType::RectangularHyperbola1: {
            Real64 const CurveValueNumer = this->coeff[0] * V1;
            Real64 const CurveValueDenom = this->coeff[1] + V1;
            return (CurveValueNumer / CurveValueDenom) + this->coeff[2];
        } break;
        case CurveType::RectangularHyperbola2: {
            Real64 const CurveValueNumer = this->coeff[0] * V1;
            Real64 const CurveValueDenom = this->coeff[1] + V1;
            return (CurveValueNumer / CurveValueDenom) + (this->coeff[2] * V1);
        } break;
        case CurveType::ExponentialDecay: {
            return this->coeff[0] + this->coeff[1] * std::exp(this->coeff[2] * V1);
        } break;
        case CurveType::DoubleExponentialDecay: {
            return this->coeff[0] + this->coeff[1] * std::exp(this->coeff[2] * V1) + this->coeff[3] * std::exp(this->coeff[4] * V1);
        } break;
        default: {
            return 0.0;
        } break;
        }
    }

    Curve *AddCurve(EnergyPlusData &state, std::string const &name)
    {
        auto *curve = new Curve;
        curve->Name = name;
        state.dataCurveManager->curves.push_back(curve);
        curve->Num = state.dataCurveManager->curves.size();
        state.dataCurveManager->curveMap.insert_or_assign(Util::makeUPPER(curve->Name), curve->Num);
        return curve;
    }

    void GetCurveInput(EnergyPlusData &state)
    {
        // wrapper for GetInput to allow unit testing when fatal inputs are detected - follow pattern from GetSetPointManagerInputs()
        bool GetInputErrorsFound = false;

        GetCurveInputData(state, GetInputErrorsFound);

        if (GetInputErrorsFound) {
            ShowFatalError(state, "GetCurveInput: Errors found in getting Curve Objects.  Preceding condition(s) cause termination.");
        }
    }

    void GetCurveInputData(EnergyPlusData &state, bool &ErrorsFound)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   May 2000
        //       MODIFIED       January 2006, Rick Strand, added a curve type (quadratic-linear)
        //                      July 2006, L. Gu, added a curve type (bicubic)
        //                      July 2006, BG added triquadratic.
        //                      April 2008, LL Added Linear Curve; July 2008, restructure for easier renaming
        //                      Feb 2009, R. Raustad - FSEC, added exponent curve
        //                      22Aug2010 Craig Wray, added new curves for fan component model:
        //                          FanPressureRise, ExponentialSkewNormal, Sigmoid, RectangularHyperbola1,
        //                          RectangularHyperbola2, ExponentialDecay
        //                      Aug.  2014, Rongpeng Zhang, added a new curve type (ChillerPartLoadWithLift)
        //                      Jan. 2017, Jason DeGraw, added WPC input into tables
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains input data for EnergyPlus equipment performance curves

        // METHODOLOGY EMPLOYED:
        // Uses "Get" routines to read in data.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        constexpr std::string_view routineName = "GetCurveInputData";

        Array1D_string Alphas(14);       // Alpha items for object
        Array1D<Real64> Numbers(10000);  // Numeric items for object
        int NumAlphas;                   // Number of Alphas for each GetObjectItem call
        int NumNumbers;                  // Number of Numbers for each GetObjectItem call
        int IOStatus;                    // Used in GetObjectItem
        std::string CurrentModuleObject; // for ease in renaming.

        // Find the number of each type of curve (note: Current Module object not used here, must rename manually)

        int const NumBiQuad = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Curve:Biquadratic");
        int const NumCubic = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Curve:Cubic");
        int const NumQuartic = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Curve:Quartic");
        int const NumQuad = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Curve:Quadratic");
        int const NumQLinear = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Curve:QuadLinear");
        int const NumQuintLinear = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Curve:QuintLinear");
        int const NumQuadLinear = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Curve:QuadraticLinear");
        int const NumCubicLinear = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Curve:CubicLinear");
        int const NumLinear = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Curve:Linear");
        int const NumBicubic = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Curve:Bicubic");
        int const NumTriQuad = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Curve:Triquadratic");
        int const NumExponent = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Curve:Exponent");
        int const NumTableLookup = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Table:Lookup");
        int const NumFanPressRise = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Curve:FanPressureRise");
        int const NumExpSkewNorm = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Curve:ExponentialSkewNormal");
        int const NumSigmoid = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Curve:Sigmoid");
        int const NumRectHyper1 = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Curve:RectangularHyperbola1");
        int const NumRectHyper2 = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Curve:RectangularHyperbola2");
        int const NumExpDecay = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Curve:ExponentialDecay");
        int const NumDoubleExpDecay = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Curve:DoubleExponentialDecay");
        int const NumChillerPartLoadWithLift =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Curve:ChillerPartLoadWithLift"); // zrp_Aug2014

        int const NumWPCValTab =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirflowNetwork:MultiZone:WindPressureCoefficientValues");

        // state.dataCurveManager->NumCurves = NumBiQuad + NumCubic + NumQuad + NumQuadLinear + NumCubicLinear + NumLinear + NumBicubic + NumTriQuad +
        //     NumExponent + NumQuartic + NumTableLookup + NumFanPressRise + NumExpSkewNorm + NumSigmoid + NumRectHyper1 + NumRectHyper2 + NumExpDecay
        //     + NumDoubleExpDecay + NumQLinear + NumQuintLinear + NumChillerPartLoadWithLift + NumWPCValTab;

        // Loop over biquadratic curves and load data
        CurrentModuleObject = "Curve:Biquadratic";
        for (int CurveIndex = 1; CurveIndex <= NumBiQuad; ++CurveIndex) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     CurveIndex,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     _,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (state.dataCurveManager->curveMap.find(Alphas(1)) != state.dataCurveManager->curveMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
            }

            auto *thisCurve = AddCurve(state, Alphas(1));

            // could add checks for blank numeric fields, and use field names for errors.
            thisCurve->curveType = CurveType::BiQuadratic;
            thisCurve->numDims = 2;
            for (int in = 0; in < 6; ++in) {
                thisCurve->coeff[in] = Numbers(in + 1);
            }
            thisCurve->inputLimits[0].min = Numbers(7);
            thisCurve->inputLimits[0].max = Numbers(8);
            thisCurve->inputLimits[1].min = Numbers(9);
            thisCurve->inputLimits[1].max = Numbers(10);
            if (NumNumbers > 10 && !state.dataIPShortCut->lNumericFieldBlanks(11)) {
                thisCurve->outputLimits.min = Numbers(11);
                thisCurve->outputLimits.minPresent = true;
            }
            if (NumNumbers > 11 && !state.dataIPShortCut->lNumericFieldBlanks(12)) {
                thisCurve->outputLimits.max = Numbers(12);
                thisCurve->outputLimits.maxPresent = true;
            }

            if (Numbers(7) > Numbers(8)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(7),
                                         Numbers(7),
                                         state.dataIPShortCut->cNumericFieldNames(8),
                                         Numbers(8)));
                ErrorsFound = true;
            }
            if (Numbers(9) > Numbers(10)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(9),
                                         Numbers(9),
                                         state.dataIPShortCut->cNumericFieldNames(10),
                                         Numbers(10)));
                ErrorsFound = true;
            }
            if (NumAlphas >= 2) {
                if (!IsCurveInputTypeValid(Alphas(2))) {
                    ShowWarningError(state, format("In {} named {} the Input Unit Type for X is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
            if (NumAlphas >= 3) {
                if (!IsCurveInputTypeValid(Alphas(3))) {
                    ShowWarningError(state, format("In {} named {} the Input Unit Type for Y is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
            if (NumAlphas >= 4) {
                if (!IsCurveOutputTypeValid(Alphas(4))) {
                    ShowWarningError(state, format("In {} named {} the Output Unit Type is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
        }

        // Loop over ChillerPartLoadWithLift curves and load data //zrp_Aug2014
        CurrentModuleObject = "Curve:ChillerPartLoadWithLift";
        for (int CurveIndex = 1; CurveIndex <= NumChillerPartLoadWithLift; ++CurveIndex) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     CurveIndex,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     _,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (state.dataCurveManager->curveMap.find(Alphas(1)) != state.dataCurveManager->curveMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
            }

            auto *thisCurve = AddCurve(state, Alphas(1));

            thisCurve->curveType = CurveType::ChillerPartLoadWithLift;
            thisCurve->numDims = 3;

            for (int in = 0; in < 12; ++in) {
                thisCurve->coeff[in] = Numbers(in + 1);
            }

            thisCurve->inputLimits[0].min = Numbers(13);
            thisCurve->inputLimits[0].max = Numbers(14);
            if (Numbers(13) > Numbers(14)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(13),
                                         Numbers(13),
                                         state.dataIPShortCut->cNumericFieldNames(14),
                                         Numbers(14)));
                ErrorsFound = true;
            }

            thisCurve->inputLimits[1].min = Numbers(15);
            thisCurve->inputLimits[1].max = Numbers(16);
            if (Numbers(15) > Numbers(16)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(15),
                                         Numbers(15),
                                         state.dataIPShortCut->cNumericFieldNames(16),
                                         Numbers(16)));
                ErrorsFound = true;
            }

            thisCurve->inputLimits[2].min = Numbers(17);
            thisCurve->inputLimits[2].max = Numbers(18);
            if (Numbers(17) > Numbers(18)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(17),
                                         Numbers(17),
                                         state.dataIPShortCut->cNumericFieldNames(18),
                                         Numbers(18)));
                ErrorsFound = true;
            }

            if (NumNumbers > 18 && !state.dataIPShortCut->lNumericFieldBlanks(19)) {
                thisCurve->outputLimits.min = Numbers(19);
                thisCurve->outputLimits.minPresent = true;
            }
            if (NumNumbers > 19 && !state.dataIPShortCut->lNumericFieldBlanks(20)) {
                thisCurve->outputLimits.max = Numbers(20);
                thisCurve->outputLimits.maxPresent = true;
            }

            if (NumAlphas >= 2) {
                if (!IsCurveInputTypeValid(Alphas(2))) {
                    ShowWarningError(state, format("In {} named {} the Input Unit Type for X is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
            if (NumAlphas >= 3) {
                if (!IsCurveInputTypeValid(Alphas(3))) {
                    ShowWarningError(state, format("In {} named {} the Input Unit Type for Y is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
            if (NumAlphas >= 4) {
                if (!IsCurveInputTypeValid(Alphas(4))) {
                    ShowWarningError(state, format("In {} named {} the Input Unit Type for Z is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
            if (NumAlphas >= 5) {
                if (!IsCurveOutputTypeValid(Alphas(5))) {
                    ShowWarningError(state, format("In {} named {} the Output Unit Type is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
        }

        // Loop over cubic curves and load data
        CurrentModuleObject = "Curve:Cubic";
        for (int CurveIndex = 1; CurveIndex <= NumCubic; ++CurveIndex) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     CurveIndex,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     _,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (state.dataCurveManager->curveMap.find(Alphas(1)) != state.dataCurveManager->curveMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
            }

            auto *thisCurve = AddCurve(state, Alphas(1));

            thisCurve->curveType = CurveType::Cubic;
            thisCurve->numDims = 1;
            for (int in = 0; in < 4; ++in) {
                thisCurve->coeff[in] = Numbers(in + 1);
            }
            thisCurve->inputLimits[0].min = Numbers(5);
            thisCurve->inputLimits[0].max = Numbers(6);
            if (NumNumbers > 6 && !state.dataIPShortCut->lNumericFieldBlanks(7)) {
                thisCurve->outputLimits.min = Numbers(7);
                thisCurve->outputLimits.minPresent = true;
            }
            if (NumNumbers > 7 && !state.dataIPShortCut->lNumericFieldBlanks(8)) {
                thisCurve->outputLimits.max = Numbers(8);
                thisCurve->outputLimits.maxPresent = true;
            }

            if (Numbers(5) > Numbers(6)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(5),
                                         Numbers(5),
                                         state.dataIPShortCut->cNumericFieldNames(6),
                                         Numbers(6)));
                ErrorsFound = true;
            }
            if (NumAlphas >= 2) {
                if (!IsCurveInputTypeValid(Alphas(2))) {
                    ShowWarningError(state, format("In {} named {} the Input Unit Type for X is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
            if (NumAlphas >= 3) {
                if (!IsCurveOutputTypeValid(Alphas(3))) {
                    ShowWarningError(state, format("In {} named {} the Output Unit Type is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
        }

        // Loop over quadrinomial curves and load data
        CurrentModuleObject = "Curve:Quartic";
        for (int CurveIndex = 1; CurveIndex <= NumQuartic; ++CurveIndex) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     CurveIndex,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     _,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (state.dataCurveManager->curveMap.find(Alphas(1)) != state.dataCurveManager->curveMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
            }

            auto *thisCurve = AddCurve(state, Alphas(1));

            thisCurve->curveType = CurveType::Quartic;
            thisCurve->numDims = 1;
            for (int in = 0; in < 5; ++in) {
                thisCurve->coeff[in] = Numbers(in + 1);
            }
            thisCurve->inputLimits[0].min = Numbers(6);
            thisCurve->inputLimits[0].max = Numbers(7);
            if (NumNumbers > 7 && !state.dataIPShortCut->lNumericFieldBlanks(8)) {
                thisCurve->outputLimits.min = Numbers(8);
                thisCurve->outputLimits.minPresent = true;
            }
            if (NumNumbers > 8 && !state.dataIPShortCut->lNumericFieldBlanks(9)) {
                thisCurve->outputLimits.max = Numbers(9);
                thisCurve->outputLimits.maxPresent = true;
            }

            if (Numbers(6) > Numbers(7)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(6),
                                         Numbers(6),
                                         state.dataIPShortCut->cNumericFieldNames(7),
                                         Numbers(7)));
                ErrorsFound = true;
            }
            if (NumAlphas >= 2) {
                if (!IsCurveInputTypeValid(Alphas(2))) {
                    ShowWarningError(state, format("In {} named {} the Input Unit Type for X is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
            if (NumAlphas >= 3) {
                if (!IsCurveOutputTypeValid(Alphas(3))) {
                    ShowWarningError(state, format("In {} named {} the Output Unit Type is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
        }

        // Loop over quadratic curves and load data
        CurrentModuleObject = "Curve:Quadratic";
        for (int CurveIndex = 1; CurveIndex <= NumQuad; ++CurveIndex) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     CurveIndex,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     _,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (state.dataCurveManager->curveMap.find(Alphas(1)) != state.dataCurveManager->curveMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
            }

            auto *thisCurve = AddCurve(state, Alphas(1));

            thisCurve->curveType = CurveType::Quadratic;
            thisCurve->numDims = 1;
            for (int in = 0; in < 3; ++in) {
                thisCurve->coeff[in] = Numbers(in + 1);
            }
            thisCurve->inputLimits[0].min = Numbers(4);
            thisCurve->inputLimits[0].max = Numbers(5);
            if (NumNumbers > 5 && !state.dataIPShortCut->lNumericFieldBlanks(6)) {
                thisCurve->outputLimits.min = Numbers(6);
                thisCurve->outputLimits.minPresent = true;
            }
            if (NumNumbers > 6 && !state.dataIPShortCut->lNumericFieldBlanks(7)) {
                thisCurve->outputLimits.max = Numbers(7);
                thisCurve->outputLimits.maxPresent = true;
            }

            if (Numbers(4) > Numbers(5)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(4),
                                         Numbers(4),
                                         state.dataIPShortCut->cNumericFieldNames(5),
                                         Numbers(5)));
                ErrorsFound = true;
            }
            if (NumAlphas >= 2) {
                if (!IsCurveInputTypeValid(Alphas(2))) {
                    ShowWarningError(state, format("In {} named {} the Input Unit Type for X is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
            if (NumAlphas >= 3) {
                if (!IsCurveOutputTypeValid(Alphas(3))) {
                    ShowWarningError(state, format("In {} named {} the Output Unit Type is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
        }

        // Loop over quadratic-linear curves and load data
        CurrentModuleObject = "Curve:QuadraticLinear";
        for (int CurveIndex = 1; CurveIndex <= NumQuadLinear; ++CurveIndex) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     CurveIndex,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     _,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (state.dataCurveManager->curveMap.find(Alphas(1)) != state.dataCurveManager->curveMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
            }

            auto *thisCurve = AddCurve(state, Alphas(1));

            thisCurve->curveType = CurveType::QuadraticLinear;
            thisCurve->numDims = 2;
            for (int in = 0; in < 6; ++in) {
                thisCurve->coeff[in] = Numbers(in + 1);
            }
            thisCurve->inputLimits[0].min = Numbers(7);
            thisCurve->inputLimits[0].max = Numbers(8);
            thisCurve->inputLimits[1].min = Numbers(9);
            thisCurve->inputLimits[1].max = Numbers(10);
            if (NumNumbers > 10 && !state.dataIPShortCut->lNumericFieldBlanks(11)) {
                thisCurve->outputLimits.min = Numbers(11);
                thisCurve->outputLimits.minPresent = true;
            }
            if (NumNumbers > 11 && !state.dataIPShortCut->lNumericFieldBlanks(12)) {
                thisCurve->outputLimits.max = Numbers(12);
                thisCurve->outputLimits.maxPresent = true;
            }

            if (Numbers(7) > Numbers(8)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(7),
                                         Numbers(7),
                                         state.dataIPShortCut->cNumericFieldNames(8),
                                         Numbers(8)));
                ErrorsFound = true;
            }
            if (Numbers(9) > Numbers(10)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(9),
                                         Numbers(9),
                                         state.dataIPShortCut->cNumericFieldNames(10),
                                         Numbers(10)));
                ErrorsFound = true;
            }
            if (NumAlphas >= 2) {
                if (!IsCurveInputTypeValid(Alphas(2))) {
                    ShowWarningError(state, format("In {} named {} the Input Unit Type for X is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
            if (NumAlphas >= 3) {
                if (!IsCurveInputTypeValid(Alphas(3))) {
                    ShowWarningError(state, format("In {} named {} the Input Unit Type for Y is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
            if (NumAlphas >= 4) {
                if (!IsCurveOutputTypeValid(Alphas(4))) {
                    ShowWarningError(state, format("In {} named {} the Output Unit Type is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
        }

        // Loop over cubic-linear curves and load data
        CurrentModuleObject = "Curve:CubicLinear";
        for (int CurveIndex = 1; CurveIndex <= NumCubicLinear; ++CurveIndex) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     CurveIndex,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     _,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (state.dataCurveManager->curveMap.find(Alphas(1)) != state.dataCurveManager->curveMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
            }

            auto *thisCurve = AddCurve(state, Alphas(1));

            thisCurve->curveType = CurveType::CubicLinear;
            thisCurve->numDims = 2;
            for (int in = 0; in < 6; ++in) {
                thisCurve->coeff[in] = Numbers(in + 1);
            }
            thisCurve->inputLimits[0].min = Numbers(7);
            thisCurve->inputLimits[0].max = Numbers(8);
            thisCurve->inputLimits[1].min = Numbers(9);
            thisCurve->inputLimits[1].max = Numbers(10);
            if (NumNumbers > 10 && !state.dataIPShortCut->lNumericFieldBlanks(11)) {
                thisCurve->outputLimits.min = Numbers(11);
                thisCurve->outputLimits.minPresent = true;
            }
            if (NumNumbers > 11 && !state.dataIPShortCut->lNumericFieldBlanks(12)) {
                thisCurve->outputLimits.max = Numbers(12);
                thisCurve->outputLimits.maxPresent = true;
            }

            if (Numbers(7) > Numbers(8)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(7),
                                         Numbers(7),
                                         state.dataIPShortCut->cNumericFieldNames(8),
                                         Numbers(8)));
                ErrorsFound = true;
            }
            if (Numbers(9) > Numbers(10)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(9),
                                         Numbers(9),
                                         state.dataIPShortCut->cNumericFieldNames(10),
                                         Numbers(10)));
                ErrorsFound = true;
            }
            if (NumAlphas >= 2) {
                if (!IsCurveInputTypeValid(Alphas(2))) {
                    ShowWarningError(state, format("In {} named {} the Input Unit Type for X is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
            if (NumAlphas >= 3) {
                if (!IsCurveInputTypeValid(Alphas(3))) {
                    ShowWarningError(state, format("In {} named {} the Input Unit Type for Y is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
            if (NumAlphas >= 4) {
                if (!IsCurveOutputTypeValid(Alphas(4))) {
                    ShowWarningError(state, format("In {} named {} the Output Unit Type is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
        }

        // Loop over linear curves and load data
        CurrentModuleObject = "Curve:Linear";
        for (int CurveIndex = 1; CurveIndex <= NumLinear; ++CurveIndex) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     CurveIndex,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     _,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (state.dataCurveManager->curveMap.find(Alphas(1)) != state.dataCurveManager->curveMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
            }

            auto *thisCurve = AddCurve(state, Alphas(1));

            thisCurve->curveType = CurveType::Linear;
            thisCurve->numDims = 1;
            for (int in = 0; in < 2; ++in) {
                thisCurve->coeff[in] = Numbers(in + 1);
            }
            thisCurve->inputLimits[0].min = Numbers(3);
            thisCurve->inputLimits[0].max = Numbers(4);
            if (NumNumbers > 4 && !state.dataIPShortCut->lNumericFieldBlanks(5)) {
                thisCurve->outputLimits.min = Numbers(5);
                thisCurve->outputLimits.minPresent = true;
            }
            if (NumNumbers > 5 && !state.dataIPShortCut->lNumericFieldBlanks(6)) {
                thisCurve->outputLimits.max = Numbers(6);
                thisCurve->outputLimits.maxPresent = true;
            }

            if (Numbers(3) > Numbers(4)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(3),
                                         Numbers(3),
                                         state.dataIPShortCut->cNumericFieldNames(4),
                                         Numbers(4)));
                ErrorsFound = true;
            }
            if (NumAlphas >= 2) {
                if (!IsCurveInputTypeValid(Alphas(2))) {
                    ShowWarningError(state, format("In {} named {} the Input Unit Type for X is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
            if (NumAlphas >= 3) {
                if (!IsCurveOutputTypeValid(Alphas(3))) {
                    ShowWarningError(state, format("In {} named {} the Output Unit Type is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
        }

        // Loop over bicubic curves and load data
        CurrentModuleObject = "Curve:Bicubic";
        for (int CurveIndex = 1; CurveIndex <= NumBicubic; ++CurveIndex) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     CurveIndex,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     _,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (state.dataCurveManager->curveMap.find(Alphas(1)) != state.dataCurveManager->curveMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
            }

            auto *thisCurve = AddCurve(state, Alphas(1));

            thisCurve->curveType = CurveType::BiCubic;
            thisCurve->numDims = 2;
            for (int in = 0; in < 10; ++in) {
                thisCurve->coeff[in] = Numbers(in + 1);
            }
            thisCurve->inputLimits[0].min = Numbers(11);
            thisCurve->inputLimits[0].max = Numbers(12);
            thisCurve->inputLimits[1].min = Numbers(13);
            thisCurve->inputLimits[1].max = Numbers(14);
            if (NumNumbers > 14 && !state.dataIPShortCut->lNumericFieldBlanks(15)) {
                thisCurve->outputLimits.min = Numbers(15);
                thisCurve->outputLimits.minPresent = true;
            }
            if (NumNumbers > 15 && !state.dataIPShortCut->lNumericFieldBlanks(16)) {
                thisCurve->outputLimits.max = Numbers(16);
                thisCurve->outputLimits.maxPresent = true;
            }

            if (Numbers(11) > Numbers(12)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(11),
                                         Numbers(11),
                                         state.dataIPShortCut->cNumericFieldNames(12),
                                         Numbers(12)));
                ErrorsFound = true;
            }
            if (Numbers(13) > Numbers(14)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(13),
                                         Numbers(13),
                                         state.dataIPShortCut->cNumericFieldNames(14),
                                         Numbers(14)));
                ErrorsFound = true;
            }
            if (NumAlphas >= 2) {
                if (!IsCurveInputTypeValid(Alphas(2))) {
                    ShowWarningError(state, format("In {} named {} the Input Unit Type for X is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
            if (NumAlphas >= 3) {
                if (!IsCurveInputTypeValid(Alphas(3))) {
                    ShowWarningError(state, format("In {} named {} the Input Unit Type for Y is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
            if (NumAlphas >= 4) {
                if (!IsCurveOutputTypeValid(Alphas(4))) {
                    ShowWarningError(state, format("In {} named {} the Output Unit Type is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
        }

        // Loop over Triquadratic curves and load data
        CurrentModuleObject = "Curve:Triquadratic";
        for (int CurveIndex = 1; CurveIndex <= NumTriQuad; ++CurveIndex) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     CurveIndex,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     _,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (state.dataCurveManager->curveMap.find(Alphas(1)) != state.dataCurveManager->curveMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
            }

            auto *thisCurve = AddCurve(state, Alphas(1));

            thisCurve->curveType = CurveType::TriQuadratic;
            thisCurve->numDims = 3;
            thisCurve->coeff[0] = Numbers(1);
            thisCurve->coeff[1] = Numbers(2);
            thisCurve->coeff[2] = Numbers(3);
            thisCurve->coeff[3] = Numbers(4);
            thisCurve->coeff[4] = Numbers(5);
            thisCurve->coeff[5] = Numbers(6);
            thisCurve->coeff[6] = Numbers(7);
            thisCurve->coeff[7] = Numbers(8);
            thisCurve->coeff[8] = Numbers(9);
            thisCurve->coeff[9] = Numbers(10);
            thisCurve->coeff[10] = Numbers(11);
            thisCurve->coeff[11] = Numbers(12);
            thisCurve->coeff[12] = Numbers(13);
            thisCurve->coeff[13] = Numbers(14);
            thisCurve->coeff[14] = Numbers(15);
            thisCurve->coeff[15] = Numbers(16);
            thisCurve->coeff[16] = Numbers(17);
            thisCurve->coeff[17] = Numbers(18);
            thisCurve->coeff[18] = Numbers(19);
            thisCurve->coeff[19] = Numbers(20);
            thisCurve->coeff[20] = Numbers(21);
            thisCurve->coeff[21] = Numbers(22);
            thisCurve->coeff[22] = Numbers(23);
            thisCurve->coeff[23] = Numbers(24);
            thisCurve->coeff[24] = Numbers(25);
            thisCurve->coeff[25] = Numbers(26);
            thisCurve->coeff[26] = Numbers(27);
            thisCurve->inputLimits[0].min = Numbers(28);
            thisCurve->inputLimits[0].max = Numbers(29);
            thisCurve->inputLimits[1].min = Numbers(30);
            thisCurve->inputLimits[1].max = Numbers(31);
            thisCurve->inputLimits[2].min = Numbers(32);
            thisCurve->inputLimits[2].max = Numbers(33);
            if (NumNumbers > 33 && !state.dataIPShortCut->lNumericFieldBlanks(34)) {
                thisCurve->outputLimits.min = Numbers(34);
                thisCurve->outputLimits.minPresent = true;
            }
            if (NumNumbers > 34 && !state.dataIPShortCut->lNumericFieldBlanks(35)) {
                thisCurve->outputLimits.max = Numbers(35);
                thisCurve->outputLimits.maxPresent = true;
            }

            if (Numbers(28) > Numbers(29)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(28),
                                         Numbers(28),
                                         state.dataIPShortCut->cNumericFieldNames(29),
                                         Numbers(29)));
                ErrorsFound = true;
            }
            if (Numbers(30) > Numbers(31)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(30),
                                         Numbers(30),
                                         state.dataIPShortCut->cNumericFieldNames(31),
                                         Numbers(31)));
                ErrorsFound = true;
            }
            if (Numbers(32) > Numbers(33)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(32),
                                         Numbers(32),
                                         state.dataIPShortCut->cNumericFieldNames(33),
                                         Numbers(33)));
                ErrorsFound = true;
            }
            if (NumAlphas >= 2) {
                if (!IsCurveInputTypeValid(Alphas(2))) {
                    ShowWarningError(state, format("In {} named {} the Input Unit Type for X is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
            if (NumAlphas >= 3) {
                if (!IsCurveInputTypeValid(Alphas(3))) {
                    ShowWarningError(state, format("In {} named {} the Input Unit Type for Y is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
            if (NumAlphas >= 4) {
                if (!IsCurveInputTypeValid(Alphas(4))) {
                    ShowWarningError(state, format("In {} named {} the Input Unit Type for Z is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
            if (NumAlphas >= 5) {
                if (!IsCurveOutputTypeValid(Alphas(5))) {
                    ShowWarningError(state, format("In {} named {} the Output Unit Type is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
        }

        // Loop over quad linear curves and load data
        CurrentModuleObject = "Curve:QuadLinear";
        for (int CurveIndex = 1; CurveIndex <= NumQLinear; ++CurveIndex) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     CurveIndex,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     _,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (state.dataCurveManager->curveMap.find(Alphas(1)) != state.dataCurveManager->curveMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
            }

            auto *thisCurve = AddCurve(state, Alphas(1));

            thisCurve->curveType = CurveType::QuadLinear;
            thisCurve->numDims = 4;
            for (int in = 0; in < 5; ++in) {
                thisCurve->coeff[in] = Numbers(in + 1);
            }
            thisCurve->inputLimits[0].min = Numbers(6);
            thisCurve->inputLimits[0].max = Numbers(7);
            thisCurve->inputLimits[1].min = Numbers(8);
            thisCurve->inputLimits[1].max = Numbers(9);
            thisCurve->inputLimits[2].min = Numbers(10);
            thisCurve->inputLimits[2].max = Numbers(11);
            thisCurve->inputLimits[3].min = Numbers(12);
            thisCurve->inputLimits[3].max = Numbers(13);

            if (NumNumbers > 13 && !state.dataIPShortCut->lNumericFieldBlanks(14)) {
                thisCurve->outputLimits.min = Numbers(14);
                thisCurve->outputLimits.minPresent = true;
            }
            if (NumNumbers > 14 && !state.dataIPShortCut->lNumericFieldBlanks(15)) {
                thisCurve->outputLimits.max = Numbers(15);
                thisCurve->outputLimits.maxPresent = true;
            }

            constexpr int NumVar = 4;
            constexpr std::array<std::string_view, NumVar> VarNames{"w", "x", "y", "z"};
            for (int i = 1; i <= NumVar; ++i) {
                int MinIndex = 2 * i + 4;
                int MaxIndex = MinIndex + 1;
                if (Numbers(MinIndex) > Numbers(MaxIndex)) { // error
                    ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                    ShowContinueError(state,
                                      format("{} [{:.2R}] > {} [{:.2R}]",
                                             state.dataIPShortCut->cNumericFieldNames(MinIndex),
                                             Numbers(MinIndex),
                                             state.dataIPShortCut->cNumericFieldNames(MaxIndex),
                                             Numbers(MaxIndex)));
                    ErrorsFound = true;
                }
                int InputTypeIndex = i + 1;
                if (NumAlphas >= InputTypeIndex) {
                    if (!IsCurveInputTypeValid(Alphas(InputTypeIndex))) {
                        ShowWarningError(
                            state, format("In {} named {} the Input Unit Type for {} is invalid.", CurrentModuleObject, Alphas(1), VarNames[i]));
                    }
                }
            }
            if (NumAlphas >= 6) {
                if (!IsCurveOutputTypeValid(Alphas(6))) {
                    ShowWarningError(state, format("In {} named {} the Output Unit Type is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
        }

        // Loop over quint linear curves and load data
        CurrentModuleObject = "Curve:QuintLinear";
        for (int CurveIndex = 1; CurveIndex <= NumQuintLinear; ++CurveIndex) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     CurveIndex,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     _,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (state.dataCurveManager->curveMap.find(Alphas(1)) != state.dataCurveManager->curveMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
            }

            auto *thisCurve = AddCurve(state, Alphas(1));

            thisCurve->curveType = CurveType::QuintLinear;
            thisCurve->numDims = 5;
            for (int in = 0; in < 6; ++in) {
                thisCurve->coeff[in] = Numbers(in + 1);
            }
            thisCurve->inputLimits[0].min = Numbers(7);
            thisCurve->inputLimits[0].max = Numbers(8);
            thisCurve->inputLimits[1].min = Numbers(9);
            thisCurve->inputLimits[1].max = Numbers(10);
            thisCurve->inputLimits[2].min = Numbers(11);
            thisCurve->inputLimits[2].max = Numbers(12);
            thisCurve->inputLimits[3].min = Numbers(13);
            thisCurve->inputLimits[3].max = Numbers(14);
            thisCurve->inputLimits[4].min = Numbers(15);
            thisCurve->inputLimits[4].max = Numbers(16);
            if (NumNumbers > 16 && !state.dataIPShortCut->lNumericFieldBlanks(17)) {
                thisCurve->outputLimits.min = Numbers(17);
                thisCurve->outputLimits.minPresent = true;
            }
            if (NumNumbers > 17 && !state.dataIPShortCut->lNumericFieldBlanks(18)) {
                thisCurve->outputLimits.max = Numbers(18);
                thisCurve->outputLimits.maxPresent = true;
            }

            constexpr int NumVar = 5;
            constexpr std::array<std::string_view, NumVar> VarNames{"v", "w", "x", "y", "z"};
            for (int i = 1; i <= NumVar; ++i) {
                int MinIndex = 2 * i + 5;
                int MaxIndex = MinIndex + 1;
                if (Numbers(MinIndex) > Numbers(MaxIndex)) { // error
                    ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                    ShowContinueError(state,
                                      format("{} [{:.2R}] > {} [{:.2R}]",
                                             state.dataIPShortCut->cNumericFieldNames(MinIndex),
                                             Numbers(MinIndex),
                                             state.dataIPShortCut->cNumericFieldNames(MaxIndex),
                                             Numbers(MaxIndex)));
                    ErrorsFound = true;
                }
                int InputTypeIndex = i + 1;
                if (NumAlphas >= InputTypeIndex) {
                    if (!IsCurveInputTypeValid(Alphas(InputTypeIndex))) {
                        ShowWarningError(
                            state, format("In {} named {} the Input Unit Type for {} is invalid.", CurrentModuleObject, Alphas(1), VarNames[i]));
                    }
                }
            }
            if (NumAlphas >= 7) {
                if (!IsCurveOutputTypeValid(Alphas(7))) {
                    ShowWarningError(state, format("In {} named {} the Output Unit Type is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
        }

        // Loop over Exponent curves and load data
        CurrentModuleObject = "Curve:Exponent";
        for (int CurveIndex = 1; CurveIndex <= NumExponent; ++CurveIndex) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     CurveIndex,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     _,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (state.dataCurveManager->curveMap.find(Alphas(1)) != state.dataCurveManager->curveMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
            }

            auto *thisCurve = AddCurve(state, Alphas(1));

            thisCurve->curveType = CurveType::Exponent;
            thisCurve->numDims = 1;
            for (int in = 0; in < 3; ++in) {
                thisCurve->coeff[in] = Numbers(in + 1);
            }
            thisCurve->inputLimits[0].min = Numbers(4);
            thisCurve->inputLimits[0].max = Numbers(5);

            if (Numbers(4) > Numbers(5)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(4),
                                         Numbers(4),
                                         state.dataIPShortCut->cNumericFieldNames(5),
                                         Numbers(5)));
                ErrorsFound = true;
            }

            if (NumNumbers > 5 && !state.dataIPShortCut->lNumericFieldBlanks(6)) {
                thisCurve->outputLimits.min = Numbers(6);
                thisCurve->outputLimits.minPresent = true;
            }
            if (NumNumbers > 6 && !state.dataIPShortCut->lNumericFieldBlanks(7)) {
                thisCurve->outputLimits.max = Numbers(7);
                thisCurve->outputLimits.maxPresent = true;
            }
            if (NumAlphas >= 2) {
                if (!IsCurveInputTypeValid(Alphas(2))) {
                    ShowWarningError(state, format("In {} named {} the Input Unit Type for X is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
            if (NumAlphas >= 3) {
                if (!IsCurveOutputTypeValid(Alphas(3))) {
                    ShowWarningError(state, format("In {} named {} the Output Unit Type is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
        }

        // Loop over Fan Pressure Rise curves and load data
        CurrentModuleObject = "Curve:FanPressureRise";
        for (int CurveIndex = 1; CurveIndex <= NumFanPressRise; ++CurveIndex) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     CurveIndex,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     _,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (state.dataCurveManager->curveMap.find(Alphas(1)) != state.dataCurveManager->curveMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
            }

            Curve *thisCurve = AddCurve(state, Alphas(1));

            thisCurve->curveType = CurveType::FanPressureRise;
            thisCurve->numDims = 2;
            for (int in = 0; in < 4; ++in) {
                thisCurve->coeff[in] = Numbers(in + 1);
            }
            thisCurve->inputLimits[0].min = Numbers(5);
            thisCurve->inputLimits[0].max = Numbers(6);
            thisCurve->inputLimits[1].min = Numbers(7);
            thisCurve->inputLimits[1].max = Numbers(8);

            if (NumNumbers > 8 && !state.dataIPShortCut->lNumericFieldBlanks(9)) {
                thisCurve->outputLimits.min = Numbers(9);
                thisCurve->outputLimits.minPresent = true;
            }
            if (NumNumbers > 9 && !state.dataIPShortCut->lNumericFieldBlanks(10)) {
                thisCurve->outputLimits.max = Numbers(10);
                thisCurve->outputLimits.maxPresent = true;
            }

            if (Numbers(5) > Numbers(6)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(5),
                                         Numbers(5),
                                         state.dataIPShortCut->cNumericFieldNames(6),
                                         Numbers(6)));
                ErrorsFound = true;
            }
            if (Numbers(7) > Numbers(8)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(7),
                                         Numbers(7),
                                         state.dataIPShortCut->cNumericFieldNames(8),
                                         Numbers(8)));
                ErrorsFound = true;
            }

        } // Fan Pressure Rise

        // Loop over Exponential Skew Normal curves and load data
        CurrentModuleObject = "Curve:ExponentialSkewNormal";
        for (int CurveIndex = 1; CurveIndex <= NumExpSkewNorm; ++CurveIndex) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     CurveIndex,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     _,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (state.dataCurveManager->curveMap.find(Alphas(1)) != state.dataCurveManager->curveMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
            }

            auto *thisCurve = AddCurve(state, Alphas(1));

            thisCurve->curveType = CurveType::ExponentialSkewNormal;
            thisCurve->numDims = 1;
            for (int in = 0; in < 4; ++in) {
                thisCurve->coeff[in] = Numbers(in + 1);
            }
            thisCurve->inputLimits[0].min = Numbers(5);
            thisCurve->inputLimits[0].max = Numbers(6);

            if (NumNumbers > 6 && !state.dataIPShortCut->lNumericFieldBlanks(7)) {
                thisCurve->outputLimits.min = Numbers(7);
                thisCurve->outputLimits.minPresent = true;
            }
            if (NumNumbers > 7 && !state.dataIPShortCut->lNumericFieldBlanks(8)) {
                thisCurve->outputLimits.max = Numbers(8);
                thisCurve->outputLimits.maxPresent = true;
            }

            if (Numbers(5) > Numbers(6)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(5),
                                         Numbers(5),
                                         state.dataIPShortCut->cNumericFieldNames(6),
                                         Numbers(6)));
                ErrorsFound = true;
            }

            if (NumAlphas >= 2) {
                if (!IsCurveInputTypeValid(Alphas(2))) {
                    ShowWarningError(state, format("In {} named {} the Input Unit Type for X is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
            if (NumAlphas >= 3) {
                if (!IsCurveOutputTypeValid(Alphas(3))) {
                    ShowWarningError(state, format("In {} named {} the Output Unit Type is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
        } // Exponential Skew Normal

        // Loop over Sigmoid curves and load data
        CurrentModuleObject = "Curve:Sigmoid";
        for (int CurveIndex = 1; CurveIndex <= NumSigmoid; ++CurveIndex) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     CurveIndex,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     _,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (state.dataCurveManager->curveMap.find(Alphas(1)) != state.dataCurveManager->curveMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
            }

            auto *thisCurve = AddCurve(state, Alphas(1));

            thisCurve->curveType = CurveType::Sigmoid;
            thisCurve->numDims = 1;
            for (int in = 0; in < 5; ++in) {
                thisCurve->coeff[in] = Numbers(in + 1);
            }
            thisCurve->inputLimits[0].min = Numbers(6);
            thisCurve->inputLimits[0].max = Numbers(7);

            if (NumNumbers > 7 && !state.dataIPShortCut->lNumericFieldBlanks(8)) {
                thisCurve->outputLimits.min = Numbers(8);
                thisCurve->outputLimits.minPresent = true;
            }
            if (NumNumbers > 8 && !state.dataIPShortCut->lNumericFieldBlanks(9)) {
                thisCurve->outputLimits.max = Numbers(9);
                thisCurve->outputLimits.maxPresent = true;
            }

            if (Numbers(6) > Numbers(7)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(6),
                                         Numbers(6),
                                         state.dataIPShortCut->cNumericFieldNames(7),
                                         Numbers(7)));
                ErrorsFound = true;
            }

            if (NumAlphas >= 2) {
                if (!IsCurveInputTypeValid(Alphas(2))) {
                    ShowWarningError(state, format("In {} named {} the Input Unit Type for X is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
            if (NumAlphas >= 3) {
                if (!IsCurveOutputTypeValid(Alphas(3))) {
                    ShowWarningError(state, format("In {} named {} the Output Unit Type is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
        } // Sigmoid

        // Loop over Rectangular Hyperbola Type 1 curves and load data
        CurrentModuleObject = "Curve:RectangularHyperbola1";
        for (int CurveIndex = 1; CurveIndex <= NumRectHyper1; ++CurveIndex) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     CurveIndex,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     _,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (state.dataCurveManager->curveMap.find(Alphas(1)) != state.dataCurveManager->curveMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
            }

            auto *thisCurve = AddCurve(state, Alphas(1));

            thisCurve->curveType = CurveType::RectangularHyperbola1;
            thisCurve->numDims = 1;
            for (int in = 0; in < 3; ++in) {
                thisCurve->coeff[in] = Numbers(in + 1);
            }
            thisCurve->inputLimits[0].min = Numbers(4);
            thisCurve->inputLimits[0].max = Numbers(5);

            if (NumNumbers > 5 && !state.dataIPShortCut->lNumericFieldBlanks(6)) {
                thisCurve->outputLimits.min = Numbers(6);
                thisCurve->outputLimits.minPresent = true;
            }
            if (NumNumbers > 6 && !state.dataIPShortCut->lNumericFieldBlanks(7)) {
                thisCurve->outputLimits.max = Numbers(7);
                thisCurve->outputLimits.maxPresent = true;
            }

            if (Numbers(4) > Numbers(5)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(4),
                                         Numbers(4),
                                         state.dataIPShortCut->cNumericFieldNames(5),
                                         Numbers(5)));
                ErrorsFound = true;
            }

            if (NumAlphas >= 2) {
                if (!IsCurveInputTypeValid(Alphas(2))) {
                    ShowWarningError(state, format("In {} named {} the Input Unit Type for X is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
            if (NumAlphas >= 3) {
                if (!IsCurveOutputTypeValid(Alphas(3))) {
                    ShowWarningError(state, format("In {} named {} the Output Unit Type is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
        } // Rectangular Hyperbola Type 1

        // Loop over Rectangular Hyperbola Type 2 curves and load data
        CurrentModuleObject = "Curve:RectangularHyperbola2";
        for (int CurveIndex = 1; CurveIndex <= NumRectHyper2; ++CurveIndex) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     CurveIndex,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     _,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (state.dataCurveManager->curveMap.find(Alphas(1)) != state.dataCurveManager->curveMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
            }

            auto *thisCurve = AddCurve(state, Alphas(1));

            thisCurve->curveType = CurveType::RectangularHyperbola2;
            thisCurve->numDims = 1;
            for (int in = 0; in < 3; ++in) {
                thisCurve->coeff[in] = Numbers(in + 1);
            }
            thisCurve->inputLimits[0].min = Numbers(4);
            thisCurve->inputLimits[0].max = Numbers(5);

            if (NumNumbers > 5 && !state.dataIPShortCut->lNumericFieldBlanks(6)) {
                thisCurve->outputLimits.min = Numbers(6);
                thisCurve->outputLimits.minPresent = true;
            }
            if (NumNumbers > 6 && !state.dataIPShortCut->lNumericFieldBlanks(7)) {
                thisCurve->outputLimits.max = Numbers(7);
                thisCurve->outputLimits.maxPresent = true;
            }

            if (Numbers(4) > Numbers(5)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(4),
                                         Numbers(4),
                                         state.dataIPShortCut->cNumericFieldNames(5),
                                         Numbers(5)));
                ErrorsFound = true;
            }

            if (NumAlphas >= 2) {
                if (!IsCurveInputTypeValid(Alphas(2))) {
                    ShowWarningError(state, format("In {} named {} the Input Unit Type for X is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
            if (NumAlphas >= 3) {
                if (!IsCurveOutputTypeValid(Alphas(3))) {
                    ShowWarningError(state, format("In {} named {} the Output Unit Type is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
        } // Rectangular Hyperbola Type 2

        // Loop over Exponential Decay curves and load data
        CurrentModuleObject = "Curve:ExponentialDecay";
        for (int CurveIndex = 1; CurveIndex <= NumExpDecay; ++CurveIndex) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     CurveIndex,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     _,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (state.dataCurveManager->curveMap.find(Alphas(1)) != state.dataCurveManager->curveMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
            }

            auto *thisCurve = AddCurve(state, Alphas(1));

            thisCurve->curveType = CurveType::ExponentialDecay;
            thisCurve->numDims = 1;
            for (int in = 0; in < 3; ++in) {
                thisCurve->coeff[in] = Numbers(in + 1);
            }
            thisCurve->inputLimits[0].min = Numbers(4);
            thisCurve->inputLimits[0].max = Numbers(5);

            if (NumNumbers > 5 && !state.dataIPShortCut->lNumericFieldBlanks(6)) {
                thisCurve->outputLimits.min = Numbers(6);
                thisCurve->outputLimits.minPresent = true;
            }
            if (NumNumbers > 6 && !state.dataIPShortCut->lNumericFieldBlanks(7)) {
                thisCurve->outputLimits.max = Numbers(7);
                thisCurve->outputLimits.maxPresent = true;
            }

            if (Numbers(4) > Numbers(5)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(4),
                                         Numbers(4),
                                         state.dataIPShortCut->cNumericFieldNames(5),
                                         Numbers(5)));
                ErrorsFound = true;
            }

            if (NumAlphas >= 2) {
                if (!IsCurveInputTypeValid(Alphas(2))) {
                    ShowWarningError(state, format("In {} named {} the Input Unit Type for X is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
            if (NumAlphas >= 3) {
                if (!IsCurveOutputTypeValid(Alphas(3))) {
                    ShowWarningError(state, format("In {} named {} the Output Unit Type is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
        } // Exponential Decay

        // ykt July,2011 Loop over DoubleExponential Decay curves and load data
        CurrentModuleObject = "Curve:DoubleExponentialDecay";
        for (int CurveIndex = 1; CurveIndex <= NumDoubleExpDecay; ++CurveIndex) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     CurveIndex,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     _,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            if (state.dataCurveManager->curveMap.find(Alphas(1)) != state.dataCurveManager->curveMap.end()) {
                ShowSevereDuplicateName(state, eoh);
                ErrorsFound = true;
            }

            auto *thisCurve = AddCurve(state, Alphas(1));

            thisCurve->curveType = CurveType::DoubleExponentialDecay;
            thisCurve->numDims = 1;
            for (int in = 0; in < 5; ++in) {
                thisCurve->coeff[in] = Numbers(in + 1);
            }
            thisCurve->inputLimits[0].min = Numbers(6);
            thisCurve->inputLimits[0].max = Numbers(7);

            if (Numbers(6) > Numbers(7)) { // error
                ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                ShowContinueError(state,
                                  format("{} [{:.2R}] > {} [{:.2R}]",
                                         state.dataIPShortCut->cNumericFieldNames(6),
                                         Numbers(6),
                                         state.dataIPShortCut->cNumericFieldNames(7),
                                         Numbers(7)));
                ErrorsFound = true;
            }

            if (NumNumbers > 7 && !state.dataIPShortCut->lNumericFieldBlanks(8)) {
                thisCurve->outputLimits.min = Numbers(8);
                thisCurve->outputLimits.minPresent = true;
            }
            if (NumNumbers > 8 && !state.dataIPShortCut->lNumericFieldBlanks(9)) {
                thisCurve->outputLimits.max = Numbers(9);
                thisCurve->outputLimits.maxPresent = true;
            }

            if (NumAlphas >= 2) {
                if (!IsCurveInputTypeValid(Alphas(2))) {
                    ShowWarningError(state, format("In {} named {} the Input Unit Type for X is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
            if (NumAlphas >= 3) {
                if (!IsCurveOutputTypeValid(Alphas(3))) {
                    ShowWarningError(state, format("In {} named {} the Output Unit Type is invalid.", CurrentModuleObject, Alphas(1)));
                }
            }
        } // Exponential Decay

        // Loop over wind pressure coefficient tables and load data
        if (NumWPCValTab > 0) {
            // Get the angle values
            CurrentModuleObject = "AirflowNetwork:MultiZone:WindPressureCoefficientArray";
            int numOfCPArray = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);

            if (numOfCPArray != 1) {
                ShowSevereError(
                    state,
                    format("GetCurveInput: Currently exactly one (\"1\") {} object per simulation is required when using the AirflowNetwork model.",
                           CurrentModuleObject));
                ErrorsFound = true;
            } else {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         CurrentModuleObject,
                                                                         1,
                                                                         Alphas,
                                                                         NumAlphas,
                                                                         Numbers,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         state.dataIPShortCut->lNumericFieldBlanks,
                                                                         _,
                                                                         state.dataIPShortCut->cAlphaFieldNames,
                                                                         state.dataIPShortCut->cNumericFieldNames);

                std::string wpcName = Alphas(1); // Name of CP array
                int numWindDir = NumNumbers;
                std::vector<Real64> windDirs(numWindDir);

                Real64 dirMin = 0;
                Real64 dirMax = 0;
                for (int j = 1; j <= NumNumbers; ++j) { // Wind direction
                    windDirs[j - 1] = Numbers(j);
                    dirMin = std::min(dirMin, Numbers(j));
                    dirMax = std::max(dirMax, Numbers(j));
                    if (j > 1) {
                        if (windDirs[j - 2] >= windDirs[j - 1]) {
                            ShowSevereError(state, format("GetCurveInput: An {} object ", CurrentModuleObject));
                            ShowContinueError(state,
                                              "has either the same values for two consecutive wind directions, or a lower wind direction value after "
                                              "a higher wind direction value.");
                            ShowContinueError(state, "Wind direction values must be entered in ascending order.");
                            ShowContinueError(state,
                                              format("{} = {:.2R} {} = {:.2R}",
                                                     state.dataIPShortCut->cNumericFieldNames(j),
                                                     windDirs[j - 2],
                                                     state.dataIPShortCut->cNumericFieldNames[j + 1],
                                                     windDirs[j - 1]));
                            ErrorsFound = true;
                        }
                    }
                }
                // Check that the first table value is zero
                if (dirMin != 0.0) {
                    ShowSevereError(state, format("GetCurveInput: An {} object ", CurrentModuleObject));
                    ShowContinueError(state, format("has a nonzero minimum value of {:.2R}", dirMin));
                    ShowContinueError(state, "Wind direction values must begin at zero.");
                    ErrorsFound = true;
                }

                // Now that we have the directions, we can read the tables themselves
                CurrentModuleObject = "AirflowNetwork:MultiZone:WindPressureCoefficientValues";
                for (int index = 1; index <= NumWPCValTab; ++index) {
                    state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                             CurrentModuleObject,
                                                                             index,
                                                                             Alphas,
                                                                             NumAlphas,
                                                                             Numbers,
                                                                             NumNumbers,
                                                                             IOStatus,
                                                                             state.dataIPShortCut->lNumericFieldBlanks,
                                                                             _,
                                                                             state.dataIPShortCut->cAlphaFieldNames,
                                                                             state.dataIPShortCut->cNumericFieldNames);
                    ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

                    if (state.dataCurveManager->curveMap.find(Alphas(1)) != state.dataCurveManager->curveMap.end()) {
                        ShowSevereDuplicateName(state, eoh);
                        ErrorsFound = true;
                    }

                    auto *thisCurve = AddCurve(state, Alphas(1));

                    // Ensure the CP array name should be the same as the name of AirflowNetwork:MultiZone:WindPressureCoefficientArray
                    if (!Util::SameString(Alphas(2), wpcName)) {
                        ShowSevereError(state,
                                        format("GetCurveInput: Invalid {} = {} in {} = ",
                                               state.dataIPShortCut->cAlphaFieldNames(2),
                                               Alphas(2),
                                               CurrentModuleObject));
                        ShowContinueError(state, format("The valid name is {}", wpcName));
                        ErrorsFound = true;
                    }

                    thisCurve->numDims = 1;

                    thisCurve->curveType = CurveType::BtwxtTableLookup;

                    thisCurve->contextString = format("Table:Lookup \"{}\"", Alphas(1));
                    std::pair<EnergyPlusData *, std::string> callbackPair{&state, thisCurve->contextString};
                    state.dataCurveManager->btwxtManager.setLoggingContext(&callbackPair);

                    thisCurve->inputLimits[0].min = 0.0;
                    thisCurve->inputLimits[0].minPresent = true;
                    thisCurve->inputLimits[0].max = 360.0;
                    thisCurve->inputLimits[0].maxPresent = true;

                    thisCurve->outputLimits.min = -1.0;
                    thisCurve->outputLimits.minPresent = true;
                    thisCurve->outputLimits.max = 1.0;
                    thisCurve->outputLimits.maxPresent = true;

                    int MaxTableNums = NumNumbers;
                    if (NumNumbers != numWindDir) {
                        ShowSevereError(state, format("GetCurveInput: For {}: ", CurrentModuleObject));
                        ShowContinueError(state,
                                          format("The number of data entries must match the number of wind directions given in the wind pressure "
                                                 "coefficient array. Number of data entries = {}",
                                                 NumNumbers));
                        ErrorsFound = true;
                    } else {
                        std::vector<double> axis;
                        std::vector<double> lookupValues;

                        for (int TableDataIndex = 1; TableDataIndex <= MaxTableNums; ++TableDataIndex) {
                            axis.push_back(windDirs[TableDataIndex - 1]);
                            lookupValues.push_back(Numbers(TableDataIndex));
                        }
                        if (axis[axis.size() - 1] < 360.0) {
                            axis.push_back(360.0);
                            lookupValues.push_back(Numbers(1));
                        }

                        try {
                            std::vector<Btwxt::GridAxis> gridAxes;
                            gridAxes.emplace_back(axis,
                                                  "",
                                                  Btwxt::InterpolationMethod::linear,
                                                  Btwxt::ExtrapolationMethod::linear,
                                                  std::pair<double, double>{0.0, 360.0},
                                                  BtwxtManager::btwxt_logger);

                            auto gridIndex = // (AUTO_OK_OBJ)
                                state.dataCurveManager->btwxtManager.addGrid(Alphas(1), gridAxes);
                            thisCurve->TableIndex = gridIndex;
                            thisCurve->GridValueIndex = state.dataCurveManager->btwxtManager.addOutputValues(gridIndex, lookupValues);
                        } catch (Btwxt::BtwxtException &e) {
                            ShowFatalError(state, "Btwxt::GridAxis construction error; program terminates.");
                        }
                    }
                }
            }
        }

        // Create case insensitive references to independent variable input data
        int numIndVars = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Table:IndependentVariable");
        if (numIndVars > 0) {
            // Set Btwxt Message Callback
            auto const &indVarInstances = state.dataInputProcessing->inputProcessor->getObjectInstances("Table:IndependentVariable");
            for (auto &instance : indVarInstances.items()) {
                auto const &fields = instance.value();
                std::string const &thisObjectName = instance.key();
                state.dataInputProcessing->inputProcessor->markObjectAsUsed("Table:IndependentVariable", thisObjectName);
                state.dataCurveManager->btwxtManager.independentVarRefs.emplace(Util::makeUPPER(thisObjectName), fields);
            }
        }

        // Create GridSpaces from Independent Variable List
        int numIndVarLists = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Table:IndependentVariableList");
        std::map<std::string, std::vector<std::pair<double, double>>>
            varListLimits; // ugly, but this is needed for legacy behavior (otherwise limits are reset by Btwxt if they are within bounds).
        std::map<std::string, std::vector<double>> varListNormalizeTargets;
        if (numIndVarLists > 0) {
            auto const &indVarListInstances = state.dataInputProcessing->inputProcessor->getObjectInstances("Table:IndependentVariableList");
            for (auto &instance : indVarListInstances.items()) {

                auto const &fields = instance.value();
                std::string const &thisObjectName = instance.key();
                state.dataInputProcessing->inputProcessor->markObjectAsUsed("Table:IndependentVariableList", thisObjectName);
                std::string varListName = Util::makeUPPER(thisObjectName);

                std::vector<Btwxt::GridAxis> gridAxes;

                // Loop through independent variables in list and add them to the grid
                for (auto &indVar : fields.at("independent_variables")) {
                    std::string indVarName = Util::makeUPPER(indVar.at("independent_variable_name").get<std::string>());
                    std::string contextString = format("Table:IndependentVariable \"{}\"", indVarName);
                    std::pair<EnergyPlusData *, std::string> callbackPair{&state, contextString};
                    state.dataCurveManager->btwxtManager.setLoggingContext(&callbackPair);

                    // Find independent variable input data
                    if (state.dataCurveManager->btwxtManager.independentVarRefs.count(indVarName)) {
                        // If found, read data
                        auto const &indVarInstance = state.dataCurveManager->btwxtManager.independentVarRefs.at(indVarName);

                        // TODO: Actually use this to define output variable units
                        if (indVarInstance.count("unit_type")) {
                            std::string unitType = indVarInstance.at("unit_type").get<std::string>();
                            if (!IsCurveInputTypeValid(unitType)) {
                                ShowSevereError(state, format("{}: Unit Type [{}] is invalid", contextString, unitType));
                            }
                        }

                        std::vector<double> axis;

                        if (indVarInstance.count("external_file_name")) {
                            std::string tmp = indVarInstance.at("external_file_name").get<std::string>();
                            fs::path filePath(tmp);
                            if (!indVarInstance.count("external_file_column_number")) {
                                ShowSevereError(state, format("{}: No column number defined for external file \"{}\"", contextString, filePath));
                                ErrorsFound = true;
                            }
                            if (!indVarInstance.count("external_file_starting_row_number")) {
                                ShowSevereError(state,
                                                format("{}: No starting row number defined for external file \"{}\"", contextString, filePath));
                                ErrorsFound = true;
                            }

                            std::size_t colNum = indVarInstance.at("external_file_column_number").get<std::size_t>() - 1;
                            std::size_t rowNum = indVarInstance.at("external_file_starting_row_number").get<std::size_t>() - 1;

                            if (!state.dataCurveManager->btwxtManager.tableFiles.count(filePath)) {
                                TableFile tableFile;
                                ErrorsFound |= tableFile.load(state, filePath);
                                state.dataCurveManager->btwxtManager.tableFiles.emplace(filePath, tableFile);
                            }

                            if (ErrorsFound) {
                                continue; // Unable to load file so continue on to see if there are other errors before fataling
                            }

                            axis = state.dataCurveManager->btwxtManager.tableFiles[filePath].getArray(state, {colNum, rowNum});

                            // remove NANs
                            axis.erase(std::remove_if(axis.begin(), axis.end(), [](const double &x) { return std::isnan(x); }), axis.end());

                            // sort
                            std::sort(axis.begin(), axis.end());

                            // remove duplicates
                            axis.erase(std::unique(axis.begin(), axis.end()), axis.end());

                        } else if (indVarInstance.count("values")) {
                            for (auto const &value : indVarInstance.at("values")) {
                                axis.push_back(value.at("value").get<Real64>());
                            }
                        } else {
                            ShowSevereError(state, format("{}: No values defined.", contextString));
                            ErrorsFound = true;
                        }

                        // This could be an enum lookup, but they are accessing enums inside Btwxt that we don't control, and it's only two options
                        // for each
                        Btwxt::InterpolationMethod interpMethod = Btwxt::InterpolationMethod::cubic; // Assume cubic as the default
                        auto interpIterator = indVarInstance.find("interpolation_method");
                        if (interpIterator != indVarInstance.end()) {
                            if (interpIterator->get<std::string>() == "Linear") {
                                interpMethod = Btwxt::InterpolationMethod::linear;
                            }
                        }
                        Btwxt::ExtrapolationMethod extrapMethod = Btwxt::ExtrapolationMethod::linear; // Assume linear as the default
                        auto extrapIterator = indVarInstance.find("extrapolation_method");
                        if (extrapIterator != indVarInstance.end()) {
                            if (extrapIterator->get<std::string>() == "Unavailable") {
                                ShowSevereError(state, format("{}: Extrapolation method \"Unavailable\" is not yet available.", contextString));
                                ErrorsFound = true;
                            } else if (extrapIterator->get<std::string>() == "Constant") {
                                extrapMethod = Btwxt::ExtrapolationMethod::constant;
                            }
                        }

                        double min_grid_value = *std::min_element(axis.begin(), axis.end());
                        double max_grid_value = *std::max_element(axis.begin(), axis.end());

                        auto minValIterator = indVarInstance.find("minimum_value");
                        Real64 min_val = (minValIterator != indVarInstance.end()) ? minValIterator->get<Real64>() : min_grid_value;
                        auto maxValIterator = indVarInstance.find("maximum_value");
                        Real64 max_val = (maxValIterator != indVarInstance.end()) ? maxValIterator->get<Real64>() : max_grid_value;
                        varListLimits[varListName].emplace_back(min_val, max_val);

                        auto normValIterator = indVarInstance.find("normalization_reference_value");
                        Real64 normalizationRefValue =
                            (normValIterator != indVarInstance.end()) ? normValIterator->get<Real64>() : std::numeric_limits<double>::quiet_NaN();

                        varListNormalizeTargets[varListName].push_back(normalizationRefValue);

                        // reset limits passed to Btwxt to avoid warnings related to different handling of limits
                        min_val = min(min_val, min_grid_value);
                        max_val = max(max_val, max_grid_value);

                        gridAxes.emplace_back(
                            axis, "", interpMethod, extrapMethod, std::pair<double, double>{min_val, max_val}, BtwxtManager::btwxt_logger);

                    } else {
                        // Independent variable does not exist
                        ShowSevereError(state, format("{}: No Table:IndependentVariable found.", contextString));
                        ErrorsFound = true;
                    }
                }
                // Add grid to btwxtManager
                state.dataCurveManager->btwxtManager.addGrid(Util::makeUPPER(thisObjectName), gridAxes);
            }
        }

        if (NumTableLookup > 0) {
            auto const &lookupInstances = state.dataInputProcessing->inputProcessor->getObjectInstances("Table:Lookup");
            for (auto &instance : lookupInstances.items()) {

                auto const &fields = instance.value();
                std::string const &thisObjectName = instance.key();
                std::string objNameUC = Util::makeUPPER(thisObjectName);
                state.dataInputProcessing->inputProcessor->markObjectAsUsed("Table:Lookup", thisObjectName);

                ErrorObjectHeader eoh{routineName, "Table:Lookup", thisObjectName};

                if (state.dataCurveManager->curveMap.find(objNameUC) != state.dataCurveManager->curveMap.end()) {
                    ShowSevereDuplicateName(state, eoh);
                    ErrorsFound = true;
                }

                auto *thisCurve = AddCurve(state, objNameUC);
                thisCurve->curveType = CurveType::BtwxtTableLookup;

                std::string indVarListName = Util::makeUPPER(fields.at("independent_variable_list_name").get<std::string>());

                thisCurve->contextString = format("Table:Lookup \"{}\"", thisCurve->Name);
                std::pair<EnergyPlusData *, std::string> callbackPair{&state, thisCurve->contextString};
                state.dataCurveManager->btwxtManager.setLoggingContext(&callbackPair);

                // TODO: Actually use this to define output variable units
                if (fields.count("output_unit_type")) {
                    std::string unitType = fields.at("output_unit_type").get<std::string>();
                    if (!IsCurveOutputTypeValid(unitType)) {
                        ShowSevereError(state, format("{}: Output Unit Type [{}] is invalid", thisCurve->contextString, unitType));
                    }
                }

                int gridIndex = state.dataCurveManager->btwxtManager.getGridIndex(state, indVarListName, ErrorsFound);
                thisCurve->TableIndex = gridIndex;
                int numDims = state.dataCurveManager->btwxtManager.getNumGridDims(gridIndex);
                thisCurve->numDims = numDims;

                for (int i = 1; i <= std::min(6, numDims); ++i) {
                    double vMin, vMax;
                    std::tie(vMin, vMax) = varListLimits.at(indVarListName)[i - 1];
                    if (i == 1) {
                        thisCurve->inputLimits[0].min = vMin;
                        thisCurve->inputLimits[0].max = vMax;
                    } else if (i == 2) {
                        thisCurve->inputLimits[1].min = vMin;
                        thisCurve->inputLimits[1].max = vMax;
                    } else if (i == 3) {
                        thisCurve->inputLimits[2].min = vMin;
                        thisCurve->inputLimits[2].max = vMax;
                    } else if (i == 4) {
                        thisCurve->inputLimits[3].min = vMin;
                        thisCurve->inputLimits[3].max = vMax;
                    } else if (i == 5) {
                        thisCurve->inputLimits[4].min = vMin;
                        thisCurve->inputLimits[4].max = vMax;
                    } else if (i == 6) {
                        thisCurve->inputLimits[5].min = vMin;
                        thisCurve->inputLimits[5].max = vMax;
                    }
                }

                if (fields.count("minimum_output")) {
                    thisCurve->outputLimits.min = fields.at("minimum_output").get<Real64>();
                    thisCurve->outputLimits.minPresent = true;
                } else {
                    thisCurve->outputLimits.min = -DBL_MAX;
                    thisCurve->outputLimits.minPresent = false;
                }

                if (fields.count("maximum_output")) {
                    thisCurve->outputLimits.max = fields.at("maximum_output").get<Real64>();
                    thisCurve->outputLimits.maxPresent = true;
                } else {
                    thisCurve->outputLimits.max = DBL_MAX;
                    thisCurve->outputLimits.maxPresent = false;
                }

                // Normalize data
                Real64 normalizationDivisor = 1.0;
                enum NormalizationMethod
                {
                    NM_NONE,
                    NM_DIVISOR_ONLY,
                    NM_AUTO_WITH_DIVISOR
                };
                NormalizationMethod normalizeMethod = NM_NONE;
                if (fields.count("normalization_method")) {
                    if (Util::SameString(fields.at("normalization_method").get<std::string>(), "DIVISORONLY")) {
                        normalizeMethod = NM_DIVISOR_ONLY;
                    } else if (Util::SameString(fields.at("normalization_method").get<std::string>(), "AUTOMATICWITHDIVISOR")) {
                        normalizeMethod = NM_AUTO_WITH_DIVISOR;
                    }
                }

                if (normalizeMethod != NM_NONE && fields.count("normalization_divisor")) {
                    normalizationDivisor = fields.at("normalization_divisor").get<Real64>();
                    if (std::abs(normalizationDivisor) < std::numeric_limits<Real64>::min()) {
                        ShowSevereError(
                            state, format("Table:Lookup named \"{}\": Normalization divisor entered as zero, which is invalid", thisCurve->Name));
                        ErrorsFound = true;
                        continue;
                    }
                }

                std::vector<double> lookupValues;
                if (fields.count("external_file_name")) {
                    std::string tmp = fields.at("external_file_name").get<std::string>();
                    fs::path filePath(tmp);

                    if (!fields.count("external_file_column_number")) {
                        ShowSevereError(state, format("{}: No column number defined for external file \"{}\"", thisCurve->contextString, filePath));
                        ErrorsFound = true;
                    }
                    if (!fields.count("external_file_starting_row_number")) {
                        ShowSevereError(state,
                                        format("{}: No starting row number defined for external file \"{}\"", thisCurve->contextString, filePath));
                        ErrorsFound = true;
                    }

                    std::size_t colNum = fields.at("external_file_column_number").get<std::size_t>() - 1;
                    std::size_t rowNum = fields.at("external_file_starting_row_number").get<std::size_t>() - 1;

                    if (!state.dataCurveManager->btwxtManager.tableFiles.count(filePath)) {
                        TableFile tableFile;
                        ErrorsFound |= tableFile.load(state, filePath);
                        state.dataCurveManager->btwxtManager.tableFiles.emplace(filePath, tableFile);
                    }

                    if (ErrorsFound) {
                        continue; // Unable to load file so continue on to see if there are other errors before fataling
                    }

                    lookupValues = state.dataCurveManager->btwxtManager.tableFiles[filePath].getArray(state, {colNum, rowNum});

                    // remove NANs
                    lookupValues.erase(std::remove_if(lookupValues.begin(), lookupValues.end(), [](const double &x) { return std::isnan(x); }),
                                       lookupValues.end());

                } else if (fields.count("values")) {
                    for (auto &value : fields.at("values")) {
                        lookupValues.push_back(value.at("output_value").get<Real64>() / normalizationDivisor);
                    }
                } else {
                    ShowSevereError(state, format("{}: No values defined.", thisCurve->contextString));
                    ErrorsFound = true;
                }

                thisCurve->GridValueIndex = state.dataCurveManager->btwxtManager.addOutputValues(gridIndex, lookupValues);

                if (normalizeMethod == NM_AUTO_WITH_DIVISOR) {
                    auto const &normalizeTarget = varListNormalizeTargets.at(indVarListName);

                    bool pointsSpecified = false;
                    bool pointsUnspecified = false;
                    for (double value : normalizeTarget) {
                        if (std::isnan(value)) {
                            pointsUnspecified = true;
                        } else {
                            pointsSpecified = true;
                        }
                    }
                    if (pointsSpecified && pointsUnspecified) {
                        ShowSevereError(state,
                                        format("{}: Table is to be normalized using AutomaticWithDivisor, but not all independent variables define a "
                                               "normalization reference value. Make sure either:",
                                               thisCurve->contextString));
                        ShowContinueError(state, "  Make sure either:");
                        ShowContinueError(state, "    a) a normalization reference value is defined for each independent variable, or");
                        ShowContinueError(state, "    b) no normalization reference values are defined.");
                        ErrorsFound = true;
                    } else if (pointsSpecified) {
                        // normalizeGridValues normalizes curve values to 1.0 at the normalization target, and returns the scalar needed to perform
                        // this normalization. The result is multiplied by the input normalizationDivisor again for the AutomaticWithDivisor case, in
                        // which normalizeGridValues returns a compound scalar.
                        normalizationDivisor = state.dataCurveManager->btwxtManager.normalizeGridValues(
                                                   gridIndex, thisCurve->GridValueIndex, normalizeTarget, normalizationDivisor) *
                                               normalizationDivisor;
                    }
                }

                if ((normalizeMethod == NM_DIVISOR_ONLY) || (normalizeMethod == NM_AUTO_WITH_DIVISOR)) {
                    if (thisCurve->outputLimits.maxPresent) {
                        thisCurve->outputLimits.max = thisCurve->outputLimits.max / normalizationDivisor;
                    }
                    if (thisCurve->outputLimits.minPresent) {
                        thisCurve->outputLimits.min = thisCurve->outputLimits.min / normalizationDivisor;
                    }
                }
            }
        }
        state.dataCurveManager->btwxtManager.tableFiles.clear();
    }

    int BtwxtManager::getGridIndex(EnergyPlusData &state, std::string &indVarListName, bool &ErrorsFound)
    {
        int gridIndex = -1;
        if (gridMap.count(indVarListName)) {
            gridIndex = gridMap.at(indVarListName);
        } else {
            // Independent variable list does not exist
            ShowSevereError(state, format("Table:Lookup \"{}\" : No Table:IndependentVariableList found.", indVarListName));
            ErrorsFound = true;
        }
        return gridIndex;
    }

    int BtwxtManager::addOutputValues(int gridIndex, std::vector<double> values)
    {
        return (int)grids[gridIndex].add_grid_point_data_set(values);
    }

    int BtwxtManager::getNumGridDims(int gridIndex)
    {
        return (int)grids[gridIndex].get_number_of_dimensions();
    }

    double BtwxtManager::getGridValue(int gridIndex, int outputIndex, const std::vector<double> &target)
    {
        return grids[gridIndex](target)[outputIndex];
    }

    double BtwxtManager::normalizeGridValues(int gridIndex, int outputIndex, const std::vector<double> &target, const double scalar)
    {
        return grids[gridIndex].normalize_grid_point_data_set_at_target(outputIndex, target, scalar);
    }

    void BtwxtManager::clear()
    {
        grids.clear();
        gridMap.clear();
        independentVarRefs.clear();
        tableFiles.clear();
    }

    bool TableFile::load(EnergyPlusData &state, fs::path const &path)
    {
        this->filePath = path;
        std::string contextString = "CurveManager::TableFile::load: ";
        fs::path fullPath = DataSystemVariables::CheckForActualFilePath(state, path, contextString);
        if (fullPath.empty()) {
            // Note: we return 'ErrorsFound' apparently
            return true;
        }
        std::ifstream file(fullPath);
        std::string line;
        numRows = 0;
        numColumns = 0;
        while (getline(file, line)) {
            ++numRows;
            std::size_t pos(0);
            std::size_t colNum(1);
            while ((pos = line.find(',')) != std::string::npos) {
                if (colNum > numColumns) {
                    numColumns = colNum;
                    contents.resize(numColumns);
                }
                contents[colNum - 1].push_back(line.substr(0, pos));
                line.erase(0, pos + 1);
                ++colNum;
            }
            // Anything after the last comma
            if (!line.empty()) {
                if (colNum > numColumns) {
                    numColumns = colNum;
                    contents.resize(numColumns);
                }
                contents[colNum - 1].push_back(line);
                ++colNum;
            }
            // flesh out columns if row ends early
            while (colNum <= numColumns) {
                contents[colNum - 1].emplace_back("");
                ++colNum;
            }
        }
        return false;
    }

    std::vector<double> &TableFile::getArray(EnergyPlusData &state, std::pair<std::size_t, std::size_t> colAndRow)
    {
        if (!arrays.count(colAndRow)) {
            // create the column from the data if it doesn't exist already
            std::size_t col = colAndRow.first;  // 0 indexed
            std::size_t row = colAndRow.second; // 0 indexed
            auto &content = contents[col];
            if (col >= numColumns) {
                ShowFatalError(state,
                               format("File \"{}\" : Requested column ({}) exceeds the number of columns ({}).", filePath, col + 1, numColumns));
            }
            if (row >= numRows) {
                ShowFatalError(state,
                               format("File \"{}\" : Requested starting row ({}) exceeds the number of rows ({}).", filePath, row + 1, numRows));
            }
            std::vector<double> array(numRows - row);
            std::transform(content.begin() + row, content.end(), array.begin(), [](std::string_view str) {
                // Convert strings to double
                size_t first_char = str.find_first_not_of(' ');
                if (first_char != std::string_view::npos) {
                    str.remove_prefix(first_char);
                }
                double result = 0;
                auto answer = fast_float::from_chars(str.data(), str.data() + str.size(), result); // (AUTO_OK_OBJ)
                if (answer.ec != std::errc()) {
                    return std::numeric_limits<double>::quiet_NaN();
                }
                return result;
            });
            arrays[colAndRow] = array;
        }
        return arrays.at(colAndRow);
    }

    void InitCurveReporting(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   October 2011
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Setting up of curve output variables caused errors in some files. Thus, separating the setup
        // from the getinput.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        for (auto *thisCurve : state.dataCurveManager->curves) {
            for (int dim = 1; dim <= thisCurve->numDims; ++dim) {
                std::string numStr = fmt::to_string(dim);
                SetupOutputVariable(state,
                                    format("Performance Curve Input Variable {} Value", numStr),
                                    Constant::Units::None,
                                    thisCurve->inputs[dim - 1],
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    thisCurve->Name);
            }
            // set the output up last so it shows up after the input in the csv file
            SetupOutputVariable(state,
                                "Performance Curve Output Value",
                                Constant::Units::None,
                                thisCurve->output,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisCurve->Name);
        }

        for (auto &thisPressCurve : state.dataBranchAirLoopPlant->PressureCurve) {
            SetupOutputVariable(state,
                                "Performance Curve Input Variable 1 Value",
                                Constant::Units::None,
                                thisPressCurve.CurveInput1,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisPressCurve.Name);
            SetupOutputVariable(state,
                                "Performance Curve Input Variable 2 Value",
                                Constant::Units::None,
                                thisPressCurve.CurveInput2,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisPressCurve.Name);
            SetupOutputVariable(state,
                                "Performance Curve Input Variable 3 Value",
                                Constant::Units::None,
                                thisPressCurve.CurveInput3,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisPressCurve.Name);
            SetupOutputVariable(state,
                                "Performance Curve Output Value",
                                Constant::Units::None,
                                thisPressCurve.CurveOutput,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisPressCurve.Name);
        }

        if (state.dataGlobal->AnyEnergyManagementSystemInModel) { // provide hook for possible EMS control
            for (auto *thisCurve : state.dataCurveManager->curves) {
                SetupEMSActuator(
                    state, "Curve", thisCurve->Name, "Curve Result", "[unknown]", thisCurve->EMSOverrideOn, thisCurve->EMSOverrideCurveValue);
            } // All performance curves
        }
        if (state.dataGlobal->AnyEnergyManagementSystemInModel) { // provide hook for possible EMS control
            for (auto &thisPressCurve : state.dataBranchAirLoopPlant->PressureCurve) {
                SetupEMSActuator(state,
                                 "Curve",
                                 thisPressCurve.Name,
                                 "Curve Result",
                                 "[unknown]",
                                 thisPressCurve.EMSOverrideOn,
                                 thisPressCurve.EMSOverrideCurveValue);
            } // All pressure curves
        }
    }

    Real64 Curve::BtwxtTableInterpolation(EnergyPlusData &state,
                                          const Real64 Var1 // 1st independent variable
    )
    {
        // TODO: Generalize for N-dims
        std::vector<double> target{Var1};

        std::pair<EnergyPlusData *, std::string> callbackPair{&state, this->contextString};
        state.dataCurveManager->btwxtManager.setLoggingContext(&callbackPair);
        return state.dataCurveManager->btwxtManager.getGridValue(this->TableIndex, this->GridValueIndex, target);
    }

    Real64 Curve::BtwxtTableInterpolation(EnergyPlusData &state,
                                          const Real64 Var1, // 1st independent variable
                                          const Real64 Var2  // 2nd independent variable
    )
    {
        // TODO: Generalize for N-dims
        std::vector<double> target{Var1, Var2};
        std::pair<EnergyPlusData *, std::string> callbackPair{&state, this->contextString};
        state.dataCurveManager->btwxtManager.setLoggingContext(&callbackPair);
        return state.dataCurveManager->btwxtManager.getGridValue(this->TableIndex, this->GridValueIndex, target);
    }

    Real64 Curve::BtwxtTableInterpolation(EnergyPlusData &state,
                                          const Real64 Var1, // 1st independent variable
                                          const Real64 Var2, // 2nd independent variable
                                          const Real64 Var3  // 3rd independent variable
    )
    {
        std::vector<double> target{Var1, Var2, Var3};

        std::pair<EnergyPlusData *, std::string> callbackPair{&state, this->contextString};
        state.dataCurveManager->btwxtManager.setLoggingContext(&callbackPair);
        return state.dataCurveManager->btwxtManager.getGridValue(this->TableIndex, this->GridValueIndex, target);
    }

    Real64 Curve::BtwxtTableInterpolation(EnergyPlusData &state,
                                          const Real64 Var1, // 1st independent variable
                                          const Real64 Var2, // 2nd independent variable
                                          const Real64 Var3, // 3rd independent variable
                                          const Real64 Var4  // 4th independent variable
    )
    {
        std::vector<double> target{Var1, Var2, Var3, Var4};

        std::pair<EnergyPlusData *, std::string> callbackPair{&state, this->contextString};
        state.dataCurveManager->btwxtManager.setLoggingContext(&callbackPair);
        return state.dataCurveManager->btwxtManager.getGridValue(this->TableIndex, this->GridValueIndex, target);
    }

    Real64 Curve::BtwxtTableInterpolation(EnergyPlusData &state,
                                          const Real64 Var1, // 1st independent variable
                                          const Real64 Var2, // 2nd independent variable
                                          const Real64 Var3, // 3rd independent variable
                                          const Real64 Var4, // 4th independent variable
                                          const Real64 Var5  // 5th independent variable
    )
    {
        std::vector<double> target{Var1, Var2, Var3, Var4, Var5};
        std::pair<EnergyPlusData *, std::string> callbackPair{&state, this->contextString};
        state.dataCurveManager->btwxtManager.setLoggingContext(&callbackPair);
        return state.dataCurveManager->btwxtManager.getGridValue(this->TableIndex, this->GridValueIndex, target);
    }

    Real64 Curve::BtwxtTableInterpolation(EnergyPlusData &state,
                                          const Real64 Var1, // 1st independent variable
                                          const Real64 Var2, // 2nd independent variable
                                          const Real64 Var3, // 3rd independent variable
                                          const Real64 Var4, // 4th independent variable
                                          const Real64 Var5, // 5th independent variable
                                          const Real64 Var6  // 6th independent variable
    )
    {
        std::vector<double> target{Var1, Var2, Var3, Var4, Var5, Var6};

        std::pair<EnergyPlusData *, std::string> callbackPair{&state, this->contextString};
        state.dataCurveManager->btwxtManager.setLoggingContext(&callbackPair);
        return state.dataCurveManager->btwxtManager.getGridValue(this->TableIndex, this->GridValueIndex, target);
    }

    bool IsCurveInputTypeValid(std::string const &InInputType) // index of curve in curve array
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Jason Glazer
        //       DATE WRITTEN   Oct 2009
        //       MODIFIED
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Returns true if the input unit type is valid

        // currently this is a bit overkill to have an enum and string view array, but this sets it up in case we want to do more with these inputs
        enum class CurveInputType
        {
            Invalid = -1,
            Dimensionless,
            Temperature,
            Pressure,
            VolumetricFlow,
            MassFlow,
            Power,
            Distance,
            Wavelength,
            Angle,
            VolumetricFlowPerPower,
            Num
        };
        constexpr std::array<std::string_view, static_cast<int>(CurveInputType::Num)> inputTypes = {
            "DIMENSIONLESS",
            "TEMPERATURE",
            "PRESSURE",
            "VOLUMETRICFLOW",
            "MASSFLOW",
            "POWER",
            "DISTANCE",
            "WAVELENGTH",
            "ANGLE",
            "VOLUMETRICFLOWPERPOWER",
        };

        if (InInputType.empty()) {
            return true; // if not used it is valid
        }
        CurveInputType found = static_cast<CurveInputType>(getEnumValue(inputTypes, Util::makeUPPER(InInputType)));
        return found != CurveInputType::Invalid;
    }

    bool IsCurveOutputTypeValid(std::string const &InOutputType) // index of curve in curve array
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Jason Glazer
        //       DATE WRITTEN   Oct 2009
        //       MODIFIED
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Returns true if the output unit type is valid

        // currently this is a bit overkill to have an enum and string view array, but this sets it up in case we want to do more with these inputs
        enum class CurveOutputType
        {
            Invalid = -1,
            Dimensionless,
            Pressure,
            Temperature,
            Capacity,
            Power,
            Num
        };
        constexpr std::array<std::string_view, static_cast<int>(CurveOutputType::Num)> outputTypes = {
            "DIMENSIONLESS", "PRESSURE", "TEMPERATURE", "CAPACITY", "POWER"};
        CurveOutputType found = static_cast<CurveOutputType>(getEnumValue(outputTypes, Util::makeUPPER(InOutputType)));
        return found != CurveOutputType::Invalid;
    }

    bool CheckCurveDims(EnergyPlusData &state,
                        int const CurveIndex,
                        std::vector<int> const &validDims,
                        const std::string_view routineName,
                        std::string_view objectType,
                        std::string_view objectName,
                        std::string_view curveFieldText)
    {
        // Returns true if errors found
        Curve const *thisCurve = state.dataCurveManager->curves(CurveIndex);
        int curveDim = thisCurve->numDims;
        if (std::find(validDims.begin(), validDims.end(), curveDim) != validDims.end()) {
            return false;
        }

        ErrorObjectHeader eoh{routineName, objectType, objectName};

        std::string validDimsString = fmt::to_string(validDims[0]);
        for (std::size_t i = 1; i < validDims.size(); i++) {
            validDimsString += format(" or {}", validDims[i]);
        }

        ShowSevereCurveDims(state, eoh, curveFieldText, thisCurve->Name, validDimsString, curveDim);
        return true;
    }

    void ShowSevereCurveDims(EnergyPlusData &state,
                             ErrorObjectHeader const &eoh,
                             std::string_view const fieldName,
                             std::string_view const curveName,
                             std::string_view const validDims,
                             int dim)
    {
        ShowSevereError(state, fmt::format("{}: {}=\"{}\"", eoh.routineName, eoh.objectType, eoh.objectName));
        ShowContinueError(state, format("...Invalid curve for {}.", fieldName));
        ShowContinueError(state, format("...Input curve=\"{}\" has dimension {}.", curveName, dim));
        ShowContinueError(state, format("...Curve type must have dimension {}.", validDims));
    }

    std::string GetCurveName(EnergyPlusData &state, int const CurveIndex) // index of curve in curve array
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Bereket Nigusse
        //       DATE WRITTEN   May 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Given a curve index, returns the curve name

        if (CurveIndex > 0) {
            return state.dataCurveManager->curves(CurveIndex)->Name;
        } else {
            return "";
        }
    }

    int GetCurveIndex(EnergyPlusData &state, std::string const &CurveName) // name of the curve
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   May 2000
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Given a curve name, returns the curve index

        auto found = state.dataCurveManager->curveMap.find(CurveName);
        return (found == state.dataCurveManager->curveMap.end()) ? 0 : found->second;
    }

    Curve *GetCurve(EnergyPlusData &state, std::string const &CurveName) // name of the curve
    {
        int curveNum = GetCurveIndex(state, CurveName);
        return (curveNum == 0) ? nullptr : state.dataCurveManager->curves(curveNum);
    }

    void GetCurveMinMaxValues(EnergyPlusData &state,
                              int const CurveIndex, // index of curve in curve array
                              Real64 &Var1Min,      // Minimum values of 1st independent variable
                              Real64 &Var1Max       // Maximum values of 1st independent variable
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   July 2006
        //       MODIFIED       B. Griffith Aug 2006 add third independent variable
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Given the curve index, returns the minimum and maximum values specified in the input
        // for the independent variables of the performance curve.

        Curve *thisCurve = state.dataCurveManager->curves(CurveIndex);
        Var1Min = thisCurve->inputLimits[0].min;
        Var1Max = thisCurve->inputLimits[0].max;
    }

    void GetCurveMinMaxValues(EnergyPlusData &state,
                              int const CurveIndex, // index of curve in curve array
                              Real64 &Var1Min,      // Minimum values of 1st independent variable
                              Real64 &Var1Max,      // Maximum values of 1st independent variable
                              Real64 &Var2Min,      // Minimum values of 2nd independent variable
                              Real64 &Var2Max       // Maximum values of 2nd independent variable
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   July 2006
        //       MODIFIED       B. Griffith Aug 2006 add third independent variable
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Given the curve index, returns the minimum and maximum values specified in the input
        // for the independent variables of the performance curve.

        Curve *thisCurve = state.dataCurveManager->curves(CurveIndex);
        Var1Min = thisCurve->inputLimits[0].min;
        Var1Max = thisCurve->inputLimits[0].max;
        Var2Min = thisCurve->inputLimits[1].min;
        Var2Max = thisCurve->inputLimits[1].max;
    }

    void GetCurveMinMaxValues(EnergyPlusData &state,
                              int const CurveIndex, // index of curve in curve array
                              Real64 &Var1Min,      // Minimum values of 1st independent variable
                              Real64 &Var1Max,      // Maximum values of 1st independent variable
                              Real64 &Var2Min,      // Minimum values of 2nd independent variable
                              Real64 &Var2Max,      // Maximum values of 2nd independent variable
                              Real64 &Var3Min,      // Minimum values of 3rd independent variable
                              Real64 &Var3Max       // Maximum values of 3rd independent variable
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   July 2006
        //       MODIFIED       B. Griffith Aug 2006 add third independent variable
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Given the curve index, returns the minimum and maximum values specified in the input
        // for the independent variables of the performance curve.

        Curve *thisCurve = state.dataCurveManager->curves(CurveIndex);
        Var1Min = thisCurve->inputLimits[0].min;
        Var1Max = thisCurve->inputLimits[0].max;
        Var2Min = thisCurve->inputLimits[1].min;
        Var2Max = thisCurve->inputLimits[1].max;
        Var3Min = thisCurve->inputLimits[2].min;
        Var3Max = thisCurve->inputLimits[2].max;
    }

    void GetCurveMinMaxValues(EnergyPlusData &state,
                              int const CurveIndex, // index of curve in curve array
                              Real64 &Var1Min,      // Minimum values of 1st independent variable
                              Real64 &Var1Max,      // Maximum values of 1st independent variable
                              Real64 &Var2Min,      // Minimum values of 2nd independent variable
                              Real64 &Var2Max,      // Maximum values of 2nd independent variable
                              Real64 &Var3Min,      // Minimum values of 3rd independent variable
                              Real64 &Var3Max,      // Maximum values of 3rd independent variable
                              Real64 &Var4Min,      // Minimum values of 4th independent variable
                              Real64 &Var4Max       // Maximum values of 4th independent variable
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   July 2006
        //       MODIFIED       B. Griffith Aug 2006 add third independent variable
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Given the curve index, returns the minimum and maximum values specified in the input
        // for the independent variables of the performance curve.

        Curve *thisCurve = state.dataCurveManager->curves(CurveIndex);
        Var1Min = thisCurve->inputLimits[0].min;
        Var1Max = thisCurve->inputLimits[0].max;
        Var2Min = thisCurve->inputLimits[1].min;
        Var2Max = thisCurve->inputLimits[1].max;
        Var3Min = thisCurve->inputLimits[2].min;
        Var3Max = thisCurve->inputLimits[2].max;
        Var4Min = thisCurve->inputLimits[3].min;
        Var4Max = thisCurve->inputLimits[3].max;
    }

    void GetCurveMinMaxValues(EnergyPlusData &state,
                              int const CurveIndex, // index of curve in curve array
                              Real64 &Var1Min,      // Minimum values of 1st independent variable
                              Real64 &Var1Max,      // Maximum values of 1st independent variable
                              Real64 &Var2Min,      // Minimum values of 2nd independent variable
                              Real64 &Var2Max,      // Maximum values of 2nd independent variable
                              Real64 &Var3Min,      // Minimum values of 3rd independent variable
                              Real64 &Var3Max,      // Maximum values of 3rd independent variable
                              Real64 &Var4Min,      // Minimum values of 4th independent variable
                              Real64 &Var4Max,      // Maximum values of 4th independent variable
                              Real64 &Var5Min,      // Minimum values of 5th independent variable
                              Real64 &Var5Max       // Maximum values of 5th independent variable
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   July 2006
        //       MODIFIED       B. Griffith Aug 2006 add third independent variable
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Given the curve index, returns the minimum and maximum values specified in the input
        // for the independent variables of the performance curve.

        Curve *thisCurve = state.dataCurveManager->curves(CurveIndex);
        Var1Min = thisCurve->inputLimits[0].min;
        Var1Max = thisCurve->inputLimits[0].max;
        Var2Min = thisCurve->inputLimits[1].min;
        Var2Max = thisCurve->inputLimits[1].max;
        Var3Min = thisCurve->inputLimits[2].min;
        Var3Max = thisCurve->inputLimits[2].max;
        Var4Min = thisCurve->inputLimits[3].min;
        Var4Max = thisCurve->inputLimits[3].max;
        Var5Min = thisCurve->inputLimits[4].min;
        Var5Max = thisCurve->inputLimits[4].max;
    }

    void GetCurveMinMaxValues(EnergyPlusData &state,
                              int const CurveIndex, // index of curve in curve array
                              Real64 &Var1Min,      // Minimum values of 1st independent variable
                              Real64 &Var1Max,      // Maximum values of 1st independent variable
                              Real64 &Var2Min,      // Minimum values of 2nd independent variable
                              Real64 &Var2Max,      // Maximum values of 2nd independent variable
                              Real64 &Var3Min,      // Minimum values of 3rd independent variable
                              Real64 &Var3Max,      // Maximum values of 3rd independent variable
                              Real64 &Var4Min,      // Minimum values of 4th independent variable
                              Real64 &Var4Max,      // Maximum values of 4th independent variable
                              Real64 &Var5Min,      // Minimum values of 5th independent variable
                              Real64 &Var5Max,      // Maximum values of 5th independent variable
                              Real64 &Var6Min,      // Minimum values of 6th independent variable
                              Real64 &Var6Max       // Maximum values of 6th independent variable
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   July 2006
        //       MODIFIED       B. Griffith Aug 2006 add third independent variable
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Given the curve index, returns the minimum and maximum values specified in the input
        // for the independent variables of the performance curve.

        Curve *thisCurve = state.dataCurveManager->curves(CurveIndex);
        Var1Min = thisCurve->inputLimits[0].min;
        Var1Max = thisCurve->inputLimits[0].max;
        Var2Min = thisCurve->inputLimits[1].min;
        Var2Max = thisCurve->inputLimits[1].max;
        Var3Min = thisCurve->inputLimits[2].min;
        Var3Max = thisCurve->inputLimits[2].max;
        Var4Min = thisCurve->inputLimits[3].min;
        Var4Max = thisCurve->inputLimits[3].max;
        Var5Min = thisCurve->inputLimits[4].min;
        Var5Max = thisCurve->inputLimits[4].max;
        Var6Min = thisCurve->inputLimits[5].min;
        Var6Max = thisCurve->inputLimits[5].max;
    }

    void SetCurveOutputMinValue(EnergyPlusData &state,
                                int const CurveIndex, // index of curve in curve array
                                bool &ErrorsFound,    // TRUE when errors occur
                                const Real64 CurveMin // Minimum value of curve output
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   Feb 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Given the curve index, sets the minimum and maximum possible value for this curve.
        // Certain curve types have set limits (e.g., PLF curve should not be greater than 1 or less than 0.7).

        if (CurveIndex > 0 && CurveIndex <= static_cast<int>(state.dataCurveManager->curves.size())) {
            Curve *thisCurve = state.dataCurveManager->curves(CurveIndex);
            thisCurve->outputLimits.min = CurveMin;
            thisCurve->outputLimits.minPresent = true;
        } else {
            ShowSevereError(
                state,
                format("SetCurveOutputMinValue: CurveIndex=[{}] not in range of curves=[1:{}].", CurveIndex, state.dataCurveManager->curves.size()));
            ErrorsFound = true;
        }
    }

    void SetCurveOutputMaxValue(EnergyPlusData &state,
                                int const CurveIndex, // index of curve in curve array
                                bool &ErrorsFound,    // TRUE when errors occur
                                const Real64 CurveMax // Maximum values of curve output
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   Feb 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Given the curve index, sets the minimum and maximum possible value for this curve.
        // Certain curve types have set limits (e.g., PLF curve should not be greater than 1 or less than 0.7).

        if (CurveIndex > 0 && CurveIndex <= static_cast<int>(state.dataCurveManager->curves.size())) {
            Curve *thisCurve = state.dataCurveManager->curves(CurveIndex);
            thisCurve->outputLimits.max = CurveMax;
            thisCurve->outputLimits.maxPresent = true;
        } else {
            ShowSevereError(state,
                            format("SetCurveOutputMinMaxValues: CurveIndex=[{}] not in range of curves=[1:{}].",
                                   CurveIndex,
                                   state.dataCurveManager->curves.size()));
            ErrorsFound = true;
        }
    }

    void GetPressureSystemInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   August 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Currently it just reads the input for pressure curve objects

        // METHODOLOGY EMPLOYED:
        // General EnergyPlus Methodology

        // SUBROUTINE PARAMETER DEFINITIONS:
        std::string_view constexpr CurveObjectName = "Curve:Functional:PressureDrop";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Array1D_string Alphas(1);   // Alpha items for object
        Array1D<Real64> Numbers(5); // Numeric items for object
        int NumAlphas;              // Number of Alphas for each GetObjectItem call
        int NumNumbers;             // Number of Numbers for each GetObjectItem call
        int IOStatus;               // Used in GetObjectItem
        bool ErrsFound(false);      // Set to true if errors in input, fatal at end of routine

        // Not sure what this thing is
        int NumPressure = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurveObjectName);
        state.dataBranchAirLoopPlant->PressureCurve.allocate(NumPressure);
        for (int CurveNum = 1; CurveNum <= NumPressure; ++CurveNum) {
            auto &thisCurve = state.dataBranchAirLoopPlant->PressureCurve(CurveNum);
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurveObjectName,
                                                                     CurveNum,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     _,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            thisCurve.Name = Alphas(1);
            thisCurve.EquivDiameter = Numbers(1);
            thisCurve.MinorLossCoeff = Numbers(2);
            thisCurve.EquivLength = Numbers(3);
            thisCurve.EquivRoughness = Numbers(4);
            if (NumNumbers > 4 && !state.dataIPShortCut->lNumericFieldBlanks(5)) {
                if (Numbers(5) != 0.0) {
                    thisCurve.ConstantFPresent = true;
                    thisCurve.ConstantF = Numbers(5);
                }
            }
        }

        if (ErrsFound) {
            ShowFatalError(state, "GetPressureCurveInput: Errors found in Curve Objects.  Preceding condition(s) cause termination.");
        }
    }

    void GetPressureCurveTypeAndIndex(EnergyPlusData &state,
                                      std::string const &PressureCurveName, // name of the curve
                                      DataBranchAirLoopPlant::PressureCurveType &PressureCurveType,
                                      int &PressureCurveIndex)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   August 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Given a curve name, returns the curve type and index

        // METHODOLOGY EMPLOYED:
        // Curve types are:
        //  PressureCurveType::Invalid     = pressure name was given, but curve is not available
        //  PressureCurveType::None        = no pressure curve for this branch
        //  PressureCurveType::Pressure    = pressure curve based on friction/minor loss
        //  PressureCurveType::Generic     = curvemanager held curve which is function of flow rate

        // Initialize
        PressureCurveType = DataBranchAirLoopPlant::PressureCurveType::None;
        PressureCurveIndex = 0;

        // Try to retrieve a curve manager object
        int TempCurveIndex = GetCurveIndex(state, PressureCurveName);

        // See if it is valid
        if (TempCurveIndex > 0) {
            // We have to check the type of curve to make sure it is single independent variable type
            CurveType GenericCurveType = state.dataCurveManager->curves(TempCurveIndex)->curveType;
            {
                if (state.dataCurveManager->curves(TempCurveIndex)->numDims == 1) {
                    PressureCurveType = DataBranchAirLoopPlant::PressureCurveType::Generic;
                    PressureCurveIndex = TempCurveIndex;
                } else {
                    ShowSevereError(state, format("Plant Pressure Simulation: Found error for curve: {}", PressureCurveName));
                    ShowContinueError(state, format("Curve type detected: {}", objectNames[static_cast<int>(GenericCurveType)]));
                    ShowContinueError(state, "Generic curves should be single independent variable such that DeltaP = f(mdot)");
                    ShowContinueError(state, " Therefore they should be of type: Linear, Quadratic, Cubic, Quartic, or Exponent");
                    ShowFatalError(state, "Errors in pressure simulation input cause program termination");
                }
            }
            return;
        }

        // Then try to retrieve a pressure curve object
        if (allocated(state.dataBranchAirLoopPlant->PressureCurve)) {
            if (size(state.dataBranchAirLoopPlant->PressureCurve) > 0) {
                TempCurveIndex = Util::FindItemInList(PressureCurveName, state.dataBranchAirLoopPlant->PressureCurve);
            } else {
                TempCurveIndex = 0;
            }
        }

        // See if it is valid
        if (TempCurveIndex > 0) {
            PressureCurveType = DataBranchAirLoopPlant::PressureCurveType::Pressure;
            PressureCurveIndex = TempCurveIndex;
            return;
        }

        // If we made it here, we didn't find either type of match

        // Last check, see if it is blank:
        if (PressureCurveName.empty()) {
            PressureCurveType = DataBranchAirLoopPlant::PressureCurveType::None;
            return;
        }

        // At this point, we had a non-blank user entry with no match
        PressureCurveType = DataBranchAirLoopPlant::PressureCurveType::Invalid;
    }

    Real64
    PressureCurveValue(EnergyPlusData &state, int const PressureCurveIndex, Real64 const MassFlow, Real64 const Density, Real64 const Viscosity)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   August 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This will evaluate the pressure drop for components which use pressure information

        // METHODOLOGY EMPLOYED:
        // Friction factor pressure drop equation:
        // DP = [f*(L/D) + K] * (rho * V^2) / 2

        auto &curve = state.dataBranchAirLoopPlant->PressureCurve(PressureCurveIndex);

        // Intermediate calculations
        Real64 const CrossSectArea = (Constant::Pi / 4.0) * pow_2(curve.EquivDiameter);
        Real64 const Velocity = MassFlow / (Density * CrossSectArea);
        Real64 const ReynoldsNumber = Density * curve.EquivDiameter * Velocity / Viscosity; // assuming mu here
        Real64 const RoughnessRatio = curve.EquivRoughness / curve.EquivDiameter;

        // update curve bookkeeping
        curve.CurveInput1 = MassFlow;
        curve.CurveInput2 = Density;
        curve.CurveInput3 = Velocity;

        // If we don't have any flow then exit out
        if (MassFlow < DataBranchAirLoopPlant::MassFlowTolerance) {
            curve.CurveOutput = 0.0;
            return 0.0;
        }

        // Calculate the friction factor and pressure drop
        Real64 FrictionFactor = curve.ConstantFPresent ? curve.ConstantF : CalculateMoodyFrictionFactor(state, ReynoldsNumber, RoughnessRatio);
        Real64 PressureCurveValue = curve.EMSOverrideOn ? curve.EMSOverrideCurveValue
                                                        : (FrictionFactor * (curve.EquivLength / curve.EquivDiameter) + curve.MinorLossCoeff) *
                                                              (Density * pow_2(Velocity)) / 2.0;
        curve.CurveOutput = PressureCurveValue;
        return PressureCurveValue;
    }

    Real64 CalculateMoodyFrictionFactor(EnergyPlusData &state, Real64 const ReynoldsNumber, Real64 const RoughnessRatio)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Edwin Lee
        //       DATE WRITTEN   August 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // This will evaluate the moody friction factor based on Reynolds number and roughness ratio

        // METHODOLOGY EMPLOYED:
        // General empirical correlations for friction factor based on Moody Chart data

        // REFERENCES:
        // Haaland, SE (1983). "Simple and Explicit Formulas for the Friction Factor in Turbulent Flow".
        //   Trans. ASIVIE, J. of Fluids Engineering 103: 89-90.

        // Check for no flow or invalid roughness before calculating values
        if (ReynoldsNumber == 0.0 || RoughnessRatio == 0.0) {
            return 0.0;
        }

        // Calculate the friction factor
        Real64 const Term1 = std::pow(RoughnessRatio / 3.7, 1.11);
        Real64 const Term2 = 6.9 / ReynoldsNumber;
        Real64 const Term3 = -1.8 * std::log10(Term1 + Term2);
        if (Term3 != 0.0) {
            return std::pow(Term3, -2.0);
        } else {
            if (!state.dataCurveManager->FrictionFactorErrorHasOccurred) {
                ShowSevereError(state, "Plant Pressure System: Error in moody friction factor calculation");
                ShowContinueError(state,
                                  format("Current Conditions: Roughness Ratio={:.7R}; Reynolds Number={:.1R}", RoughnessRatio, ReynoldsNumber));
                ShowContinueError(state, "These conditions resulted in an unhandled numeric issue.");
                ShowContinueError(state, "Please contact EnergyPlus support/development team to raise an alert about this issue");
                ShowContinueError(state, "This issue will occur only one time.  The friction factor has been reset to 0.04 for calculations");
                state.dataCurveManager->FrictionFactorErrorHasOccurred = true;
            }
            return 0.04;
        }
    }

    void checkCurveIsNormalizedToOne(EnergyPlusData &state,
                                     std::string const &callingRoutineObj, // calling routine with object type
                                     std::string const &objectName,        // parent object where curve is used
                                     int const curveIndex,                 // index to curve object
                                     std::string const &cFieldName,        // object field name
                                     std::string const &cFieldValue,       // user input curve name
                                     Real64 const Var1)                    // required 1st independent variable
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         R. Raustad
        //       DATE WRITTEN   May 2017

        // PURPOSE OF THIS FUNCTION:
        // checks that curve output is within 10% of 1 at curve rating point

        if (curveIndex > 0) {
            Real64 const CurveVal = CurveValue(state, curveIndex, Var1);
            if (CurveVal > 1.10 || CurveVal < 0.90) {
                ShowWarningError(state, format("{}=\"{}\" curve values", callingRoutineObj, objectName));
                ShowContinueError(state, format("... {} = {} output is not equal to 1.0 (+ or - 10%) at rated conditions.", cFieldName, cFieldValue));
                ShowContinueError(state, format("... Curve output at rated conditions = {:.3T}", CurveVal));
            }
        }
    }

    void checkCurveIsNormalizedToOne(EnergyPlusData &state,
                                     std::string const &callingRoutineObj, // calling routine with object type
                                     std::string const &objectName,        // parent object where curve is used
                                     int const curveIndex,                 // index to curve object
                                     std::string const &cFieldName,        // object field name
                                     std::string const &cFieldValue,       // user input curve name
                                     Real64 const Var1,                    // required 1st independent variable
                                     Real64 const Var2)                    // 2nd independent variable
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         R. Raustad
        //       DATE WRITTEN   May 2017

        // PURPOSE OF THIS FUNCTION:
        // checks that curve output is within 10% of 1 at curve rating point

        if (curveIndex > 0) {
            Real64 const CurveVal = CurveValue(state, curveIndex, Var1, Var2);
            if (CurveVal > 1.10 || CurveVal < 0.90) {
                ShowWarningError(state, format("{}=\"{}\" curve values", callingRoutineObj, objectName));
                ShowContinueError(state, format("... {} = {} output is not equal to 1.0 (+ or - 10%) at rated conditions.", cFieldName, cFieldValue));
                ShowContinueError(state, format("... Curve output at rated conditions = {:.3T}", CurveVal));
            }
        }
    }

} // namespace Curve

} // namespace EnergyPlus
