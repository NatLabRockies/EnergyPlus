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

// C++ Headers
#include <cmath>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataPrecisionGlobals.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/RootFinder.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::RootFinder {
// Module containing the derivative-free, root finder routines for smooth,
// "mostly" monotonic functions in one variable.
// The algorithm is guaranteed to find the root if:
// - it is bracketed between min/max bounds
// - there is an odd number of roots between the min/max bounds
// Note that there is an even number of roots between the min/max bounds then it is possible
// that the algorithm will terminate with a min or max constrained solution instead of the
// actual root.

// MODULE INFORMATION:
//       AUTHOR         Dimitri Curtil (LBNL)
//       DATE WRITTEN   February 2006
//       MODIFIED       na
//       RE-ENGINEERED  na

// PURPOSE OF THIS MODULE:
// To encapsulate the data and algorithms required to
// find the root of a 1-dimensional function F(X) = Y .
// At any moment during the solution process, the following condition is satisfied:
// - For an overall increasing function F(X):
//  MinPoint%X <= LowerPoint%X <= CurrentPoint%X <= UpperPoint%X <= MaxPoint%X
//  MinPoint%Y <= LowerPoint%Y <= CurrentPoint%Y <= UpperPoint%Y <= MaxPoint%Y
// - For an overall decreasing function F(X):
//  MinPoint%X <= LowerPoint%X <= CurrentPoint%X <= UpperPoint%X <= MaxPoint%X
//  MinPoint%Y >= LowerPoint%Y >= CurrentPoint%Y >= UpperPoint%Y >= MaxPoint%Y
// Requirements for function F():
// - smooth, continuous, deterministic function
// - root XRoot bracketed within the min and max bounds, referred to as XMin, XMax
// - XMin and XMax must remain constant during iterations
// - If F(X) is increasing, set action to iSlopeIncreasing
//   This implies that if X1 < X2, then F(X1) < F(X2) must be satisfied.
// - If F(X) is decreasing, set action to iSlopeDecreasing
//   This implies that if X1 < X2, then F(X1) > F(X2) must be satisfied.
// Note that the residual function is allowed to be non-monotonic between the min and max
// points as long as the slope specification is satisfied at the min and max points.
// Unconstrained convergence conditions:
// - ABS(F(XRoot)) <= ATolY
// - ABS(X2-X1)    <= TolX * (ABS(X2)+ABS(X1))/2 + ATolX
// Constrained convergence conditions:
// For iSlopeIncreasing action:
//   - YMin >= 0
//   - YMax <= 0
// For iSlopeDecreasing action:
//   - YMin <= 0
//   - YMax >= 0

// METHODOLOGY EMPLOYED:
// Implements an hybrid solution method consisting of:
// - bisection method (aka interval halving)
// - false position method (aka regula falsi)
// - secant method
// - Brent's method (inverse quadratic interpolation)
// with safeguards against:
// - out-of-range error
// - singularity (i.e., locally singular Jacobian)
// - bad slope
// - round-off error
// Typical usage of the root finder, assuming that :
// - the function is called MyFunc()
// - the function is strictly increasing between the min and max bounds
// - the root is bracketed between XMin=0 and XMax=1
// - the function is defined with the prototype real FUNCTION MyFunc( X )
// - the solution method to use is the Brent's method
// - the absolute tolerance for Y=MyFunc(X) is ATolY=1.0E-4
// As a safeguard the iterative process stops after 50 iterations if the root has not been
// located within the specified tolerance yet.
// <BEGIN CODE SNIPPET>
// TYPE(RootFinderDataType)  :: RF
// INTEGER                   :: IterationCount
// REAL(r64)                 :: X, Y
// LOGICAL                   :: IsDoneFlag
// CALL SetupRootFinder(   &
//   RF,                   & ! RootFinderData
//   iSlopeIncreasing,     & ! SlopeType
//   iMethodBrent,         & ! MethodType
//   1.0d-6,               & ! TolX
//   1.0d-6,               & ! ATolX
//   1.0d-4                & ! ATolY
// )
// CALL InitializeRootFinder(   &
//   RF,                   & ! RootFinderData
//   0.0d0,                & ! XMin
//   1.0d0                 & ! XMax
// )
// IterationCount = 0
// IsDoneFlag = .FALSE.
// RF%XCandidate = 0.1d0 ! Sets X to initial value XInit
// DO WHILE ( .NOT.IsDoneFlag )
//   IterationCount = IterationCount+1
//   IF ( IterationCount>50 ) THEN
//     WRITE(*,*) 'Error: Too many iterations..."
//     EXIT
//   END IF
//   ! Evaluate function with new root candidate
//   X = RF%XCandidate
//   Y = MyFunc( X )
//   ! Update root finder data with new iterate
//   CALL IterateRootFinder( RF, X, Y, IsDoneFlag )
// END DO
// ! Write root finder status description to standard output
// CALL WriteRootFinderStatus( 6, RF )
// <END CODE SNIPPET>

// REFERENCES:
// "Numerical Recipes in FORTRAN", Chapter 9 "Root Finding and Nonlinear Sets of Equations", pp.340-352
// Used for formulas, but not for code.

// Using/Aliasing
using namespace DataRootFinder;

void SetupRootFinder(EnergyPlusData &state,
                     RootFinderDataType &RootFinderData,                // Data used by root finding algorithm
                     DataRootFinder::Slope const SlopeType,             // Either Slope::Increasing or Slope::Decreasing
                     DataRootFinder::RootFinderMethod const MethodType, // Any of the iMethod<name> code but iMethodNone
                     Real64 const TolX,                                 // Relative tolerance for X variables
                     Real64 const ATolX,                                // Absolute tolerance for X variables
                     Real64 const ATolY                                 // Absolute tolerance for Y variables
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   February 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine loads the numerical controls for the root finder.

    // Load assumed action for underlying function F(X)
    if (SlopeType != DataRootFinder::Slope::Increasing && SlopeType != DataRootFinder::Slope::Decreasing) {
        ShowSevereError(state, "SetupRootFinder: Invalid function slope specification. Valid choices are:");
        ShowContinueError(state, EnergyPlus::format("SetupRootFinder: Slope::Increasing={}", DataRootFinder::Slope::Increasing));
        ShowContinueError(state, EnergyPlus::format("SetupRootFinder: Slope::Decreasing={}", DataRootFinder::Slope::Decreasing));
        ShowFatalError(state, "SetupRootFinder: Preceding error causes program termination.");
    }
    RootFinderData.Controls.SlopeType = SlopeType;

    // Load solution method
    if (MethodType != RootFinderMethod::Bisection && MethodType != RootFinderMethod::FalsePosition && MethodType != RootFinderMethod::Secant &&
        MethodType != RootFinderMethod::Brent) {

        ShowSevereError(state, "SetupRootFinder: Invalid solution method specification. Valid choices are:");
        ShowContinueError(state, EnergyPlus::format("SetupRootFinder: iMethodBisection={}", RootFinderMethod::Bisection));
        ShowContinueError(state, EnergyPlus::format("SetupRootFinder: iMethodFalsePosition={}", RootFinderMethod::FalsePosition));
        ShowContinueError(state, EnergyPlus::format("SetupRootFinder: iMethodSecant={}", RootFinderMethod::Secant));
        ShowContinueError(state, EnergyPlus::format("SetupRootFinder: iMethodBrent={}", RootFinderMethod::Brent));
        ShowFatalError(state, "SetupRootFinder: Preceding error causes program termination.");
    }
    RootFinderData.Controls.MethodType = MethodType;

    // Load relative tolerance parameter for X variables
    if (TolX < 0.0) {
        ShowFatalError(state, "SetupRootFinder: Invalid tolerance specification for X variables. TolX >= 0");
    }
    RootFinderData.Controls.TolX = TolX;

    // Load absolute tolerance parameter for X variables
    if (ATolX < 0.0) {
        ShowFatalError(state, "SetupRootFinder: Invalid absolute tolerance specification for X variables. ATolX >= 0");
    }
    RootFinderData.Controls.ATolX = ATolX;

    // Load absolute tolerance parameter for Y variables
    if (ATolY < 0.0) {
        ShowFatalError(state, "SetupRootFinder: Invalid absolute tolerance specification for Y variables. ATolY >= 0");
    }
    RootFinderData.Controls.ATolY = ATolY;

    // Reset internal data for root finder with fictive min and max values
    ResetRootFinder(RootFinderData, DataPrecisionGlobals::constant_zero, DataPrecisionGlobals::constant_zero);
}

