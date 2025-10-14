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

import argparse
import json
import os
from concurrent.futures import Executor, ProcessPoolExecutor, as_completed
from enum import StrEnum
from functools import partial
from pathlib import Path
from typing import Any, Callable, Iterable, Iterator, Sequence, TypedDict

ROOT_DIR = Path(__file__).parent.parent.parent
TESTFILES_DIR = ROOT_DIR / "testfiles"
IDD_PATH = ROOT_DIR / "idd/Energy+.idd.in"
SRC_DIR = ROOT_DIR / "src/EnergyPlus"
TST_DIR = ROOT_DIR / "tst/EnergyPlus"


class LogLevel(StrEnum):
    """Enum for error types."""

    ERROR = "Error"
    WARNING = "Warning"
    INFO = "Info"

    def to_int(self):
        order = {LogLevel.ERROR: 3, LogLevel.WARNING: 2, LogLevel.INFO: 1}
        return order[self]

    def to_gha(self):
        """Convert to GitHub Actions annotation level."""
        if self == LogLevel.ERROR:
            return "error"
        elif self == LogLevel.WARNING:
            return "warning"
        elif self == LogLevel.INFO:
            return "notice"

    def __lt__(self, other):
        """Define less-than for ordering."""
        if not isinstance(other, LogLevel):
            return NotImplemented
        return self.to_int() < other.to_int()

    def __le__(self, other):
        """Define less-than for ordering."""
        if not isinstance(other, LogLevel):
            return NotImplemented
        return self.to_int() <= other.to_int()

    def __gt__(self, other):
        """Define less-than for ordering."""
        if not isinstance(other, LogLevel):
            return NotImplemented
        return self.to_int() > other.to_int()

    def __ge__(self, other):
        """Define less-than for ordering."""
        if not isinstance(other, LogLevel):
            return NotImplemented
        return self.to_int() >= other.to_int()

    def __eq__(self, other):
        """Define equality for ordering."""
        if not isinstance(other, LogLevel):
            return NotImplemented
        return self.to_int() == other.to_int()

    def __hash__(self):
        """Define hash for using as dict key."""
        # If need this one because I overrode __eq__
        return hash(self.value)


class ErrorDictionary(TypedDict, total=False):
    """Type hint for error dictionaries."""

    tool: str
    filename: str
    file: str
    line: int
    messagetype: LogLevel
    message: str


class LogMessage:

    def __init__(
        self,
        tool: str,
        filepath: Path,
        loglevel: LogLevel,
        message: str,
        line_number: int | None = None,
        line: str | None = None,
    ):
        self.tool = tool
        self.filepath = filepath
        self.relative_file_path = relative_path_from_root(filepath)
        self.loglevel = loglevel
        self.message = message
        self.line_number = line_number
        self.line = line

    def __repr__(self):
        """Get an unambiguous string representation of the error."""
        return (
            f"Error(tool={self.tool!r}, filepath={self.relative_file_path.as_posix()!r}, "
            f"loglevel={self.loglevel.value!r}, message={self.message!r}, "
            f"line_number={self.line_number!r}, line={self.line!r})"
        )

    def __str__(self):
        """Get a human-readable string representation of the error."""

        msg = f"{self.loglevel} - {self.tool}: {self.message}\n"
        msg += f"{self.relative_file_path}"
        if self.line_number is not None:
            msg += f"#L{self.line_number}"
        if self.line is not None:
            msg += f" - {self.line}"

        return msg

    def to_dict(self, include_file_name: bool = True):
        """Get a dictionary representation of the error."""
        err_dict = {
            "tool": self.tool,
            "loglevel": self.loglevel,
            "filename": self.filepath.name,
            "file": str(self.relative_file_path),
            "line": self.line_number,
            "message": self.message,
        }
        if self.line_number is None:
            err_dict.pop("line")
        if not include_file_name:
            err_dict.pop("filename")
        return err_dict

    def to_json(self):
        """Get a JSON-serializable dictionary representation of the error."""
        return json.dumps(self.to_dict())

    def to_github_annotation(self):
        """Get a GitHub annotation string representation of the error."""

        # Encode newlines for GitHub Actions
        safe_message = self.message
        if self.line is not None:
            safe_message += "\n" + self.line
        safe_message = safe_message.replace("\n", "%0A").replace(":", "%3A")

        # Ensure line number fallback
        line_number = self.line_number or 1

        msg = (
            f"::{self.loglevel.to_gha()} "
            f"file={self.relative_file_path},"
            f"line={line_number},"
            f"title={self.tool}::{safe_message}"
        )

        return msg


