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

#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
#include <utility>

#include <EnergyPlus/DataGlobalConstants.hh>

namespace EnergyPlus::GroundHeatExchangers::GLHEC {

namespace {

    constexpr Real64 small = 1.0e-12;

    template <typename T> T clampMin(T value, T minimum)
    {
        return (value < minimum) ? minimum : value;
    }

    Real64 linInterp(Real64 x, Real64 xLow, Real64 xHigh, Real64 yLow, Real64 yHigh)
    {
        if (std::abs(xHigh - xLow) < small) {
            return yLow;
        }
        return (x - xLow) / (xHigh - xLow) * (yHigh - yLow) + yLow;
    }

    class LinearInterpolator
    {
    public:
        void setXY(std::vector<Real64> xVals, std::vector<Real64> yVals)
        {
            x = std::move(xVals);
            y = std::move(yVals);
        }

        [[nodiscard]] Real64 interpolate(Real64 q) const
        {
            if (x.empty() || y.empty()) {
                return 0.0;
            }
            if (x.size() == 1) {
                return y[0];
            }
            if (q <= x.front()) {
                return y.front();
            }
            if (q >= x.back()) {
                return y.back();
            }
            auto const upper = std::upper_bound(x.begin(), x.end(), q);
            std::size_t const idxH = static_cast<std::size_t>(std::distance(x.begin(), upper));
            std::size_t const idxL = idxH - 1;
            return linInterp(q, x[idxL], x[idxH], y[idxL], y[idxH]);
        }

        [[nodiscard]] std::vector<Real64> interpolate(std::vector<Real64> const &queries) const
        {
            std::vector<Real64> values;
            values.reserve(queries.size());
            if (x.empty() || y.empty()) {
                values.assign(queries.size(), 0.0);
                return values;
            }
            if (x.size() == 1) {
                values.assign(queries.size(), y[0]);
                return values;
            }

            for (Real64 const q : queries) {
                if (q < x.front()) {
                    values.push_back(linInterp(q, x[0], x[1], y[0], y[1]));
                } else if (q >= x.back()) {
                    std::size_t const n = x.size();
                    values.push_back(linInterp(q, x[n - 2], x[n - 1], y[n - 2], y[n - 1]));
                } else {
                    auto const upper = std::upper_bound(x.begin(), x.end(), q);
                    std::size_t const idxH = static_cast<std::size_t>(std::distance(x.begin(), upper));
                    std::size_t const idxL = idxH - 1;
                    values.push_back(linInterp(q, x[idxL], x[idxH], y[idxL], y[idxH]));
                }
            }
            return values;
        }

    private:
        std::vector<Real64> x;
        std::vector<Real64> y;
    };

    template <typename T> void removeIndices(std::vector<T> &values, std::vector<std::size_t> const &indicesToRemove)
    {
        if (indicesToRemove.empty() || values.empty()) {
            return;
        }

        std::vector<bool> removeMask(values.size(), false);
        for (std::size_t const idx : indicesToRemove) {
            if (idx < values.size()) {
                removeMask[idx] = true;
            }
        }

        std::vector<T> filtered;
        filtered.reserve(values.size());
        for (std::size_t i = 0; i < values.size(); ++i) {
            if (!removeMask[i]) {
                filtered.push_back(values[i]);
            }
        }
        values = std::move(filtered);
    }

    class SubHourAggregation
    {
    public:
        void setup(std::vector<Real64> const &lntts, std::vector<Real64> const &gValues, std::vector<Real64> const &gBValues, Real64 timeScale)
        {
            interpG.setXY(lntts, gValues);
            interpGb.setXY(lntts, gBValues.empty() ? gValues : gBValues);
            ts = clampMin(timeScale, small);
            reset();
        }

        void reset()
        {
            energy.clear();
            dts.clear();
            energy.push_back(0.0);
            dts.push_back(secondsPerHour);
            prevUpdateTime = 0;
        }