void ResetRootFinder(RootFinderDataType &RootFinderData, // Data used by root finding algorithm
                     Real64 const XMin,                  // Minimum X value allowed
                     Real64 const XMax                   // Maximum X value allowed
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   February 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine initializes the data for the root finder.

    // Reset min point
    RootFinderData.MinPoint.X = XMin;
    RootFinderData.MinPoint.Y = 0.0;
    RootFinderData.MinPoint.DefinedFlag = false;

    // Reset max point
    RootFinderData.MaxPoint.X = XMax;
    RootFinderData.MaxPoint.Y = 0.0;
    RootFinderData.MaxPoint.DefinedFlag = false;

    // Reset lower point
    RootFinderData.LowerPoint.X = 0.0;
    RootFinderData.LowerPoint.Y = 0.0;
    RootFinderData.LowerPoint.DefinedFlag = false;

    // Reset upper point
    RootFinderData.UpperPoint.X = 0.0;
    RootFinderData.UpperPoint.Y = 0.0;
    RootFinderData.UpperPoint.DefinedFlag = false;

    // Reset previous point
    RootFinderData.CurrentPoint.X = 0.0;
    RootFinderData.CurrentPoint.Y = 0.0;
    RootFinderData.CurrentPoint.DefinedFlag = false;

    // Reset iterate history with last 3 best points
    RootFinderData.NumHistory = 0;
    for (auto &e : RootFinderData.History) {
        e.X = e.Y = 0.0;
        e.DefinedFlag = false;
    }

    // Reset increments over successive iterations
    RootFinderData.Increment.X = 0.0;
    RootFinderData.Increment.Y = 0.0;
    RootFinderData.Increment.DefinedFlag = false;

    RootFinderData.XCandidate = 0.0;

    // Reset default state
    RootFinderData.StatusFlag = RootFinderStatus::None;
    RootFinderData.CurrentMethodType = RootFinderMethod::None;
    RootFinderData.ConvergenceRate = -1.0;
}

void InitializeRootFinder(EnergyPlusData &state,
                          RootFinderDataType &RootFinderData, // Data used by root finding algorithm
                          Real64 const XMin,                  // Minimum X value allowed
                          Real64 const XMax                   // Maximum X value allowed
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   February 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine initializes the min and max for the root finder before
    // finding a new root.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 SavedXCandidate;
    Real64 XMinReset;

    XMinReset = XMin;
    if (XMin > XMax) {
        if (XMax == 0.0) {
            XMinReset = XMax;
        } else {
            ShowFatalError(
                state, EnergyPlus::format("InitializeRootFinder: Invalid min/max bounds XMin={:.6f} must be smaller than XMax={:.6f}", XMin, XMax));
        }
    }

    // First save current candidate value before it is overridden in ResetRootFinder()
    SavedXCandidate = RootFinderData.XCandidate;

    // Reset internal data for root finder with actual min and max values
    // NOTE: This resets the value of RootFinderData%XCandidate to zero
    ResetRootFinder(RootFinderData, XMinReset, XMax);

    // Enforce min/max constraints on previous candidate if available
    // NOTE: If XMin == XMax then this forces the value of XCandidateto the desired solution
    RootFinderData.XCandidate = min(RootFinderData.MaxPoint.X, max(SavedXCandidate, RootFinderData.MinPoint.X));
}

void IterateRootFinder(EnergyPlusData &state,
                       RootFinderDataType &RootFinderData, // Data used by root finding algorithm
                       Real64 const X,                     // X value of current iterate
                       Real64 const Y,                     // Y value of current iterate
                       bool &IsDoneFlag                    // If TRUE indicates that the iteration should be stopped
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   March 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is the workhorse of the root finder framework.
    // It should be invoked with every new candidate (X,Y) until
    // convergence is achieved or abnormal termination is detected.
    // The subroutine performs the following tasks:
    // - it checks for convergence
    // - it updates the internal data with the current iterate (X,Y)
    // - it computes a new root candidate if not converged yet.
    // Sets IsDoneFlag to FALSE when iteration should continue with the new candidate value.
    // Sets IsDoneFlag to TRUE  when iteration should be stopped because convergence has been achieved
    // or because of a fatal error.
    // Note that only upon normal termination (iStatusOK<...> codes)
    // will the XCandidate value contain the root.
    // If the root has not been located yet the XCandidate value contains
    // the next candidate root to try.
    // Status                          IsDoneFlag          XCandidate
    // ========================================================================
    // iStatusErrorRange               TRUE                na
    // iStatusErrorSingular            TRUE                na
    // iStatusErrorSlope               TRUE                na
    // iStatusErrorBracket             TRUE                na
    // ------------------------------------------------------------------------
    // iStatusOKMin                    TRUE                MinPoint%X
    // iStatusOKMax                    TRUE                MaxPoint%X
    // iStatusOK                       TRUE                X
    // iStatusOKRoundOff               TRUE                X
    // ------------------------------------------------------------------------
    // iStatusNone                     FALSE               AdvanceRootFinder()
    // iStatusWarningNonMonotonic      FALSE               AdvanceRootFinder()
    // iStatusWarningSingular          FALSE               AdvanceRootFinder()

    // METHODOLOGY EMPLOYED:
    // The methodology reflects the same approach implemented in the subroutine CalcSimpleController()
    // whereby the iteration was exited by checking the following conditions in this specified
    // sequence:
    //   1. Check for singular function so that YMin /= YMax
    //   2. Check for slope condition for the residual function
    //      - increasing: YMin < YMax
    //      - decreasing: YMin > YMax
    //   3. Check for min constraint
    //      - increasing: YMin <= 0
    //      - decreasing: YMin >= 0
    //   4. Check for max constraint
    //      - increasing: YMax > 0
    //      - decreasing: YMax < 0
    //   5. Check unconstrained convergence
    // Note that the slope condition was not explicitly checked in the original implementation
    // in CalcSimpleController().
    // Finally, we also check the X increments between successive iterations to detect possible
    // cases whereby the allowed precision in the X space limits the precision attainable in
    // the Y space. This check is implemented in:
    // - CheckIncrementRoundOff()
    // - CheckBracketRoundOff()

    // Reset status flag
    RootFinderData.StatusFlag = RootFinderStatus::None;

    // Check that MinPoint%X <= X <= MaxPoint%X
    if (!CheckMinMaxRange(RootFinderData, X)) {
        RootFinderData.StatusFlag = RootFinderStatus::ErrorRange;

        // Fatal error: No need to continue iterating
        IsDoneFlag = true;
        return;
    }

    // Update min/max support points with current iterate
    UpdateMinMax(RootFinderData, X, Y);

    //----------------------------------------------------------------------------
    // Check "global" singularity and bad slope conditions between min and
    // max points
    //----------------------------------------------------------------------------

    // NOTE: Performed before checking min and max constraints to mimic original implementation
    //       in ManagerControllers()
    if (RootFinderData.MinPoint.DefinedFlag && RootFinderData.MaxPoint.DefinedFlag) {

        // Check that min and max points are distinct
        if (RootFinderData.MinPoint.X == RootFinderData.MaxPoint.X) {
            RootFinderData.StatusFlag = RootFinderStatus::OKMin;
            RootFinderData.XCandidate = RootFinderData.MinPoint.X;

            // Solution found: No need to continue iterating
            IsDoneFlag = true;
            return;
        }

        if (CheckMinConstraint(state, RootFinderData)) {
            RootFinderData.StatusFlag = RootFinderStatus::OKMin;
            RootFinderData.XCandidate = RootFinderData.MinPoint.X;

            // Solution found: No need to continue iterating
            IsDoneFlag = true;
            return;
        }

        // Check singularity condition between min and max points
        if (!CheckNonSingularity(RootFinderData)) {
            RootFinderData.StatusFlag = RootFinderStatus::ErrorSingular;

            // Fatal error: No need to continue iterating
            IsDoneFlag = true;
            return;
        }

        // Check slope condition between min and max points
        if (!CheckSlope(state, RootFinderData)) {
            RootFinderData.StatusFlag = RootFinderStatus::ErrorSlope;

            // Fatal error: No need to continue iterating
            IsDoneFlag = true;
            return;
        }
    }

    //----------------------------------------------------------------------------
    // Check that F(X) is not min or max constrained
    //----------------------------------------------------------------------------

    // Check min constraint before max constraint to mimic original implementation
    // in ManagerControllers()
    if (RootFinderData.MinPoint.DefinedFlag) {
        if (CheckMinConstraint(state, RootFinderData)) {
            RootFinderData.StatusFlag = RootFinderStatus::OKMin;
            RootFinderData.XCandidate = RootFinderData.MinPoint.X;

            // Solution found: No need to continue iterating
            IsDoneFlag = true;
            return;
        }
    }

    // Min point should always be evaluated first to ensure that we return with the min
    // consrained solution even in cases where the residual function has inconsistent slope.
    // TODO: Force to evaluate min point before exiting with max constrained solution
    //       in order to be able to detect singularity and bad slope conditions.
    if (RootFinderData.MaxPoint.DefinedFlag) {
        if (CheckMaxConstraint(state, RootFinderData)) {

            RootFinderData.StatusFlag = RootFinderStatus::OKMax;
            RootFinderData.XCandidate = RootFinderData.MaxPoint.X;

            // Solution found: No need to continue iterating
            IsDoneFlag = true;
            return;
        }
    }

    //----------------------------------------------------------------------------
    // Check convergence of current iterate
    //----------------------------------------------------------------------------

    // Check unconstrained convergence after we are sure that the candidate X value lies
    // within the allowed min/max range
    if (CheckRootFinderConvergence(RootFinderData, Y)) {
        RootFinderData.StatusFlag = RootFinderStatus::OK;
        RootFinderData.XCandidate = X;

        // Update root finder internal data with current iterate (X,Y)
        UpdateRootFinder(state, RootFinderData, X, Y);

        // Solution found: No need to continue iterating
        IsDoneFlag = true;
        return;
    }

    // Check last as this was not done in the original implementation
    // Essentially we stop the iteration if:
    // - the distance between the lower and upper bounds is smaller than the user-specified
    //   tolerance for the X variables. (USING brackets from previous iteration)
    if (CheckBracketRoundOff(RootFinderData)) {
        RootFinderData.StatusFlag = RootFinderStatus::OKRoundOff;

        // Solution found: No need to continue iterating
        IsDoneFlag = true;
        return;
    }

    //----------------------------------------------------------------------------
    // If the current iterate lies within the current lower and upper brackets,
    // proceed with normal processing to identify the next root candidate:
    // - update lower/upper bracket with current iterate
    // - update history
    // - update increments across successive iterations
    // - update current point
    // - compute next candidate (see AdvanceRootFinder() ).
    //----------------------------------------------------------------------------

    // Check that current iterate is within the current lower and upper points
    if (!CheckLowerUpperBracket(RootFinderData, X)) {
        RootFinderData.StatusFlag = RootFinderStatus::ErrorBracket;

        // Fatal error: No need to continue iterating
        IsDoneFlag = true;
        return;
    }

    // Update root finder internal data with current iterate (X,Y)
    UpdateRootFinder(state, RootFinderData, X, Y);

    // Compute new root candidate and store value in in RootFinderData%XCandidate
    // - First attempt to bracket root within lower and upper points
    // - Then use whatever requested solution method in SetupRootFinder() to
    //   compute the next candidate.
    AdvanceRootFinder(state, RootFinderData);

    // Indicates that we should continue iterating with new candidate
    IsDoneFlag = false;
}