ErrorMessage = partial(LogMessage, loglevel=LogLevel.ERROR)
WarningMessage = partial(LogMessage, loglevel=LogLevel.WARNING)
InfoMessage = partial(LogMessage, loglevel=LogLevel.INFO)


def flatten_list_of_lists(list_of_lists: Sequence[Sequence[Any] | None]) -> list[Any]:
    """Flatten a list of lists into a single list."""
    return [item for sublist in list_of_lists if sublist for item in sublist]


def is_github_actions() -> bool:
    """Check if the script is running in a GitHub Actions environment."""
    return "GITHUB_ACTIONS" in os.environ


def groupby_log_level(log_messages: Sequence[LogMessage | None]) -> dict[LogLevel, list[LogMessage]]:
    """Convert a LogMessage to an ErrorDictionary.

    Args:
        log_messages: List of LogMessage objects to group.

    Returns:
        A dictionary grouping LogMessage objects by their LogLevel, ordered by greater to lower severity
    """
    error_dict: dict[LogLevel, list[LogMessage]] = {}
    for item in log_messages:
        if item is None:
            continue
        if item.loglevel not in error_dict:
            error_dict[item.loglevel] = []

        error_dict[item.loglevel].append(item)
    return {lvl: error_dict[lvl] for lvl in LogLevel if lvl in error_dict}


def _make_github_step_summary_str(error_dict: dict[LogLevel, list[LogMessage]]) -> str:
    """Create a GitHub Actions step summary file with the log messages.

    Args:
        error_dict: A dictionary grouping LogMessage objects by their LogLevel.
    """
    try:
        import tabulate
    except ImportError:
        print("tabulate not installed, cannot create GitHub step summary")
        return ""

    sorted_log_messages = [log_msg for log_msgs in error_dict.values() for log_msg in log_msgs]
    table_data = [log_msg.to_dict(include_file_name=False) for log_msg in sorted_log_messages]
    table_md = tabulate.tabulate(table_data, headers="keys", tablefmt="github")
    checks = ", ".join(set([log_msg.tool for log_msg in sorted_log_messages]))

    summary_parts = [f"{len(msgs)} {lvl.value}(s)" for lvl, msgs in error_dict.items()]
    summary = f"We found {', '.join(summary_parts)}."

    table_str = f"""
## Check: {checks}

**{summary}**

<detail>

<summary>Check failures</summary>

{table_md}

</details>

"""

    return table_str


def add_github_step_summary_str(error_dict: dict[LogLevel, list[LogMessage]]) -> None:
    """Create a GitHub Actions step summary file with the log messages.

    Args:
        error_dict: A dictionary grouping LogMessage objects by their LogLevel.
    """
    summary_path = os.environ.get("GITHUB_STEP_SUMMARY")
    if not summary_path:
        return
    summary_str = _make_github_step_summary_str(error_dict)
    if not summary_str:
        return
    with Path(summary_path).open("a", encoding="utf-8") as f:
        f.write(summary_str)


def report_log_messages(
    log_messages: Sequence[LogMessage | None], fail_threshold: LogLevel = LogLevel.ERROR, verbose: bool = False
) -> bool:
    """Report log messages

    Args:
        log_messages: List of LogMessage objects to report.
        fail_threshold: The log level at which we start reporting failure (>= this level is a failure).
        verbose: Whether to print a summary even if no issues are found.

    Returns:
        The number of errors reported
    """

    result = True
    if not log_messages:
        if verbose:
            print("No issues found.")
        return result

    # Minimum log level to print
    minimum_log_level = LogLevel.INFO if verbose else LogLevel.WARNING

    # This already sorts and removes None
    error_dict = groupby_log_level(log_messages=log_messages)
    if not error_dict:
        print("No issues found")
        return result

    for logLevel, msgs in error_dict.items():
        if logLevel >= fail_threshold:
            result = False
        if logLevel < minimum_log_level:
            continue
        for msg in msgs:
            print(msg.to_json())
            if is_github_actions():
                print(msg.to_github_annotation())

    summary_parts = [f"{len(msgs)} {lvl.value}(s)" for lvl, msgs in error_dict.items()]
    summary = f"{'Success' if result else 'Fail'}: We found {', '.join(summary_parts)}."
    print(summary)
    if result:
        if verbose:
            print("No issues found.")
        return result

    if is_github_actions():
        add_github_step_summary_str(error_dict=error_dict)

    return result


def exit_hook(success: bool) -> None:
    """Exit the script with appropriate status code."""
    raise SystemExit(0 if success else 1)


