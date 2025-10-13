#!/usr/bin/env python
# -*- coding: utf-8 -*-
# EnergyPlus, Copyright (c) 1996-2025, The Board of Trustees of the University
# of Illinois, The Regents of the University of California, through Lawrence
# Berkeley National Laboratory (subject to receipt of any required approvals
# from the U.S. Dept. of Energy), Oak Ridge National Laboratory, managed by UT-
# Battelle, Alliance for Sustainable Energy, LLC, and other contributors. All
# rights reserved.
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
In a lot of modules, a boolean flag such as `ErrorsFound` is passed by
reference constantly to several functions, and a Fatal will be issued at the
end if this boolean turns out to be `true`. Each single function that takes
this boolean should be responsible to turn it to 'true' as needed, but
shouldn't be forcing it to false, because it risks erasing previous errors.

Eg:
    bool ErrorsFound = false;
    functionA(&ErrorsFound);
    functionB(&ErrorsFound);

if functionB forces ErrorsFound to 'false' at the beginning and does pass, it
would override the output of functionA which may have found errors!

cf: https://github.com/NREL/EnergyPlus/issues/7147
Written in Winter 2019.
"""

__author__ = "Julien Marrec, EffiBEM"
__email__ = "julien@effibem.com"

import re
from pathlib import Path
from typing import Any

from base_hook import (
    SRC_DIR,
    ErrorMessage,
    LogLevel,
    LogMessage,
    WarningMessage,
    exit_hook,
    flatten_list_of_lists,
    get_base_parser,
    parallel_apply,
    report_log_messages,
)

INCLUDE_WARNINGS = True
IS_CI = True

# Files for which to ignore missing header warning
EXPECT_MISSING_HEADER = [
    SRC_DIR / x
    for x in [
        "main.cc",
        "test_ep_as_library.cc",
        "api/EnergyPlusPgm.cc",
        "WindowsGuiLauncher.cc",
    ]
]

EXPECT_MISSING_NAMESPACE = []

# Finds a boolean argument passed by reference
# Optional_bool acts like one, Array_XD_bool is another possibility
RE_BOOL = re.compile(
    r"\b(?P<booltype>" r"(?:(?:Array\dD_)?(?:bool\s*&\s+|bool\s+&\s*))|" r"Optional_bool\s+)(?P<boolname>\w+)"
)

CHECKED_AND_OKED = {
    "AirflowNetworkBalanceManager.cc": {"ManageAirflowNetworkBalance": ["ResimulateAirZone"]},
    "BranchInputManager.cc": {
        # Updated docstring
        "GetBranchFanTypeName": ["ErrFound"],
        # Updated docstring
        "GetLoopMixer": ["IsMixer"],
        # Updated docstring
        "GetLoopSplitter": [
            "IsSplitter",
        ],
        # Singular makes it pretty clear + docstring
        "TestBranchIntegrity": ["ErrFound"],
    },
    "BranchNodeConnections.cc": {
        # Docstring
        "GetComponentData": ["IsParent"]
    },
    "DElightManagerF.cc": {"CheckForGeometricTransform": ["doTransform", "doTransform"]},
    "DataHeatBalance.cc": {"AddVariableSlatBlind": ["errFlag"], "ComputeNominalUwithConvCoeffs": ["isValid"]},
    "DataRuntimeLanguage.cc": {"ValidateEMSProgramName": ["errFlag"], "ValidateEMSVariableName": ["errFlag"]},
    "DataSizing.cc": {"resetHVACSizingGlobals": ["firstPassFlag"]},
    "DaylightingManager.cc": {"CheckForGeometricTransform": ["doTransform", "doTransform"]},
    "DemandManager.cc": {"LoadInterface": ["CanReduceDemand"]},
    "ElectricPowerServiceManager.cc": {
        "simulateKineticBatteryModel": ["charging", "discharging"],
        "simulateSimpleBucketModel": ["charging", "discharging"],
        "simulateLiIonNmcBatteryModel": ["charging", "discharging"],
    },
    "EMSManager.cc": {"ManageEMS": ["anyProgramRan"]},
    "Fans.cc": {
        # This is a method but it just uses the function name
        "getInputsForDesignHeatGain": ["_fanCompModel"]
    },
    "FaultsManager.cc": {"SetFaultyCoilSATSensor": ["FaultyCoilSATFlag"]},
    "FuelCellElectricGenerator.cc": {
        # Never used anywhere
        "FigureTransientConstraints": ["Constrained"],
        # This one is never used anywhere
        "ManageElectStorInteractions": ["Constrained"],
    },
    "Furnaces.cc": {"HeatPumpRunFrac": ["errFlag"]},
    "General.cc": {"ScanForReports": ["DoReport"]},
    "GeneratorDynamicsManager.cc": {
        "ManageGeneratorFuelFlow": ["ConstrainedIncreasingMdot", "ConstrainedDecreasingMdot"]
    },
    "HVACControllers.cc": {
        # Always processed right after, + docstring
        "CheckCoilWaterInletNode": ["NodeNotFound"],
        "CheckTempAndHumRatCtrl": ["IsConvergedFlag"],
        "CheckSimpleController": ["IsConvergedFlag"],
        "ExitCalcController": ["IsUpToDateFlag"],
        "FindRootSimpleController": ["IsConvergedFlag"],
        # docstring, and used only once with dedicated unused bool
        "GetControllerActuatorNodeNum": ["NodeNotFound"],
        "InitController": ["IsConvergedFlag"],
        "ManageControllers": ["AllowWarmRestartFlag"],
        "ResetController": ["IsConvergedFlag"],
    },
    "HVACDXSystem.cc": {"ControlDXSystem": ["HXUnitOn"]},
    "HeatBalanceIntRadExchange.cc": {"UpdateMovableInsulationFlag": ["change"]},
    "HeatBalanceManager.cc": {"SearchWindow5DataFile": ["ConstructionFound", "EOFonFile"]},
    # Used only once, with a flag set to false beforehand anyway, and processed
    # right after the function call
    "InternalHeatGains.cc": {"GetInternalGainDeviceIndex": ["ErrorFound"]},
    # Used with a dedicated bool set to false just before
    "LowTempRadiantSystem.cc": {"InitLowTempRadiantSystem": ["InitErrorsFound"]},
    "MixedAir.cc": {
        "SimOAComponent": ["OAHeatingCoil", "OACoolingCoil", "OAHX"],
        "CalcOAEconomizer": [
            "HighHumidityOperationFlag",
        ],
        "Checksetpoints": [
            "EconomizerOperationFlag",
        ],
    },
    "NonZoneEquipmentManager.cc": {"ManageNonZoneEquipment": ["SimNonZoneEquipment"]},
    "OutAirNodeManager.cc": {
        # Always used with a dedicated bool as return value
        "CheckAndAddAirNodeNumber": ["Okay"]
    },
    "OutputProcessor.cc": {
        "DetermineMeterIPUnits": ["ErrorsFound"],
        "GetStandardMeterResourceType": ["ErrorsFound"],
        "ReportTSMeters": ["PrintESOTimeStamp"],
    },
    "OutputReportTabular.cc": {
        "ComputeTableBodyUsingMovingAvg": ["resCellsUsd"],
        "parseStatLine": [
            "isKoppen",
            "heatingDesignlinepassed",
            "coolingDesignlinepassed",
            "desConditionlinepassed",
            "insideLiquidPrecipitation",
        ],
        "produceDualUnitsFlags": ["produce_Sql", "produce_Tab"],
    },
    "PackagedTerminalHeatPump.cc": {"HeatPumpRunFrac": ["errFlag"]},
    "PlantCondLoopOperation.cc": {
        "ActivateEMSControls": [
            "LoopShutDownFlag",
        ],
        # Used only once with dedicated flag as return value
        # and processed right after
        "GetPlantOperationInput": ["GetInputOK"],
    },
    # The boolean is useless since RAFNNodeNum would return 0 if not found
    "RoomAirModelManager.cc": {"GetRAFNNodeNum": ["Errorfound"]},
    "RootFinder.cc": {
        # Used with dedicated bool as return value
        "IterateRootFinder": ["IsDoneFlag"]
    },
    "SetPointManager.cc": {
        "setupSetPointAndFlags": [
            "RunSubOptCondEntTemp",
            "RunOptCondEntTemp",
            "RunFinalOptCondEntTemp",
        ]
    },
    "SimAirServingZones.cc": {"SolveAirLoopControllers": ["AirLoopConvergedFlag"]},
    "SolarShading.cc": {
        # Used with a dedicated bool as return value
        "CHKGSS": ["CannotShade"]
    },
    "SurfaceGeometry.cc": {"CheckForReversedLayers": ["RevLayerDiffs"]},
    "SwimmingPool.cc": {
        # This is an inverse one-way toggle (can only set it to false if true
        # when passed)
        "InitSwimmingPoolPlantLoopIndex": ["MyPlantScanFlagPool", "MyPlantScanFlagPool"]
    },
    "SystemReports.cc": {
        # Processed right after
        "FindDemandSideMatch": ["MatchFound"],
        "FindFirstLastPtr": ["ConnectionFlag"],
    },
    "UnitarySystem.cc": {
        "controlCoolingSystemToSP": ["HXUnitOn"],
        # Used with dedicated bool
        "heatPumpRunFrac": ["errFlag"],
        "isWaterCoilHeatRecoveryType": ["nodeNotFound"],
        "simulate": ["CoolActive", "HeatActive"],
    },
    "UserDefinedComponents.cc": {"SimCoilUserDefined": ["HeatingActive", "CoolingActive"]},
    "UtilityRoutines.cc": {
        "ProcessNumber": [
            "ErrorFlag",
        ],
        "VerifyName": [
            "ErrorFound",
            "IsBlank",
        ],
    },
    "Vectors.cc": {
        "CalcCoPlanarNess": ["IsCoPlanar"],
        "CompareTwoVectors": ["areSame"],
        # Dedicated bool used, Error treated right away after
        "PlaneEquation": ["error"],
    },
    "WaterCoils.cc": {
        # Used once, with dedicated bool, and error treated right away
        "CheckActuatorNode": ["NodeNotFound"],
        # Used once, with dedicated bool, and error treated right away
        "CheckForSensorAndSetPointNode": ["NodeNotFound"],
    },
    "WaterThermalTanks.cc": {
        # Used once, with dedicated bool, and error treated right away
        "ValidatePLFCurve": ["IsValid"]
    },
    "WeatherManager.cc": {
        # Docstring is explicit that this is a return value True/False
        "GetNextEnvironment": [
            "Available",
        ],
        # ErrorFound being singular, that's ok. Plus, used with a dedicated
        # bool (which happens to be never checked for after call)
        "InterpretWeatherDataLine": ["ErrorFound"],
        # Docstring is explicit
        "ReportWeatherAndTimeInformation": ["printEnvrnStamp"],
    },
    "WindowAC.cc": {
        "ControlCycWindACOutput": [
            "HXUnitOn",
        ]
    },
    "WindowComplexManager.cc": {"CheckGasCoefs": ["feedData"]},
    "ZoneEquipmentManager.cc": {
        # Updated docstring
        "ManageZoneEquipment": ["SimZone"],
        "ReportZoneSizingDOASInputs": ["headerAlreadyPrinted"],
    },
}


###############################################################################
#                              F U N C T I O N S                              #
###############################################################################


def infer_header_from_source(source_file: Path) -> Path:
    """
    Guess the header file that matches a source_file.
    Throws if doesn't exist
    """
    target_ext = ".hh"
    header_file = source_file.parent / source_file.name.replace(".in.cc", target_ext).replace(".cc", target_ext)
    if not header_file.is_file():
        raise ValueError(f"Cannot find header file: {header_file}")

    return header_file


def format_found_function(found_function: dict[str, Any], one_line=False) -> str:
    """
    Helper to display a dict entry from `parse_function_signatures_in_header`
    """
    if one_line:
        args = " ".join([line.strip() for line in found_function["args"].splitlines()])
    else:
        args = found_function["args"]
    return_type = found_function["return_type"]
    function_name = found_function["function_name"]
    post_qualifiers = found_function["post_qualifiers"]

    return f"{return_type} {function_name}({args}){post_qualifiers}"


def parse_function_signatures_in_header(header_file: Path) -> list[dict[str, Any]]:
    """
    Opens the header file, and look for function signatures,
    returning only the ones that do include a bool passed by reference

    Args:
    -----
    * header_file (Path): path to the header file.

    Returns:
    --------
    * found_functions (list of dict): each entry of the list is a dict that
    has the following keys:
        ['return_type', 'function_name', 'args', 'post_qualifiers']

    """

    signature_pattern = (
        r"^(?:\t+| )+(?:static|virtual)?\s*"
        r"(?P<return_type>[^\s]+)\s+\b(?P<function_name>\w+)"
        r"\((?P<args>.*?)\)\s*"
        r"(?P<post_qualifiers>.*?)(?:override)?\s*;"
    )
    signature_re = re.compile(signature_pattern, re.MULTILINE | re.DOTALL)

    try:
        content = header_file.read_text()
    except UnicodeDecodeError as e:
        # This shouldn't anymore, we have check_non_utf8_files.py
        raise e
        if INCLUDE_WARNINGS:
            warning = WarningMessage(
                tool="find_byref_bool_overide",
                filepath=header_file,
                message=f"Cannot decode {header_file} as UTF-8, falling back to latin-1",
            )
            print(warning.to_json())
        content = header_file.read_text(encoding="latin-1")

    # Try to identify namespace name
    found_namespaces = []
    re_namespace = re.compile(r"^\s*(?:namespace|struct|class)\s+" r"(?P<namespace>\w+)")
    for line in content.splitlines():
        m = re_namespace.search(line)
        if m:
            found_namespaces.append(m.groupdict()["namespace"])

    if not found_namespaces:
        raise ValueError(f"Cannot find namespace for {header_file}")
    found_functions = []
    for m in signature_re.finditer(content):
        d = m.groupdict()
        bools = [m2.groupdict() for m2 in RE_BOOL.finditer(d["args"])]
        if bools:
            d["bools"] = bools
            d["namespaces"] = found_namespaces
            found_functions.append(d)
    return found_functions


def check_if_oked(file: Path, function_name: str, boolname: str) -> bool:
    file_name = file.name
    if file_name not in CHECKED_AND_OKED:
        return False
    if function_name not in CHECKED_AND_OKED[file_name]:
        return False
    if boolname not in CHECKED_AND_OKED[file_name][function_name]:
        return False
    return True


def lookup_errors_in_source_file(source_file: Path, found_functions: list[dict[str, str]]) -> list[LogMessage]:
    """
    Looks up the function bodies corresponding to each function
    in found_functions, and checks if a passed-by-reference bool is forced to
    false

    Args:
    -----
    * source_file (str): path to the .cc file
    * found_functions (list of dict): see `parse_function_signatures_in_header`

    Returns:
    --------
    * errors (list of dict): one entry per error, with the following keys:
        ['file', 'function', 'line_num', 'line']

    """
    log_messages: list[LogMessage] = []

    try:
        content = source_file.read_text()
    except UnicodeDecodeError as e:
        # This shouldn't anymore, we have check_non_utf8_files.py
        raise e
        if INCLUDE_WARNINGS:
            log_messages.append(
                WarningMessage(
                    tool="find_byref_bool_overide",
                    filepath=source_file,
                    message=f"Cannot decode {source_file} as UTF-8, falling back to latin-1",
                )
            )
        content = source_file.read_text(encoding="latin-1")

    lines = content.splitlines()

    # We look for the opening of the function in question
    cc_pat = r"{r}\s+(?:(?:{m})::)?{n}\s*\((?P<args>.*?)\)\s*{{"

    for i, found_function in enumerate(found_functions):

        fname = found_function["function_name"]

        signature_pattern = cc_pat.format(
            r=found_function["return_type"], m="|".join(found_function["namespaces"]), n=fname
        )
        re_signature = re.compile(signature_pattern, re.MULTILINE | re.DOTALL)
        m = re_signature.search(content)
        if not m:
            d = format_found_function(found_function)
            if INCLUDE_WARNINGS:
                log_messages.append(
                    WarningMessage(
                        tool="find_byref_bool_overide",
                        filepath=source_file,
                        message=f"Cannot find function {fname}: {d}",
                    )
                )
                # Skip iteration
                continue

        args = m.groupdict()["args"]
        bools = [_m.groupdict() for _m in RE_BOOL.finditer(args)]

        fbody_start_line_num = content[: m.end()].count("\n")
        line_num = fbody_start_line_num
        n_braces = lines[line_num].count("{") - lines[line_num].count("}")
        # Shouldn't happen
        if n_braces == 0:
            log_messages.append(
                WarningMessage(
                    tool="find_byref_bool_overide",
                    filepath=source_file,
                    line_number=line_num + 1,
                    message=f"n_braces is zero which is unexpected",
                )
            )
            while n_braces == 0:
                n_braces = lines[line_num].count("{") - lines[line_num].count("}")
                line_num += 1
        while n_braces > 0:
            line_num += 1
            # Remove the comment portion
            line = lines[line_num].split("//")[0].strip()
            n_braces += line.count("{") - line.count("}")

            for b_dict in bools:
                b = b_dict["boolname"]

                # If checked and Okay'ed, we skip it
                if check_if_oked(file=source_file, function_name=fname, boolname=b):
                    # print("Skipped")
                    continue

                pat = r"\b{b}\s*=\s*false;".format(b=b)
                re_this_bool = re.compile(pat)
                if re_this_bool.search(line):
                    b_info = "{}{}".format(b_dict["booltype"], b_dict["boolname"])
                    log_messages.append(
                        ErrorMessage(
                            tool="find_byref_bool_overide",
                            filepath=source_file,
                            line_number=line_num,
                            line=line,
                            message=f"Boolean flag `{b_info}` reset to false: {fname}()",
                        )
                    )

    return log_messages


###############################################################################
#                                   M A I N                                   #
###############################################################################


def find_byref_bool_override(source_file: Path) -> list[LogMessage]:
    """Check a single file.

    Args:
    -----
    * source_file (Path): The .cc file to check

    Returns:
    --------
    * log_messages (list of dict): one entry per error or warning
    """

    log_messages: list[LogMessage] = []

    # No point continuing if we can't find the header, or there is no namespace
    if source_file in EXPECT_MISSING_HEADER or source_file in EXPECT_MISSING_NAMESPACE:
        return None

    try:
        header_file = infer_header_from_source(source_file=source_file)
    except ValueError:
        log_messages.append(
            WarningMessage(
                tool="find_byref_bool_overide",
                filepath=source_file,
                message=f"Cannot find corresponding header file",
            )
        )
        return log_messages

    try:
        found_functions = parse_function_signatures_in_header(header_file=header_file)
    except ValueError as e:
        log_messages.append(
            WarningMessage(
                tool="find_byref_bool_overide",
                filepath=source_file,
                message=str(e),
            )
        )
        return log_messages

    if not found_functions:
        return log_messages

    log_messages += lookup_errors_in_source_file(source_file, found_functions)

    return log_messages


if __name__ == "__main__":
    parser = get_base_parser(
        description="Find ByRef bool override", files_arg_help=f"Cpp Files to check (if omitted, checks src/EnergyPlus)"
    )

    args = parser.parse_args()
    if args.files:
        n_ori = len(args.files)
        source_files = [
            f
            for f in args.files
            if f.suffix == ".cc"
            # Original was NOT recursive
            and f.parent == SRC_DIR
        ]
        if args.verbose:
            print(f"Checking {len(source_files)} of {n_ori} specified files")
    else:
        # Glob all .cc files
        # TODO: this should be changed to rglob to grab files in subdirs
        source_files = SRC_DIR.glob("*.cc")
        source_files = [f for f in source_files if f.parent.name != "api"]
        if args.verbose:
            print(f"Checking all {len(source_files)} .cc files in {SRC_DIR}")

    errors_list_of_lists = parallel_apply(func=find_byref_bool_override, filepaths=source_files)
    log_messages = flatten_list_of_lists(list_of_lists=errors_list_of_lists)
    success = report_log_messages(log_messages=log_messages, fail_threshold=LogLevel.ERROR, verbose=args.verbose)
    exit_hook(success=success)