RootFinderStatus CheckInternalConsistency(EnergyPlusData &state, RootFinderDataType const &RootFinderData) // Data used by root finding algorithm
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   March 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function checks whether the lower and upper points (if defined)
    // determine a consistent interval bracketting the root.
    // Returns the status code accordingly.
    // This function does not modify the argument RooFinderData.
    // Only used internally for debugging.

    // Return value
    RootFinderStatus CheckInternalConsistency;

    // Default initialization
    CheckInternalConsistency = RootFinderStatus::None;

    // Internal consistency check involving both support points
    if (RootFinderData.LowerPoint.DefinedFlag && RootFinderData.UpperPoint.DefinedFlag) {

        // Check that the existing lower and upper points do bracket the root
        if (RootFinderData.LowerPoint.X > RootFinderData.UpperPoint.X) {
            CheckInternalConsistency = RootFinderStatus::ErrorRange;
            return CheckInternalConsistency;
        }

        // Check for non-monotonicity between the existing lower and upper points
        switch (RootFinderData.Controls.SlopeType) {
        case DataRootFinder::Slope::Increasing: {
            // Y-value of lower point must be strictly smaller than Y-value of upper point
            if (RootFinderData.LowerPoint.Y > RootFinderData.UpperPoint.Y) {
                CheckInternalConsistency = RootFinderStatus::WarningNonMonotonic;
                return CheckInternalConsistency;
            }
        } break;
        case DataRootFinder::Slope::Decreasing: {
            // Y-value of lower point must be strictly larger than Y-value of upper point
            if (RootFinderData.LowerPoint.Y < RootFinderData.UpperPoint.Y) {
                CheckInternalConsistency = RootFinderStatus::WarningNonMonotonic;
                return CheckInternalConsistency;
            }
        } break;
        default: {
            // Should never happen
            ShowSevereError(state, "CheckInternalConsistency: Invalid function slope specification. Valid choices are:");
            ShowContinueError(state, EnergyPlus::format("CheckInternalConsistency: Slope::Increasing={}", DataRootFinder::Slope::Increasing));
            ShowContinueError(state, EnergyPlus::format("CheckInternalConsistency: Slope::Decreasing={}", DataRootFinder::Slope::Decreasing));
            ShowFatalError(state, "CheckInternalConsistency: Preceding error causes program termination.");
        } break;
        }

        // Check for in singularity with respect to the existing lower and upper points
        // Only check if the lower and upper points are distinct!
        if (RootFinderData.UpperPoint.X > RootFinderData.LowerPoint.X) {
            if (RootFinderData.UpperPoint.Y == RootFinderData.LowerPoint.Y) {
                CheckInternalConsistency = RootFinderStatus::ErrorSingular;
                return CheckInternalConsistency;
            }
        }
    }

    // Check min constraint for min point if already defined
    if (RootFinderData.MinPoint.DefinedFlag) {
        switch (RootFinderData.Controls.SlopeType) {
        case DataRootFinder::Slope::Increasing: {
            if (RootFinderData.MinPoint.Y >= 0.0) {
                CheckInternalConsistency = RootFinderStatus::OKMin;
                return CheckInternalConsistency;
            }
        } break;
        case DataRootFinder::Slope::Decreasing: {
            if (RootFinderData.MinPoint.Y <= 0.0) {
                CheckInternalConsistency = RootFinderStatus::OKMin;
                return CheckInternalConsistency;
            }
        } break;
        default: {
            // Should never happen
            ShowSevereError(state, "CheckInternalConsistency: Invalid function slope specification. Valid choices are:");
            ShowContinueError(state, EnergyPlus::format("CheckInternalConsistency: Slope::Increasing={}", DataRootFinder::Slope::Increasing));
            ShowContinueError(state, EnergyPlus::format("CheckInternalConsistency: Slope::Decreasing={}", DataRootFinder::Slope::Decreasing));
            ShowFatalError(state, "CheckInternalConsistency: Preceding error causes program termination.");
        } break;
        }
    }

    // Check max constraint for max point if already defined
    if (RootFinderData.MaxPoint.DefinedFlag) {
        switch (RootFinderData.Controls.SlopeType) {
        case DataRootFinder::Slope::Increasing: {
            if (RootFinderData.MaxPoint.Y <= 0.0) {
                CheckInternalConsistency = RootFinderStatus::OKMax;
                return CheckInternalConsistency;
            }
        } break;
        case DataRootFinder::Slope::Decreasing: {
            if (RootFinderData.MaxPoint.Y >= 0.0) {
                CheckInternalConsistency = RootFinderStatus::OKMax;
                return CheckInternalConsistency;
            }
        } break;
        default: {
            // Should never happen
            ShowSevereError(state, "CheckInternalConsistency: Invalid function slope specification. Valid choices are:");
            ShowContinueError(state, EnergyPlus::format("CheckInternalConsistency: Slope::Increasing={}", DataRootFinder::Slope::Increasing));
            ShowContinueError(state, EnergyPlus::format("CheckInternalConsistency: Slope::Decreasing={}", DataRootFinder::Slope::Decreasing));
            ShowFatalError(state, "CheckInternalConsistency: Preceding error causes program termination.");
        } break;
        }
    }

    return CheckInternalConsistency;
}

bool CheckRootFinderCandidate(RootFinderDataType const &RootFinderData, // Data used by root finding algorithm
                              Real64 const X                            // X value for current iterate
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   February 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function checks whether the root candidate X lies within the specified
    // min/max limits as well as within the current lower and upper brackets (if defined).
    // Returns TRUE if X value is a valid root candidate.
    // Returns FALSE otherwise.

    // Return value
    bool CheckRootFinderCandidate;

    if (CheckMinMaxRange(RootFinderData, X) && CheckLowerUpperBracket(RootFinderData, X)) {
        CheckRootFinderCandidate = true;
    } else {
        CheckRootFinderCandidate = false;
    }

    return CheckRootFinderCandidate;
}

