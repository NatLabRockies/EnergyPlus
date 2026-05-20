#!/usr/bin/env python3

from pathlib import Path
import json
import shutil
import subprocess
import sys


SCRIPT = Path(__file__).with_name("ReadVarsESO.py")
TEST_ESO = Path(__file__).parent / "testdata" / "readvars_discovery.eso"


def run_readvars(args, cwd=None):
    return subprocess.run(
        [sys.executable, str(SCRIPT)] + args,
        cwd=cwd,
        check=True,
        capture_output=True,
        text=True,
    )


def test_list_command():
    result = run_readvars(
        [
            "list",
            str(TEST_ESO),
            "--frequency",
            "hourly",
            "--search",
            "temperature",
            "--format",
            "json",
        ]
    )
    rows = json.loads(result.stdout)
    assert len(rows) == 1
    assert rows[0]["number"] == 8
    assert rows[0]["frequency"] == "Hourly"
    assert rows[0]["key"] == "ZONE ONE"
    assert rows[0]["variable"] == "Zone Mean Air Temperature"
    assert rows[0]["units"] == "C"


def test_list_command_filters_time_stamp_records():
    result = run_readvars(["list", str(TEST_ESO), "--format", "json"])
    rows = json.loads(result.stdout)
    assert [row["number"] for row in rows] == [7, 8, 9]
    assert all("When Annual Report Variables Requested" not in row["frequency"] for row in rows)
    assert all(row["variable"] != "Calendar Year of Simulation" for row in rows)


def test_read_command_converts_everything_by_default():
    run_dir = Path.cwd() / "readvars_read_all_runtime"
    if run_dir.exists():
        shutil.rmtree(run_dir)
    run_dir.mkdir()
    try:
        shutil.copy(TEST_ESO, run_dir / "eplusout.eso")

        run_readvars(["read", "eplusout.eso"], cwd=run_dir)
        output = (run_dir / "eplusout.csv").read_text(encoding="utf-8")
        assert output.splitlines() == [
            "Date/Time,Environment:Outdoor Dry Bulb [C](Hourly),ZONE ONE:Zone Mean Air Temperature [C](Hourly),ZONE ONE:Zone Air System Sensible Heating Rate [W](TimeStep)",
            " 01/01  01:00:00,-5.0,20.0,",
            " 01/01  02:00:00,-4.0,21.0,",
        ]
        assert not (run_dir / "readvars.audit").exists()
    finally:
        shutil.rmtree(run_dir, ignore_errors=True)


def test_read_command_supports_output_and_filters():
    run_dir = Path.cwd() / "readvars_read_filtered_runtime"
    if run_dir.exists():
        shutil.rmtree(run_dir)
    run_dir.mkdir()
    try:
        shutil.copy(TEST_ESO, run_dir / "eplusout.eso")

        run_readvars(
            [
                "read",
                "eplusout.eso",
                "--output",
                "selected.csv",
                "--frequency",
                "hourly",
                "--search",
                "temperature",
            ],
            cwd=run_dir,
        )
        output = (run_dir / "selected.csv").read_text(encoding="utf-8")
        assert output.splitlines() == [
            "Date/Time,ZONE ONE:Zone Mean Air Temperature [C](Hourly)",
            " 01/01  01:00:00,20.0",
            " 01/01  02:00:00,21.0",
        ]
    finally:
        shutil.rmtree(run_dir, ignore_errors=True)


def test_legacy_conversion_still_works():
    run_dir = Path.cwd() / "readvars_cli_test_runtime"
    if run_dir.exists():
        shutil.rmtree(run_dir)
    run_dir.mkdir()
    try:
        shutil.copy(TEST_ESO, run_dir / "eplusout.eso")
        (run_dir / "custom.rvi").write_text(
            "eplusout.eso\n"
            "custom.csv\n"
            "ZONE ONE,Zone Mean Air Temperature\n",
            encoding="utf-8",
        )

        run_readvars(["custom.rvi", "hourly", "fixheader"], cwd=run_dir)
        output = (run_dir / "custom.csv").read_text(encoding="utf-8")
        assert output.splitlines() == [
            "Date/Time,ZONE ONE:Zone Mean Air Temperature [C](Hourly)",
            " 01/01  01:00:00,20.0 ",
            " 01/01  02:00:00,21.0 ",
        ]
    finally:
        shutil.rmtree(run_dir, ignore_errors=True)


if __name__ == "__main__":
    test_list_command()
    test_list_command_filters_time_stamp_records()
    test_read_command_converts_everything_by_default()
    test_read_command_supports_output_and_filters()
    test_legacy_conversion_still_works()
