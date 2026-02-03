// Methods for parsing XML files.

/*
********************************************************************
Copyright Notice
----------------

Building Controls Virtual Test Bed (BCVTB) Copyright (c) 2008, The
Regents of the University of California, through Lawrence Berkeley
National Laboratory (subject to receipt of any required approvals from
the U.S. Dept. of Energy). All rights reserved.

If you have questions about your rights to use or distribute this
software, please contact Berkeley Lab's Technology Transfer Department
at TTD@lbl.gov

NOTICE.  This software was developed under partial funding from the U.S.
Department of Energy.  As such, the U.S. Government has been granted for
itself and others acting on its behalf a paid-up, nonexclusive,
irrevocable, worldwide license in the Software to reproduce, prepare
derivative works, and perform publicly and display publicly.  Beginning
five (5) years after the date permission to assert copyright is obtained
from the U.S. Department of Energy, and subject to any subsequent five
(5) year renewals, the U.S. Government is granted for itself and others
acting on its behalf a paid-up, nonexclusive, irrevocable, worldwide
license in the Software to reproduce, prepare derivative works,
distribute copies to the public, perform publicly and display publicly,
and to permit others to do so.


Modified BSD License agreement
------------------------------

Building Controls Virtual Test Bed (BCVTB) Copyright (c) 2008, The
Regents of the University of California, through Lawrence Berkeley
National Laboratory (subject to receipt of any required approvals from
the U.S. Dept. of Energy).  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.
   3. Neither the name of the University of California, Lawrence
      Berkeley National Laboratory, U.S. Dept. of Energy nor the names
      of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

You are under no obligation whatsoever to provide any bug fixes,
patches, or upgrades to the features, functionality or performance of
the source code ("Enhancements") to anyone; however, if you choose to
make your Enhancements available either publicly, or directly to
Lawrence Berkeley National Laboratory, without imposing a separate
written license agreement for such Enhancements, then you hereby grant
the following license: a non-exclusive, royalty-free perpetual license
to install, use, modify, prepare derivative works, incorporate into
other computer software, distribute, and sublicense such enhancements or
derivative works thereof, in binary and source code form.

********************************************************************
*/

#ifndef utilXml_h_INCLUDED
#define utilXml_h_INCLUDED

///////////////////////////////////////////////////////////
/// \file    utilXml.h
/// \brief   Methods for getting xml values
///          using the expat libray
///
/// \author  Rui Zhang
///          Carnegie Mellon University
///          ruiz@cmu.edu
/// \date    2009-08-11
///
/// \version $Id: utilXml.h 55724 2009-09-16 17:51:58Z mwetter $
///
/// This file provides methods to get general xml values \c getxmlvalue
/// using simple xpath expressions
/// values will be in the same order as they are in the xml file
///
/// This file also provides methods to get the EnergyPlus \c getepvariables.
/// The variables returned will be in the same order as they are in the
/// configuration file.
/// \sa getxmlvalue()
/// \sa getxmlvaluesf()
/// \sa getepvariables()
///
//////////////////////////////////////////////////////////

#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
#include <expat.h>
//#include <errno.h>

#if defined(__amigaos__) && defined(__USE_INLINE__)
#  include <proto/expat.h>
#endif

#ifdef XML_LARGE_SIZE
#  if defined(XML_USE_MSC_EXTENSIONS) && _MSC_VER < 1400
#    define XML_FMT_INT_MOD "I64"
#  else
#    define XML_FMT_INT_MOD "ll"
#  endif
#else
#  define XML_FMT_INT_MOD "l"
#endif

#define BUFFSIZE 8192

////////////////////////////////////////////////////////////////
///\struct A simple stack structure to keep track of the parent elements
////////////////////////////////////////////////////////////////
typedef struct Stack2
{
  char** head;
  int top;
  int cur;
} Stack2;

extern Stack2 expStk;  ///< Variables for getxmlvalue function