bool CheckMinMaxRange(RootFinderDataType const &RootFinderData, // Data used by root finding algorithm
                      Real64 const X                            // X value for current iterate
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   February 2006
    //       MODIFIED       Brent Griffith (NREL) added DefinedFlag traps
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function checks whether the current iterate X lies within the specified min/max limits
    // or not.
    // Returns TRUE if current iterate satisfies min/max constraints.
    // Returns FALSE if current iterate is out-of-range.

    // Return value
    bool CheckMinMaxRange;

    if (RootFinderData.MinPoint.DefinedFlag) {
        if (X < RootFinderData.MinPoint.X) {
            CheckMinMaxRange = false;
            return CheckMinMaxRange;
        }
    }

    if (RootFinderData.MaxPoint.DefinedFlag) {
        if (X > RootFinderData.MaxPoint.X) {
            CheckMinMaxRange = false;
            return CheckMinMaxRange;
        }
    }

    CheckMinMaxRange = true;

    return CheckMinMaxRange;
}

bool CheckLowerUpperBracket(RootFinderDataType const &RootFinderData, // Data used by root finding algorithm
                            Real64 const X                            // X value for current iterate
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   February 2006
    //       MODIFIED       Brent Griffith, March 2010, changed to LowerPoint%X <= X <= UpperPoint%X
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function checks whether the current iterate X lies within the current lower
    // and upper points or not (if defined):
    //   LowerPoint%X < X < UpperPoint%X
    // Returns TRUE if current iterate lies within the lower/upper bracket.
    // Returns FALSE otherwise.

    // Return value
    bool CheckLowerUpperBracket;

    if (RootFinderData.LowerPoint.DefinedFlag) {
        if (X < RootFinderData.LowerPoint.X) {
            CheckLowerUpperBracket = false;
            return CheckLowerUpperBracket;
        }
    }

    if (RootFinderData.UpperPoint.DefinedFlag) {
        if (X > RootFinderData.UpperPoint.X) {
            CheckLowerUpperBracket = false;
            return CheckLowerUpperBracket;
        }
    }

    CheckLowerUpperBracket = true;

    return CheckLowerUpperBracket;
}

bool CheckSlope(EnergyPlusData &state, RootFinderDataType const &RootFinderData) // Data used by root finding algorithm
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   February 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function checks whether the current iterate (X,Y) satisfies the slope
    // requirement between the min and max points.
    // Returns FALSE if the slope requirement is NOT satisfied.
    // Returns TRUE if the slope requirement is satisfied.
    // PRECONDITION:
    // - Function assumes that both the min and max points are defined.
    // POSTCONDITION:
    // - RootFinderData is NOT changed by this function.

    // Return value
    bool CheckSlope;

    // Check that the slope requirement is respected at the min and max points
    // Note that the singularity check takes care of RootFinderData%MinPoint%Y == RootFinderData%MaxPoint%Y
    // therefore we use strict comparison operators < and >.
    switch (RootFinderData.Controls.SlopeType) {
    case DataRootFinder::Slope::Increasing: {
        if (RootFinderData.MinPoint.Y < RootFinderData.MaxPoint.Y) {
            CheckSlope = true;
            return CheckSlope;
        }
    } break;
    case DataRootFinder::Slope::Decreasing: {
        if (RootFinderData.MinPoint.Y > RootFinderData.MaxPoint.Y) {
            CheckSlope = true;
            return CheckSlope;
        }
    } break;
    default: {
        // Should never happen
        ShowSevereError(state, "CheckSlope: Invalid function slope specification. Valid choices are:");
        ShowContinueError(state, EnergyPlus::format("CheckSlope: Slope::Increasing={}", DataRootFinder::Slope::Increasing));
        ShowContinueError(state, EnergyPlus::format("CheckSlope: Slope::Decreasing={}", DataRootFinder::Slope::Decreasing));
        ShowFatalError(state, "CheckSlope: Preceding error causes program termination.");
    } break;
    }

    CheckSlope = false;

    return CheckSlope;
}

bool CheckNonSingularity(RootFinderDataType const &RootFinderData) // Data used by root finding algorithm
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   February 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function checks whether the min and max points define a locally, singular
    // equation system. In a 1-dimensional system, "singularity" is detected if the
    // min and max points have the same y-values, thereby producing a zero slope
    // across the min/max range.
    // Returns TRUE if the function satisfies the non-singularity condition.
    // Returns FALSE otherwise (i.e., F(X) essentially displays a zero slope
    // between the min and max points) .
    // PRECONDITION:
    // - Function assumes that both the min and max points are defined.
    // POSTCONDITION:
    // - RootFinderData is NOT changed by this function.

    // Return value
    bool CheckNonSingularity;

    // FUNCTION PARAMETER DEFINITIONS:
    // Safety factor used to detect a singular residual function between the min and max
    // points.
    // NOTE: Requesting exactly the same value is obtained by setting SafetyFactor = 0.0
    Real64 constexpr SafetyFactor(0.1);

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    Real64 DeltaY; // Difference between min and max Y-values
    Real64 ATolY;  // Absolute tolerance used to detected equal min and max Y-values

    // Added this check based on an absolute tolerance test for y values to avoid incorrectly detecting
    // functions with bad slope due to numerical noise.
    // Typically, this takes care of situations where the controlled equipment in ManageControllers()
    // would be misdiagnosed as displaying the "wrong slope" instead of being treated as "singular"
    // (i.e. in inactive mode).
    DeltaY = std::abs(RootFinderData.MinPoint.Y - RootFinderData.MaxPoint.Y);
    ATolY = SafetyFactor * RootFinderData.Controls.ATolY;

    if (std::abs(DeltaY) <= ATolY) {
        CheckNonSingularity = false;
    } else {
        CheckNonSingularity = true;
    }

    return CheckNonSingularity;
}

bool CheckMinConstraint(EnergyPlusData &state, RootFinderDataType const &RootFinderData) // Data used by root finding algorithm
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   February 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function checks whether the min point satisfies the min constraint
    // condition or not.
    // PRECONDITION:
    // - Function assumes that the min point is defined.
    // POSTCONDITION:
    // - RootFinderData is NOT changed by this function.

    // Return value
    bool CheckMinConstraint;

    switch (RootFinderData.Controls.SlopeType) {
    case DataRootFinder::Slope::Increasing: {
        if (RootFinderData.MinPoint.Y >= 0.0) {
            CheckMinConstraint = true;
            return CheckMinConstraint;
        }
    } break;
    case DataRootFinder::Slope::Decreasing: {
        if (RootFinderData.MinPoint.Y <= 0.0) {
            CheckMinConstraint = true;
            return CheckMinConstraint;
        }
    } break;
    default: {
        // Should never happen
        ShowSevereError(state, "CheckMinConstraint: Invalid function slope specification. Valid choices are:");
        ShowContinueError(state, EnergyPlus::format("CheckMinConstraint: Slope::Increasing={}", DataRootFinder::Slope::Increasing));
        ShowContinueError(state, EnergyPlus::format("CheckMinConstraint: Slope::Decreasing={}", DataRootFinder::Slope::Decreasing));
        ShowFatalError(state, "CheckMinConstraint: Preceding error causes program termination.");
    } break;
    }

    CheckMinConstraint = false;

    return CheckMinConstraint;
}

bool CheckMaxConstraint(EnergyPlusData &state, RootFinderDataType const &RootFinderData) // Data used by root finding algorithm
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   February 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function checks whether the max point satisfies the max constraint
    // condition or not.
    // PRECONDITION:
    // - Function assumes that the max point is defined.
    // POSTCONDITION:
    // - RootFinderData is NOT changed by this function.

    // Return value
    bool CheckMaxConstraint;

    // Check for max constrained convergence with respect to the new iterate (X,Y)
    switch (RootFinderData.Controls.SlopeType) {
    case DataRootFinder::Slope::Increasing: {
        if (RootFinderData.MaxPoint.Y <= 0.0) {
            CheckMaxConstraint = true;
            return CheckMaxConstraint;
        }
    } break;
    case DataRootFinder::Slope::Decreasing: {
        if (RootFinderData.MaxPoint.Y >= 0.0) {
            CheckMaxConstraint = true;
            return CheckMaxConstraint;
        }
    } break;
    default: {
        // Should never happen
        ShowSevereError(state, "CheckMaxConstraint: Invalid function slope specification. Valid choices are:");
        ShowContinueError(state, EnergyPlus::format("CheckMaxConstraint: Slope::Increasing={}", DataRootFinder::Slope::Increasing));
        ShowContinueError(state, EnergyPlus::format("CheckMaxConstraint: Slope::Decreasing={}", DataRootFinder::Slope::Decreasing));
        ShowFatalError(state, "CheckMaxConstraint: Preceding error causes program termination.");
    } break;
    }

    CheckMaxConstraint = false;

    return CheckMaxConstraint;
}

