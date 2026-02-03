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

#ifndef TARCOGParams_hh_INCLUDED
#define TARCOGParams_hh_INCLUDED

// EnergyPlus Headers
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus::TARCOGParams {

Real64 constexpr e(2.718281828459);
Real64 constexpr DeflectionRelaxation(0.005); // Deflection relaxation parameter
int constexpr DeflectionMaxIterations(400);   // maximum number of deflection iterations
Real64 constexpr DeflectionErrorMargin(0.01); // maximum temperature difference on layers for deflection iterations

int constexpr maxlay(100);          // maximum number of layers (including laminates) (100)
int constexpr MaxGap(maxlay - 1);   // maximum number of gaps (between layers)
int constexpr maxlay1(maxlay + 1);  // maximum number of 'gaps', including in and out (maxlay+1)
int constexpr maxlay2(maxlay * 2);  // maximum number of glass surfaces (maxlay*2)
int constexpr maxlay3(maxlay2 + 1); // maximum number of ? (maxlay2+1)

//  Layer types:
enum class TARCOGLayerType : int
{
    Invalid = -1,
    SPECULAR,
    VENETBLIND_HORIZ,
    WOVSHADE,
    PERFORATED,
    DIFFSHADE,
    BSDF,
    VENETBLIND_VERT,
    Num
};

extern const std::array<std::string_view, (int)TARCOGLayerType::Num> layerTypeNamesUC;

//  Thermal models:
enum class TARCOGThermalModel : int
{
    Invalid = -1,
    ISO15099,
    SCW,
    CSM,
    CSM_WithSDThickness,
    Num
};

extern const std::array<std::string_view, (int)TARCOGThermalModel::Num> thermalModelNamesUC;

int constexpr YES_SupportPillar = 1; // Exsqueeze me?

// Deflection parameters
enum class DeflectionCalculation : int
{
    Invalid = -1,
    NONE,
    TEMPERATURE,
    GAP_WIDTHS,
    Num
};

extern const std::array<std::string_view, (int)DeflectionCalculation::Num> deflectionCalculationNamesUC;

// definition of parameters for deflection sum.  These parameters define maximum number of loop to which sum
// will perform. By equation, these numbers will go to infinite and some test showed that going to NMax and MMax
// values would produce enough precision

constexpr int MMax = 5; // top m value for which "deflection sum" will be calculated
constexpr int NMax = 5; // top n value for which "deflection sum" will be calculated

int constexpr NumOfIterations(100);

// Program will examine convergence parameter in each iteration.  That convergence parameter should decrease each time.
// In case that is not happening program will tolerate certain number of tries before declare convergence
// (or decrease relaxation parameter)
int constexpr NumOfTries(5);
// integer, parameter :: NewtonIterations = 75 ! shows when to switch to Newton
Real64 constexpr RelaxationStart(0.6);    // Has to be between 0 and 1
Real64 constexpr RelaxationDecrease(0.1); // Step for which relaxation parameter will decrease

// Convergence parameters
Real64 constexpr ConvergenceTolerance(1e-2); // tolerance used within iterations

// Airflow iterations
Real64 constexpr AirflowConvergenceTolerance(1e-2);
Real64 constexpr AirflowRelaxationParameter(0.9);

Real64 constexpr TemperatureQuessDiff(1.0); // in case outside and inside temperatures are identical

// Coefficients for new airflow algorithm.
// Robert Hart, Howdy Goudey & D. Charlie Curcija (2017): Experimental
// validation and model development for thermal transmittances of porous window screens
// and horizontal louvred blind systems, Journal of Building Performance Simulation, DOI:
// 10.1080/19401493.2017.1323010

Real64 constexpr C1_VENET_HORIZONTAL(0.016);
Real64 constexpr C2_VENET_HORIZONTAL(-0.63);
Real64 constexpr C3_VENET_HORIZONTAL(0.53);
Real64 constexpr C4_VENET_HORIZONTAL(0.043);

Real64 constexpr C1_VENET_VERTICAL(0.041);
Real64 constexpr C2_VENET_VERTICAL(0.000);
Real64 constexpr C3_VENET_VERTICAL(0.270);
Real64 constexpr C4_VENET_VERTICAL(0.012);

Real64 constexpr C1_SHADE(0.078);
Real64 constexpr C2_SHADE(1.2);
Real64 constexpr C3_SHADE(1.0);
Real64 constexpr C4_SHADE(1.0);

} // namespace EnergyPlus::TARCOGParams

#endif
