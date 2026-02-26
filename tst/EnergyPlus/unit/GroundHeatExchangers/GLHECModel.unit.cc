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
#include <EnergyPlus/GroundHeatExchangers/GLHEC/Model.hh>

#include <cmath>
#include <vector>

#include <gtest/gtest.h>

using namespace EnergyPlus;
using namespace EnergyPlus::GroundHeatExchangers::GLHEC;

namespace {

ModelConfig makeConfig()
{
    ModelConfig cfg;
    cfg.boreholeLength = 100.0;
    cfg.boreholeDiameter = 0.109982;
    cfg.shankSpacing = 0.04556;

    cfg.groutConductivity = 0.744;
    cfg.groutDensity = 2200.0;
    cfg.groutSpecificHeat = 1772.7272727273;

    cfg.pipeConductivity = 0.389;
    cfg.pipeDensity = 950.0;
    cfg.pipeSpecificHeat = 1863.1578947368;
    cfg.pipeInnerDiameter = 0.02184;
    cfg.pipeOuterDiameter = 0.02670;

    cfg.soilConductivity = 2.423;
    cfg.soilDiffusivity = 1.03e-6;

    cfg.numBoreholes = 10;
    cfg.numSegments = 8;
    cfg.numIterations = 2;
    cfg.groutFraction = 0.5;

    cfg.lntts = {-12.0, -10.0, -8.0, -6.0, -4.0, -2.0, 0.0, 2.0};
    cfg.gValues = {0.0, 0.10, 0.30, 0.65, 1.05, 1.45, 1.90, 2.20};
    cfg.gBValues = cfg.gValues;

    return cfg;
}

FluidPropertyFunctions makeFluidFuncs()
{
    FluidPropertyFunctions funcs;
    funcs.cp = [](Real64) { return 4180.0; };
    funcs.rho = [](Real64) { return 997.0; };
    funcs.viscosity = [](Real64) { return 1.0e-3; };
    funcs.conductivity = [](Real64) { return 0.60; };
    return funcs;
}

ModelConfig makePrototypeLikeConfig()
{
    ModelConfig cfg;
    cfg.boreholeLength = 76.2;
    cfg.boreholeDiameter = 0.114;
    cfg.shankSpacing = 0.0469;

    cfg.groutConductivity = 0.85;
    cfg.groutDensity = 2500.0;
    cfg.groutSpecificHeat = 1560.0;

    cfg.pipeConductivity = 0.39;
    cfg.pipeDensity = 950.0;
    cfg.pipeSpecificHeat = 1900.0;
    cfg.pipeInnerDiameter = 0.0218;
    cfg.pipeOuterDiameter = 0.0267;

    cfg.soilConductivity = 2.7;
    cfg.soilDiffusivity = 2.7 / (2500.0 * 880.0);

    cfg.numBoreholes = 1;
    cfg.numSegments = 10;
    cfg.numIterations = 2;
    cfg.groutFraction = 0.5;

    cfg.aggregation.expansionRate = 1.5;
    cfg.aggregation.binsPerLevel = 9;
    cfg.aggregation.simulationHorizonSeconds = 8760.0 * 3600.0;

    cfg.lntts = {-17.075114469809652, -16.627802251765988, -16.331956868675046, -16.153708637268725, -15.959552622827768, -15.77583148567939,
                 -15.682430310590989, -15.591181639125844, -15.405957322673929, -15.313726098457895, -15.217215198077051, -15.125548009551228,
                 -15.034893641283096, -14.939583461478772, -14.847031904113527, -14.754739835252108, -14.661033872590263, -14.569065383447843,
                 -14.475277853347674, -14.380833849431198, -14.288205693795122, -14.196204155432493, -14.102729560252651, -14.008852045449823,
                 -13.916478725318807, -13.823945223220736, -13.731075501987444, -13.637824206995248, -13.544239264838964, -13.451115475629575,
                 -13.358420327512839, -13.2656070279071,   -13.172365101166834, -13.079504748273239, -12.986770782860688, -12.893651698697814,
                 -12.800587389042345, -12.70761593569868,  -12.614525512632667, -12.52123757820911,  -12.428015210032074, -12.335099651310028,
                 -12.242093878427184, -12.148976233736313, -12.055968486045487, -11.962972229371559, -11.869981339358326, -11.777053546396756,
                 -11.684054318962536, -11.590956105944782, -11.497903536842914, -11.404879940036198, -11.311854335522456, -11.218794337158441,
                 -11.125741419279972, -11.032725286003604, -10.939716092757104, -10.846654041512448, -10.753608576410747, -10.660605202704337,
                 -10.567568901988443, -10.47451163808514,  -10.381473119346294, -10.28845540129876,  -10.19541578861004,  -10.102388070966573,
                 -10.009347841117759, -9.9163012056817497, -9.8232786085743538, -9.73024635104019,   -9.6372137108808698, -9.5441806323696312,
                 -9.4511467373365736, -9.3581170082252108, -9.2650841704847497, -9.172048759581525,  -9.0790130948574852, -8.9859781305394382,
                 -8.8929422609011795, -8.7999079175091044, -8.7068735276925988, -8.6138406411697463, -8.5208083076518033, -8.427775096313507,
                 -8.3347408046839213, -8.2416330604701553};

    cfg.gValues = {0.0,
                   0.0,
                   0.0,
                   0.0,
                   0.0,
                   0.0,
                   0.0,
                   0.0,
                   0.0,
                   0.0,
                   0.0,
                   0.0,
                   0.0,
                   0.0,
                   0.0,
                   0.0,
                   0.0,
                   0.0,
                   0.0,
                   0.028697692672201947,
                   0.16339564189400591,
                   0.39327037984064112,
                   0.50389456883620731,
                   0.54695324690661495,
                   0.58875261192606554,
                   0.61783257574788653,
                   0.65801278168159738,
                   0.78873155164181841,
                   0.88228609685010839,
                   0.93910764585584006,
                   1.0012676082691361,
                   1.1205677647618273,
                   1.1987460294171912,
                   1.2574993816078572,
                   1.3378092865578339,
                   1.4281407217225641,
                   1.5151395142688655,
                   1.5852001726168479,
                   1.6613621044219196,
                   1.7467449063896214,
                   1.8480244254278784,
                   1.9209020018623595,
                   1.9902883975365804,
                   2.084705199593365,
                   2.1550243702289102,
                   2.2014434008991048,
                   2.289503427220219,
                   2.3590624939083842,
                   2.4375690842178557,
                   2.5160768546333778,
                   2.5924863543159158,
                   2.6579691104092289,
                   2.7150337596936618,
                   2.7972212977700956,
                   2.8467817516534972,
                   2.9329572415227858,
                   3.008453856740366,
                   3.106681865004925,
                   3.1490749450787567,
                   3.191614994861546,
                   3.2520549577221134,
                   3.3094300605348148,
                   3.3735590368801427,
                   3.455360812349086,
                   3.4946273633674028,
                   3.5586651615672968,
                   3.6160827173698085,
                   3.6667717710707315,
                   3.710886403325091,
                   3.7575826652378361,
                   3.8116730387671018,
                   3.8477486812218298,
                   3.9010544628374566,
                   3.9570877386851762,
                   4.0000787914203375,
                   4.0458294516432316,
                   4.1028019631225465,
                   4.1501005489265745,
                   4.1948909786978765,
                   4.2261122235003485,
                   4.2748815807902218,
                   4.3197052900799413,
                   4.3675379331172488,
                   4.4075060875664631,
                   4.4668499186316284,
                   4.5120700271156817};
    cfg.gBValues = cfg.gValues;
    return cfg;
}

FluidPropertyFunctions makePrototypeLikeFluidFuncs()
{
    auto waterCp = [](Real64 t) {
        constexpr Real64 acp0 = 4.21534;
        constexpr Real64 acp1 = -0.00287819;
        constexpr Real64 acp2 = 7.4729e-05;
        constexpr Real64 acp3 = -7.79624e-07;
        constexpr Real64 acp4 = 3.220424e-09;
        return (acp0 + t * acp1 + std::pow(t, 2) * acp2 + std::pow(t, 3) * acp3 + std::pow(t, 4) * acp4) * 1000.0;
    };

    auto waterMu = [](Real64 t) {
        if (t < 20.0) {
            constexpr Real64 am0 = -3.30233;
            constexpr Real64 am1 = 1301.0;
            constexpr Real64 am2 = 998.333;
            constexpr Real64 am3 = 8.1855;
            constexpr Real64 am4 = 0.00585;
            Real64 const exponent = am0 + am1 / (am2 + (t - 20.0) * (am3 + am4 * (t - 20.0)));
            return std::pow(10.0, exponent) * 0.1;
        }
        if (t > 100.0) {
            constexpr Real64 am10 = 0.68714;
            constexpr Real64 am11 = -0.0059231;
            constexpr Real64 am12 = 2.1249e-05;
            constexpr Real64 am13 = -2.69575e-08;
            return (am10 + t * am11 + std::pow(t, 2) * am12 + std::pow(t, 3) * am13) * 0.001;
        }
        constexpr Real64 am5 = 1.002;
        constexpr Real64 am6 = -1.3272;
        constexpr Real64 am7 = -0.001053;
        constexpr Real64 am8 = 105.0;
        Real64 const exponent = (t - 20.0) * (am6 + (t - 20.0) * am7) / (t + am8);
        return am5 * std::pow(10.0, exponent) * 0.001;
    };

    auto waterRho = [](Real64 t) {
        constexpr Real64 ar0 = 999.83952;
        constexpr Real64 ar1 = 16.945176;
        constexpr Real64 ar2 = -0.0079870401;
        constexpr Real64 ar3 = -4.6170461e-05;
        constexpr Real64 ar4 = 1.0556302e-07;
        constexpr Real64 ar5 = -2.8054253e-10;
        constexpr Real64 ar6 = 0.01687985;
        return (ar0 + t * ar1 + std::pow(t, 2) * ar2 + std::pow(t, 3) * ar3 + std::pow(t, 4) * ar4 + std::pow(t, 5) * ar5) / (1.0 + ar6 * t);
    };

    auto waterK = [](Real64 t) {
        constexpr Real64 ak0 = 0.560101;
        constexpr Real64 ak1 = 0.00211703;
        constexpr Real64 ak2 = -1.05172e-05;
        constexpr Real64 ak3 = 1.497323e-08;
        constexpr Real64 ak4 = -1.48553e-11;
        return ak0 + t * ak1 + std::pow(t, 2) * ak2 + std::pow(t, 3) * ak3 + std::pow(t, 4) * ak4;
    };

    FluidPropertyFunctions funcs;
    funcs.cp = std::move(waterCp);
    funcs.rho = std::move(waterRho);
    funcs.viscosity = std::move(waterMu);
    funcs.conductivity = std::move(waterK);
    return funcs;
}

} // namespace