extern char* att;         ///< Local global variable for function \c getxmlvalue
extern char* vals;        ///< Local global variable for function \c getxmlvalue
extern size_t* numVals;   ///< Local global variable for function \c getxmlvalue
extern int PARSEVALUE;    ///< flag for parsing xml values 1 if parse, 0 if not parse
extern int ERROR_STATUS;  ///< flag for xml element handler error status settings

////////////////////////////////////////////////////////////////
/// local global variables for function \c getepvariables
////////////////////////////////////////////////////////////////
extern char* outputVarsName;  ///< the string pointer to the parsed output variable names
extern char* outputVarsType;  ///< the string pointer to the parsed output variable types
extern int* numOutputVars;    ///< the integer pointer to the number of output variables
extern char* inputVars;       ///< the string pointer to the input variables
extern int* numInputVars;     ///< the integer pointer to the number of input variables
extern int* inputVarsType;    ///< the integer array to store the types of each input variables
extern char** inputKeys;      ///< the string array to store the types of input variable types
extern int numInputKeys;      ///< the number of input variable types
extern int source;            ///< flag for function /c getepvariables 0=EnergyPlus, 1=Ptolemy
extern size_t const* strLen;  ///< the length of string parsed to this function

////////////////////////////////////////////////////////////////
///  This method frees the local memory allocated
///
///\param strArr 1D string array to be freed
///\param n the size of the 1D string array
////////////////////////////////////////////////////////////////
void freeResource(char** strArr, int n);

////////////////////////////////////////////////////////////////
///  This method will return the input and output variable for EnergyPlus
///  in sequence
///
///\param fileName the variable configuration file name.
///\param myOutputVarsName Array to store the output variable names found.
///\param myOutputvarsType Array to store the output variable types found.
///\param myNumOutputVars Integer holder to store number of output variables found.
///\param myInputKeys Array to store the input variable keys.
///\param myNumInputKeys Integer holder to store number of input variable keys.
///\param myInputVars Array to store the name of input variables found.
///\param myNumInputVars Integer holder to store number of input variables found.
///\param myInputVarsType Integer array to store the corresponding input variable types in myInputVars.
///\param myStrLen The length of the string that is passed to this function.
///
////////////////////////////////////////////////////////////////
int getepvariables(char const* const fileName, char* const myOutputVarsName, char* const myOutputVarsType, int* const myNumOutputVars,
                   char const* const myInputKeys, int const* const myNumInputKeys, char* const myInputVars, int* const myNumInputVars,
                   int* const myInputVarsType, size_t const* const myStrLen);

////////////////////////////////////////////////////////////////
///  This method will return the input and output variable for EnergyPlus
///  in sequence. The difference with getepvariables is that it does not
///  validate the configuration file
///
///\param fileName the variable configuration file name.
///\param myOutputVarsName Array to store the output variable names found.
///\param myOutputvarsType Array to store the output variable types found.
///\param myNumOutputVars Integer holder to store number of output variables found.
///\param myInputKeys Array to store the input variable keys.
///\param myNumInputKeys Integer holder to store number of input variable keys.
///\param myInputVars Array to store the name of input variables found.
///\param myNumInputVars Integer holder to store number of input variables found.
///\param myInputVarsType Integer array to store the corresponding input variable types in myInputVars.
///\param myStrLen The length of the string that is passed to this function.
///
////////////////////////////////////////////////////////////////
int getepvariablesFMU(char const* const fileName, char* const myOutputVarsName, char* const myOutputVarsType, int* const myNumOutputVars,
                      char const* const myInputKeys, int const* const myNumInputKeys, char* const myInputVars, int* const myNumInputVars,
                      int* const myInputVarsType, size_t const* const myStrLen);

////////////////////////////////////////////////////////////////
/// Stack operation, this function will pop one element from stack
/// and will free the resource unused
////////////////////////////////////////////////////////////////
int stackPopBCVTB();

////////////////////////////////////////////////////////////////
/// Stack operation, will push one element into the stack
/// and will allocate memory for the new element, hence is deep copy
////////////////////////////////////////////////////////////////
int stackPushBCVTB(char const* str);

