#!/usr/bin/env python
# EnergyPlus, Copyright (c) 1996-present, The Board of Trustees of the
# University of Illinois, The Regents of the University of California, through
# Lawrence Berkeley National Laboratory (subject to receipt of any required
# approvals from the U.S. Dept. of Energy), Oak Ridge National Laboratory,
# managed by UT-Battelle, Alliance for Energy Innovation, LLC, and other
# contributors. All rights reserved.
#
# NOTICE: This Software was developed under funding from the U.S. Department of
# Energy and the U.S. Government consequently retains certain rights. As such,
# the U.S. Government has been granted for itself and others acting on its
# behalf a paid-up, nonexclusive, irrevocable, worldwide license in the
# Software to reproduce, distribute copies to the public, prepare derivative
# works, and perform publicly and display publicly, and to permit others to do
# so.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# (1) Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#
# (2) Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#
# (3) Neither the name of the University of California, Lawrence Berkeley
#     National Laboratory, the University of Illinois, U.S. Dept. of Energy nor
#     the names of its contributors may be used to endorse or promote products
#     derived from this software without specific prior written permission.
#
# (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in
#     stand-alone form without changes from the version obtained under this
#     License, or (ii) Licensee makes a reference solely to the software
#     portion of its product, Licensee must refer to the software as
#     "EnergyPlus version X" software, where "X" is the version number Licensee
#     obtained under this License and may not use a different name for the
#     software. Except as specifically required in this Section (4), Licensee
#     shall not use in a company name, a product name, in advertising,
#     publicity, or other promotional activities any name, trade name,
#     trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or
#     confusingly similar designation, without the U.S. Department of Energy's
#     prior written consent.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

"""Generate src/EnergyPlus/PsychPsatTable.hh: constexpr cubic Hermite table for PsyPsatFnTemp.

Uniform grid of step H anchored at the triple point (0.01 C), covering [-100, 200] C.
Each interval stores 4 power-basis coefficients in t in [0, 1]:
    Psat = c0 + t * (c1 + t * (c2 + t * c3))
built from Psat and its exact dPsat/dT at both ends, using the same branch (ice or water) as the interval,
so the ice/water kink at 273.16 K sits on a node. Values are written as hex floats (exact round trip).

Usage: python scripts/dev/generate_psat_table.py [--h 0.5]
"""

import argparse
import math
from pathlib import Path

import licensetext

REPO = Path(__file__).resolve().parents[2]
OUT = REPO / "src" / "EnergyPlus" / "PsychPsatTable.hh"

KELVIN = 273.15
TRIPLE_POINT_K = 273.16
T_MIN, T_MAX = -100.0, 200.0

# Must match PsyPsatFnTemp_raw (Psychrometrics.cc, non-IF97 branch)
C1, C2, C3, C4, C5, C6, C7 = (
    -5674.5359,
    6.3925247,
    -0.9677843e-2,
    0.62215701e-6,
    0.20747825e-8,
    -0.9484024e-12,
    4.1635019,
)
C8, C9, C10, C11, C12, C13 = -5800.2206, 1.3914993, -0.048640239, 0.41764768e-4, -0.14452093e-7, 6.5459673


def psat_ice(Tk):
    return math.exp(C1 / Tk + C2 + Tk * (C3 + Tk * (C4 + Tk * (C5 + C6 * Tk))) + C7 * math.log(Tk))


def psat_water(Tk):
    return math.exp(C8 / Tk + C9 + Tk * (C10 + Tk * (C11 + Tk * C12)) + C13 * math.log(Tk))


def dlnpsat_ice(Tk):
    return -C1 / Tk**2 + C3 + Tk * (2 * C4 + Tk * (3 * C5 + 4 * C6 * Tk)) + C7 / Tk


def dlnpsat_water(Tk):
    return -C8 / Tk**2 + C10 + Tk * (2 * C11 + 3 * C12 * Tk) + C13 / Tk


def psat_raw(T):
    Tk = T + KELVIN
    if Tk < 173.15:
        return 0.001405102123874164
    if Tk < TRIPLE_POINT_K:
        return psat_ice(Tk)
    if Tk <= 473.15:
        return psat_water(Tk)
    return 1555073.745636215