TEST(GLHECModel, ZeroFlowMaintainsInletTemperature)
{
    Model model(makeConfig(), makeFluidFuncs());
    model.reset(15.0);

    ModelStepInputs inputs;
    inputs.timeSeconds = 600;
    inputs.timeStepSeconds = 600;
    inputs.massFlowRate = 0.0;
    inputs.inletTemp = 18.0;
    inputs.farFieldGroundTemp = 15.0;

    auto const outputs = model.simulate(inputs);

    EXPECT_NEAR(outputs.outletTemp, inputs.inletTemp, 1.0e-10);
    EXPECT_NEAR(outputs.heatRate, 0.0, 1.0e-10);
    EXPECT_NEAR(outputs.boreholeHeatRate, 0.0, 1.0e-10);
}

TEST(GLHECModel, RepeatedCallsAtSameTimeDoNotAccumulate)
{
    Model model(makeConfig(), makeFluidFuncs());
    model.reset(15.0);

    ModelStepInputs inputs;
    inputs.timeSeconds = 600;
    inputs.timeStepSeconds = 600;
    inputs.massFlowRate = 1.0;
    inputs.inletTemp = 20.0;
    inputs.farFieldGroundTemp = 15.0;

    auto const first = model.simulate(inputs);
    auto const second = model.simulate(inputs);

    EXPECT_NEAR(first.outletTemp, second.outletTemp, 1.0e-9);
    EXPECT_NEAR(first.heatRate, second.heatRate, 1.0e-6);
    EXPECT_NEAR(first.boreholeHeatRate, second.boreholeHeatRate, 1.0e-6);
    EXPECT_NEAR(first.boreholeWallTemp, second.boreholeWallTemp, 1.0e-9);
}