def relative_path_from_root(path: Path, root=ROOT_DIR) -> Path:
    """Get the path relative to the root directory."""
    if not path.is_absolute():
        path = path.resolve()
    if not path.is_relative_to(root):
        raise ValueError(f"Path '{path}' is not under the root directory '{root}'")
    return path.relative_to(root)


def _walk_with_exclusion(
    base_dir: Path, dirs_to_skip: Sequence[str], extensions: Iterable[str] | None = None
) -> Iterator[Path]:
    """Walk a directory tree, yielding files with given extensions, skipping specified directory names.

    Args:
        base_dir (Path): The base directory to start the walk.
        dirs_to_skip (Sequence[str]): Directory names to skip.
        extensions (Sequence[str] | None): File extensions to include. If None, include all files.

    Yields: Path objects for files matching the criteria.
    """
    base_dir = base_dir.resolve()
    for root_path, dirnames, filenames in base_dir.walk(top_down=True):
        # Slice assignment to exclude dirs in-place so they don't get walked into
        dirnames[:] = [d for d in dirnames if d not in dirs_to_skip]
        for filename in filenames:
            filepath = root_path / filename
            # Using endswith to allow for multiple extensions such as .cc.in or tar.gz
            if extensions is None or any(filename.endswith(ext) for ext in extensions):
                yield filepath


def glob_with_extension(base_dir: Path, extensions: Iterable[str]) -> Iterator[Path]:
    """Yield files in base_dir matching the given extensions."""
    for ext in extensions:
        if not ext.startswith("."):
            raise ValueError(f"Extension '{ext}' must start with a dot.")
        yield from base_dir.glob(f"*{ext}")


def collect_files(
    base_dir: Path,
    extensions: Iterable[str] | None = None,
    recursive: bool = True,
    dirs_to_skip: Sequence[str] | None = None,
) -> Iterator[Path]:
    """Collect files with given extensions from the base directory and its subdirectories."""
    if dirs_to_skip and not recursive:
        raise ValueError("dirs_to_skip can only be used with recursive=True")
    if recursive:
        yield from _walk_with_exclusion(base_dir=base_dir, dirs_to_skip=dirs_to_skip or (), extensions=extensions)

    else:
        # Non-recursive case — yield files directly from glob
        if extensions is None:
            # Yield all files
            yield from base_dir.glob("*")
        else:
            yield from glob_with_extension(base_dir=base_dir, extensions=extensions)


def argparse_type_valid_absolute_file(path_str: str) -> Path:
    """Argparse type to check for a valid file and convert to absolute path."""
    path = Path(path_str).resolve()
    if not path.is_file():
        raise argparse.ArgumentTypeError(f"'{path}' is not a valid file")
    return path


def get_base_parser(
    description: str, include_files_arg: bool = True, files_arg_help: str | None = None
) -> argparse.ArgumentParser:
    """Get the base parser for all scripts.

    This parser includes common arguments like `verbose` and `filenames` (nargs)
    """
    parser = argparse.ArgumentParser(description=description)
    if files_arg_help is None:
        files_arg_help = "Files to check (if omitted, checks whole repo)"
    if include_files_arg:
        parser.add_argument(
            "files",
            nargs="*",
            type=argparse_type_valid_absolute_file,
            help=files_arg_help,
        )
    parser.add_argument("-v", "--verbose", dest="verbose", action="store_true", default=False, help="operate verbosely")
    return parser


def parallel_apply(
    func: Callable[..., Any],
    filepaths: Sequence[Path],
    *args: Any,
    max_workers: int | None = None,
    pool_executor: type[Executor] = ProcessPoolExecutor,
    **kwargs: Any,
) -> list[Any]:
    """
    Apply a function to each Path in parallel, forwarding fixed arguments.

    Args:
        func: A callable that accepts a Path as its first argument.
        paths: Sequence of Path objects to process.
        *args: Fixed positional arguments to pass to func.
        max_workers: Optional max parallel workers (defaults to CPU count).
        **kwargs: Fixed keyword arguments to pass to func.

    Returns:
        A list of results, preserving the order of `paths`.
    """
    results: list[Any] = [None] * len(filepaths)
    with pool_executor(max_workers=max_workers) as executor:  # type: ignore[call-arg]
        future_to_index = {executor.submit(func, filepath, *args, **kwargs): i for i, filepath in enumerate(filepaths)}
        for future in as_completed(future_to_index):
            i = future_to_index[future]
            results[i] = future.result()
    return results