bool CheckRootFinderConvergence(RootFinderDataType const &RootFinderData, // Data used by root finding algorithm
                                Real64 const Y                            // Y value for current iterate
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   February 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function checks whether the current iterate (X,Y) satisfies the
    // unconstrained convergence criterion or not.

    // Return value
    bool CheckRootFinderConvergence;

    // Check for unconstrained convergence
    if (std::abs(Y) <= RootFinderData.Controls.ATolY) {
        CheckRootFinderConvergence = true;
        return CheckRootFinderConvergence;
    }

    CheckRootFinderConvergence = false;

    return CheckRootFinderConvergence;
}

bool CheckBracketRoundOff(RootFinderDataType const &RootFinderData) // Data used by root finding algorithm
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   February 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function checks whether the current lower and upper brackets satisfies
    // the round-off criterion or not.

    // Return value
    bool CheckBracketRoundOff;

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    Real64 DeltaUL; // Distance between lower and upper points
    Real64 TypUL;   // Typical value for values lying within lower/upper interval
    Real64 TolUL;   // Tolerance to satisfy for lower-upper distance

    // Check for round-off error in Lower/Upper interval
    if (RootFinderData.LowerPoint.DefinedFlag && RootFinderData.UpperPoint.DefinedFlag) {
        DeltaUL = RootFinderData.UpperPoint.X - RootFinderData.LowerPoint.X;
        TypUL = (std::abs(RootFinderData.UpperPoint.X) + std::abs(RootFinderData.LowerPoint.X)) / 2.0;
        TolUL = RootFinderData.Controls.TolX * std::abs(TypUL) + RootFinderData.Controls.ATolX;

        // Halve tolerance to reflect the fact that solution can be anywhere between the lower and upper points.
        if (std::abs(DeltaUL) <= 0.5 * std::abs(TolUL)) {
            CheckBracketRoundOff = true;
            return CheckBracketRoundOff;
        }
    }

    CheckBracketRoundOff = false;

    return CheckBracketRoundOff;
}

void UpdateMinMax(RootFinderDataType &RootFinderData, // Data used by root finding algorithm
                  Real64 const X,                     // X value for current iterate
                  Real64 const Y                      // Y value for current iterate, F(X)=Y
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   February 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine updates the min/max support points in the root finder data.

    // METHODOLOGY EMPLOYED:
    // PRECONDITION:
    // na
    // POSTCONDITION:
    // - RootFinderData%MinPoint possibly updated
    // - RootFinderData%MaxPoint possibly updated

    // Update min support point
    if (X == RootFinderData.MinPoint.X) {
        RootFinderData.MinPoint.Y = Y;
        RootFinderData.MinPoint.DefinedFlag = true;
    }

    // Update max support point
    if (X == RootFinderData.MaxPoint.X) {
        RootFinderData.MaxPoint.Y = Y;
        RootFinderData.MaxPoint.DefinedFlag = true;
    }
}

void UpdateBracket(EnergyPlusData &state,
                   RootFinderDataType &RootFinderData, // Data used by root finding algorithm
                   Real64 const X,                     // X value for current iterate
                   Real64 const Y                      // Y value for current iterate, F(X)=Y
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   March 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine updates the lower/upper support points in the root finder data
    // with the current iterate (X,Y).

    // METHODOLOGY EMPLOYED:
    // PRECONDITION:
    // - The current iterate (X,Y) must satisfy:
    //   MinPoint%X <= LowerPoint%X < X < UpperPoint%X <= MaxPoint%X
    // - RootFinderData%StatusFlag == iStatusNone
    // POSTCONDITION:
    // - RootFinderData%LowerPoint possibly updated
    // - RootFinderData%UpperPoint possibly updated
    // - RootFinderData%StatusFlag possibly updated with:
    //   - iStatusWarningNonMonotonic
    //   - iStatusWarningSingular

    switch (RootFinderData.Controls.SlopeType) {
    case DataRootFinder::Slope::Increasing: {
        // Update lower point
        if (Y <= 0.0) {
            if (!RootFinderData.LowerPoint.DefinedFlag) {
                RootFinderData.LowerPoint.DefinedFlag = true;
                RootFinderData.LowerPoint.X = X;
                RootFinderData.LowerPoint.Y = Y;
            } else {
                if (X >= RootFinderData.LowerPoint.X) {
                    if (Y == RootFinderData.LowerPoint.Y) {
                        RootFinderData.StatusFlag = RootFinderStatus::WarningSingular;
                    } else if (Y < RootFinderData.LowerPoint.Y) {
                        RootFinderData.StatusFlag = RootFinderStatus::WarningNonMonotonic;
                    }
                    // Update lower point with current iterate
                    RootFinderData.LowerPoint.X = X;
                    RootFinderData.LowerPoint.Y = Y;
                } else {
                    // Should never happen if CheckLowerUpperBracket() is called before
                    ShowSevereError(state, "UpdateBracket: Current iterate is smaller than the lower bracket.");
                    ShowContinueError(state, EnergyPlus::format("UpdateBracket: X={:.15f}, Y={:.15f}", X, Y));
                    ShowContinueError(state,
                                      EnergyPlus::format(
                                          "UpdateBracket: XLower={:.15f}, YLower={:.15f}", RootFinderData.LowerPoint.X, RootFinderData.LowerPoint.Y));
                    ShowFatalError(state, "UpdateBracket: Preceding error causes program termination.");
                }
            }

            // Update upper point
        } else {
            if (!RootFinderData.UpperPoint.DefinedFlag) {
                RootFinderData.UpperPoint.DefinedFlag = true;
                RootFinderData.UpperPoint.X = X;
                RootFinderData.UpperPoint.Y = Y;
            } else {
                if (X <= RootFinderData.UpperPoint.X) {
                    if (Y == RootFinderData.UpperPoint.Y) {
                        RootFinderData.StatusFlag = RootFinderStatus::WarningSingular;
                    } else if (Y > RootFinderData.UpperPoint.Y) {
                        RootFinderData.StatusFlag = RootFinderStatus::WarningNonMonotonic;
                    }
                    // Update upper point with current iterate
                    RootFinderData.UpperPoint.X = X;
                    RootFinderData.UpperPoint.Y = Y;
                } else {
                    // Should never happen if CheckLowerUpperBracket() is called before
                    ShowSevereError(state, "UpdateBracket: Current iterate is greater than the upper bracket.");
                    ShowContinueError(state, EnergyPlus::format("UpdateBracket: X={:.15f}, Y={:.15f}", X, Y));
                    ShowContinueError(state,
                                      EnergyPlus::format(
                                          "UpdateBracket: XUpper={:.15f}, YUpper={:.15f}", RootFinderData.UpperPoint.X, RootFinderData.UpperPoint.Y));
                    ShowFatalError(state, "UpdateBracket: Preceding error causes program termination.");
                }
            }
        }
        // Monotone, decreasing function
    } break;
    case DataRootFinder::Slope::Decreasing: {
        // Update lower point
        if (Y >= 0.0) {
            if (!RootFinderData.LowerPoint.DefinedFlag) {
                RootFinderData.LowerPoint.DefinedFlag = true;
                RootFinderData.LowerPoint.X = X;
                RootFinderData.LowerPoint.Y = Y;
            } else {
                if (X >= RootFinderData.LowerPoint.X) {
                    if (Y == RootFinderData.LowerPoint.Y) {
                        RootFinderData.StatusFlag = RootFinderStatus::WarningSingular;
                    } else if (Y > RootFinderData.LowerPoint.Y) {
                        RootFinderData.StatusFlag = RootFinderStatus::WarningNonMonotonic;
                    }
                    // Update lower point with current iterate
                    RootFinderData.LowerPoint.X = X;
                    RootFinderData.LowerPoint.Y = Y;
                } else {
                    // Should never happen if CheckLowerUpperBracket() is called before
                    ShowSevereError(state, "UpdateBracket: Current iterate is smaller than the lower bracket.");
                    ShowContinueError(state, EnergyPlus::format("UpdateBracket: X={:.15f}, Y={:.15f}", X, Y));
                    ShowContinueError(state,
                                      EnergyPlus::format(
                                          "UpdateBracket: XLower={:.15f}, YLower={:.15f}", RootFinderData.LowerPoint.X, RootFinderData.LowerPoint.Y));
                    ShowFatalError(state, "UpdateBracket: Preceding error causes program termination.");
                }
            }

            // Update upper point
        } else {
            if (!RootFinderData.UpperPoint.DefinedFlag) {
                RootFinderData.UpperPoint.DefinedFlag = true;
                RootFinderData.UpperPoint.X = X;
                RootFinderData.UpperPoint.Y = Y;
            } else {
                if (X <= RootFinderData.UpperPoint.X) {
                    if (Y == RootFinderData.UpperPoint.Y) {
                        RootFinderData.StatusFlag = RootFinderStatus::WarningSingular;
                    } else if (Y < RootFinderData.UpperPoint.Y) {
                        RootFinderData.StatusFlag = RootFinderStatus::WarningNonMonotonic;
                    }
                    // Update upper point with current iterate
                    RootFinderData.UpperPoint.X = X;
                    RootFinderData.UpperPoint.Y = Y;
                } else {
                    // Should never happen if CheckLowerUpperBracket() is called before
                    ShowSevereError(state, "UpdateBracket: Current iterate is greater than the upper bracket.");
                    ShowContinueError(state, EnergyPlus::format("UpdateBracket: X={:.15f}, Y={:.15f}", X, Y));
                    ShowContinueError(state,
                                      EnergyPlus::format(
                                          "UpdateBracket: XUpper={:.15f}, YUpper={:.15f}", RootFinderData.UpperPoint.X, RootFinderData.UpperPoint.Y));
                    ShowFatalError(state, "UpdateBracket: Preceding error causes program termination.");
                }
            }
        }
    } break;
    default: {
        // Should never happen
        ShowSevereError(state, "UpdateBracket: Invalid function slope specification. Valid choices are:");
        ShowContinueError(state, EnergyPlus::format("UpdateBracket: Slope::Increasing={}", DataRootFinder::Slope::Increasing));
        ShowContinueError(state, EnergyPlus::format("UpdateBracket: Slope::Decreasing={}", DataRootFinder::Slope::Decreasing));
        ShowFatalError(state, "UpdateBracket: Preceding error causes program termination.");
    } break;
    }
}