TEST(GLHECModel, AggregationTuningChangesLongTermResponse)
{
    ModelConfig coarse = makeConfig();
    coarse.aggregation.expansionRate = 2.0;
    coarse.aggregation.binsPerLevel = 3;
    coarse.aggregation.simulationHorizonSeconds = 20.0 * 24.0 * 3600.0;

    ModelConfig fine = makeConfig();
    fine.aggregation.expansionRate = 1.2;
    fine.aggregation.binsPerLevel = 16;
    fine.aggregation.simulationHorizonSeconds = 20.0 * 24.0 * 3600.0;

    Model coarseModel(coarse, makeFluidFuncs());
    Model fineModel(fine, makeFluidFuncs());
    coarseModel.reset(15.0);
    fineModel.reset(15.0);

    ModelStepInputs inputs;
    inputs.timeStepSeconds = 600;
    inputs.massFlowRate = 1.0;
    inputs.inletTemp = 20.0;
    inputs.farFieldGroundTemp = 15.0;

    ModelStepOutputs coarseOut;
    ModelStepOutputs fineOut;
    for (int i = 1; i <= 300; ++i) {
        inputs.timeSeconds = i * inputs.timeStepSeconds;
        coarseOut = coarseModel.simulate(inputs);
        fineOut = fineModel.simulate(inputs);
    }

    EXPECT_GT(std::abs(coarseOut.boreholeWallTemp - fineOut.boreholeWallTemp), 1.0e-6);
}

