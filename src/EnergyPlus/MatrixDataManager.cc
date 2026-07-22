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
#include <format>

// ObjexxFCL Headers
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/MatrixDataManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace MatrixDataManager {

    // Module containing the routines dealing with Matrix input objects and services

    // MODULE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   June 2010

    // PURPOSE OF THIS MODULE:
    // Process user input for Matrix: input data objects
    // Provide central services for other routines to access
    // matrix input data.

    // METHODOLOGY EMPLOYED:
    // Basic calls to InputProcessor, series of simple get and set routines

    // REFERENCES:
    // none

    // OTHER NOTES:
    // first implemented for complex fenestration

    // Data
    // MODULE PARAMETER DEFINITIONS:

    // SUBROUTINE SPECIFICATIONS FOR MODULE <module_name>:

    // todo, flush out the following routines, see CurveManager for patterns
    // PUBLIC GetMatrixValue
    // PUBLIC GetMatrixCheck
    // PUBLIC GetMatrixType
    // PUBLIC GetMatrixMinMaxValues
    // PUBLIC SetMatrixOutputMinMaxValues
    // PUBLIC GetMatrixName

    // Object Data

// MSVC was complaining that it detected a divide by zero in the Row = (El - 1) / NumCols + 1 line, indicating it thought NumCols was zero
// the compiler should never have been able to identify that, as NumCols is based directly on rNumericArgs, which is based on input values
// Apparently, interaction between the high level optimizer that does flow-graph transformations and backend that emits warnings can cause
// false positives.  The warning simply needs to be muted.  Placing the pragma at the statement itself was not sufficient for muting, so I
// placed the pragma out here at this level and it worked.  Note that this warning was only showing up on release builds, not debug builds
#pragma warning(push)
#pragma warning(disable : 4723)
    void GetMatrixInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   June 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // get input for Matrix objects

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumTwoDimMatrix;     // count of Matrix:TwoDimension objects
        int MatIndex;            // do loop counter
        int MatNum;              // index management
        int NumAlphas;           // Number of Alphas for each GetObjectItem call
        int NumNumbers;          // Number of Numbers for each GetObjectItem call
        int IOStatus;            // Used in GetObjectItem
        bool ErrorsFound(false); // Set to true if errors in input, fatal at end of routine
        auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;

        cCurrentModuleObject = "Matrix:TwoDimension";
        NumTwoDimMatrix = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        state.dataMatrixDataManager->NumMats = NumTwoDimMatrix;

        state.dataMatrixDataManager->MatData.allocate(state.dataMatrixDataManager->NumMats);

        MatNum = 0;
        for (MatIndex = 1; MatIndex <= NumTwoDimMatrix; ++MatIndex) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     MatIndex,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     _,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);
            ++MatNum;

            state.dataMatrixDataManager->MatData(MatNum).Name = state.dataIPShortCut->cAlphaArgs(1);
            int NumRows = std::floor(state.dataIPShortCut->rNumericArgs(1));
            int NumCols = std::floor(state.dataIPShortCut->rNumericArgs(2));
            int NumElements = NumRows * NumCols;

            // test
            if (NumElements < 1) {
                ShowSevereError(state, std::format("GetMatrixInput: for {}: {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state,
                                  std::format("Check {} and {} total number of elements in matrix must be 1 or more",
                                              state.dataIPShortCut->cNumericFieldNames(1),
                                              state.dataIPShortCut->cNumericFieldNames(2)));
                ErrorsFound = true;
            }
            if ((NumNumbers - 2) < NumElements) {
                ShowSevereError(state, std::format("GetMatrixInput: for {}: {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)));
                ShowContinueError(state,
                                  std::format("Check input, total number of elements does not agree with {} and {}",
                                              state.dataIPShortCut->cNumericFieldNames(1),
                                              state.dataIPShortCut->cNumericFieldNames(2)));
                ErrorsFound = true;
            }
            state.dataMatrixDataManager->MatData(MatNum).MatrixType = TwoDimensional;
            // Note With change to row-major arrays the "row" and "col" usage here is transposed
            auto &matrix(state.dataMatrixDataManager->MatData(MatNum).Mat2D);
            matrix.allocate(NumCols, NumRows); // This is standard order for a NumRows X NumCols matrix
            Array2<Real64>::size_type l(0);
            for (int ElementNum = 1; ElementNum <= NumElements; ++ElementNum, l += matrix.size()) {
                int const RowIndex = (ElementNum - 1) / NumCols + 1;
                int const ColIndex = mod((ElementNum - 1), NumCols) + 1;
                matrix(ColIndex, RowIndex) = state.dataIPShortCut->rNumericArgs(ElementNum + 2); // Matrix is read in row-by-row
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, "GetMatrixInput: Errors found in Matrix objects. Preceding condition(s) cause termination.");
        }
    }
#pragma warning(pop)

    int MatrixIndex(EnergyPlusData &state, std::string const &MatrixName)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   June 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Return integer index or pointer to MatData structure array

        // METHODOLOGY EMPLOYED:
        // inputs name of matrix and returns integer index
        // currently uses Util::FindItemInList( which is case sensitive

        // Return value
        int MatrixIndexPtr; // Function result

        // FUNCTION LOCAL VARIABLE DECLARATIONS:

        if (state.dataUtilityRoutines->GetMatrixInputFlag) {
            GetMatrixInput(state);
            state.dataUtilityRoutines->GetMatrixInputFlag = false;
        }

        if (state.dataMatrixDataManager->NumMats > 0) {
            MatrixIndexPtr = Util::FindItemInList(MatrixName, state.dataMatrixDataManager->MatData);
        } else {
            MatrixIndexPtr = 0;
        }

        return MatrixIndexPtr;
    }

    void Get2DMatrix(EnergyPlusData &state,
                     int const Idx, // pointer index to location in MatData
                     Array2S<Real64> Mat2D)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   June 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // pass matrix to calling routine

        if (Idx > 0) { // protect hard crash
            Mat2D = state.dataMatrixDataManager->MatData(Idx).Mat2D;
        } else {
            // do nothing (?) throw dev error
        }
    }

    void Get2DMatrixDimensions(EnergyPlusData &state,
                               int const Idx, // pointer index to location in MatData
                               int &NumRows,
                               int &NumCols)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   June 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // <description>

        if (Idx > 0) {
            NumRows = state.dataMatrixDataManager->MatData(Idx).Mat2D.isize(2);
            NumCols = state.dataMatrixDataManager->MatData(Idx).Mat2D.isize(1);
        } else {
            // do nothing (?) throw dev error?
        }
    }

} // namespace MatrixDataManager

} // namespace EnergyPlus
