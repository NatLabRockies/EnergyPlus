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
#pragma once

#include <functional>
#include <memory>
#include <vector>

#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus::GroundHeatExchangers::GLHEC {

struct FluidPropertyFunctions
{
    std::function<Real64(Real64)> cp;
    std::function<Real64(Real64)> rho;
    std::function<Real64(Real64)> viscosity;
    std::function<Real64(Real64)> conductivity;
};

struct DynamicAggregationSettings
{
    Real64 expansionRate = 1.62;
    int binsPerLevel = 9;
    Real64 simulationHorizonSeconds = 50.0 * 365.0 * 24.0 * 3600.0;
};

struct OdeSolverSettings
{
    Real64 absoluteTolerance = 1.0e-10;
    Real64 relativeTolerance = 1.0e-8;
    int stepsPerTimeStep = 20;
    Real64 minInitialStep = 1.0;
};

struct ModelConfig
{
    Real64 boreholeLength = 0.0;
    Real64 boreholeDiameter = 0.0;
    Real64 shankSpacing = 0.0;

    Real64 groutConductivity = 0.0;
    Real64 groutDensity = 0.0;
    Real64 groutSpecificHeat = 0.0;

    Real64 pipeConductivity = 0.0;
    Real64 pipeDensity = 0.0;
    Real64 pipeSpecificHeat = 0.0;
    Real64 pipeInnerDiameter = 0.0;
    Real64 pipeOuterDiameter = 0.0;

    Real64 soilConductivity = 0.0;
    Real64 soilDiffusivity = 0.0;

    unsigned int numBoreholes = 1;
    int numSegments = 10;
    int numIterations = 2;
    Real64 groutFraction = 0.5;
    int pipeTransitCells = 16;
    bool applyPipeTransitDelay = true;

    DynamicAggregationSettings aggregation{};
    OdeSolverSettings ode{};
    std::vector<Real64> lntts;
    std::vector<Real64> gValues;
    std::vector<Real64> gBValues;
};

struct ModelStepInputs
{
    int timeSeconds = 0;
    int timeStepSeconds = 0;
    Real64 massFlowRate = 0.0;
    Real64 inletTemp = 0.0;
    Real64 farFieldGroundTemp = 0.0;
};

struct ModelStepOutputs
{
    Real64 outletTemp = 0.0;
    Real64 heatRate = 0.0;
    Real64 boreholeHeatRate = 0.0;
    Real64 boreholeWallTemp = 0.0;
    Real64 avgFluidTemp = 0.0;
};

class Model
{
public:
    Model(ModelConfig config, FluidPropertyFunctions fluidProps);
    ~Model();

    void reset(Real64 initialTemp);
    [[nodiscard]] ModelStepOutputs simulate(ModelStepInputs const &inputs);

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace EnergyPlus::GroundHeatExchangers::GLHEC