TEST(GLHECModel, PrototypeTrajectoryRegressionCheckpoints)
{
    struct Checkpoint
    {
        int step;
        Real64 loadInletTemp;
        Real64 loadOutletTemp;
        Real64 heatRate;
        Real64 boreholeHeatRate;
        Real64 boreholeWallTemp;
    };

    // Reference checkpoints from prototype GLHEC run (temps2.csv) at 600 s timesteps.
    std::vector<Checkpoint> const checkpoints = {
        {0, 16.1, 19.6839, 2995.95, 444.649, 16.1},
        {1, 16.1021, 19.6861, 2983.52, 422.268, 16.5592},
        {2, 16.1191, 19.7031, 2975.57, 541.087, 16.6577},
        {5, 16.2218, 19.8059, 1530.6, 667.578, 17.1253},
        {10, 18.6001, 22.1861, 2910.73, 1064.14, 17.9731},
        {20, 21.5371, 25.1248, 2855.75, 1552.08, 19.3305},
        {50, 26.1979, 29.7875, 2909.74, 2279.93, 22.0499},
        {100, 29.6169, 33.2072, 2965.61, 2727.24, 24.2576},
        {200, 31.7675, 35.3581, 2990.76, 2926.44, 25.8696},
        {500, 33.3051, 36.8957, 2997.83, 2981.82, 27.2588},
        {1000, 34.1981, 37.7888, 2999.15, 2991.89, 28.1256},
        {2000, 35.0311, 38.6218, 2999.76, 2996.23, 28.9477},
        {5000, 36.0945, 39.6851, 3000.14, 2998.68, 30.0055},
        {10000, 36.8861, 40.4766, 3000.29, 2999.5, 30.7955},
        {20000, 37.6724, 41.2628, 3000.39, 2999.92, 31.5815},
        {50000, 38.7114, 42.3017, 3000.48, 3000.19, 32.6206},
        {52559, 38.7701, 42.3604, 3000.48, 3000.2, 32.6793},
    };

    auto const fluid = makePrototypeLikeFluidFuncs();
    Model model(makePrototypeLikeConfig(), fluid);
    model.reset(16.1);

    constexpr Real64 flowRate = 0.2;
    constexpr Real64 loadRate = 3000.0;
    constexpr int dtSeconds = 600;
    constexpr Real64 farFieldGroundTemp = 16.1;

    Real64 demandInletTemp = 16.1;
    std::size_t nextCheckpoint = 0;

    for (int step = 0; step <= checkpoints.back().step; ++step) {
        Real64 const cp = fluid.cp(demandInletTemp);
        Real64 const glheInletTemp = demandInletTemp + loadRate / (flowRate * cp);

        ModelStepInputs inputs;
        inputs.timeSeconds = step * dtSeconds;
        inputs.timeStepSeconds = dtSeconds;
        inputs.massFlowRate = flowRate;
        inputs.inletTemp = glheInletTemp;
        inputs.farFieldGroundTemp = farFieldGroundTemp;

        auto const outputs = model.simulate(inputs);

        if (nextCheckpoint < checkpoints.size() && step == checkpoints[nextCheckpoint].step) {
            auto const &cpRef = checkpoints[nextCheckpoint];

            EXPECT_NEAR(demandInletTemp, cpRef.loadInletTemp, 1.5);
            EXPECT_NEAR(glheInletTemp, cpRef.loadOutletTemp, 1.5);
            EXPECT_NEAR(outputs.boreholeWallTemp, cpRef.boreholeWallTemp, 0.5);

            if (step >= 500) {
                EXPECT_NEAR(outputs.heatRate, cpRef.heatRate, 150.0);
                EXPECT_NEAR(outputs.boreholeHeatRate, cpRef.boreholeHeatRate, 150.0);
            }
            ++nextCheckpoint;
        }

        demandInletTemp = outputs.outletTemp;
    }
}