void UpdateHistory(RootFinderDataType &RootFinderData, // Data used by root finding algorithm
                   Real64 const X,                     // X value for current iterate
                   Real64 const Y                      // Y value for current iterate, F(X)=Y
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   February 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine updates the min/max support points in the root finder data.

    // METHODOLOGY EMPLOYED:
    // PRECONDITION:
    // - The current iterate (X,Y) must be a valid iterate:
    //   MinPoint%X <= LowerPoint%X < X < UpperPoint%X <= MaxPoint%X
    // POSTCONDITION:
    // - RootFinderData%History(:) updated with last 3 best iterates

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int NumHistory;

    // Update history with best iterates so that:
    //   ABS(History(1)%Y) <= ABS(History(2)%Y) <= ABS(History(3)%Y)
    // Note that the history points are sorted so that
    //   SIGN(History(1)%Y) = -SIGN(History(3)%Y)
    // to ensure that the history points bracket the candidate root.
    for (auto &e : RootFinderData.History) {
        e.X = e.Y = 0.0;
        e.DefinedFlag = false;
    }

    NumHistory = 0;
    if (RootFinderData.LowerPoint.DefinedFlag) {
        ++NumHistory;
        RootFinderData.History(NumHistory).DefinedFlag = RootFinderData.LowerPoint.DefinedFlag;
        RootFinderData.History(NumHistory).X = RootFinderData.LowerPoint.X;
        RootFinderData.History(NumHistory).Y = RootFinderData.LowerPoint.Y;
    }
    if (RootFinderData.UpperPoint.DefinedFlag) {
        ++NumHistory;
        RootFinderData.History(NumHistory).DefinedFlag = RootFinderData.UpperPoint.DefinedFlag;
        RootFinderData.History(NumHistory).X = RootFinderData.UpperPoint.X;
        RootFinderData.History(NumHistory).Y = RootFinderData.UpperPoint.Y;
    }
    ++NumHistory;
    RootFinderData.History(NumHistory).DefinedFlag = true;
    RootFinderData.History(NumHistory).X = X;
    RootFinderData.History(NumHistory).Y = Y;

    RootFinderData.NumHistory = NumHistory;
    SortHistory(NumHistory, RootFinderData.History);
}

void UpdateRootFinder(EnergyPlusData &state,
                      RootFinderDataType &RootFinderData, // Data used by root finding algorithm
                      Real64 const X,                     // X value for current iterate
                      Real64 const Y                      // Y value for current iterate, F(X)=Y
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   February 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine updates the root finder internal data to account for
    // the current iterate (X,Y):
    // - Lower / Upper support points
    // - Increments for successive iterates
    // - Convergence rate

    // METHODOLOGY EMPLOYED:
    // PRECONDITION:
    // - The current iterate (X,Y) must be a valid iterate:
    //   MinPoint%X <= LowerPoint%X < X < UpperPoint%X <= MaxPoint%X
    // - Invoke UpdateRootFinder() only if:
    //   - CheckRootFinderCandidate() returned TRUE
    //   - CheckNonSingularity() returned TRUE
    //   - CheckSlope() returned TRUE
    // POSTCONDITION:
    // - RootFinderData%LowerPoint possibly updated
    // - RootFinderData%UpperPoint possibly updated
    // - RootFinderData%CurrentPoint updated with current iterate (X,Y)
    // - RootFinderData%History(:) updated with last 3 best iterates
    // - RootFinderData%Increment updated
    // - RootFinderData%ConvergenceRate updated

    // Update history with best iterates so that:
    //   ABS(History(1)%Y) <= ABS(History(2)%Y) <= ABS(History(3)%Y)
    // Note that we must update the history before updating the lower/upper points
    UpdateHistory(RootFinderData, X, Y);

    // Update lower and upper points
    UpdateBracket(state, RootFinderData, X, Y);

    // Update increments and convergence rate
    if (RootFinderData.CurrentPoint.DefinedFlag) {
        RootFinderData.Increment.DefinedFlag = true;
        RootFinderData.Increment.X = X - RootFinderData.CurrentPoint.X;
        RootFinderData.Increment.Y = Y - RootFinderData.CurrentPoint.Y;

        if (std::abs(RootFinderData.CurrentPoint.Y) > 0.0) {
            // NOTE: Should be smaller than one for convergent process
            RootFinderData.ConvergenceRate = std::abs(Y) / std::abs(RootFinderData.CurrentPoint.Y);
        } else {
            // NOTE: Should never happen
            RootFinderData.ConvergenceRate = -1.0;
        }
    }

    // Finally update CurrentPoint (must be done last)
    RootFinderData.CurrentPoint.DefinedFlag = true;
    RootFinderData.CurrentPoint.X = X;
    RootFinderData.CurrentPoint.Y = Y;
}

void SortHistory(int const N,                // Number of points to sort in history array
                 Array1D<PointType> &History // Array of PointType variables. At least N of them
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   March 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine orders the N points in the history array in increasing
    // order of ABS(Y) values.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int I;
    int J;
    Real64 XTemp;
    Real64 YTemp;

    // Nothing to do if only one point stored in history
    if (N <= 1) {
        return;
    }

    for (I = 1; I <= N - 1; ++I) {
        for (J = I + 1; J <= N; ++J) {
            if (History(J).DefinedFlag) {
                // Swap I and J elements
                if (std::abs(History(J).Y) < std::abs(History(I).Y)) {
                    XTemp = History(I).X;
                    YTemp = History(I).Y;
                    History(I).X = History(J).X;
                    History(I).Y = History(J).Y;
                    History(J).X = XTemp;
                    History(J).Y = YTemp;
                }
            }
        }
    }
}

