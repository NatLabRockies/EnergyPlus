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

import glob as gb
import json
import os
import re
import sys
import warnings

INCLUDE_WARNINGS = True
IS_CI = True

# Get a path that'll work if run directly from this folder (when running
# locally usually) or the root of the repo (decent_ci for eg)
REPO_ROOT = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "..")
SRC_DIR = os.path.abspath(os.path.join(REPO_ROOT, "src", "EnergyPlus"))

# Files for which to ignore missing header warning
EXPECT_MISSING_HEADER = [
    "src/EnergyPlus/main.cc",
    "src/EnergyPlus/test_ep_as_library.cc",
    "EnergyPlusPgm.cc",
    "src/EnergyPlus/WindowsGuiLauncher.cc",
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


def infer_header_from_source(source_file):
    """
    Guess the header file that matches a source_file.
    Throws if doesn't exist
    """
    header_file = source_file.replace(".in.cc", ".hh").replace(".cc", ".hh")
    if not os.path.isfile(header_file):
        raise ValueError("Cannot find header file: {}".format(header_file))

    return header_file


def format_found_function(found_function, one_line=False):
    """
    Helper to display a dict entry from `parse_function_signatures_in_header`
    """
    if one_line:
        args = " ".join([line.strip() for line in found_function["args"].splitlines()])
    else:
        args = found_function["args"]

    return "{} {}({}){}".format(
        found_function["return_type"], found_function["function_name"], args, found_function["post_qualifiers"]
    )


def parse_function_signatures_in_header(header_file):
    """
    Opens the header file, and look for function signatures,
    returning only the ones that do include a bool passed by reference

    Args:
    -----
    * header_file (str): path to the header file.

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

    # Relative path, for cleaner reporting
    rel_file = os.path.relpath(header_file, start=SRC_DIR)

    try:
        with open(header_file, "r") as f:
            content = f.read()
    except UnicodeDecodeError:
        if INCLUDE_WARNINGS:
            msg = "Cannot decode {} as UTF-8, falling back to " "latin-1".format(rel_file)
            if IS_CI:
                ci_msg = {"tool": "find_byref_bool_overide", "file": rel_file, "messagetype": "warning", "message": msg}
                print(json.dumps(ci_msg))
            else:
                warnings.warn(msg)
        with open(header_file, "r", encoding="latin-1") as f:
            content = f.read()

    # Try to identify namespace name
    found_namespaces = []
    re_namespace = re.compile(r"^\s*(?:namespace|struct|class)\s+" r"(?P<namespace>\w+)")
    for line in content.splitlines():
        m = re_namespace.search(line)
        if m:
            found_namespaces.append(m.groupdict()["namespace"])

    if not found_namespaces:
        raise ValueError("Cannot find namespace for {}".format(header_file))
    found_functions = []
    for m in signature_re.finditer(content):
        d = m.groupdict()
        bools = [m2.groupdict() for m2 in RE_BOOL.finditer(d["args"])]
        if bools:
            d["bools"] = bools
            d["namespaces"] = found_namespaces
            found_functions.append(d)
    return found_functions


def check_if_oked(rel_file, function_name, boolname):
    file_name = os.path.basename(rel_file)
    if file_name not in CHECKED_AND_OKED:
        return False
    if function_name not in CHECKED_AND_OKED[file_name]:
        return False
    if boolname not in CHECKED_AND_OKED[file_name][function_name]:
        return False
    return True


def lookup_errors_in_source_file(source_file, found_functions):
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

    # Relative path, for cleaner reporting
    rel_file = os.path.relpath(source_file, start=REPO_ROOT)

    try:
        with open(source_file, "r") as f:
            content = f.read()
    except UnicodeDecodeError:
        if INCLUDE_WARNINGS:
            msg = "Cannot decode {} as UTF-8, falling back to " "latin-1".format(rel_file)
            if IS_CI:
                ci_msg = {"tool": "find_byref_bool_overide", "file": rel_file, "messagetype": "warning", "message": msg}
                print(json.dumps(ci_msg))
            else:
                warnings.warn(msg)

        with open(source_file, "r", encoding="latin-1") as f:
            content = f.read()

    lines = content.splitlines()

    errors = []

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
                msg = "In file {f}, cannot find function {n}:\n" "{d}".format(f=rel_file, n=fname, d=d)
                if IS_CI:
                    ci_msg = {
                        "tool": "find_byref_bool_overide",
                        "file": rel_file,
                        "messagetype": "warning",
                        "message": msg,
                    }
                    print(json.dumps(ci_msg))
                else:
                    warnings.warn(msg)

                # Skip iteration
                continue

        args = m.groupdict()["args"]
        bools = [_m.groupdict() for _m in RE_BOOL.finditer(args)]

        fbody_start_line_num = content[: m.end()].count("\n")
        line_num = fbody_start_line_num
        n_braces = lines[line_num].count("{") - lines[line_num].count("}")
        # Shouldn't happen
        if n_braces == 0:
            msg = "In file {f}, line num = {n}: \n" "n_braces is zero which is unexpected.".format(
                f=rel_file, n=line_num
            )
            if IS_CI:
                ci_msg = {
                    "tool": "find_byref_bool_overide",
                    "file": rel_file,
                    "messagetype": "warning",
                    "message": msg,
                    "line_num": line_num,
                }
                print(json.dumps(ci_msg))
            else:
                warnings.warn(msg)

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
                if check_if_oked(rel_file=rel_file, function_name=fname, boolname=b):
                    # print("Skipped")
                    continue

                pat = r"\b{b}\s*=\s*false;".format(b=b)
                re_this_bool = re.compile(pat)
                if re_this_bool.search(line):
                    b_info = "{}{}".format(b_dict["booltype"], b_dict["boolname"])

                    msg = "Boolean flag `{b}` reset to false in {f}:" "{function}(), on line {n}"  # :\n"{line}")
                    msg = msg.format(b=b_info, f=rel_file, function=fname, n=line_num, line=line)

                    errors.append(
                        {
                            "file": rel_file,
                            "function_name": fname,
                            "function": found_function,
                            "bool": b_dict,
                            "line_num": line_num,
                            "line": line,
                            "msg": msg,
                        }
                    )

    return errors


def generate_potential_false_positive_dict(all_errors):
    """
    A helper to generate a list of all errors, so you can more easily update it
    above
    """
    errs_by_file = {}
    for error in all_errors:
        f = error["file"]
        if f not in errs_by_file:
            errs_by_file[f] = {}
        function_name = error["function_name"]
        b = error["bool"]["boolname"]
        if function_name not in errs_by_file[f]:
            errs_by_file[f][function_name] = []
        errs_by_file[f][function_name].append(b)
    print(json.dumps(errs_by_file, indent=4, sort_keys=True))
    return errs_by_file


###############################################################################
#                                   M A I N                                   #
###############################################################################


def get_all_errors(source_files):
    """
    Run everything, given a list of source_files

    Args:
    -----
    * source_files (list of str): list of paths

    Returns:
    --------

    all_errors (list of dict): one entry per error, with the following keys:
        ['file', 'function', 'line_num', 'line']
    """

    all_errors = []

    for source_file in source_files:
        rel_file = os.path.relpath(source_file, start=REPO_ROOT)
        try:
            header_file = infer_header_from_source(source_file)
        except ValueError:
            if (rel_file not in EXPECT_MISSING_HEADER) and INCLUDE_WARNINGS:
                msg = "Cannot find header file for " "{}".format(rel_file)
                if IS_CI:
                    ci_msg = {
                        "tool": "find_byref_bool_overide",
                        "file": rel_file,
                        "messagetype": "warning",
                        "message": msg,
                    }
                    print(json.dumps(ci_msg), file=sys.stderr)
                else:
                    warnings.warn(msg)
            continue

        try:
            found_functions = parse_function_signatures_in_header(header_file)
        except ValueError as e:
            if (rel_file not in EXPECT_MISSING_NAMESPACE) and INCLUDE_WARNINGS:
                if IS_CI:
                    ci_msg = {
                        "tool": "find_byref_bool_override",
                        "file": rel_file,
                        "messagetype": "warning",
                        "message": str(e),
                    }
                    print(json.dumps(ci_msg), file=sys.stderr)
                else:
                    warnings.warn(str(e))
            continue
        if not found_functions:
            # print("No problem for {}".format(rel_file))
            pass
        else:
            errors = lookup_errors_in_source_file(source_file, found_functions)
            if errors:
                all_errors += errors

    # Sort errors by file
    all_errors.sort(key=lambda x: x["file"], reverse=False)

    return all_errors


def output_errors_to_console(all_errors):
    for error in all_errors:
        if IS_CI:
            ci_msg = {
                "tool": "find_byref_bool_overide",
                "file": error["file"],
                "line": error["line_num"],
                "messagetype": "error",
                "message": error["msg"],
            }
            print(json.dumps(ci_msg), file=sys.stderr)
        else:
            print(error["msg"], file=sys.stderr)


if __name__ == "__main__":
    # Glob all .cc files
    source_files = gb.glob(os.path.join(SRC_DIR, "*.cc"))
    all_errors = get_all_errors(source_files)
    output_errors_to_console(all_errors)
    if len(all_errors) > 0:
        sys.exit(1)
