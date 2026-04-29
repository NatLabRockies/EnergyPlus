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
Parse the LaTeX log file.
"""
import copy
import json
import os
import os.path
import re
import sys

DEBUG = False
VERBOSE = False
EXIT_CODE_ISSUES_FOUND = 0  # set up to not return a non-zero exit code
EXIT_CODE_GOOD = 0


SEVERITY_WARNING = "WARNING"
SEVERITY_ERROR = "ERROR"


LINE_RE = re.compile(r".*\((.*?\.tex)")
TEX_ERROR = re.compile(r"^! (.*)$")
GENERAL_ERROR = re.compile(r"^(! )?(LaTeX|pdfTeX|Package|Class) ((.*) )?Error.*?:(.*)")
GENERAL_WARNING = re.compile(r"^(! )?(LaTeX|pdfTeX|Package|Class) ((.*) )?Warning.*?:(.*)$")
LATEX_WARNING = re.compile(r"^LaTeX Warning: (.*)$")
LABEL_MULTIPLY_DEFINED_WARN = re.compile(r"^ Label `([^']*)' multiply defined\.$")
HYPER_UNDEFINED_WARN = re.compile(r"^ Hyper reference `([^']*)' on page (\d+) " + r"undefined on input line (\d+)\.$")
LINE_NO = re.compile(r"l\.(\d+)")
ON_INPUT_LINE = re.compile(r"on input line (\d+)\.")
ISSUES_TO_SKIP = {
    "There were undefined references.",
    "There were multiply-defined labels.",
}


def parse_on_input_line(line):
    """
    - line: str, the line to parse
    RETURN: None or int
    """
    m = ON_INPUT_LINE.search(line)
    if m is not None:
        return int(m.group(1))
    return None


def parse_current_tex_file(line, root_dir):
    """
    Parse a single latex log line to find the current LaTeX file being
    processed if any.
    - line: str, the line to parse
    - root_dir: str, the root directory of the file if found (for
      normalization)
    RETURN: None or string
    """
    m = LINE_RE.search(line)
    if m is not None:
        pp_dir = root_dir
        pp_file = os.path.join(pp_dir, m.group(1))
        try:
            return os.path.relpath(pp_file, start=pp_dir)
        except Exception:
            # not a file under root_dir -- ignore
            return None
    return None


def parse_warning_start(line):
    """
    - line: str, line
    RETURN: None OR
    (Tuple str, str, str)
    - the warning type
    - warning type continued (usually for package)
    - the message
    """
    m = GENERAL_WARNING.match(line)
    if m is not None:
        return (m.group(2), m.group(4), m.group(5))
    return None


def parse_error_start(line):
    """
    - line: str, line
    RETURN: None OR
    (Tuple str, str, str)
    - the error type
    - error type continued (usually for package)
    - the message
    """
    m = GENERAL_ERROR.match(line)
    if m is not None:
        return (m.group(2), m.group(4), m.group(5))
    return None


def parse_tex_error(line):
    """
    Parse a single latex log line indicating an error/warning message
    beginning.
    - line: str, the line to parse
    RETURN: None or string
    """
    m = TEX_ERROR.match(line)
    if m is not None:
        return m.group(1)
    return None


def find_locations(root_dir, target, target_ext=".tex", verbose=False):
    """
    - root_dir: str, the root of the directory tree to explore
    - target: str, the target to find
    - target_ext: str, the file extensions to browse
    RETURN: Array of {'file': string, 'line': int} values
    """
    locations = []
    pp_dir = root_dir
    for dir_name, subdirs, files in os.walk(root_dir):
        for file_name in files:
            path = os.path.join(dir_name, file_name)
            _, ext = os.path.splitext(file_name)
            if target_ext is not None and ext != target_ext:
                continue
            if verbose:
                print("checking {}...".format(path))
            pp_path = path
            try:
                with open(path, encoding="utf8", errors="ignore") as f:
                    for line_number, line in enumerate(f.readlines()):
                        if target in line:
                            path_to_log = os.path.relpath(pp_path, start=pp_dir)
                            locations.append({"file": path_to_log, "line": line_number + 1})
            except Exception as e:
                import traceback

                print("issue encountered in finding locations: {}".format(e))
                traceback.print_tb(sys.last_traceback)
                sys.exit(EXIT_CODE_ISSUES_FOUND)
    return locations


def find_undefined_hyperref(root_dir, label):
    """
    - root_dir: str, the root of the directory tree to explore
    - label: str, the label that is undefined
    RETURN: Array of {'file': string, 'line': int} values
    """
    target = "\\hyperref[" + label + "]"
    return find_locations(root_dir, target)


def find_multiply_defined_labels(root_dir, label):
    """
    - root_dir: str, the root of the directory tree to explore
    - label: str, the label that is multiply defined
    RETURN: Array of {'file': string, 'line': int} values
    """
    target = "\\label{" + label + "}"
    return find_locations(root_dir, target)


def parse_line_number(message):
    """
    - message: string
    RETURN: None or int, the line number if found
    """
    m = LINE_NO.search(message)
    if m is not None:
        return int(m.group(1))
    return None


def to_s(x):
    """
    A custom version of str(.) that prevents None from coming out as "None"
    - x: any, the thing to "stringify"
    RETURN: string
    """
    if x is None:
        return ""
    return str(x)


class LogParser:
    """
    The LaTeX Log Parser.
    """

    def __init__(self, log_path, src_dir, verbose=False):
        self.log_path = log_path
        self.src_dir = src_dir
        self.verbose = verbose

    def _read_tex_error(self):
        line_no = parse_line_number(self._current_issue)
        self._issues["issues"].append(
            {
                "level": SEVERITY_ERROR,
                "type": self._type,
                "path": self._current_tex_file,
                "line": {"start": line_no, "end": line_no},
                "message": self._current_issue.strip(),
            }
        )

    def _read_warning(self):
        warns = []
        m1 = LABEL_MULTIPLY_DEFINED_WARN.match(self._current_issue)
        m2 = HYPER_UNDEFINED_WARN.match(self._current_issue)
        if m1 is not None:
            label = m1.group(1)
            locations = find_multiply_defined_labels(self.src_dir, label)
            for loc in locations:
                warns.append(
                    {
                        "level": SEVERITY_WARNING,
                        "type": "Label multiply defined",
                        "path": loc["file"],
                        "line": {"start": loc["line"], "end": loc["line"]},
                        "message": self._current_issue.strip(),
                        "label": m1.group(1),
                    }
                )
        elif m2 is not None:
            label = m2.group(1)
            locations = find_undefined_hyperref(self.src_dir, label)
            for loc in locations:
                warns.append(
                    {
                        "level": SEVERITY_WARNING,
                        "type": "Hyper reference undefined",
                        "path": loc["file"],
                        "line": {"start": loc["line"], "end": loc["line"]},
                        "message": self._current_issue.strip(),
                        "label": m2.group(1),
                    }
                )
        elif self._current_issue.strip() not in ISSUES_TO_SKIP:
            line_no = parse_on_input_line(self._current_issue)
            warns = [
                {
                    "level": SEVERITY_WARNING,
                    "type": self._type,
                    "path": self._current_tex_file,
                    "line": {"start": line_no, "end": line_no},
                    "message": self._current_issue.strip(),
                }
            ]
        self._issues["issues"] += warns

    def _read_error(self):
        if self._current_issue not in ISSUES_TO_SKIP:
            self._read_tex_error()

    def _read_issue(self, line):
        is_ws = line.strip() == ""
        first_line = self._issue_line == 1
        nonempty = len(line) > 1
        if is_ws and (not (first_line and nonempty)):
            if self._in_tex_err:
                self._read_tex_error()
            elif self._in_warn:
                self._read_warning()
            elif self._in_err:
                self._read_error()
            else:
                raise Exception("unhandled issue read")
            self._reset_after_read()
        else:
            self._current_issue += line
        self._issue_line += 1

    def _init_state(self):
        self._previous_line = None
        self._issue_line = None
        self._in_tex_err = False
        self._in_warn = False
        self._in_err = False
        self._type = None
        self._current_tex_file = None
        self._current_issue = None

    def _reset_after_read(self):
        self._in_tex_err = False
        self._in_warn = False
        self._in_err = False
        self._type = None
        self._current_issue = None

    def _report(self, message):
        if self.verbose:
            print(message)

    def _assemble_full_line(self, line):
        if self._previous_line is not None:
            full_line = self._previous_line.strip() + line
        else:
            full_line = line
        return full_line

    def __call__(self):
        self._issues = {"log_file_path": self.log_path, "issues": []}
        try:
            with open(self.log_path, encoding="utf8", errors="ignore") as rd:
                self._init_state()
                for line in rd.readlines():
                    if self._in_tex_err or self._in_warn or self._in_err:
                        self._read_issue(line)
                    else:
                        self._issue_line = None
                        full_line = self._assemble_full_line(line)
                        tex_file = parse_current_tex_file(full_line, self.src_dir)
                        issue_start = parse_tex_error(line)
                        warning_start = parse_warning_start(line)
                        err_start = parse_error_start(line)
                        if tex_file is not None:
                            self._current_tex_file = tex_file
                            self._report("processing {} ...".format(tex_file))
                        elif issue_start is not None:
                            self._issue_line = 1
                            self._in_tex_err = True
                            self._type = "TeX Error"
                            self._current_issue = issue_start
                            self._report("- issue {}".format(issue_start))
                        elif err_start is not None:
                            self._issue_line = 1
                            self._in_err = True
                            self._type = (to_s(err_start[0]) + " " + to_s(err_start[1])).strip()
                            self._current_issue = err_start[2]
                            self._report("- error {}".format(self._current_issue))
                        elif warning_start is not None:
                            self._issue_line = 1
                            self._in_warn = True
                            self._type = (to_s(warning_start[0]) + " " + to_s(warning_start[1])).strip()
                            self._current_issue = warning_start[2]
                            self._report("- warning {}".format(self._current_issue))
                    self._previous_line = line
        except Exception:
            sys.exit(EXIT_CODE_ISSUES_FOUND)
        return self._issues


def parse_log(log_path, src_dir, verbose=VERBOSE):
    """
    - log_path: str, the path to the LaTeX log file
    - src_dir: str, the path to the LaTeX source directory
    - verbose: bool, if True, print to sys.stdout
    RETURN: {   "log_file_path": str,
                "bangs": [{"tex_file": str, "issue": str}*],
                "warnings": [{"type": str, "message": str, "key": str}*]
            }
    """
    log_parser = LogParser(log_path, src_dir, verbose)
    return log_parser()


def update_paths_to_be_from_repo_root(issues, repo_root_to_src):
    """
    - issues: (Array {'file': string, ...})
    - repo_root_to_src: string, path from repository root to directory where
      src/ folder is.
    RETURN: (Array {'file': string, ...})
    """
    updated = []
    for issue in issues:
        update = copy.deepcopy(issue)
        if "path" in update:
            update["path"] = os.path.join(repo_root_to_src, update["path"])
        updated.append(update)
    return updated


def find_issues(log_path, json_error_path, src_dir):
    """
    - log_path: string, path to LaTeX log file to read
    - json_error_path: string, path to JSON error file to write
    - src_dir: string, path to the source directory of the processed LaTeX
      files
    RETURNS: Bool
    SIDE_EFFECTS:
    - if significant errors are found in the log_path, writes json_error_path
      with a summary of issues. Also writes issues to stdout
    - will ONLY write the json error file if an issue is found
    - returns True if issues found, else false
    - if json_error_path exists when this script is run, the file is deleted
      (since the presense of the file indicates that issues are found, we don't
      want the file to persist between calls)
    """
    if os.path.exists(json_error_path):
        os.remove(json_error_path)
    try:
        errs = parse_log(log_path, src_dir)
        repo_root = os.path.abspath(os.path.join(os.path.dirname(log_path), "..", ".."))
        repo_root_to_src = os.path.relpath(src_dir, start=repo_root)
        errs["issues"] = update_paths_to_be_from_repo_root(errs["issues"], repo_root_to_src)
    except Exception as e:
        import traceback

        print("issue encountered in parsing log: {}".format(e))
        traceback.print_tb(sys.last_traceback)
        sys.exit(EXIT_CODE_ISSUES_FOUND)
    num_issues = len(errs["issues"])
    if num_issues > 0:
        if VERBOSE:
            print("ISSUES: {}".format(num_issues))
        f = sys.stdout
        for issue in errs["issues"]:
            f.write("[LATEX")
            f.write(issue["level"] + "::")
            f.write(issue["path"] + ":" + str(issue["line"]["start"]))
            msg = issue["message"].replace("\r\n", " ")
            msg = msg.replace("\n", " ")
            msg = msg.replace("\r", " ")
            msg = msg.replace("\t", " ")
            msg = " ".join(msg.split())
            f.write("::" + msg + "]\n")
        with open(json_error_path, "w", encoding="utf-8") as f:
            json.dump(errs, f, ensure_ascii=False, indent=4)
    return num_issues > 0


def run_tests():
    """
    A simple unit test suite for the parser.
    """
    out = parse_current_tex_file(
        "[70] [71] [72] [73] [74] [75]) " + "(./src/overview/group-compliance-objects.tex [76",
        "/home/user/stuff/input-output-reference",
    )
    msg = "out = {}".format(out)
    assert out == "src/overview/group-compliance-objects.tex", msg
    out = parse_current_tex_file(
        "Underfull \\hbox (badness 10000) in paragraph at lines " + "1042--1043",
        "/home/user/stuff/input-output-reference",
    )
    assert out is None, "out = {}".format(out)
    out = parse_current_tex_file(
        "]) (./src/overview/group-location-climate-weather-file-access" + ".tex [77] [78] [79",
        "/home/user/stuff/input-output-reference",
    )
    assert out == ("src/overview/group-location-climate-weather-file-access.tex")
    out = parse_current_tex_file(
        "(./src/appendix-a-units-and-abbreviations/"
        + "standard-energyplus-conditions.tex) "
        + "(./src/appendix-a-units-and-abbreviations/"
        + "standard-energyplus-units.tex",
        "/home/user/stuff/input-output-reference",
    )
    assert out == ("src/appendix-a-units-and-abbreviations/" + "standard-energyplus-units.tex"), "out == {}".format(out)
    out = parse_tex_error("! Misplaced alignment tab character &.")
    msg = "out == {}".format(out)
    assert out == "Misplaced alignment tab character &.", msg
    out = parse_warning_start("LaTeX Warning: Hyper reference " + "`airterminalsingleductuncontrolled' on page 2467")
    assert out[2] == (" Hyper reference `airterminalsingleductuncontrolled' " + "on page 2467"), "out = {}".format(out)


if __name__ == "__main__":
    if len(sys.argv) == 2 and (sys.argv[1] == "test"):
        print("TESTING:")
        run_tests()
        print("all tests passed")
        sys.exit(EXIT_CODE_GOOD)
    if len(sys.argv) != 4:
        print("USAGE python " + sys.argv[0] + " <latex-log-file-path> " + "<latex-source-dir> <json-err-path>")
        print("- <latex-log-file-path>: file path to LaTeX log file to parse")
        print("- <latex-source-dir>: path to LaTeX source directory")
        print("- <json-err-path>: file path to a JSON version of the error " + "file; only written if issues found")
        print("NOTE: no error file is written if no significant " + "errors/warnings found")
        print("To run tests, call `python {} test'".format(sys.argv[0]))
        sys.exit(EXIT_CODE_ISSUES_FOUND)
    log_path = sys.argv[1]
    src_dir = sys.argv[2]
    json_err = sys.argv[3]
    if VERBOSE:
        print("log_path: {}".format(log_path))
        print("src_dir : {}".format(src_dir))
        print("json_err: {}".format(json_err))
    issues_found = find_issues(log_path, json_err, src_dir)
    if issues_found:
        sys.exit(EXIT_CODE_ISSUES_FOUND)