        [[nodiscard]] Real64 aggregate(int timeSeconds, Real64 energyJPerMeter)
        {
            if (timeSeconds <= prevUpdateTime) {
                return 0.0;
            }

            energy.push_back(energyJPerMeter);
            dts.push_back(timeSeconds - prevUpdateTime);

            std::vector<int> dtsFlipped = dts;
            std::reverse(dtsFlipped.begin(), dtsFlipped.end());

            std::vector<Real64> dtUpper(dts.size(), 0.0);
            Real64 cumulative = 0.0;
            for (std::size_t i = 0; i < dtsFlipped.size(); ++i) {
                cumulative += static_cast<Real64>(dtsFlipped[i]);
                dtUpper[i] = cumulative;
            }
            std::reverse(dtUpper.begin(), dtUpper.end());

            std::vector<Real64> dtLower(dts.size(), 0.0);
            for (std::size_t i = 0; i < dts.size(); ++i) {
                dtLower[i] = dtUpper[i] - static_cast<Real64>(dts[i]);
            }

            std::vector<std::size_t> idxFull;
            std::vector<std::size_t> idxPartial;
            for (std::size_t i = 0; i < dtLower.size(); ++i) {
                if (dtLower[i] >= secondsPerHour) {
                    idxFull.push_back(i);
                } else if (dtUpper[i] > secondsPerHour && dtLower[i] < secondsPerHour) {
                    idxPartial.push_back(i);
                }
            }

            Real64 loadToShift = 0.0;
            for (std::size_t const i : idxFull) {
                loadToShift += energy[i];
            }

            if (!idxPartial.empty()) {
                std::size_t const idx = idxPartial.front();
                Real64 const upperEdge = dtUpper[idx];
                Real64 const lowerEdge = dtLower[idx];
                Real64 const denom = clampMin(upperEdge - lowerEdge, small);
                Real64 const fraction = (upperEdge - secondsPerHour) / denom;
                loadToShift += fraction * energy[idx];
                energy[idx] = (1.0 - fraction) * energy[idx];
                dts[idx] = std::max(1, static_cast<int>(std::llround((1.0 - fraction) * dts[idx])));
            }

            removeIndices(energy, idxFull);
            removeIndices(dts, idxFull);
            prevUpdateTime = timeSeconds;
            return loadToShift;
        }

        [[nodiscard]] std::vector<Real64> getLoadRates() const
        {
            std::vector<Real64> rates;
            rates.reserve(energy.size());
            for (std::size_t i = 0; i < energy.size(); ++i) {
                rates.push_back(energy[i] / clampMin(static_cast<Real64>(dts[i]), small));
            }
            return rates;
        }

        [[nodiscard]] std::vector<int> const &getTimeBins() const
        {
            return dts;
        }

        [[nodiscard]] int getPrevUpdateTime() const
        {
            return prevUpdateTime;
        }

    private:
        static constexpr int secondsPerHour = 3600;
        LinearInterpolator interpG;
        LinearInterpolator interpGb;
        Real64 ts = 1.0;
        int prevUpdateTime = 0;
        std::vector<Real64> energy;
        std::vector<int> dts;
    };

    class DynamicAggregation
    {
    public:
        void setup(ModelConfig const &config, Real64 timeScale)
        {
            interpG.setXY(config.lntts, config.gValues);
            std::vector<Real64> gB = config.gBValues;
            if (gB.empty()) {
                gB = config.gValues;
            }
            interpGb.setXY(config.lntts, gB);
            subHour.setup(config.lntts, config.gValues, gB, timeScale);
            ts = clampMin(timeScale, small);
            expansionRate = std::max(1.0, config.aggregation.expansionRate);
            binsPerLevel = std::max(1, config.aggregation.binsPerLevel);
            simulationHorizonSeconds = std::max(static_cast<Real64>(secondsPerHour), config.aggregation.simulationHorizonSeconds);
            dts.clear();
            energy.clear();

            Real64 dt = static_cast<Real64>(secondsPerHour);
            Real64 t = static_cast<Real64>(secondsPerHour);
            while (true) {
                for (int i = 0; i < binsPerLevel; ++i) {
                    t += dt;
                    energy.insert(energy.begin(), 0.0);
                    dts.insert(dts.begin(), std::max(1, static_cast<int>(std::llround(dt))));
                    if (t >= simulationHorizonSeconds) {
                        reset();
                        return;
                    }
                }
                dt *= expansionRate;
            }
        }

        void reset()
        {
            std::fill(energy.begin(), energy.end(), 0.0);
            subHour.reset();
            prevUpdateTime = 0;
        }

