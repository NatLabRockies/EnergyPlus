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

#ifndef HeatBalanceIntRadExchange_hh_INCLUDED
#define HeatBalanceIntRadExchange_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1S.hh>
#include <ObjexxFCL/Array2A.hh>
#include <ObjexxFCL/Array2S.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

#define EP_HBIRE_SEQ

namespace HeatBalanceIntRadExchange {

    void CalcInteriorRadExchange(EnergyPlusData &state,
                                 Array1S<Real64> const SurfaceTemp,                  // Current surface temperatures
                                 int const SurfIterations,                           // Number of iterations in calling subroutine
                                 Array1D<Real64> &NetLWRadToSurf,                    // Net long wavelength radiant exchange from other surfaces
                                 ObjexxFCL::Optional_int_const ZoneToResimulate = _, // if passed in, then only calculate for this zone
                                 std::string_view CalledFrom = "");

    void UpdateMovableInsulationFlag(EnergyPlusData &state,
                                     bool &MovableInsulationChange, // set to true if there is a change in the movable insulation state
                                     int const SurfNum              // surface number of surface being investigated
    );

    void InitInteriorRadExchange(EnergyPlusData &state);

    void InitSolarViewFactors(EnergyPlusData &state);

    void AlignInputViewFactors(EnergyPlusData &state,
                               std::string const &cCurrentModuleObject, // Object type
                               bool &ErrorsFound                        // True when errors are found
    );

    void GetInputViewFactors(EnergyPlusData &state,
                             std::string const &EnclosureName, // Needed to check for user input view factors.
                             int const N,                      // NUMBER OF SURFACES
                             Array2A<Real64> F,                // USER INPUT DIRECT VIEW FACTOR MATRIX (N X N)
                             const Array1D_int &SPtr,          // pointer to actual surface number
                             bool &NoUserInputF,               // Flag signifying no input F's for this
                             bool &ErrorsFound                 // True when errors are found in number of fields vs max args
    );

    void GetInputViewFactorsbyName(EnergyPlusData &state,
                                   std::string const &ZoneName, // Needed to check for user input view factors.
                                   int const N,                 // NUMBER OF SURFACES
                                   Array2A<Real64> F,           // USER INPUT DIRECT VIEW FACTOR MATRIX (N X N)
                                   const Array1D_int &SPtr,     // pointer to actual surface number
                                   bool &NoUserInputF,          // Flag signifying no input F's for this
                                   bool &ErrorsFound            // True when errors are found in number of fields vs max args
    );

    void CalcApproximateViewFactors(EnergyPlusData &state,
                                    int const N,                    // NUMBER OF SURFACES
                                    const Array1D<Real64> &A,       // AREA VECTOR- ASSUMED,BE N ELEMENTS LONG
                                    const Array1D<Real64> &Azimuth, // Facing angle of the surface (in degrees)
                                    const Array1D<Real64> &Tilt,    // Tilt angle of the surface (in degrees)
                                    Array2A<Real64> F,              // APPROXIMATE DIRECT VIEW FACTOR MATRIX (N X N)
                                    const Array1D_int &SPtr         // pointer to REAL(r64) surface number (for error message)
    );

    void FixViewFactors(EnergyPlusData &state,
                        int const N,                      // NUMBER OF SURFACES
                        const Array1D<Real64> &A,         // AREA VECTOR- ASSUMED,BE N ELEMENTS LONG
                        Array2A<Real64> F,                // APPROXIMATE DIRECT VIEW FACTOR MATRIX (N X N)
                        std::string &enclName,            // Name of Enclosure being fixed
                        std::vector<int> const &zoneNums, // Zones which are part of this enclosure
                        Real64 &OriginalCheckValue,       // check of SUM(F) - N
                        Real64 &FixedCheckValue,          // check after fixed of SUM(F) - N
                        Real64 &FinalCheckValue,          // the one to go with
                        int &NumIterations,               // number of iterations to fixed
                        Real64 &RowSum,                   // RowSum of Fixed
                        bool const anyIntMassInZone       // are there any surfaces in the zone that are thermal mass
    );

    bool DoesZoneHaveInternalMass(EnergyPlusData &state,
                                  int const numZoneSurfaces,     // Number of surfaces in the zone
                                  const Array1D_int &surfPointer // Pointers to the surfaces in the zone
    );

    void CalcScriptF(EnergyPlusData &state,
                     int const N,              // Number of surfaces
                     Array1D<Real64> const &A, // AREA VECTOR- ASSUMED,BE N ELEMENTS LONG
                     Array2<Real64> const &F,  // DIRECT VIEW FACTOR MATRIX (N X N)
                     Array1D<Real64> &EMISS,   // VECTOR OF SURFACE EMISSIVITIES
                     Array2<Real64> &ScriptF   // MATRIX OF SCRIPT F FACTORS (N X N) //Tuned Transposed
    );

    void CalcFMRT(EnergyPlusData &state,
                  int const N,              // Number of surfaces
                  Array1D<Real64> const &A, // AREA VECTOR- ASSUMED,BE N ELEMENTS LONG
                  Array1D<Real64> &FMRT     // VECTOR OF MEAN RADIANT TEMPERATURE "VIEW FACTORS"
    );

    void CalcFp(int const N,                  // Number of surfaces
                Array1D<Real64> const &EMISS, // VECTOR OF SURFACE EMISSIVITIES
                Array1D<Real64> const &FMRT,  // VECTOR OF MEAN RADIANT TEMPERATURE "VIEW FACTORS"
                Array1D<Real64> &Fp           // VECTOR OF OPPENHEIM RESISTNACE VALUES
    );

    void CalcMatrixInverse(Array2<Real64> &A, // Matrix: Gets reduced to L\U form
                           Array2<Real64> &I  // Returned as inverse matrix
    );

    int GetRadiantSystemSurface(EnergyPlusData &state,
                                std::string const &cCurrentModuleObject, // Calling Object type
                                std::string const &RadSysName,           // Calling Object name
                                int const RadSysZoneNum,                 // Radiant system zone number
                                std::string const &SurfaceName,          // Referenced surface name
                                bool &ErrorsFound                        // True when errors are found
    );

} // namespace HeatBalanceIntRadExchange

struct HeatBalanceIntRadExchgData : BaseGlobalStruct
{

    int MaxNumOfRadEnclosureSurfs = 0;            // Max saved to get large enough space for SurfaceTempInKto4th
    bool CarrollMethod = false;                   // Use Carroll MRT method
    bool CalcInteriorRadExchangefirstTime = true; // Logical flag for one-time initializations

    // variables added as part of strategy to reduce calculation time - Glazer 2011-04-22
    Array1D<Real64> SurfaceTempRad;
    Array1D<Real64> SurfaceTempInKto4th;
    Array1D<Real64> SurfaceEmiss;
    bool ViewFactorReport = false; // Flag to output view factor report in eio file
    int LargestSurf = 0;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->MaxNumOfRadEnclosureSurfs = 0;
        this->CarrollMethod = false;
        this->CalcInteriorRadExchangefirstTime = true;
        this->SurfaceTempRad.deallocate();
        this->SurfaceTempInKto4th.deallocate();
        this->SurfaceEmiss.deallocate();
        this->ViewFactorReport = false;
        this->LargestSurf = 0;
    }
};

} // namespace EnergyPlus

#endif
