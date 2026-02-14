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

#ifndef MatrixDataManager_hh_INCLUDED
#define MatrixDataManager_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>
#include <ObjexxFCL/Array2S.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace MatrixDataManager {

    // Using/Aliasing

    // Data
    // MODULE PARAMETER DEFINITIONS:
    // INTEGER, PARAMETER :: OneDimensional = 1
    int constexpr TwoDimensional = 2;
    // INTEGER, PARAMETER :: ThreeDimensional = 3

    // DERIVED TYPE DEFINITIONS:
    // na

    // MODULE VARIABLE DECLARATIONS:

    // SUBROUTINE SPECIFICATIONS FOR MODULE <module_name>:

    // todo, flush out the following routines, see CurveManager for patterns
    // PUBLIC GetMatrixValue
    // PUBLIC GetMatrixCheck
    // PUBLIC GetMatrixType
    // PUBLIC GetMatrixMinMaxValues
    // PUBLIC SetMatrixOutputMinMaxValues
    // PUBLIC GetMatrixName

    // Types

    struct MatrixDataStruct
    {
        // Members
        std::string Name; // Matrix Name
        int MatrixType;
        // REAL(r64), DIMENSION(:), ALLOCATABLE     :: Mat1D ! hold data if one dimensional
        Array2D<Real64> Mat2D; // hold data if two dimensional
        // REAL(r64), DIMENSION(:,:,:), Allocatable :: Mat3D ! hold data if three dimensional

        // Default Constructor
        MatrixDataStruct() : MatrixType(0)
        {
        }
    };

    // Object Data

    // Functions

    void GetMatrixInput(EnergyPlusData &state);

    int MatrixIndex(EnergyPlusData &state, std::string const &MatrixName);

    void Get2DMatrix(EnergyPlusData &state,
                     int const Idx, // pointer index to location in MatData
                     Array2S<Real64> Mat2D);

    void Get2DMatrixDimensions(EnergyPlusData &state,
                               int const Idx, // pointer index to location in MatData
                               int &NumRows,
                               int &NumCols);

} // namespace MatrixDataManager

struct MatrixDataManagerData : BaseGlobalStruct
{

    Array1D<MatrixDataManager::MatrixDataStruct> MatData;
    int NumMats; // number of matracies in input file

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        MatData.clear();
        NumMats = int();
    }
};

} // namespace EnergyPlus

#endif