        void aggregate(int timeSeconds, Real64 energyJPerMeter)
        {
            if (timeSeconds < prevUpdateTime) {
                reset();
            }
            if (timeSeconds <= prevUpdateTime) {
                return;
            }

            Real64 const shiftedFromSubHour = subHour.aggregate(timeSeconds, energyJPerMeter);
            int const elapsedTime = timeSeconds - prevUpdateTime;

            std::vector<Real64> fractionShift;
            fractionShift.reserve(dts.size());
            for (int const dt : dts) {
                fractionShift.push_back(static_cast<Real64>(elapsedTime) / static_cast<Real64>(dt));
            }
            if (!fractionShift.empty()) {
                fractionShift[0] = 0.0;
            }

            std::vector<Real64> delta;
            delta.reserve(energy.size());
            for (std::size_t i = 0; i < energy.size(); ++i) {
                Real64 const thisDelta = energy[i] * fractionShift[i];
                delta.push_back(thisDelta);
                energy[i] -= thisDelta;
            }

            std::vector<Real64> rolledDelta(delta.size(), 0.0);
            for (std::size_t i = 0; i < delta.size(); ++i) {
                rolledDelta[i] = delta[(i + 1) % delta.size()];
            }

            for (std::size_t i = 0; i < energy.size(); ++i) {
                energy[i] += rolledDelta[i];
            }
            if (!energy.empty()) {
                energy.back() += shiftedFromSubHour;
            }

            prevUpdateTime = timeSeconds;
        }

        [[nodiscard]] std::pair<Real64, Real64> temporalSuperposition(int dtSeconds) const
        {
            if (dts.empty() || dtSeconds <= 0) {
                return {0.0, 0.0};
            }

            std::vector<Real64> longTermRates;
            longTermRates.reserve(energy.size());
            for (std::size_t i = 0; i < energy.size(); ++i) {
                longTermRates.push_back(energy[i] / clampMin(static_cast<Real64>(dts[i]), small));
            }

            std::vector<Real64> shortTermRates = subHour.getLoadRates();

            std::vector<Real64> q = longTermRates;
            q.insert(q.end(), shortTermRates.begin(), shortTermRates.end());
            if (q.empty()) {
                return {0.0, 0.0};
            }

            std::vector<Real64> dq(q.size(), 0.0);
            dq[0] = q[0];
            for (std::size_t i = 1; i < q.size(); ++i) {
                dq[i] = q[i] - q[i - 1];
            }

            std::vector<int> allDts = dts;
            std::vector<int> const &subHourDts = subHour.getTimeBins();
            allDts.insert(allDts.end(), subHourDts.begin(), subHourDts.end());
            allDts.push_back(dtSeconds);

            std::vector<int> reversedDts = allDts;
            std::reverse(reversedDts.begin(), reversedDts.end());

            std::vector<Real64> cumulativeTimes(reversedDts.size(), 0.0);
            Real64 cumulative = 0.0;
            for (std::size_t i = 0; i < reversedDts.size(); ++i) {
                cumulative += static_cast<Real64>(reversedDts[i]);
                cumulativeTimes[i] = cumulative;
            }
            std::reverse(cumulativeTimes.begin(), cumulativeTimes.end());
            cumulativeTimes.pop_back();

            std::vector<Real64> lntts;
            lntts.reserve(cumulativeTimes.size());
            for (Real64 const t : cumulativeTimes) {
                lntts.push_back(std::log(clampMin(t / ts, small)));
            }

            std::vector<Real64> g = interpG.interpolate(lntts);
            std::vector<Real64> gB = interpGb.interpolate(lntts);

            Real64 dotG = 0.0;
            Real64 dotGb = 0.0;
            for (std::size_t i = 0; i < dq.size(); ++i) {
                dotG += dq[i] * g[i];
                dotGb += dq[i] * gB[i];
            }
            return {dotG, dotGb};
        }

    private:
        static constexpr int secondsPerHour = 3600;
        LinearInterpolator interpG;
        LinearInterpolator interpGb;
        SubHourAggregation subHour;
        Real64 ts = 1.0;
        Real64 expansionRate = 1.62;
        int binsPerLevel = 9;
        Real64 simulationHorizonSeconds = 50.0 * 365.0 * 24.0 * static_cast<Real64>(secondsPerHour);
        int prevUpdateTime = 0;
        std::vector<Real64> energy;
        std::vector<int> dts;
    };

    struct SegmentInputs
    {
        Real64 flowRate = 0.0;
        Real64 inletTemp1 = 0.0;
        Real64 inletTemp2 = 0.0;
        Real64 boundaryTemp = 0.0;
        Real64 bhResistance = 0.0;
        Real64 dcResistance = 0.0;
    };