void AdvanceRootFinder(EnergyPlusData &state, RootFinderDataType &RootFinderData) // Data used by root finding algorithm
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   February 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine computes the next candidate value based on the information available so far.
    // Stores new value into RootFinderData%XCandidate
    // PRECONDITION:
    // na
    // POSTCONDITION:
    // - LowerPoint%X < XCandidate < UpperPoint%X
    // - RootFinderData%CurrentMethodType update with current solution method.

    // METHODOLOGY EMPLOYED:
    // The subroutine first attempts to bracket the root within a lower and upper point.
    // Once it is bracketed, then we use the specified solution methods (Bisection,
    // False position, Secant and Brent) to compute the next candidate.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    auto &XNext = state.dataGeneral->XNext;

    //----------------------------------------------------------------------------
    // First attempt to bracket root between a lower point and an upper point.
    //----------------------------------------------------------------------------

    // Detect the lower bracket
    if (!RootFinderData.LowerPoint.DefinedFlag) {
        RootFinderData.CurrentMethodType = RootFinderMethod::Bracket;
        // If we have 2 points already, try to detect lower point using the Secant formula
        if (BracketRoot(RootFinderData, XNext)) {
            RootFinderData.XCandidate = XNext;
        } else {
            if (!RootFinderData.MinPoint.DefinedFlag) {
                RootFinderData.XCandidate = RootFinderData.MinPoint.X;
            } else {
                // Should never happen
                ShowFatalError(state, "AdvanceRootFinder: Cannot find lower bracket.");
            }
        }

        // Detect the upper bracket
    } else if (!RootFinderData.UpperPoint.DefinedFlag) {
        RootFinderData.CurrentMethodType = RootFinderMethod::Bracket;
        // If we have 2 points already, try to detect upper point using the Secant formula
        if (BracketRoot(RootFinderData, XNext)) {
            RootFinderData.XCandidate = XNext;
        } else {
            if (!RootFinderData.MaxPoint.DefinedFlag) {
                RootFinderData.XCandidate = RootFinderData.MaxPoint.X;
            } else {
                // Should never happen
                ShowFatalError(state, "AdvanceRootFinder: Cannot find upper bracket.");
            }
        }

        //----------------------------------------------------------------------------
        // Root finding can start ...
        // Assumptions:
        // - the lower and upper support points are defined.
        // - the increments are defined (at least 2 history points are available)
        //----------------------------------------------------------------------------
    } else {
        switch (RootFinderData.StatusFlag) {
        case RootFinderStatus::OKRoundOff: {
            // Should never happen if we exit the root finder upon detecting round-off condition
            RootFinderData.XCandidate = BisectionMethod(RootFinderData);
        } break;
        case RootFinderStatus::WarningSingular:
        case RootFinderStatus::WarningNonMonotonic: {
            // Following local singularity or non-monotonicity warnings we attempt
            // to recover with the false position method to avoid running into trouble
            // because the latest iterate did nt produce any improvement compared to
            // the previous lower and upper brackets.
            RootFinderData.XCandidate = FalsePositionMethod(RootFinderData);
        } break;
        default: {
            // Assuming that the root is bracketed between the lower and upper points,
            // we execute the requested solution method to produce the next candidate value
            // for the root.
            switch (RootFinderData.Controls.MethodType) {
            case RootFinderMethod::Bisection: {
                // Bisection method (aka interval halving)
                RootFinderData.XCandidate = BisectionMethod(RootFinderData);
            } break;
            case RootFinderMethod::FalsePosition: {
                // False position method (aka regula falsi)
                RootFinderData.XCandidate = FalsePositionMethod(RootFinderData);
            } break;
            case RootFinderMethod::Secant: {
                // Secant method
                RootFinderData.XCandidate = SecantMethod(RootFinderData);
            } break;
            case RootFinderMethod::Brent: {
                // Brent method
                RootFinderData.XCandidate = BrentMethod(RootFinderData);
            } break;
            default: {
                ShowSevereError(state, "AdvanceRootFinder: Invalid solution method specification. Valid choices are:");
                ShowContinueError(state, EnergyPlus::format("AdvanceRootFinder: iMethodBisection={}", RootFinderMethod::Bisection));
                ShowContinueError(state, EnergyPlus::format("AdvanceRootFinder: iMethodFalsePosition={}", RootFinderMethod::FalsePosition));
                ShowContinueError(state, EnergyPlus::format("AdvanceRootFinder: iMethodSecant={}", RootFinderMethod::Secant));
                ShowContinueError(state, EnergyPlus::format("AdvanceRootFinder: iMethodBrent={}", RootFinderMethod::Brent));
                ShowFatalError(state, "AdvanceRootFinder: Preceding error causes program termination.");
            } break;
            }
        } break;
        }
    }
}

bool BracketRoot(RootFinderDataType const &RootFinderData, // Data used by root finding algorithm
                 Real64 &XNext                             // Next value
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   February 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function attempts to compute a new point that will bracket the root
    // using the secant formula to take advantage of the slope between the last 2
    // iterates.
    // Returns TRUE if successfully computed a new bracket in XNext.
    // Else returns FALSE and does not update the XNext argument.
    // Should only be used while in braketing mode (iMethodBracket).
    // When the lower and upper brackets are detected then the FUNCTION SecantMethod
    // should be used instead.
    // PRECONDITION:
    // na
    // POSTCONDITION:
    // - MinPoint%X <= XNext <= MaxPoint%X
    // - LowerPoint%X < XNext < UpperPoint%X

    // Return value
    bool BracketRoot;

    // Cannot use Secant method unless there are at least 2 points
    // Also do not use Secant method more than once, i.e. NumHistory==3, in order to avoid
    // the pathological case whereby the secant method always comes up short of bracketing
    // the root because the function slope flattens as we come closer to either min/max point.
    if (RootFinderData.NumHistory != 2) {
        BracketRoot = false;
        return BracketRoot;
    }

    // Should not use Secant method if the last 2 points produced a warning
    if (RootFinderData.StatusFlag == RootFinderStatus::WarningSingular || RootFinderData.StatusFlag == RootFinderStatus::WarningNonMonotonic) {
        BracketRoot = false;
        return BracketRoot;
    }

    // Try to compute next root candidate using Secant formula
    if (SecantFormula(RootFinderData, XNext)) {

        // Check that next candidate is consistent with min/max constraints and lower/upper brackets
        if (CheckRootFinderCandidate(RootFinderData, XNext)) {
            BracketRoot = true;
            return BracketRoot;
        }
    }

    BracketRoot = false;

    return BracketRoot;
}

Real64 BisectionMethod(RootFinderDataType &RootFinderData) // Data used by root finding algorithm
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   February 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function computes the next iterate using the bisection method (aka interval halving).
    // Convergence rate is at best linear.
    // PRECONDITION:
    // Lower and upper points must be defined and distinct.
    // POSTCONDITION:
    // - LowerPoint%X < XCandidate < UpperPoint%X
    // - RootFinderData%CurrentMethodType update with current solution method.

    // Return value
    Real64 BisectionMethod;

    RootFinderData.CurrentMethodType = RootFinderMethod::Bisection;
    BisectionMethod = (RootFinderData.LowerPoint.X + RootFinderData.UpperPoint.X) / 2.0;

    return BisectionMethod;
}

Real64 FalsePositionMethod(RootFinderDataType &RootFinderData) // Data used by root finding algorithm
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   February 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function computes the next iterate using the false position method (aka regula falsi).
    // If new iterate does not lie within the lower and upper points then
    // the Bisection method is used instead.
    // Convergence rate is at best superlinear.
    // PRECONDITION:
    // Lower and upper points must be defined and distinct.
    // POSTCONDITION:
    // - LowerPoint%X < XCandidate < UpperPoint%X
    // - RootFinderData%CurrentMethodType update with current solution method.

    // Return value
    Real64 FalsePositionMethod;

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    Real64 XCandidate;
    Real64 Num;
    Real64 Den;

    Num = RootFinderData.UpperPoint.X - RootFinderData.LowerPoint.X;
    Den = RootFinderData.UpperPoint.Y - RootFinderData.LowerPoint.Y;

    if (Den != 0.0) {
        // False position method
        RootFinderData.CurrentMethodType = RootFinderMethod::FalsePosition;
        XCandidate = RootFinderData.LowerPoint.X - RootFinderData.LowerPoint.Y * Num / Den;

        // Check that new candidate is within range and brackets
        if (!CheckRootFinderCandidate(RootFinderData, XCandidate)) {
            // Recovery method
            XCandidate = BisectionMethod(RootFinderData);
        }
    } else {
        // Recovery method
        XCandidate = BisectionMethod(RootFinderData);
    }

    FalsePositionMethod = XCandidate;
    return FalsePositionMethod;
}

Real64 SecantMethod(RootFinderDataType &RootFinderData) // Data used by root finding algorithm
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   February 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function computes the next iterate using the secant method.
    // If new iterate does not lie within the lower and upper points then
    // the false position method is used instead.
    // Convergence rate is at best superlinear.
    // PRECONDITION:
    // There must be at least 2 history points so that RootFinderData%Increment is defined.
    // See FUNCTION SecantFormula.
    // POSTCONDITION:
    // - LowerPoint%X < XCandidate < UpperPoint%X
    // - RootFinderData%CurrentMethodType update with current solution method.

    // Return value
    Real64 SecantMethod;

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    Real64 XCandidate;

    // Recover with false position
    if (SecantFormula(RootFinderData, XCandidate)) {
        // Secant method
        RootFinderData.CurrentMethodType = RootFinderMethod::Secant;

        // Check that new candidate is within range and brackets
        if (!CheckRootFinderCandidate(RootFinderData, XCandidate)) {
            // Recovery method
            XCandidate = FalsePositionMethod(RootFinderData);
        }
    } else {
        // Recovery method
        XCandidate = FalsePositionMethod(RootFinderData);
    }

    SecantMethod = XCandidate;
    return SecantMethod;
}

bool SecantFormula(RootFinderDataType const &RootFinderData, // Data used by root finding algorithm
                   Real64 &XNext                             // Result from Secant formula if possible to compute
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   April 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function computes the next iterate using the secant formula.
    // If the new iterate cannot be computed the function returns FALSE, else TRUE.
    // Convergence rate is at best superlinear.
    // PRECONDITION:
    // There must be at least 2 history points so that RootFinderData%Increment is defined.
    // POSTCONDITION:
    // XNext contains the result from applying the Secant formula.
    // If XNext could not be computed then leave XNext unchanged.

    // Return value
    bool SecantFormula;

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    Real64 Num;
    Real64 Den;

    Num = RootFinderData.Increment.X;
    Den = RootFinderData.Increment.Y;

    // Cannot use secant with infinite slope (Den==0).
    // Cannot use secant with null slope (Num==0).
    if (Den != 0.0 && Num != 0.0) {
        XNext = RootFinderData.CurrentPoint.X - RootFinderData.CurrentPoint.Y * Num / Den;
        SecantFormula = true;
    } else {
        SecantFormula = false;
    }

    return SecantFormula;
}

