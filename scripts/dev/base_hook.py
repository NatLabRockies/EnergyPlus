import argparse
import json
import os
from concurrent.futures import Executor, ProcessPoolExecutor, as_completed
from enum import StrEnum
from functools import partial, total_ordering
from pathlib import Path
from typing import Any, Callable, Iterator, Sequence, TypedDict

ROOT_DIR = Path(__file__).parent.parent.parent
TESTFILES_DIR = ROOT_DIR / "testfiles"
IDD_PATH = ROOT_DIR / "idd/Energy+.idd.in"
SRC_DIR = ROOT_DIR / "src/EnergyPlus"
TST_DIR = ROOT_DIR / "tst/EnergyPlus"


@total_ordering
class LogLevel(StrEnum):
    """Enum for error types."""

    ERROR = "Error"
    WARNING = "Warning"
    INFO = "Info"

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
        order = {LogLevel.ERROR: 3, LogLevel.WARNING: 2, LogLevel.INFO: 1}
        return order[self] < order[other]

    def __eq__(self, other):
        """Define equality for ordering."""
        if not isinstance(other, LogLevel):
            return NotImplemented
        return self.value == other.value

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


def flatten_list_of_lists(list_of_lists: list[list[Any]]) -> list[Any]:
    """Flatten a list of lists into a single list."""
    return [item for sublist in list_of_lists for item in sublist]


def is_github_actions() -> bool:
    """Check if the script is running in a GitHub Actions environment."""
    return "GITHUB_ACTIONS" in os.environ


def groupby_log_level(log_messages: list[LogMessage]) -> dict[LogLevel, list[LogMessage]]:
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
        return

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
    log_messages: list[LogMessage], fail_threshold: LogLevel = LogLevel.ERROR, verbose: bool = False
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

    # This already sorts and removes None
    error_dict = groupby_log_level(log_messages=log_messages)

    for logLevel, msgs in error_dict.items():
        if logLevel >= fail_threshold:
            result = False
        for msg in msgs:
            print(msg.to_json())
            if is_github_actions():
                print(msg.to_github_annotation())

    summary_parts = [f"{len(msgs)} {lvl.value}(s)" for lvl, msgs in error_dict.items()]
    summary = f"We found {', '.join(summary_parts)}."
    print(summary)

    if is_github_actions():
        add_github_step_summary_str(error_dict=error_dict)

    return result


def exit_hook(success: bool) -> None:
    """Exit the script with appropriate status code."""
    raise SystemExit(0 if success else 1)


ErrorMessage = partial(LogMessage, loglevel=LogLevel.ERROR)
WarningMessage = partial(LogMessage, loglevel=LogLevel.WARNING)
InfoMessage = partial(LogMessage, loglevel=LogLevel.INFO)

# Type alias for a callback function that takes an ErrorDictionary and returns None
CallBackErrorFunc = Callable[[ErrorDictionary], None]


def relative_path_from_root(path: Path, root=ROOT_DIR) -> Path:
    """Get the path relative to the root directory."""
    if not path.is_absolute():
        path = path.resolve()
    if not path.is_relative_to(root):
        raise ValueError(f"Path '{path}' is not under the root directory '{root}'")
    return path.relative_to(root)


def _walk_with_exclusion(
    base_dir: Path, dirs_to_skip: Sequence[str], extensions: Sequence[str] | None = None
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


def glob_with_extension(base_dir: Path, extensions: Sequence[str]) -> Iterator[Path]:
    """Yield files in base_dir matching the given extensions."""
    for ext in extensions:
        if not ext.startswith("."):
            raise ValueError(f"Extension '{ext}' must start with a dot.")
        yield from base_dir.glob(f"*{ext}")


def collect_files(
    base_dir: Path,
    extensions: Sequence[str] | None = None,
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


def get_base_parser(description: str, include_files_arg: bool = True) -> argparse.ArgumentParser:
    """Get the base parser for all scripts.

    This parser includes common arguments like `verbose` and `filenames` (nargs)
    """
    parser = argparse.ArgumentParser(description=description)
    if include_files_arg:
        parser.add_argument(
            "files",
            nargs="*",
            type=argparse_type_valid_absolute_file,
            help="Files to check (if omitted, checks whole repo)",
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
    with pool_executor(max_workers=max_workers) as executor:
        future_to_index = {executor.submit(func, filepath, *args, **kwargs): i for i, filepath in enumerate(filepaths)}
        for future in as_completed(future_to_index):
            i = future_to_index[future]
            results[i] = future.result()
    return results
