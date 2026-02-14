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

// ObjexxFCL Headers
#include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/DataRuntimeLanguage.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

// Data only module for EMS runtime language

namespace DataRuntimeLanguage {

    // MODULE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   June 2006
    //       MODIFIED       Brent Griffith, May 2009
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:

    // METHODOLOGY EMPLOYED: na

    // Data
    // module should be available to other modules and routines.
    // Thus, all variables in this module must be PUBLIC.

    void ValidateEMSVariableName(EnergyPlusData &state,
                                 std::string const &cModuleObject, // the current object name
                                 std::string const &cFieldValue,   // the field value
                                 std::string const &cFieldName,    // the current field name
                                 bool &errFlag,                    // true if errors found in this routine, false otherwise.
                                 bool &ErrorsFound                 // true if errors found in this routine, untouched otherwise.
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   May 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Consolidate error checking on EMS variable names.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view InvalidStartCharacters("0123456789");

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        errFlag = false;
        if (has(cFieldValue, ' ')) {
            ShowSevereError(state, format("{}=\"{}\", Invalid variable name entered.", cModuleObject, cFieldValue));
            ShowContinueError(state, format("...{}; Names used as EMS variables cannot contain spaces", cFieldName));
            errFlag = true;
            ErrorsFound = true;
        }
        if (has(cFieldValue, '-')) {
            ShowSevereError(state, format("{}=\"{}\", Invalid variable name entered.", cModuleObject, cFieldValue));
            ShowContinueError(state, format("...{}; Names used as EMS variables cannot contain \"-\" characters.", cFieldName));
            errFlag = true;
            ErrorsFound = true;
        }
        if (has(cFieldValue, '+')) {
            ShowSevereError(state, format("{}=\"{}\", Invalid variable name entered.", cModuleObject, cFieldValue));
            ShowContinueError(state, format("...{}; Names used as EMS variables cannot contain \"+\" characters.", cFieldName));
            errFlag = true;
            ErrorsFound = true;
        }
        if (has(cFieldValue, '.')) {
            ShowSevereError(state, format("{}=\"{}\", Invalid variable name entered.", cModuleObject, cFieldValue));
            ShowContinueError(state, format("...{}; Names used as EMS variables cannot contain \".\" characters.", cFieldName));
            errFlag = true;
            ErrorsFound = true;
        }
        if ((!cFieldValue.empty()) && (is_any_of(cFieldValue[0], InvalidStartCharacters))) {
            ShowSevereError(state, format("{}=\"{}\", Invalid variable name entered.", cModuleObject, cFieldValue));
            ShowContinueError(state, format("...{}; Names used as EMS variables cannot start with numeric characters.", cFieldName));
            errFlag = true;
            ErrorsFound = true;
        }
    }

    void ValidateEMSProgramName(EnergyPlusData &state,
                                std::string const &cModuleObject, // the current object name
                                std::string const &cFieldValue,   // the field value
                                std::string const &cFieldName,    // the current field name
                                std::string const &cSubType,      // sub type = Program or Subroutine
                                bool &errFlag,                    // true if errors found in this routine, false otherwise.
                                bool &ErrorsFound                 // true if errors found in this routine, untouched otherwise.
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Linda Lawrie
        //       DATE WRITTEN   May 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Consolidate error checking on EMS variable names.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        // INTEGER :: pos

        errFlag = false;
        if (has(cFieldValue, ' ')) {
            ShowSevereError(state, format("{}=\"{}\", Invalid variable name entered.", cModuleObject, cFieldValue));
            ShowContinueError(state, format("...{}; Names used for EMS {} cannot contain spaces", cFieldName, cSubType));
            errFlag = true;
            ErrorsFound = true;
        }
        if (has(cFieldValue, '-')) {
            ShowSevereError(state, format("{}=\"{}\", Invalid variable name entered.", cModuleObject, cFieldValue));
            ShowContinueError(state, format("...{}; Names used for EMS {} cannot contain \"-\" characters.", cFieldName, cSubType));
            errFlag = true;
            ErrorsFound = true;
        }
        if (has(cFieldValue, '+')) {
            ShowSevereError(state, format("{}=\"{}\", Invalid variable name entered.", cModuleObject, cFieldValue));
            ShowContinueError(state, format("...{}; Names used for EMS {} cannot contain \"+\" characters.", cFieldName, cSubType));
            errFlag = true;
            ErrorsFound = true;
        }
        //  pos=SCAN(cFieldValue(1:1),InvalidStartCharacters)
        //  IF (pos > 0) THEN
        //    CALL ShowSevereError(state, TRIM(cModuleObject)//'="'//TRIM(cFieldValue)//'", Invalid variable name entered.')
        //    CALL ShowContinueError(state, '...'//TRIM(cFieldName)//'; Names used as EMS variables cannot start with numeric characters.')
        //    errFlag=.TRUE.
        //    ErrorsFound = .TRUE.
        //  ENDIF
    }

} // namespace DataRuntimeLanguage

} // namespace EnergyPlus