    class Segment
    {
    public:
        Segment(ModelConfig const &config, Real64 segmentLength)
            : length(segmentLength), diameter(config.boreholeDiameter), groutFraction(config.groutFraction), groutDensity(config.groutDensity),
              groutSpecificHeat(config.groutSpecificHeat), pipeDensity(config.pipeDensity), pipeSpecificHeat(config.pipeSpecificHeat),
              pipeInnerDiameter(config.pipeInnerDiameter), pipeOuterDiameter(config.pipeOuterDiameter), odeSettings(config.ode)
        {
            reset(20.0);
        }

        void reset(Real64 temp)
        {
            y = {temp, temp, temp, temp, temp};
            heatRate = 0.0;
        }

        void step(Real64 timeStepSeconds, SegmentInputs const &inputs, FluidPropertyFunctions const &fluidProps)
        {
            if (timeStepSeconds <= 0.0) {
                return;
            }
            Context ctx;
            ctx.seg = this;
            ctx.inputs = inputs;
            ctx.fluidCp = clampMin(fluidProps.cp(inputs.inletTemp1), small);
            ctx.fluidRho = clampMin(fluidProps.rho(inputs.inletTemp1), small);

            rk4Step(timeStepSeconds, ctx);
            Real64 const rB = clampMin(inputs.bhResistance, small);
            heatRate = ((y[3] - inputs.boundaryTemp) / rB + (y[4] - inputs.boundaryTemp) / rB) * length;
        }

        [[nodiscard]] Real64 outlet1() const
        {
            return y[0];
        }

        [[nodiscard]] Real64 outlet2() const
        {
            return y[1];
        }

        [[nodiscard]] Real64 boreholeHeatRate() const
        {
            return heatRate;
        }

    private:
        struct Context
        {
            Segment *seg = nullptr;
            SegmentInputs inputs;
            Real64 fluidCp = 0.0;
            Real64 fluidRho = 0.0;
        };

        void rhs(Context const &ctx, double const yvals[], double dydt[]) const
        {
            Real64 const rF = 1.0 / clampMin(ctx.inputs.flowRate * ctx.fluidCp, small);
            Real64 const rB = clampMin(ctx.inputs.bhResistance, small);
            Real64 const r12 = clampMin(ctx.inputs.dcResistance, small);
            Real64 const cF = clampMin(ctx.fluidRho * ctx.fluidCp * pipeFluidVolume(), small);
            Real64 const cG1 =
                clampMin(groutFraction * groutSpecificHeat * groutDensity * groutVolume() + pipeSpecificHeat * pipeDensity * pipeWallVolume(), small);
            Real64 const cG2 = clampMin(
                ((1.0 - groutFraction) * groutSpecificHeat * groutDensity * groutVolume() + pipeSpecificHeat * pipeDensity * pipeWallVolume()) / 2.0,
                small);

            dydt[0] =
                ((ctx.inputs.inletTemp1 - yvals[0]) / rF + (yvals[2] - yvals[0]) * length / (r12 / 2.0) + (yvals[3] - yvals[0]) * length / rB) / cF;
            dydt[1] =
                ((ctx.inputs.inletTemp2 - yvals[1]) / rF + (yvals[2] - yvals[1]) * length / (r12 / 2.0) + (yvals[4] - yvals[1]) * length / rB) / cF;
            dydt[2] = ((yvals[0] - yvals[2]) * length / (r12 / 2.0) + (yvals[1] - yvals[2]) * length / (r12 / 2.0)) / cG1;
            dydt[3] = ((yvals[0] - yvals[3]) * length / rB + (ctx.inputs.boundaryTemp - yvals[3]) * length / rB) / cG2;
            dydt[4] = ((yvals[1] - yvals[4]) * length / rB + (ctx.inputs.boundaryTemp - yvals[4]) * length / rB) / cG2;
        }

