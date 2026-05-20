#!/usr/bin/env python3
"""Read EnergyPlus ESO/MTR output files and write delimited tables.

This is a Python port of the historical ReadVarsESO Fortran utility.  It keeps
the same legacy command-line interface while also exposing modern ``list`` and
``read`` subcommands for inspecting and converting ESO/MTR files directly.
"""

from __future__ import annotations

import argparse
import csv
from dataclasses import dataclass
import json
from pathlib import Path
import sys
import time
from typing import Iterable, TextIO


NUM_ALLOWED = 255
UNLIMITED_WARNING_COUNT = 3500
MAX_OUTPUT_VALUE_LENGTH = 25

FREQUENCY_MARKERS = {
    1: "!TimeStep",
    2: "!Hourly",
    3: "!Daily",
    4: "!Monthly",
    5: "!RunPeriod",
}

FREQUENCY_ALIASES = {
    "timestep": 1,
    "time-step": 1,
    "detailed": 1,
    "detail": 1,
    "hourly": 2,
    "daily": 3,
    "monthly": 4,
    "annual": 5,
    "runperiod": 5,
    "run-period": 5,
}

MODERN_FREQUENCY_ALIASES = {
    "timestep": {"timestep", "time step", "time-step"},
    "time-step": {"timestep", "time step", "time-step"},
    "detailed": {"detailed"},
    "detail": {"detailed"},
    "hourly": {"hourly"},
    "daily": {"daily"},
    "monthly": {"monthly"},
    "annual": {"annual"},
    "runperiod": {"runperiod", "run period", "environment"},
    "run-period": {"runperiod", "run period", "environment"},
}

MODERN_COMMAND_ALIASES = {
    "--list": "list",
    "--variables": "list",
    "--read": "read",
}

MONTHS = [
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December",
]


@dataclass
class Options:
    var_file_name: str
    get_vars_from_eso: bool
    frequency: int
    limited: bool
    fix_header: bool


@dataclass
class Requests:
    track_numbers: list[int]
    ignore_numbers: list[int]
    find_variables: list[str]
    find_variable_processed: list[int]
    ignore_find_variables: list[str]


@dataclass
class DictionaryRecord:
    number: int
    line: str
    label: str
    key: str = ""
    variable: str = ""
    units: str = ""
    frequency: str = ""
    raw_frequency: str = ""


@dataclass
class SelectedVariable:
    number: int
    label: str
    found: bool


class ReadVarsFatal(Exception):
    def __init__(self, messages: Iterable[str], exit_code: int = 1):
        self.messages = list(messages)
        self.exit_code = exit_code
        super().__init__("\n".join(self.messages))


class EarlyExit(Exception):
    pass


def display_string(message: str) -> None:
    print(message)


def audit_write(audit: TextIO | None, message: str = "") -> None:
    if audit is not None:
        audit.write(f"{message}\n")


def fatal(audit: TextIO | None, messages: Iterable[str], exit_code: int = 1) -> None:
    materialized = list(messages)
    for message in materialized:
        display_string(message)
        audit_write(audit, message)
    raise ReadVarsFatal([], exit_code)


def parse_options(argv: list[str]) -> Options:
    if not argv:
        return Options("", True, 0, True, False)

    var_file_name = argv[0].lstrip()
    get_vars_from_eso = var_file_name == ""
    frequency = 0
    limited = True
    fix_header = False

    for raw_arg in argv[1:]:
        arg = raw_arg.strip().lower()
        if arg.startswith("t") or arg.startswith("de"):
            frequency = 1
        if arg.startswith("h"):
            frequency = 2
        if arg.startswith("da"):
            frequency = 3
        if arg.startswith("m"):
            frequency = 4
        if arg.startswith("a") or arg.startswith("r"):
            frequency = 5
        if arg.startswith("u") or arg.startswith("n"):
            limited = False
        if arg.startswith("f"):
            fix_header = True

    return Options(var_file_name, get_vars_from_eso, frequency, limited, fix_header)


def is_modern_cli(argv: list[str]) -> bool:
    if not argv:
        return False

    first = argv[0].lower()
    return first in {"list", "variables", "read", "-h", "--help"} or first in MODERN_COMMAND_ALIASES


def normalized_modern_args(argv: list[str]) -> list[str]:
    if not argv:
        return argv

    first = argv[0].lower()
    replacement = MODERN_COMMAND_ALIASES.get(first)
    if replacement is None:
        return argv

    return [replacement] + argv[1:]