////////////////////////////////////////////////////////////////
/// This is a general function that returns the value according to \c exp
///
/// \c exp mimics the xPath expression.
/// Its format is //el1/../eln[@attr]
/// which will return the \c attr value of \c eln,
/// where \c eln is the n-th child of \c el1
///
/// Example: //variable/EnergyPlus[@name] will return the name attributes of EnergyPlus
/// which is equivalent to //EnergyPlus[@name]
///
///\param fileName the xml file name.
///\param exp the xPath expression.
///\param myVals string to store the found values, semicolon separated.
///\param mynumVals number of values found.
///\param myStrLen length of the string that is passed.
////////////////////////////////////////////////////////////////
int getxmlvalues(char const* const fileName, char const* const exp, char* const myVals, size_t* const myNumVals, size_t const myStrLen);

////////////////////////////////////////////////////////////////
/// This method returns the number of xmlvalues given xPath expressions.
/// This method will call the function \c getxmlvalues
///
/// \c exp mimics the xPath expression.
/// Its format is //el1/../eln[@attr]
/// which will return the \c attr value of \c eln,
/// where \c eln is the n-th child of \c el1
///
/// Example: //variable/EnergyPlus[@name] will return the name attributes of EnergyPlus
/// which is equivalent to //EnergyPlus[@name]
///
///\param fileName the name of the xml file
///\param exp the xPath expression
////////////////////////////////////////////////////////////////
size_t getnumberofxmlvalues(char const* const fileName, char const* const exp);

////////////////////////////////////////////////////////////////
/// This method returns the xmlvalues parsed given xPath expressions.
/// This method will first perform a validation check with DTDValidator
/// For compatibility with BCVTB 0.2 this function is mainly for E+
/// to get the input and output variables in variables.cfg. Thus the
/// dtd file for the validity checking is the variables.dtd.
/// Then the function calls \c getxmlvalues to get the variables
/// and appends ";" at the end of the parsed string.
///
/// Return value: 0 normal; -1 error
///
/// \c exp mimics the xPath expression.
/// Its format is //el1/../eln[@attr]
/// which will return the \c attr value of \c eln,
/// where \c eln is the n-th child of \c el1
///
/// Example: //variable/EnergyPlus[@name] will return the name attributes of EnergyPlus
/// which is equivalent to //EnergyPlus[@name]
///
///\param fileName the xml file name;
///\param exp the xPath expression.
///\param atrName the attribute name.
///\param nVal number of attribute values found.
///\param str string to store the found values, semicolon separated.
///\param strLen the string length allocated
////////////////////////////////////////////////////////////////
int getxmlvaluesf(char const* const fileName, char const* const exp, char const* const atrName, size_t* const nVal, char* str, size_t* const strLen);

////////////////////////////////////////////////////////////////
/// This method returns one xmlvalue for a given xPath expressions.
/// The function will call the function \c getxmlvalues to get the variables
/// without ";" at the end of the parsed string
///
/// Return values: 0 normal; -1 error
///
/// \c exp mimics the xPath expression.
/// Its format is //el1/../eln[@attr]
/// which will return the \c attr value of \c eln,
/// where \c eln is the n-th child of \c el1
///
/// Example: //variable/EnergyPlus[@name] will return the name attributes of EnergyPlus
/// which is equivalent to //EnergyPlus[@name]
///
///\param fileName the xml file name.
///\param exp the xPath expression.
///\param str string to store the found values, semicolon separated.
///\param nVals number of values found.
///\param strLen the string length allocated.
////////////////////////////////////////////////////////////////
int getxmlvalue(char const* const fileName, char const* const exp, char* const str, size_t* const nVals, int const strLen);

////////////////////////////////////////////////////////////////
/// This method checks the validity of the variables
/// configuration xml file for a given dtd file that is
/// specified in the variables configuration file
///
/// Return values: -1 Error in the file
///                 0 File is validate
///
///
////////////////////////////////////////////////////////////////
int check_variable_cfg_Validate(char const* const fileName);

#endif  // utilXml_h_INCLUDED
