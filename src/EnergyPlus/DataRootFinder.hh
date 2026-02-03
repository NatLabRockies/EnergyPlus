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

#ifndef DataRootFinder_hh_INCLUDED
#define DataRootFinder_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus::DataRootFinder {

enum class Slope
{
    Invalid = -1, // Undefined slope specification
    Increasing,   // For overall increasing function F(X) between min and max points
    Decreasing,   // For overall decreasing function F(X) between min and max points
    Num
};

enum class RootFinderStatus
{
    Invalid = -1,
    ErrorSingular, // Error because the overall slope appears to be flat between the min and max points, implying that the
                   // function might be singular over the interval: F(XMin) == F(XMax)

    ErrorSlope, // Error because the overall slope assumption is not observed at the min and max points:
                // - for an increasing function F(X), we expect F(XMin) < F(XMax)  otherwise error
                // - for a decreasing function F(X),  we expect F(XMin) > F(XMax)  otherwise error
                // Note that this error status does not detect strict monotonicity at points
                // between the min and max points.

    ErrorBracket, // Error because the current candidate X does not lie within the current lower an upper points:
                  // X < XLower or X > XUpper

    ErrorRange, // Error because the current candidate X does not lie within the min and max points:
                // X < XMin or X > XMax

    None,                // Indeterminate error state (not converged), also default state
    OK,                  // Unconstrained convergence achieved with root solution so that: XMin < XRoot < XMax
    OKMin,               // Constrained convergence achieved with solution XRoot==XMin
    OKMax,               // Constrained convergence achieved with solution XRoot==XMax
    OKRoundOff,          // Reached requested tolerance in X variables although Y=F(X) does not satisfy unconstrained convergence check
    WarningNonMonotonic, // Error because F(X) is not strictly monotonic between the lower and upper points
    WarningSingular,     // Error because F(X) == YLower or F(X) == YUpper
    Num
};

enum class RootFinderMethod
{
    Invalid = -1,
    None,          // No solution method (used internally only when root finder is reset)
    Bracket,       // Bracketing mode (used internally only to bracket root)
    Bisection,     // Step performed using bisection method (aka interval halving)
    FalsePosition, // Step performed using false position method (aka regula falsi)
    Secant,        // Step performed using secant method
    Brent,         // Step performed using Brent's method
    Num
};

struct ControlsType
{
    // Members
    DataRootFinder::Slope SlopeType; // Set to any of the iSlope<...> codes
    RootFinderMethod MethodType;     // Desired solution method.
    // Set to any of the iMethod<...> codes except for iMethodNone and iMethodBracket
    Real64 TolX;  // Relative tolerance for variable X
    Real64 ATolX; // Absolute tolerance for variable X
    Real64 ATolY; // Absolute tolerance for variable Y

    // Default Constructor
    ControlsType() : SlopeType(DataRootFinder::Slope::Invalid), MethodType(RootFinderMethod::None), TolX(1.0e-3), ATolX(1.0e-3), ATolY(1.0e-3)
    {
    }
};

struct PointType
{
    // Members
    bool DefinedFlag; // Set to true if point has been set; false otherwise
    Real64 X;         // X value
    Real64 Y;         // Y value = F(X)

    // Default Constructor
    PointType() : DefinedFlag(false), X(0.0), Y(0.0)
    {
    }
};

struct RootFinderDataType
{
    // Members
    ControlsType Controls;
    RootFinderStatus StatusFlag; // Current status of root finder
    // Valid values are any of the STATUS_<code> constants
    RootFinderMethod CurrentMethodType; // Solution method used to perform current step
    Real64 XCandidate;                  // Candidate X value to use next when evaluating F(X)
    Real64 ConvergenceRate;             // Convergence rate achieved over the last 2 successive iterations
    PointType Increment;                // Increment between last 2 iterations
    PointType MinPoint;                 // Point { XMin, F(XMin) }
    PointType MaxPoint;                 // Point { XMax, F(XMax) }
    PointType LowerPoint;               // Point { XLower, F(XLower) } so that XLower <= XRoot
    PointType UpperPoint;               // Point { XUpper, F(XUpper) } so that XRoot <= YUpper
    PointType CurrentPoint;             // Last evaluated point { X, F(X) }
    int NumHistory;                     // Number of points stored in History
    Array1D<PointType> History;         // Vector containing last 3 best iterates

    // Default Constructor
    RootFinderDataType()
        : StatusFlag(RootFinderStatus::None), CurrentMethodType(RootFinderMethod::None), XCandidate(0.0), ConvergenceRate(0.0), NumHistory(0),
          History(3)
    {
    }
};

} // namespace EnergyPlus::DataRootFinder

#endif