def build(h):
    anchor = TRIPLE_POINT_K - KELVIN
    k_lo = math.floor((T_MIN - anchor) / h)
    k_hi = math.ceil((T_MAX - anchor) / h)
    T0 = anchor + k_lo * h
    coefs = []
    for k in range(k_lo, k_hi):
        x0, x1 = anchor + k * h, anchor + (k + 1) * h
        ice = (k + 1) <= 0  # right node at or below the triple point
        P, dln = (psat_ice, dlnpsat_ice) if ice else (psat_water, dlnpsat_water)
        Tk0, Tk1 = x0 + KELVIN, x1 + KELVIN
        P0, P1 = P(Tk0), P(Tk1)
        D0, D1 = P0 * dln(Tk0) * h, P1 * dln(Tk1) * h
        coefs.append((x0, x1, (P0, D0, 3 * (P1 - P0) - 2 * D0 - D1, 2 * (P0 - P1) + D0 + D1)))
    return T0, coefs


def evaluate(T0, inv_h, coefs, T):
    u = (T - T0) * inv_h
    j = int(u)
    t = u - j
    c = coefs[j][2]
    return c[0] + t * (c[1] + t * (c[2] + t * c[3]))


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--h", type=float, default=0.5, help="grid step [K] (default 0.5)")
    args = parser.parse_args()
    h = args.h
    inv_h = 1.0 / h

    T0, coefs = build(h)
    n = len(coefs)
    # The C++ fast path is taken for T_MIN < T < T_MAX, with no index clamping: check the grid covers it.
    assert T0 <= T_MIN and coefs[-1][1] >= T_MAX
    assert int((math.nextafter(T_MAX, -math.inf) - T0) * inv_h) <= n - 1

    # Accuracy check against the exact formula
    worst, worst_T = 0.0, None
    N = 300_000
    for i in range(1, N):
        T = T_MIN + (T_MAX - T_MIN) * i / N
        ref = psat_raw(T)
        rel = abs(evaluate(T0, inv_h, coefs, T) - ref) / ref
        if rel > worst:
            worst, worst_T = rel, T

    out = licensetext.current().splitlines() + [
        "",
        "#ifndef PsychPsatTable_hh_INCLUDED",
        "#define PsychPsatTable_hh_INCLUDED",
        "",
        "// Generated by scripts/dev/generate_psat_table.py. Do not edit by hand.",
        "",
        "// C++ Headers",
        "#include <array>",
        "",
        "// EnergyPlus Headers",
        "#include <EnergyPlus/EnergyPlus.hh>",
        "",
        "namespace EnergyPlus::Psychrometrics::PsatTable {",
        "",
        f"// Cubic Hermite interpolation of PsyPsatFnTemp_raw, step {h} K, grid anchored at the triple point (0.01 C).",
        f"// Max relative error vs PsyPsatFnTemp_raw on [{T_MIN:g}, {T_MAX:g}] C: {worst:.1e} (at {worst_T:.2f} C).",
        "// Interval j: t = (T - T0) * inv_h - j in [0, 1], Psat = c0 + t * (c1 + t * (c2 + t * c3)) [Pa].",
        f"constexpr Real64 T_min = {T_MIN!r};",
        f"constexpr Real64 T_max = {T_MAX!r};",
        f"constexpr Real64 T0 = {T0.hex()}; // {T0!r}",
        f"constexpr Real64 inv_h = {inv_h!r};",
        f"constexpr int n = {n};",
        "",
        "constexpr std::array<std::array<Real64, 4>, n> coefs = {{",
    ]
    # Align trailing comments like clang-format does
    rows = [(f"    {{{', '.join(v.hex() for v in c)}}},", f"// [{x0:.2f}, {x1:.2f}]") for x0, x1, c in coefs]
    width = max(len(code) for code, _ in rows)
    out += [f"{code:<{width}} {comment}" for code, comment in rows]
    out += [
        "}};",
        "",
        "} // namespace EnergyPlus::Psychrometrics::PsatTable",
        "",
        "#endif",
        "",
    ]
    OUT.write_text("\n".join(out))
    print(f"Wrote {OUT.relative_to(REPO)}: h={h}, {n} intervals, max rel err {worst:.2e} at T={worst_T:.3f} C")


if __name__ == "__main__":
    main()
