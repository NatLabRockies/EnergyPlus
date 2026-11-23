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
#ifndef NDEBUG
#    include <stdexcept>
#endif
#include <format>

// EnergyPlus Headers
#include <EnergyPlus/Timer.hh>

namespace EnergyPlus {

void Timer::tick()
{
    m_end = TimePointType{};
    m_start = ClockType::now();
}

void Timer::tock()
{
#ifndef NDEBUG
    if (m_start == TimePointType{}) {
        throw std::runtime_error("Timer was not started");
    }
#endif
    m_end = ClockType::now();
    m_duration = m_duration + std::chrono::duration_cast<DurationType>(m_end - m_start);
}

Timer::DurationType Timer::duration() const
{
#ifndef NDEBUG
    if (m_end == TimePointType{}) {
        throw std::runtime_error("Timer was not stopped");
    }
#endif
    return m_duration;
}

std::string Timer::formatAsHourMinSecs() const
{
    // We're working with ms as base.
    // TODO: do we want to report elapsed time of 25 hours as 1 day 1hr 0min 0sec or 25hr 0min 0sec?
    // constexpr auto num_ms_in_day = 1000 * 3600 * 24;
    // if (duration().count() > num_ms_in_day) {
    //     return std::format("{:.2%j days %Hhr %Mmin %Ssec}\n", duration());
    // } else {
    //     return std::format("{:%Hhr %Mmin %Ssec}\n", duration());
    // }
    auto count = duration().count();
    auto Hours = count / 3600000;
    count -= Hours * 3600000;

    auto Minutes = count / 60000;
    count -= Minutes * 60000;
    double Seconds = count / 1000.0;
    if (Seconds < 0.0) {
        Seconds = 0.0;
    }
    return std::format("{:02}hr {:02}min {:5.2F}sec", Hours, Minutes, Seconds);
}

Real64 Timer::elapsedSeconds() const
{
    return duration().count() / 1000.0;
}

} // namespace EnergyPlus
