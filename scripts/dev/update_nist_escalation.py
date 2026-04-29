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

"""
A python helper with zero non system dependencies to parse the `encost<YY>.txt`
files from NIST and produce a nicely formatted
`LCCusePriceEscalationDataSet<YYYY>.idf` as a result

This assumes you just put the next encostxx.txt in the datasets/ folder prior
to running the script.

The `encost<YY>.txt` can be found at:
https://www.energy.gov/eere/femp/building-life-cycle-cost-programs
"""

from pathlib import Path
from typing import Any, List, Optional, Tuple

ROOT_DIR = Path(__file__).resolve().parent
DATASETS_DIR = ROOT_DIR / "datasets"

NUMBER_YEARS = 31

FUEL_NAME_CONVERSIONS = {
    # NIST 135 Name: EnergyPlus Name
    "electricity": "Electricity",
    "distillate oil": "FuelOilNo1",
    "natural gas": "NaturalGas",
    "lpg": "Propane",
    "residual oil": "FuelOilNo2",
    "coal": "Coal",
}

HEADER_FMT = """! The source of the values for the following objects is:
!
!    NISTIR 85-3273-34
!
!    ENERGY PRICE INDICES AND DISCOUNT FACTORS
!    FOR LIFE-CYCLE COST ANALYSIS - {year}
!
!    Annual Supplement to NIST Handbook 135
!
!    Priya D. Lavappa
!    Joshua D. Kneifel
!
! These are factors for the United States and regions of the United States.
!
! The factors are from a file available upon request from NIST called ENCOSTxx.TXT and are
! converted into EnergyPlus objects using scripts/dev/update_nist_escalation.py
! https://www.energy.gov/eere/femp/building-life-cycle-cost-programs

"""


def parse_encost(fpath: Path) -> Tuple[List[dict[str, Any]], int]:
    """
    Processed the encost file into a python structure which is a list of dicts

    Returns:
    ---------
    * results [List[dict]]: the structure
    * escalation_start_year [int]: the escalation start_year
    """

    fname = fpath.with_suffix("").name
    year = int(f"20{fname.replace('encost', '')}")
    print(f"Start parsing {fpath=}, {year=}")

    with open(fpath, "r") as f:
        content = f.read()
    lines = [x.strip() for x in content.splitlines()]

    region = None
    years: list[int] | None = None
    escalation_start_year: int | None = None
    fuel = None
    prices = None
    results: list[dict[str, Any]] = []

    for i, line in enumerate(lines):
        if not line:
            region = None
            years = None
            # print("Reset.\n")
            continue

        if region is None:
            region = line.expandtabs(1)
            # print(f"{region=}")
            continue

        if years is None:
            years = [int(x) for x in line.split()]
            if len(years) != NUMBER_YEARS:
                print(f"Warning: expected {NUMBER_YEARS} years, " f"got {len(years)}: {years}")
            if years[0] != year:
                raise ValueError(
                    f"Given the name of the file {fpath.name=}, we assumed " f"we would find {year=} but got {years[0]}"
                )
            if escalation_start_year is not None:
                if escalation_start_year != years[1]:
                    raise ValueError("Inconsistent escalation start year")

            escalation_start_year = years[1]
            continue

        if fuel is None:
            fuel = line
            prices = None
            continue

        if prices is None:
            prices = [float(x) for x in line.split()]
            if len(prices) != NUMBER_YEARS:
                print(f"Warning: expected {NUMBER_YEARS} years, " f"got {len(prices)} prices: {prices}")

            escalations = [x / prices[0] for x in prices[1:]]
            this_dict = {
                "region": region,
                "fuel": fuel,
                "escalations": dict(zip(years[1:], escalations)),
                # 'prices': dict(zip(years, prices)),
            }
            results.append(this_dict)
            fuel = None

    assert escalation_start_year is not None, "Escalation start year not found"

    return results, escalation_start_year


def plot_escalations(results: List[dict], escalation_start_year: Optional[int] = None):
    # Lazy import, these are stdlib deps, so only for local use
    import matplotlib.pyplot as plt
    import numpy as np
    import pandas as pd

    pandas_results = []
    for lcc in results:
        this_dict = {}
        this_dict["region"] = lcc["region"]
        this_dict["fuel"] = lcc["fuel"]
        for k, v in lcc["escalations"].items():
            this_dict[k] = v
        pandas_results.append(this_dict)

    df = pd.DataFrame(pandas_results).set_index(["region", "fuel"]).T

    grouped = df.groupby("region", axis=1)  # type: ignore[call-overload]

    ncols = 2
    nrows = int(np.ceil(grouped.ngroups / ncols))

    fig, axes = plt.subplots(nrows=nrows, ncols=ncols, figsize=(16, 24), sharey=True)

    for key, ax in zip(grouped.groups.keys(), axes.flatten()):
        grouped.get_group(key)[key].plot(ax=ax)
        ax.set_title(f"Region: {key}")

    ax.legend()
    if escalation_start_year is not None:
        fig.suptitle(f"Year {escalation_start_year}")
    plt.tight_layout()
    plt.show()


def format_field(field_str: str, field_name: str, last_field: Optional[bool] = False) -> str:
    """
    Helper to format the IDF
    """
    sep = ";" if last_field else ","
    content = f"    {field_str}{sep}"
    n = len(content)
    if n < 28:
        offset = 28 - n + 1
        content += " " * offset
    else:
        content += "  "
    content += f"!- {field_name}"
    return content


def produce_idf(results: List[dict], escalation_start_year: int):
    """
    Will produce the IDF file itself and save it on disk
    """
    content = HEADER_FMT.format(year=escalation_start_year - 1).splitlines()
    for lcc in results:
        content.append("LifeCycleCost:UsePriceEscalation,")
        region = lcc["region"]
        fuel = lcc["fuel"]
        lcc_name = f"{region}-{fuel}"
        if fuel.lower() in FUEL_NAME_CONVERSIONS:
            # print(f"Replacing {fuel=}")
            fuel = FUEL_NAME_CONVERSIONS[fuel.lower()]

        content.append(format_field(lcc_name, "Name"))
        content.append(format_field(fuel, "Resource"))
        content.append(format_field(str(escalation_start_year), "Escalation Start Year"))
        content.append(format_field("January", "Escalation Start Month"))

        escalations = lcc["escalations"]
        for i, (year, value) in enumerate(sorted(escalations.items(), reverse=False)):
            content.append(
                format_field(
                    f"{value:.4f}",
                    f"Year {i+1: <2} Escalation [{year}]",
                    last_field=(i + 1 == len(escalations)),
                )
            )
        content.append("")

    fpath = DATASETS_DIR / f"LCCusePriceEscalationDataSet{escalation_start_year - 1}.idf"
    with open(fpath, "w") as f:
        f.write("\n".join(content) + "\n")
    print(f"Saved as {fpath=}")


if __name__ == "__main__":
    encosts = list(DATASETS_DIR.glob("encost*.txt"))
    for fpath in encosts:
        results, escalation_start_year = parse_encost(fpath=fpath)
        # plot_escalations(results,
        #                  escalation_start_year=escalation_start_year)
        produce_idf(results=results, escalation_start_year=escalation_start_year)
