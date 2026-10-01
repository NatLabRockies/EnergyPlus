#ifndef fmiPlatformTypes_h
#define fmiPlatformTypes_h

/* Standard header file to define the argument types of the
   functions of the Model Execution Interface.
   This header file must be utilized both by the model and
   by the simulation engine.

   Revisions:
   - October 2010: First public Version


   Copyright © 2008-2010, MODELISAR consortium. All rights reserved.
   This file is licensed by the copyright holders under the BSD License
   (http://www.opensource.org/licenses/bsd-license.html):


   ----------------------------------------------------------------------------
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   - Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.
   - Neither the name of the copyright holders nor the names of its
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
   ----------------------------------------------------------------------------
*/

/* Platform (combination of machine, compiler, operating system) */
#define fmiPlatform "standard32"

/* Type definitions of variables passed as arguments
   Version "standard32" means:

   fmiComponent     : 32 bit pointer
   fmiValueReference: 32 bit
   fmiReal          : 64 bit
   fmiInteger       : 32 bit
   fmiBoolean       :  8 bit
   fmiString        : 32 bit pointer

*/
// mwetter: added ifndef
#ifndef fmiModelTypes_h
typedef void *fmiComponent;
typedef unsigned int fmiValueReference;
typedef double fmiReal;
typedef int fmiInteger;
typedef char fmiBoolean;
typedef const char *fmiString;

/* Values for fmiBoolean  */
#define fmiTrue 1
#define fmiFalse 0

/* Undefined value for fmiValueReference (largest unsigned int value) */
#define fmiUndefinedValueReference (fmiValueReference)(-1)

#endif // added mwetter
#endif