        void rk4Step(Real64 timeStepSeconds, Context const &ctx)
        {
            Real64 const maxStableSubstep = stableSubstepSeconds(ctx);
            int const minSubsteps = std::max(20, std::max(1, odeSettings.stepsPerTimeStep) * 5);
            int const stableSubsteps = static_cast<int>(std::ceil(timeStepSeconds / clampMin(maxStableSubstep, small)));
            int const n = std::max(minSubsteps, stableSubsteps);
            Real64 const h = timeStepSeconds / n;
            std::array<Real64, 5> k1{}, k2{}, k3{}, k4{}, y2{}, y3{}, y4{};
            for (int i = 0; i < n; ++i) {
                rhs(ctx, y.data(), k1.data());
                for (std::size_t j = 0; j < y.size(); ++j) {
                    y2[j] = y[j] + h * 0.5 * k1[j];
                }
                rhs(ctx, y2.data(), k2.data());
                for (std::size_t j = 0; j < y.size(); ++j) {
                    y3[j] = y[j] + h * 0.5 * k2[j];
                }
                rhs(ctx, y3.data(), k3.data());
                for (std::size_t j = 0; j < y.size(); ++j) {
                    y4[j] = y[j] + h * k3[j];
                }
                rhs(ctx, y4.data(), k4.data());
                for (std::size_t j = 0; j < y.size(); ++j) {
                    y[j] += h * (k1[j] + 2.0 * k2[j] + 2.0 * k3[j] + k4[j]) / 6.0;
                }
            }
        }

        [[nodiscard]] Real64 stableSubstepSeconds(Context const &ctx) const
        {
            Real64 const rF = 1.0 / clampMin(ctx.inputs.flowRate * ctx.fluidCp, small);
            Real64 const rB = clampMin(ctx.inputs.bhResistance, small);
            Real64 const r12 = clampMin(ctx.inputs.dcResistance, small);
            Real64 const cF = clampMin(ctx.fluidRho * ctx.fluidCp * pipeFluidVolume(), small);
            Real64 const cG1 =
                clampMin(groutFraction * groutSpecificHeat * groutDensity * groutVolume() + pipeSpecificHeat * pipeDensity * pipeWallVolume(), small);
            Real64 const cG2 = clampMin(
                ((1.0 - groutFraction) * groutSpecificHeat * groutDensity * groutVolume() + pipeSpecificHeat * pipeDensity * pipeWallVolume()) / 2.0,
                small);

            Real64 const fluidConductance = 1.0 / rF + 2.0 * length / r12 + length / rB;
            Real64 const coreConductance = 4.0 * length / r12;
            Real64 const wallConductance = 2.0 * length / rB;
            Real64 const tauFluid = cF / clampMin(fluidConductance, small);
            Real64 const tauCore = cG1 / clampMin(coreConductance, small);
            Real64 const tauWall = cG2 / clampMin(wallConductance, small);
            Real64 const tauMin = std::min({tauFluid, tauCore, tauWall});

            // Keep the explicit RK4 step well below the fastest segment time constant.
            return std::max(0.1, 0.25 * tauMin);
        }

        [[nodiscard]] Real64 pipeFluidVolume() const
        {
            return Constant::Pi * std::pow(pipeInnerDiameter, 2) / 4.0 * length;
        }

        [[nodiscard]] Real64 pipeWallVolume() const
        {
            return (Constant::Pi * std::pow(pipeOuterDiameter, 2) / 4.0 - Constant::Pi * std::pow(pipeInnerDiameter, 2) / 4.0) * length;
        }

        [[nodiscard]] Real64 groutVolume() const
        {
            Real64 const seg = Constant::Pi * std::pow(diameter, 2) / 4.0 * length;
            Real64 const pipe = 2.0 * Constant::Pi * std::pow(pipeOuterDiameter, 2) / 4.0 * length;
            return std::max(0.0, seg - pipe);
        }