def build_modern_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="ReadVarsESO",
        description="Convert EnergyPlus ESO/MTR files or inspect their available output variables.",
        epilog=(
            "Examples:\n"
            "  ReadVarsESO list eplusout.eso --frequency hourly --search temperature\n"
            "  ReadVarsESO --list eplusout.eso --format csv\n"
            "  ReadVarsESO read eplusout.eso --output hourly.csv --frequency hourly"
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    subparsers = parser.add_subparsers(dest="command")

    list_parser = subparsers.add_parser("list", aliases=["variables"], help="List variables in an ESO/MTR data dictionary.")
    list_parser.add_argument("input_file", nargs="?", default="eplusout.eso", help="ESO/MTR file to inspect.")
    list_parser.add_argument(
        "-f",
        "--frequency",
        choices=sorted(MODERN_FREQUENCY_ALIASES),
        help="Only show variables reported at this frequency.",
    )
    list_parser.add_argument(
        "-s",
        "--search",
        help="Only show variables whose key, variable name, units, frequency, or label contains this text.",
    )
    list_parser.add_argument(
        "--format",
        choices=["table", "csv", "json"],
        default="table",
        help="Output format.",
    )

    read_parser = subparsers.add_parser("read", help="Convert an ESO/MTR file to CSV.")
    read_parser.add_argument("input_file", nargs="?", default="eplusout.eso", help="ESO/MTR file to convert.")
    read_parser.add_argument(
        "-o",
        "--output",
        help="CSV output file. Defaults to the input file name with a .csv extension.",
    )
    read_parser.add_argument(
        "-f",
        "--frequency",
        choices=sorted(MODERN_FREQUENCY_ALIASES),
        help="Only include variables reported at this frequency.",
    )
    read_parser.add_argument(
        "-s",
        "--search",
        help="Only include variables whose key, variable name, units, frequency, or label contains this text.",
    )

    return parser


def read_lines(path: Path) -> list[str]:
    with path.open("r", encoding="utf-8-sig", errors="replace") as handle:
        return [line.rstrip("\r\n") for line in handle]


def strip_comment_for_file_name(line: str, audit: TextIO | None) -> str | None:
    line = line.lstrip()
    if line.startswith("!"):
        audit_write(audit, f" ignoring comment line={line.rstrip()}")
        return None

    comment_position = line.find("!")
    if comment_position > 0:
        audit_write(audit, f"comment stripped on line:{line.rstrip()}")
        line = line[:comment_position]
    elif comment_position == 0:
        line = ""

    return line.strip()


def separator_for_output(output_file_name: str) -> str:
    suffix = Path(output_file_name.strip()).suffix.lower()
    if suffix == ".tab":
        return "\t"
    if suffix == ".txt":
        return " "
    return ","


def process_number(text: str) -> float:
    valid_first = "0123456789.+-\t"
    if not text or text[0] not in valid_first:
        return -999.0

    stripped = text.strip()
    if not stripped:
        return -999.0

    token = stripped.split()[0]
    try:
        return float(token)
    except ValueError:
        return -999.0


def normalize_variable_request(line: str) -> str:
    bracket_position = line.find("[")
    if bracket_position != -1:
        line = line[:bracket_position]

    temp_var = line.lstrip()
    comma_position = temp_var.find(",")
    if comma_position != -1:
        return temp_var[: comma_position + 1].rstrip() + temp_var[comma_position + 1 :].lstrip()

    return temp_var.strip()


def parse_rvi_variable_requests(lines: list[str], audit: TextIO | None) -> Requests:
    requests = Requests([], [], [], [], [])
    done = False

    for raw_line in lines:
        if done:
            break

        line = raw_line.replace("\t", " ").lstrip()
        comment_position = line.find("!")
        if comment_position > 0:
            audit_write(audit, f" stripping comment from line={line.rstrip()}")
            line = line[:comment_position]
        elif comment_position == 0:
            audit_write(audit, f" ignoring comment line={line.rstrip()}")
            continue

        ignore_this_one = False
        if "," not in line:
            if line.startswith("~"):
                ignore_this_one = True
                number = process_number(line[1:])
            else:
                number = process_number(line)
        else:
            number = -999.0

        if line.strip() == "":
            number = 0.0

        if number > 0:
            report_number = int(number)
            if ignore_this_one:
                requests.ignore_numbers.append(report_number)
            else:
                requests.track_numbers.append(report_number)
            continue

        if number < 0:
            if line.startswith("~"):
                request = normalize_variable_request(line[1:])
                requests.ignore_find_variables.append(request)
            else:
                request = normalize_variable_request(line)
                requests.find_variables.append(request)
                requests.find_variable_processed.append(0)
        else:
            done = True

    return requests


def read_rvi_configuration(options: Options, audit: TextIO | None) -> tuple[str, str, str, bool, list[str]]:
    if options.get_vars_from_eso:
        input_file_name = "eplusout.eso"
        if not Path(input_file_name).is_file():
            fatal(
                audit,
                [
                    f"Requested ESO file={input_file_name}",
                    "does not exist.  ReadVarsESO program terminated.",
                    "ReadVarsESO program terminated.",
                ],
            )
        return input_file_name, "eplusout.csv", ",", True, []

    var_file = Path(options.var_file_name)
    if not var_file.is_file():
        fatal(
            audit,
            [
                f"Requested Report Variable input file={options.var_file_name}",
                "does not exist.  Check eplusout.err file for possible explanations.",
                "ReadVarsESO program terminated.",
            ],
        )

    audit_write(audit, f"processing:{options.var_file_name}")
    rvi_lines = read_lines(var_file)
    cursor = 0

    input_file_name = ""
    while cursor < len(rvi_lines) and not input_file_name:
        parsed = strip_comment_for_file_name(rvi_lines[cursor], audit)
        cursor += 1
        if parsed is None:
            continue
        input_file_name = parsed or "eplusout.eso"

    if not input_file_name:
        input_file_name = "eplusout.eso"
        output_file_name = "eplusout.csv"
        if not Path(input_file_name).is_file():
            fatal(
                audit,
                [
                    f"Requested ESO file={input_file_name}",
                    "does not exist.  ReadVarsESO program terminated.",
                    "ReadVarsESO program terminated.",
                ],
            )
        return input_file_name, output_file_name, ",", True, []

    if not Path(input_file_name).is_file():
        fatal(
            audit,
            [
                f"Requested ESO file={input_file_name}",
                "does not exist.  ReadVarsESO program terminated.",
                "ReadVarsESO program terminated.",
            ],
        )

    audit_write(audit, f"input file:{input_file_name}")

    output_file_name = ""
    while cursor < len(rvi_lines) and not output_file_name:
        parsed = strip_comment_for_file_name(rvi_lines[cursor], audit)
        cursor += 1
        if parsed is None:
            continue
        output_file_name = parsed or "eplusout.csv"

    if not output_file_name:
        output_file_name = "eplusout.csv"

    separator = separator_for_output(output_file_name)

    remaining_lines = rvi_lines[cursor:]
    get_vars_from_eso = False
    if not remaining_lines:
        get_vars_from_eso = True
    else:
        probe = remaining_lines[0].lstrip()
        if probe.strip() == "" or probe.strip() == "0":
            get_vars_from_eso = True
            remaining_lines = remaining_lines[1:]

    return input_file_name, output_file_name, separator, get_vars_from_eso, remaining_lines


def parse_report_number(line: str) -> int | None:
    comma_position = line.find(",")
    if comma_position == -1:
        return None

    try:
        return int(float(line[:comma_position].strip()))
    except ValueError:
        return None


def build_header_label(line: str) -> str:
    fields = line.split(",", 2)
    if len(fields) < 3:
        return ""

    remainder = fields[2]
    bang_position = remainder.find("!")
    if bang_position == -1:
        return remainder.strip().replace(",", ":")

    label = remainder[:bang_position].rstrip() + "("
    frequency_part = remainder[bang_position + 1 :].rstrip()
    bracket_position = frequency_part.find("[")

    if bracket_position != -1:
        label += frequency_part[: max(bracket_position - 1, 0)]
        close_bracket_position = frequency_part.find("]", bracket_position + 1)
        if close_bracket_position != -1:
            next_position = close_bracket_position + 1
            if next_position < len(frequency_part) and frequency_part[next_position] == ",":
                label += frequency_part[next_position:]
        label += ")"
    else:
        label += frequency_part + ")"

    return label.replace(",", ":")


def split_variable_units(variable_with_units: str) -> tuple[str, str]:
    variable_with_units = variable_with_units.strip()
    close_position = variable_with_units.rfind("]")
    open_position = variable_with_units.rfind("[", 0, close_position)
    if open_position == -1 or close_position == -1:
        return variable_with_units, ""

    variable = variable_with_units[:open_position].rstrip()
    units = variable_with_units[open_position + 1 : close_position].strip()
    return variable, units


def normalized_frequency(raw_frequency: str) -> str:
    raw_frequency = raw_frequency.strip()
    bracket_position = raw_frequency.find("[")
    if bracket_position != -1:
        raw_frequency = raw_frequency[:bracket_position].rstrip()

    comma_position = raw_frequency.find(",")
    if comma_position != -1:
        raw_frequency = raw_frequency[:comma_position].rstrip()

    return raw_frequency


def parse_dictionary_record(line: str) -> DictionaryRecord | None:
    number = parse_report_number(line)
    if number is None or number <= 5:
        return None

    fields = line.split(",", 2)
    if len(fields) < 3:
        return DictionaryRecord(number, line, build_header_label(line))

    remainder = fields[2]
    bang_position = remainder.find("!")
    if bang_position == -1:
        definition = remainder.strip()
        raw_frequency = ""
    else:
        definition = remainder[:bang_position].strip()
        raw_frequency = remainder[bang_position + 1 :].strip()

    comma_position = definition.find(",")
    if comma_position == -1:
        key = ""
        variable_with_units = definition
    else:
        key = definition[:comma_position].strip()
        variable_with_units = definition[comma_position + 1 :].strip()

    variable, units = split_variable_units(variable_with_units)
    return DictionaryRecord(
        number=number,
        line=line,
        label=build_header_label(line),
        key=key,
        variable=variable,
        units=units,
        frequency=normalized_frequency(raw_frequency),
        raw_frequency=raw_frequency,
    )


def dictionary_records(eso_lines: list[str], audit: TextIO | None) -> tuple[list[DictionaryRecord], int]:
    end_index = None
    for index, line in enumerate(eso_lines):
        if line.strip() == "End of Data Dictionary":
            end_index = index
            break

    if end_index is None:
        fatal(
            audit,
            [
                "EOF encountered during read of ESO header records",
                "probable EnergyPlus error condition -- check eplusout.err",
                "ReadVarsESO program terminated.",
            ],
        )

    records: list[DictionaryRecord] = []
    for line in eso_lines[:end_index]:
        record = parse_dictionary_record(line)
        if record is not None:
            records.append(record)

    return records, end_index


def is_allowed_frequency(line: str, frequency: int) -> bool:
    marker = FREQUENCY_MARKERS.get(frequency)
    return marker is None or marker in line


def record_matches_modern_frequency(record: DictionaryRecord, frequency_alias: str | None) -> bool:
    if not frequency_alias:
        return True

    normalized = record.frequency.strip().lower().replace("-", " ")
    return normalized in MODERN_FREQUENCY_ALIASES[frequency_alias]


def myindex(text: str, substring: str) -> int:
    if not substring:
        return -1
    return text.upper().find(substring.upper())


def is_ignored(record: DictionaryRecord, requests: Requests) -> bool:
    if record.number in requests.ignore_numbers:
        return True

    for ignore in requests.ignore_find_variables:
        if myindex(record.line, ignore.strip()) != -1:
            return True

    return False


def request_matches_dictionary_line(line: str, request: str, exact: bool) -> bool:
    request = request.strip()
    position = myindex(line, request)
    if position == -1:
        return False

    if position == 0 or line[position - 1] != ",":
        return False

    if not exact:
        return True

    suffix = line[position:]
    bracket_position = suffix.find("[")
    bang_position = suffix.find("!")
    if bracket_position != -1:
        end_position = bracket_position
    elif bang_position != -1:
        end_position = bang_position
    else:
        end_position = len(suffix)

    return suffix[:end_position].rstrip().upper() == request.upper()


def warn_too_many_variables(audit: TextIO | None, limit: int = NUM_ALLOWED) -> None:
    message = f"too many variables requested, will go with first {limit}"
    display_string(message)
    audit_write(audit, message)


def selected_with_limit(
    selected: list[SelectedVariable],
    limited: bool,
    audit: TextIO | None,
) -> list[SelectedVariable]:
    if limited and len(selected) > NUM_ALLOWED:
        warn_too_many_variables(audit)
        return selected[:NUM_ALLOWED]

    if not limited and len(selected) > UNLIMITED_WARNING_COUNT:
        messages = [
            "potentially too many variables requested.  program may crash.",
            f" number requested={len(selected)}",
            f" program has been tested through max={UNLIMITED_WARNING_COUNT}",
        ]
        for message in messages:
            display_string(message)
            audit_write(audit, message)

    return selected


def stage_named_variables(
    records: list[DictionaryRecord],
    requests: Requests,
    numeric_track_numbers: set[int],
) -> list[DictionaryRecord]:
    staged: list[DictionaryRecord] = []
    staged_numbers: set[int] = set()

    for exact in (True, False):
        for record in records:
            for index, request in enumerate(requests.find_variables):
                if not request:
                    continue
                if not exact and requests.find_variable_processed[index] > 0:
                    continue
                if not request_matches_dictionary_line(record.line, request, exact):
                    continue

                if exact:
                    requests.find_variable_processed[index] += 1

                if record.number in numeric_track_numbers or record.number in staged_numbers:
                    break

                if not exact:
                    requests.find_variable_processed[index] -= 1

                staged.append(record)
                staged_numbers.add(record.number)
                break

    return staged


def append_selected(
    selected: list[SelectedVariable],
    record: DictionaryRecord,
) -> None:
    selected.append(SelectedVariable(record.number, record.label, True))


def select_variables(
    records: list[DictionaryRecord],
    requests: Requests,
    get_vars_from_eso: bool,
    frequency: int,
    limited: bool,
    audit: TextIO | None,
) -> list[SelectedVariable]:
    allowed_records = [
        record
        for record in records
        if is_allowed_frequency(record.line, frequency) and not is_ignored(record, requests)
    ]

    if get_vars_from_eso:
        selected = [SelectedVariable(record.number, record.label, True) for record in allowed_records]
        return selected_with_limit(selected, limited, audit)

    selected = [SelectedVariable(number, "", False) for number in requests.track_numbers]

    for record in allowed_records:
        for variable in selected:
            if variable.number == record.number:
                variable.label = record.label
                variable.found = True

    staged = stage_named_variables(allowed_records, requests, set(requests.track_numbers))
    remaining = staged[:]

    for request in requests.find_variables:
        if "," not in request:
            continue
        for record in remaining[:]:
            if myindex(record.label, request.strip()) != -1:
                append_selected(selected, record)
                remaining.remove(record)

    for request in requests.find_variables:
        comparable_request = request.strip()
        comma_position = comparable_request.find(",")
        if comma_position != -1:
            comparable_request = comparable_request[:comma_position] + ":" + comparable_request[comma_position + 1 :]

        for record in remaining[:]:
            if myindex(record.label, comparable_request) != -1:
                append_selected(selected, record)
                remaining.remove(record)

    return selected_with_limit(selected, limited, audit)


def record_matches_search(record: DictionaryRecord, search_text: str | None) -> bool:
    if not search_text:
        return True

    haystack = " ".join(
        [
            record.key,
            record.variable,
            record.units,
            record.frequency,
            record.label,
            record.line,
        ]
    ).upper()
    return search_text.upper() in haystack


def is_dictionary_time_stamp_record(record: DictionaryRecord) -> bool:
    raw_frequency = record.raw_frequency.strip().upper()
    return raw_frequency.startswith("WHEN ") and raw_frequency.endswith("REQUESTED")


def records_for_modern_cli(input_file: str) -> list[DictionaryRecord]:
    input_path = Path(input_file)
    if not input_path.is_file():
        raise ReadVarsFatal([f"Input file does not exist: {input_file}"])

    records, _ = dictionary_records(read_lines(input_path), None)
    return [record for record in records if not is_dictionary_time_stamp_record(record)]


def filter_modern_records(
    records: list[DictionaryRecord],
    frequency: str | None,
    search: str | None,
) -> list[DictionaryRecord]:
    filtered = [record for record in records if not is_dictionary_time_stamp_record(record)]

    if frequency:
        filtered = [record for record in filtered if record_matches_modern_frequency(record, frequency)]

    return [record for record in filtered if record_matches_search(record, search)]


def default_modern_output_file(input_file: str) -> Path:
    return Path(input_file).with_suffix(".csv")


def convert_modern_read(
    input_file: str,
    output_file: str | None,
    frequency: str | None,
    search: str | None,
) -> Path:
    input_path = Path(input_file)
    if not input_path.is_file():
        raise ReadVarsFatal([f"Input file does not exist: {input_file}"])

    output_path = Path(output_file) if output_file else default_modern_output_file(input_file)
    eso_lines = read_lines(input_path)
    records, dictionary_end_index = dictionary_records(eso_lines, None)
    selected = [
        SelectedVariable(record.number, record.label, True)
        for record in filter_modern_records(records, frequency, search)
    ]

    try:
        with output_path.open("w", encoding="utf-8", newline="") as output:
            write_header(output, selected, ",", True, None)
            process_data_records(
                eso_lines,
                dictionary_end_index + 2,
                selected,
                output,
                str(output_path),
                ",",
                None,
                legacy_spacing=False,
            )
    except OSError as err:
        raise ReadVarsFatal(
            [
                f"Output file cannot be opened: {output_path}",
                str(err),
            ]
        ) from err

    return output_path


def truncate_for_table(value: object, width: int) -> str:
    text = str(value)
    if len(text) <= width:
        return text
    if width <= 3:
        return text[:width]
    return text[: width - 3] + "..."


def print_table(rows: list[dict[str, object]], columns: list[tuple[str, str, int]], empty_message: str) -> None:
    if not rows:
        print(empty_message)
        return

    widths = []
    for header, key, maximum_width in columns:
        content_width = max(len(str(row.get(key, ""))) for row in rows)
        widths.append(min(max(len(header), content_width), maximum_width))

    header_line = "  ".join(header.ljust(widths[index]) for index, (header, _, _) in enumerate(columns))
    divider_line = "  ".join(("-" * widths[index]) for index in range(len(columns)))
    print(header_line)
    print(divider_line)

    for row in rows:
        values = []
        for index, (_, key, _) in enumerate(columns):
            values.append(truncate_for_table(row.get(key, ""), widths[index]).ljust(widths[index]))
        print("  ".join(values))


def dictionary_record_as_row(record: DictionaryRecord) -> dict[str, object]:
    return {
        "number": record.number,
        "frequency": record.frequency,
        "key": record.key,
        "variable": record.variable,
        "units": record.units,
        "label": record.label,
    }


def write_records(records: list[DictionaryRecord], output_format: str) -> None:
    rows = [dictionary_record_as_row(record) for record in records]

    if output_format == "json":
        print(json.dumps(rows, indent=2))
        return

    if output_format == "csv":
        writer = csv.DictWriter(
            sys.stdout,
            fieldnames=["number", "frequency", "key", "variable", "units", "label"],
            lineterminator="\n",
        )
        writer.writeheader()
        writer.writerows(rows)
        return

    print_table(
        rows,
        [
            ("Report #", "number", 8),
            ("Frequency", "frequency", 12),
            ("Key", "key", 36),
            ("Variable", "variable", 70),
            ("Units", "units", 24),
        ],
        "No variables matched.",
    )


def run_modern_cli(argv: list[str]) -> int:
    parser = build_modern_parser()
    args = parser.parse_args(normalized_modern_args(argv))
    if args.command is None:
        parser.print_help()
        return 0

    try:
        if args.command in {"list", "variables"}:
            records = filter_modern_records(records_for_modern_cli(args.input_file), args.frequency, args.search)
            write_records(records, args.format)
            return 0

        if args.command == "read":
            output_path = convert_modern_read(args.input_file, args.output, args.frequency, args.search)
            print(f"Wrote {output_path}")
            return 0
    except ReadVarsFatal as err:
        for message in err.messages:
            print(message, file=sys.stderr)
        return err.exit_code

    parser.print_help()
    return 1


def write_header(output: TextIO, selected: list[SelectedVariable], separator: str, fix_header: bool, audit: TextIO | None) -> None:
    header_parts = ["Date/Time"]
    for variable in selected:
        if variable.found:
            header_parts.append(variable.label.strip())
        else:
            message = f"line 904 variable ={variable.number} not found"
            display_string(message)
            audit_write(audit, message)

    output.write(separator.join(header_parts))
    output.write("\n" if fix_header else " \n")


def split_numeric_fields(line: str, count: int) -> list[float]:
    fields = line.split(",")
    if len(fields) < count:
        raise ValueError
    return [float(fields[index].strip()) for index in range(count)]


def format_time_stamp(month: int, day: int, hour_of_day: int, start_minute: float, end_minute: float) -> str:
    current_hour = hour_of_day - 1
    current_minute = int(end_minute)
    current_second = int((end_minute - current_minute) * 60.0)
    if end_minute == 60.0:
        current_hour = hour_of_day
        current_minute = 0
        if start_minute == 0.0:
            current_second = 0
    return f" {month:02d}/{day:02d}  {current_hour:02d}:{current_minute:02d}:{current_second:02d}"


def format_month_day(month: int, day: int) -> str:
    return f" {month:02d}/{day:02d}"


def flush_row(
    output: TextIO,
    label: str,
    out_data: list[str],
    out_found: list[bool],
    separator: str,
    legacy_spacing: bool = True,
) -> None:
    if not any(out_found):
        return

    row_parts = [label.rstrip()]
    for index, found in enumerate(out_found):
        row_parts.append(out_data[index].rstrip() if found else "")
    output.write(separator.join(row_parts))
    output.write(" \n" if legacy_spacing else "\n")

    for index in range(len(out_data)):
        out_data[index] = ""
        out_found[index] = False


def process_data_records(
    eso_lines: list[str],
    data_start_index: int,
    selected: list[SelectedVariable],
    output: TextIO,
    output_file_name: str,
    separator: str,
    audit: TextIO | None,
    legacy_spacing: bool = True,
) -> None:
    out_data = [""] * len(selected)
    out_found = [False] * len(selected)
    track_index = {variable.number: index for index, variable in enumerate(selected)}

    no_details = True
    no_month_day = True
    no_month = True
    current_date = ""
    current_month_day = ""
    current_month = ""
    current_period = ""
    previous_hour_of_day: int | None = None
    previous_end_minute: float | None = None

    for line in eso_lines[data_start_index:]:
        if line.strip() == "End of Data":
            if not no_details:
                label = current_date
            elif not no_month_day:
                label = current_month_day
            elif not no_month:
                label = current_month
            else:
                label = current_period
            flush_row(output, label, out_data, out_found, separator, legacy_spacing)
            return

        if line.strip() == "":
            fatal(
                audit,
                [
                    f"Output file={output_file_name}",
                    "error occurred during processing.",
                    "Blank line in middle of processing.",
                    "Likely fatal error during EnergyPlus execution.",
                    "ReadVarsESO program terminated.",
                ],
            )

        comma_position = line.find(",")
        if comma_position == -1:
            processing_error(output_file_name, line, audit)

        try:
            line_number = int(float(line[:comma_position].strip()))
        except ValueError:
            processing_error(output_file_name, line, audit)

        if line_number == 1:
            continue

        if line_number == 2:
            no_details = False
            try:
                fields = split_numeric_fields(line, 8)
            except ValueError:
                processing_error(output_file_name, line, audit)

            month = int(fields[2])
            day = int(fields[3])
            hour_of_day = int(fields[5])
            start_minute = fields[6]
            end_minute = fields[7]

            if current_date and (hour_of_day != previous_hour_of_day or end_minute != previous_end_minute):
                flush_row(output, current_date, out_data, out_found, separator, legacy_spacing)

            previous_hour_of_day = hour_of_day
            previous_end_minute = end_minute
            current_date = format_time_stamp(month, day, hour_of_day, start_minute, end_minute)
            continue

        if line_number == 3:
            if no_details:
                no_month_day = False
                try:
                    fields = split_numeric_fields(line, 5)
                except ValueError:
                    processing_error(output_file_name, line, audit)

                if current_month_day:
                    flush_row(output, current_month_day, out_data, out_found, separator, legacy_spacing)

                month = int(fields[2])
                day = int(fields[3])
                current_month_day = format_month_day(month, day)
            continue

        if line_number == 4:
            if no_details and no_month_day:
                no_month = False
                try:
                    fields = split_numeric_fields(line, 3)
                except ValueError:
                    processing_error(output_file_name, line, audit)

                if current_month:
                    flush_row(output, current_month, out_data, out_found, separator, legacy_spacing)

                month_index = int(fields[2]) - 1
                if month_index < 0 or month_index >= len(MONTHS):
                    processing_error(output_file_name, line, audit)
                current_month = MONTHS[month_index]
            continue

        if line_number == 5:
            if no_details and no_month_day and no_month:
                try:
                    fields = split_numeric_fields(line, 2)
                except ValueError:
                    processing_error(output_file_name, line, audit)

                if current_period:
                    flush_row(output, current_period, out_data, out_found, separator, legacy_spacing)

                current_period = f"simdays={int(fields[1])}"
            continue

        selected_index = track_index.get(line_number)
        if selected_index is None:
            continue

        value_start = comma_position + 1
        value_remainder = line[value_start:]
        next_comma_position = value_remainder.find(",")
        if next_comma_position == -1:
            value = value_remainder
        else:
            value = value_remainder[:next_comma_position]

        out_data[selected_index] = value.rstrip()[:MAX_OUTPUT_VALUE_LENGTH]
        out_found[selected_index] = True

    fatal(
        audit,
        [
            "EOF encountered on eplusout.eso while reading data",
            "probable EnergyPlus error condition -- check eplusout.err",
            "ReadVarsESO program terminated.",
        ],
    )


def processing_error(output_file_name: str, line: str, audit: TextIO | None) -> None:
    fatal(
        audit,
        [
            f"Output file={output_file_name}",
            "error occurred during processing.",
            "Apparent line in error (1st 50 characters):",
            line[:50].rstrip(),
            "ReadVarsESO program terminated.",
        ],
    )


def elapsed_string(start_time: float) -> str:
    elapsed = time.process_time() - start_time
    hours = int(elapsed // 3600)
    elapsed -= hours * 3600
    minutes = int(elapsed // 60)
    seconds = elapsed - minutes * 60
    return f"{hours:02d}hr {minutes:02d}min {seconds:5.2f}sec"


def run(argv: list[str]) -> int:
    if is_modern_cli(argv):
        return run_modern_cli(argv)

    display_string("ReadVarsESO program starting.")
    start_time = time.process_time()
    audit: TextIO | None = None
    output: TextIO | None = None
    errors_happened = False

    try:
        audit = Path("readvars.audit").open("a", encoding="utf-8")
        audit_write(audit, "ReadVarsESO")

        options = parse_options(argv)
        input_file_name, output_file_name, separator, get_vars_from_eso, rvi_lines = read_rvi_configuration(options, audit)

        try:
            output = Path(output_file_name).open("w", encoding="utf-8", newline="")
        except OSError:
            fatal(
                audit,
                [
                    f"Output file={output_file_name}",
                    "cannot be opened.  It may be open in another program.",
                    "Please close it and try again.",
                    "ReadVarsESO program terminated.",
                ],
            )
        audit_write(audit, f"output file:{output_file_name}")

        requests = Requests([], [], [], [], [])
        if not get_vars_from_eso:
            requests = parse_rvi_variable_requests(rvi_lines, audit)
            if (
                not requests.track_numbers
                and not requests.find_variables
                and not requests.ignore_numbers
                and not requests.ignore_find_variables
            ):
                display_string("You chose no variables")
                audit_write(audit, "You chose no variables")
                raise EarlyExit
            if not requests.track_numbers and not requests.find_variables:
                get_vars_from_eso = True

        if get_vars_from_eso:
            audit_write(audit, f"getting all vars from:{input_file_name}")

        eso_lines = read_lines(Path(input_file_name))
        records, dictionary_end_index = dictionary_records(eso_lines, audit)
        selected = select_variables(records, requests, get_vars_from_eso, options.frequency, options.limited, audit)
        audit_write(audit, f" number variables requested for output={len(selected)}")

        write_header(output, selected, separator, options.fix_header, audit)
        data_start_index = dictionary_end_index + 2
        process_data_records(eso_lines, data_start_index, selected, output, output_file_name, separator, audit)

    except EarlyExit:
        pass
    except ReadVarsFatal as err:
        errors_happened = True
        return err.exit_code
    finally:
        if output is not None:
            output.close()

        if audit is not None:
            runtime_message = f"ReadVars Run Time={elapsed_string(start_time)}"
            display_string(runtime_message)
            audit_write(audit, runtime_message)
            if not errors_happened:
                display_string("ReadVarsESO program completed successfully.")
                audit_write(audit, "ReadVarsESO program completed successfully.")
            audit.close()

    return 0


def main() -> None:
    sys.exit(run(sys.argv[1:]))


if __name__ == "__main__":
    main()
