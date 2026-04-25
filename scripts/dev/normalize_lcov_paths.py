#!/usr/bin/env python3

import argparse
import sys
from pathlib import Path


def build_source_index(workspace: Path) -> dict[str, list[Path]]:
    source_index: dict[str, list[Path]] = {}
    source_root = workspace / "src" / "EnergyPlus"
    for source_file in source_root.rglob("*"):
        if source_file.is_file():
            relative_path = source_file.relative_to(workspace)
            source_index.setdefault(source_file.name, []).append(relative_path)
    return source_index


def repo_relative_source(
    source_file: str,
    workspace: Path,
    build_directory: Path | None,
    source_index: dict[str, list[Path]],
) -> Path | None:
    source_path = Path(source_file)
    candidates: list[Path] = []

    if source_path.is_absolute():
        candidates.append(source_path)
    else:
        candidates.append(workspace / source_path)
        if build_directory is not None:
            candidates.append(build_directory / source_path)

    for candidate in candidates:
        try:
            relative_path = candidate.resolve().relative_to(workspace.resolve())
        except ValueError:
            continue

        parts = relative_path.parts
        for index in range(len(parts) - 1):
            if parts[index] == "src" and parts[index + 1] == "EnergyPlus":
                relative_path = Path(*parts[index:])
                break

        if not relative_path.as_posix().startswith("src/EnergyPlus/"):
            continue

        if (workspace / relative_path).is_file():
            return relative_path

    basename_matches = source_index.get(source_path.name, [])
    if len(basename_matches) == 1:
        return basename_matches[0]

    return None


def flush_record(
    record_lines: list[str],
    relative_source: Path | None,
    workspace: Path,
    absolute_records: list[str],
    relative_records: list[str],
) -> int:
    if relative_source is None:
        return 0

    da_lines = sum(1 for line in record_lines if line.startswith("DA:"))
    if da_lines == 0:
        return 0

    absolute_source = (workspace / relative_source).as_posix()
    relative_source_text = relative_source.as_posix()

    for line in record_lines:
        if line.startswith("SF:"):
            absolute_records.append(f"SF:{absolute_source}")
            relative_records.append(f"SF:{relative_source_text}")
        else:
            absolute_records.append(line)
            relative_records.append(line)

    return da_lines


def normalize_lcov_paths(
    input_file: Path,
    workspace: Path,
    build_directory: Path | None,
    absolute_output: Path,
    relative_output: Path,
) -> int:
    source_index = build_source_index(workspace)
    absolute_records: list[str] = []
    relative_records: list[str] = []
    record_lines: list[str] = []
    relative_source: Path | None = None
    da_lines = 0

    with input_file.open("r", encoding="utf-8") as input_stream:
        for raw_line in input_stream:
            line = raw_line.rstrip("\n")
            if line.startswith("SF:"):
                relative_source = repo_relative_source(line[3:], workspace, build_directory, source_index)
                record_lines.append(line)
            elif line == "end_of_record":
                record_lines.append(line)
                da_lines += flush_record(record_lines, relative_source, workspace, absolute_records, relative_records)
                record_lines = []
                relative_source = None
            else:
                record_lines.append(line)

    absolute_output.parent.mkdir(parents=True, exist_ok=True)
    relative_output.parent.mkdir(parents=True, exist_ok=True)
    absolute_output.write_text("\n".join(absolute_records) + ("\n" if absolute_records else ""), encoding="utf-8")
    relative_output.write_text("\n".join(relative_records) + ("\n" if relative_records else ""), encoding="utf-8")

    return da_lines


def main() -> int:
    parser = argparse.ArgumentParser(description="Filter LCOV records to real repo source files and normalize SF paths.")
    parser.add_argument("input_file", type=Path)
    parser.add_argument("--workspace", type=Path, required=True)
    parser.add_argument("--build-directory", type=Path)
    parser.add_argument("--absolute-output", type=Path, required=True)
    parser.add_argument("--relative-output", type=Path, required=True)
    args = parser.parse_args()

    da_lines = normalize_lcov_paths(
        input_file=args.input_file,
        workspace=args.workspace,
        build_directory=args.build_directory,
        absolute_output=args.absolute_output,
        relative_output=args.relative_output,
    )

    if da_lines == 0:
        print("No LCOV data records matched real files under src/EnergyPlus", file=sys.stderr)
        return 1

    print(f"Wrote normalized LCOV data with {da_lines} DA records")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