        Real64 length = 0.0;
        Real64 diameter = 0.0;
        Real64 groutFraction = 0.5;
        Real64 groutDensity = 0.0;
        Real64 groutSpecificHeat = 0.0;
        Real64 pipeDensity = 0.0;
        Real64 pipeSpecificHeat = 0.0;
        Real64 pipeInnerDiameter = 0.0;
        Real64 pipeOuterDiameter = 0.0;
        OdeSolverSettings odeSettings{};
        std::array<Real64, 5> y{};
        Real64 heatRate = 0.0;
    };

} // namespace

struct Model::Impl
{
    explicit Impl(ModelConfig cfg, FluidPropertyFunctions props) : config(std::move(cfg)), fluidProps(std::move(props))
    {
        if (config.lntts.empty() || config.gValues.empty() || config.lntts.size() != config.gValues.size()) {
            throw std::runtime_error("GLHEC requires g-function data");
        }
        if (!config.gBValues.empty() && config.gBValues.size() != config.lntts.size()) {
            throw std::runtime_error("GLHEC gB-function data size mismatch");
        }
        if (!fluidProps.cp || !fluidProps.rho || !fluidProps.viscosity || !fluidProps.conductivity) {
            throw std::runtime_error("GLHEC requires fluid property callbacks");
        }
        config.numSegments = std::max(1, config.numSegments);
        config.numIterations = std::max(1, config.numIterations);
        config.numBoreholes = std::max(1u, config.numBoreholes);
        if (config.boreholeLength <= 0.0 || config.boreholeDiameter <= 0.0 || config.pipeInnerDiameter <= 0.0 ||
            config.pipeOuterDiameter <= config.pipeInnerDiameter) {
            throw std::runtime_error("GLHEC geometry inputs are invalid");
        }
        int const nSeg = config.numSegments;
        Real64 const segLength = config.boreholeLength / static_cast<Real64>(nSeg);
        for (int i = 0; i < nSeg; ++i) {
            segments.emplace_back(config, segLength);
        }
        theta1 = config.shankSpacing / clampMin(config.boreholeDiameter, small);
        theta2 = config.boreholeDiameter / clampMin(config.pipeOuterDiameter, small);
        theta3 = 1.0 / clampMin(theta1 * theta2, small);
        sigma = (config.groutConductivity - config.soilConductivity) / clampMin(config.groutConductivity + config.soilConductivity, small);
        ts = std::pow(config.boreholeLength, 2) / clampMin(9.0 * config.soilDiffusivity, small);
        c0 = 1.0 / (2.0 * Constant::Pi * clampMin(config.soilConductivity, small));
        aggregation.setup(config, ts);
    }

    void reset(Real64 temp)
    {
        lastSimTime = -1;
        haveStepStartSnapshot = false;
        outletTemp = temp;
        bhWallTemp = temp;
        energy = 0.0;
        uBendTemp = temp;
        aggregation.reset();
        for (auto &seg : segments) {
            seg.reset(temp);
        }
    }

    [[nodiscard]] ModelStepOutputs simulate(ModelStepInputs const &inputs)
    {
        if (inputs.timeStepSeconds <= 0) {
            ModelStepOutputs outputs;
            outputs.outletTemp = inputs.inletTemp;
            outputs.heatRate = 0.0;
            outputs.boreholeHeatRate = 0.0;
            outputs.boreholeWallTemp = bhWallTemp;
            outputs.avgFluidTemp = inputs.inletTemp;
            return outputs;
        }

        if (inputs.timeSeconds < lastSimTime) {
            reset(inputs.farFieldGroundTemp);
        }

        if (lastSimTime < inputs.timeSeconds) {
            aggregationAtStepStart = aggregation;
            segmentsAtStepStart = segments;
            energyAtStepStart = energy;
            uBendTempAtStepStart = uBendTemp;
            outletTempAtStepStart = outletTemp;
            bhWallTempAtStepStart = bhWallTemp;
            haveStepStartSnapshot = true;
            lastSimTime = inputs.timeSeconds;
        } else if (lastSimTime == inputs.timeSeconds && haveStepStartSnapshot) {
            aggregation = aggregationAtStepStart;
            segments = segmentsAtStepStart;
            energy = energyAtStepStart;
            uBendTemp = uBendTempAtStepStart;
            outletTemp = outletTempAtStepStart;
            bhWallTemp = bhWallTempAtStepStart;
        }

        aggregation.aggregate(inputs.timeSeconds, energy);
        auto const hist = aggregation.temporalSuperposition(inputs.timeStepSeconds);
        bhWallTemp = inputs.farFieldGroundTemp + hist.first * c0;

        Real64 const numBoreholes = static_cast<Real64>(std::max(1u, config.numBoreholes));
        Real64 const massFlowRatePerBorehole = inputs.massFlowRate / numBoreholes;

        if (massFlowRatePerBorehole <= small) {
            energy = 0.0;
            ModelStepOutputs outputs;
            outputs.outletTemp = inputs.inletTemp;
            outputs.heatRate = 0.0;
            outputs.boreholeHeatRate = 0.0;
            outputs.boreholeWallTemp = bhWallTemp;
            outputs.avgFluidTemp = inputs.inletTemp;
            return outputs;
        }

        Real64 const rB = calcBHAverageResistance(inputs.inletTemp, massFlowRatePerBorehole);
        Real64 const r12 = calcDirectCouplingResistance(inputs.inletTemp, massFlowRatePerBorehole, rB);

        SegmentInputs segIn;
        segIn.flowRate = massFlowRatePerBorehole;
        segIn.boundaryTemp = bhWallTemp;
        segIn.bhResistance = rB;
        segIn.dcResistance = r12;

        for (int iter = 0; iter < std::max(1, config.numIterations); ++iter) {
            for (std::size_t i = 0; i < segments.size(); ++i) {
                segIn.inletTemp1 = (i == 0) ? inputs.inletTemp : segments[i - 1].outlet1();
                segIn.inletTemp2 = (i == segments.size() - 1) ? uBendTemp : segments[i + 1].outlet2();
                segments[i].step(inputs.timeStepSeconds, segIn, fluidProps);
            }
            uBendTemp = segments.back().outlet1();
        }

        outletTemp = segments.front().outlet2();

        Real64 boreholeHeatRateToGroundPerBorehole = 0.0;
        for (auto const &seg : segments) {
            boreholeHeatRateToGroundPerBorehole += seg.boreholeHeatRate();
        }

        Real64 const cp = fluidProps.cp(inputs.inletTemp);
        Real64 const heatRateToGround = inputs.massFlowRate * cp * (inputs.inletTemp - outletTemp);
        Real64 const historyHeatRatePerBorehole = (config.numBoreholes > 1) ? (heatRateToGround / numBoreholes) : boreholeHeatRateToGroundPerBorehole;
        energy = historyHeatRatePerBorehole / clampMin(config.boreholeLength, small) * static_cast<Real64>(inputs.timeStepSeconds);

        ModelStepOutputs outputs;
        outputs.outletTemp = outletTemp;
        outputs.heatRate = -heatRateToGround;
        outputs.boreholeHeatRate = -boreholeHeatRateToGroundPerBorehole * numBoreholes;
        outputs.boreholeWallTemp = bhWallTemp;
        outputs.avgFluidTemp = 0.5 * (inputs.inletTemp + outletTemp);
        return outputs;
    }