Real64 BrentMethod(RootFinderDataType &RootFinderData) // Data used by root finding algorithm
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   March 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function computes the next iterate using the Brent's method.
    // If new iterate does not lie within the lower and upper points then
    // the secant method is used instead.
    // Convergence rate is at best quadratic.
    // PRECONDITION:
    // Lower and upper points must be defined and distinct.
    // POSTCONDITION:
    // - LowerPoint%X < XCandidate < UpperPoint%X
    // - RootFinderData%CurrentMethodType update with current solution method.

    // METHODOLOGY EMPLOYED:
    // Inverse quadratic interpolation using the last 3 best iterates.
    // The next root estimate is x = B + P/Q whereby B is the current best estimate
    // of the root.

    // Return value
    Real64 BrentMethod;

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    Real64 XCandidate;
    Real64 A;
    Real64 FA;
    Real64 B;
    Real64 FB;
    Real64 C;
    Real64 FC;
    Real64 R;
    Real64 S;
    Real64 T;
    Real64 P;
    Real64 Q;

    // Only attempt Brent's method if enough history points are available
    // and if the root finder is converging (not diverging) since the previous
    // iterate.
    // We assume that;
    // - the root is bracketed between the lower and upper points (see AdvanceRootFinder() ).
    // - there are at least 3 history points
    if (RootFinderData.NumHistory == 3) {

        A = RootFinderData.History(2).X;
        FA = RootFinderData.History(2).Y;
        B = RootFinderData.History(1).X;
        FB = RootFinderData.History(1).Y;
        C = RootFinderData.History(3).X;
        FC = RootFinderData.History(3).Y;

        // Should never happen if CheckRootFinderConvergence() is invoked prior to this subroutine
        if (FC == 0.0) {
            BrentMethod = C;
            return BrentMethod;
            // Should never happen if CheckRootFinderConvergence() is invoked prior to this subroutine
        }
        if (FA == 0.0) {
            BrentMethod = A;
            return BrentMethod;
        }
        R = FB / FC;
        S = FB / FA;
        T = FA / FC;

        P = S * (T * (R - T) * (C - B) - (1.0 - R) * (B - A));
        Q = (T - 1.0) * (R - 1.0) * (S - 1.0);

        // Only accept correction if it is small enough (75% of previous increment)
        if (std::abs(P) <= 0.75 * std::abs(Q * RootFinderData.Increment.X)) {
            RootFinderData.CurrentMethodType = RootFinderMethod::Brent;
            XCandidate = B + P / Q;

            // Check that new candidate is within range and brackets
            if (!CheckRootFinderCandidate(RootFinderData, XCandidate)) {
                // Recovery method
                XCandidate = FalsePositionMethod(RootFinderData);
            }
        } else {
            // Recover from bad correction with bisection
            // Bisection produced the best numerical performance in testing compared to
            // - Secant
            // - False position (very slow recovery)
            XCandidate = BisectionMethod(RootFinderData);
        }

    } else {
        // Not enough history to try Brent's method yet: use Secant's method
        XCandidate = SecantMethod(RootFinderData);
    }

    BrentMethod = XCandidate;
    return BrentMethod;
}

void WriteRootFinderTraceHeader(InputOutputFile &TraceFile) // Unit for trace file
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   March 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine writes the header for the trace file to the specified
    // file unit using CSV formatting.

    print(TraceFile,
          "{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},",
          "Status",
          "Method",
          "CurrentPoint%X",
          "CurrentPoint%Y",
          "XCandidate",
          "ConvergenceRate",
          "MinPoint%X",
          "MinPoint%Y",
          "LowerPoint%X",
          "LowerPoint%Y",
          "UpperPoint%X",
          "UpperPoint%Y",
          "MaxPoint%X",
          "MaxPoint%Y",
          "History(1)%X",
          "History(1)%Y",
          "History(2)%X",
          "History(2)%Y",
          "History(3)%X",
          "History(3)%Y");
}

void WriteRootFinderTrace(InputOutputFile &TraceFile,              // Unit for trace file
                          RootFinderDataType const &RootFinderData // Data used by root finding algorithm
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   March 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine writes the current state of the root finder data to the trace file
    // unit using CSV formatting.

    print(TraceFile, "{},{},", RootFinderData.StatusFlag, RootFinderData.CurrentMethodType);

    // Only show current point if defined.
    WritePoint(TraceFile, RootFinderData.CurrentPoint, false);

    print(TraceFile, "{:20.10F},{:20.10F},", RootFinderData.XCandidate, RootFinderData.ConvergenceRate);

    // Always show min and max points.
    // Only show lower and upper points if defined.
    WritePoint(TraceFile, RootFinderData.MinPoint, true);
    WritePoint(TraceFile, RootFinderData.LowerPoint, false);
    WritePoint(TraceFile, RootFinderData.UpperPoint, false);
    WritePoint(TraceFile, RootFinderData.MaxPoint, true);
    // Only show history points if defined.
    WritePoint(TraceFile, RootFinderData.History(1), false);
    WritePoint(TraceFile, RootFinderData.History(2), false);
    WritePoint(TraceFile, RootFinderData.History(3), false);
}

void WritePoint(InputOutputFile &TraceFile, // Unit for trace file
                PointType const &PointData, // Point data structure
                bool const ShowXValue)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   March 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine writes the current point data to the trace file
    // unit using CSV formatting.
    // If not defined writes an empty string instead.

    if (PointData.DefinedFlag) {
        print(TraceFile, "{:20.10F},{:20.10F},", PointData.X, PointData.Y);
    } else {
        if (ShowXValue) {
            print(TraceFile, "{:20.10F},,", PointData.X);
        } else {
            print(TraceFile, ",,");
        }
    }
}

void DebugRootFinder(InputOutputFile &DebugFile,              // File unit where to write debugging info
                     RootFinderDataType const &RootFinderData // Data used by root finding algorithm
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   April 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine writes the current min/max range and lower/upper bracket to
    // the standard output file.
    // Used only for debugging.

    print(DebugFile, "Current = ");
    WritePoint(DebugFile, RootFinderData.CurrentPoint, true);
    print(DebugFile, "\n");

    print(DebugFile, "Min     = ");
    WritePoint(DebugFile, RootFinderData.MinPoint, true);
    print(DebugFile, "\n");

    print(DebugFile, "Lower   = ");
    WritePoint(DebugFile, RootFinderData.LowerPoint, false);
    print(DebugFile, "\n");

    print(DebugFile, "Upper   = ");
    WritePoint(DebugFile, RootFinderData.UpperPoint, false);
    print(DebugFile, "\n");

    print(DebugFile, "Max     = ");
    WritePoint(DebugFile, RootFinderData.MaxPoint, true);
    print(DebugFile, "\n");
}

void WriteRootFinderStatus(InputOutputFile &File,                   // File unit where to write the status description
                           RootFinderDataType const &RootFinderData // Data used by root finding algorithm
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Dimitri Curtil (LBNL)
    //       DATE WRITTEN   May 2006
    //       MODIFIED
    //       RE-ENGINEERED  na

    switch (RootFinderData.StatusFlag) {
    case RootFinderStatus::OK: {
        print(File, "Found unconstrained root");
    } break;
    case RootFinderStatus::OKMin: {
        print(File, "Found min constrained root");
    } break;
    case RootFinderStatus::OKMax: {
        print(File, "Found max constrained root");
    } break;
    case RootFinderStatus::OKRoundOff: {
        print(File, "Detected round-off convergence in bracket");
    } break;
    case RootFinderStatus::WarningSingular: {
        print(File, "Detected singularity warning");
    } break;
    case RootFinderStatus::WarningNonMonotonic: {
        print(File, "Detected non-monotonicity warning");
    } break;
    case RootFinderStatus::ErrorRange: {
        print(File, "Detected out-of-range error");
    } break;
    case RootFinderStatus::ErrorBracket: {
        print(File, "Detected bracket error");
    } break;
    case RootFinderStatus::ErrorSlope: {
        print(File, "Detected slope error");
    } break;
    case RootFinderStatus::ErrorSingular: {
        print(File, "Detected singularity error");
    } break;
    default: {
        print(File, "Detected bad root finder status");
    } break;
    }
}

} // namespace EnergyPlus::RootFinder