    [[nodiscard]] Real64 pipeResistance(Real64 massFlowRate, Real64 temp) const
    {
        if (massFlowRate <= 0.0) {
            return 0.0;
        }

        auto frictionFactor = [](Real64 reynoldsNum) {
            constexpr Real64 lowerLimit = 1500.0;
            constexpr Real64 upperLimit = 5000.0;
            if (reynoldsNum < lowerLimit) {
                return 64.0 / clampMin(reynoldsNum, 1.0);
            }
            if (reynoldsNum < upperLimit) {
                Real64 const fLow = 64.0 / clampMin(reynoldsNum, 1.0);
                Real64 const fHigh = std::pow(0.79 * std::log(reynoldsNum) - 1.64, -2.0);
                Real64 const sf = 1.0 / (1.0 + std::exp(-(reynoldsNum - 3000.0) / 450.0));
                return (1.0 - sf) * fLow + sf * fHigh;
            }
            return std::pow(0.79 * std::log(reynoldsNum) - 1.64, -2.0);
        };

        auto turbulentNusselt = [&](Real64 reynoldsNum, Real64 prandtlNum) {
            Real64 const f = frictionFactor(reynoldsNum);
            return (f / 8.0) * (reynoldsNum - 1000.0) * prandtlNum / (1.0 + 12.7 * std::sqrt(f / 8.0) * (std::pow(prandtlNum, 2.0 / 3.0) - 1.0));
        };

        Real64 const mu = clampMin(fluidProps.viscosity(temp), small);
        Real64 const cp = clampMin(fluidProps.cp(temp), small);
        Real64 const k = clampMin(fluidProps.conductivity(temp), small);
        Real64 const di = clampMin(config.pipeInnerDiameter, small);
        Real64 const doPipe = std::max(config.pipeOuterDiameter, di + small);
        Real64 const re = 4.0 * massFlowRate / (mu * Constant::Pi * di);
        Real64 const pr = cp * mu / k;
        Real64 constexpr nuLow = 4.01;

        Real64 nu = 0.0;
        if (re < 2000.0) {
            nu = nuLow;
        } else if (re < 4000.0) {
            Real64 const nuHigh = turbulentNusselt(re, pr);
            Real64 const sf = 1.0 / (1.0 + std::exp(-(re - 3000.0) / 150.0));
            nu = (1.0 - sf) * nuLow + sf * nuHigh;
        } else {
            nu = turbulentNusselt(re, pr);
        }
        nu = clampMin(nu, small);

        Real64 const conv = 1.0 / (nu * Constant::Pi * k);
        Real64 const cond = std::log(doPipe / di) / (2.0 * Constant::Pi * clampMin(config.pipeConductivity, small));
        return conv + cond;
    }

    [[nodiscard]] Real64 calcBHAverageResistance(Real64 temp, Real64 flowRate) const
    {
        Real64 const beta = std::max(0.0, std::min(0.999, 2.0 * Constant::Pi * config.groutConductivity * pipeResistance(flowRate, temp)));
        Real64 const t14 = std::pow(theta1, 4);
        Real64 const d = clampMin(1.0 - t14, small);
        Real64 const term1 = std::log(theta2 / clampMin(2.0 * theta1 * std::pow(d, sigma), small));
        Real64 const num2 = std::pow(theta3, 2) * std::pow(1.0 - (4.0 * sigma * t14) / d, 2);
        Real64 const den2 =
            clampMin((1.0 + beta) / clampMin(1.0 - beta, small) + std::pow(theta3, 2) * (1.0 + (16.0 * sigma * t14) / std::pow(d, 2)), small);
        return (1.0 / (4.0 * Constant::Pi * config.groutConductivity)) * (beta + term1 - num2 / den2);
    }

    [[nodiscard]] Real64 calcDirectCouplingResistance(Real64 temp, Real64 flowRate, Real64 rB) const
    {
        Real64 const beta = std::max(0.0, std::min(0.999, 2.0 * Constant::Pi * config.groutConductivity * pipeResistance(flowRate, temp)));
        Real64 const t12 = std::pow(theta1, 2);
        Real64 const t14 = std::pow(theta1, 4);
        Real64 const num2 = std::pow(theta3, 2) * std::pow(1.0 - t14 + 4.0 * sigma * t12, 2);
        Real64 const den2 = clampMin(((1.0 + beta) / clampMin(1.0 - beta, small)) * std::pow(1.0 - t14, 2) -
                                         std::pow(theta3, 2) * std::pow(1.0 - t14, 2) + 8.0 * sigma * t12 * std::pow(theta3, 2) * (1.0 + t14),
                                     small);
        Real64 const rA = (1.0 / (Constant::Pi * config.groutConductivity)) *
                          (beta + std::log(std::pow(1.0 + t12, sigma) / clampMin(theta3 * std::pow(1.0 - t12, sigma), small)) - num2 / den2);
        Real64 const denom = 4.0 * rB - rA;
        if (std::abs(denom) < small) {
            return 70.0;
        }
        Real64 const r12 = (4.0 * rA * rB) / denom;
        return (r12 > small && std::isfinite(r12)) ? r12 : 70.0;
    }

    ModelConfig config;
    FluidPropertyFunctions fluidProps;
    DynamicAggregation aggregation;
    std::vector<Segment> segments;
    Real64 theta1 = 0.0;
    Real64 theta2 = 0.0;
    Real64 theta3 = 0.0;
    Real64 sigma = 0.0;
    Real64 ts = 0.0;
    Real64 c0 = 0.0;
    Real64 energy = 0.0;
    Real64 uBendTemp = 20.0;
    Real64 outletTemp = 20.0;
    Real64 bhWallTemp = 20.0;
    int lastSimTime = -1;
    bool haveStepStartSnapshot = false;
    DynamicAggregation aggregationAtStepStart;
    std::vector<Segment> segmentsAtStepStart;
    Real64 energyAtStepStart = 0.0;
    Real64 uBendTempAtStepStart = 20.0;
    Real64 outletTempAtStepStart = 20.0;
    Real64 bhWallTempAtStepStart = 20.0;
};

Model::Model(ModelConfig config, FluidPropertyFunctions fluidProps) : impl(std::make_unique<Impl>(std::move(config), std::move(fluidProps)))
{
}

Model::~Model() = default;

void Model::reset(Real64 initialTemp)
{
    impl->reset(initialTemp);
}

ModelStepOutputs Model::simulate(ModelStepInputs const &inputs)
{
    return impl->simulate(inputs);
}

} // namespace EnergyPlus::GroundHeatExchangers::GLHEC
