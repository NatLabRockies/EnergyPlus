# EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the University
# of Illinois, The Regents of the University of California, through Lawrence
# Berkeley National Laboratory (subject to receipt of any required approvals
# from the U.S. Dept. of Energy), Oak Ridge National Laboratory, managed by UT-
# Battelle, Alliance for Energy Innovation, LLC, and other contributors. All
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

import datetime
import re
import textwrap
from abc import ABC, abstractmethod
from pathlib import Path

from base_hook import ErrorMessage, LogMessage, relative_path_from_root

#
# The previous year that is in the license. It should be a string
#
_previous_year = "2025"
#
# From file "EnergyPlus License DRAFT 112015 100 fixed.txt"
#
# This is the license text EXACTLY as agreed upon with LBNL IPO, so don't
# ever change it. EVER. YES I MEAN EVER.
_original = """// EnergyPlus, Copyright (c) 1996-2015, The Board of Trustees of the University of Illinois and
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy). All rights
// reserved.
//
// If you have questions about your rights to use or distribute this software, please contact
// Berkeley Lab's Innovation & Partnerships Office at IPO@lbl.gov.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without Lawrence Berkeley National Laboratory's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// You are under no obligation whatsoever to provide any bug fixes, patches, or upgrades to the
// features, functionality or performance of the source code ("Enhancements") to anyone; however,
// if you choose to make your Enhancements available either publicly, or directly to Lawrence
// Berkeley National Laboratory, without imposing a separate written license agreement for such
// Enhancements, then you hereby grant the following license: a non-exclusive, royalty-free
// perpetual license to install, use, modify, prepare derivative works, incorporate into other
// computer software, distribute, and sublicense such enhancements or derivative works thereof,
// in binary and source code form.
"""


def merge_paragraphs(text: str) -> str:
    """Merge license text lines into a single line per paragraph."""
    lines = []
    current = ""
    for line in text.splitlines():
        line = line.rstrip()
        if line == "//":
            lines.extend([current.lstrip(), ""])
            current = ""
        else:
            current += " " + line[2:].lstrip()
    lines.append(current.lstrip())
    return "\n".join(lines) + "\n"


def pythonize(
    text,
    line_limit: int = 79,
    toolname: str = "unspecified",
) -> str:
    """Convert the C++ comment text into Python comments"""
    paragraphs = [el for el in merge_paragraphs(text).splitlines() if el != ""]
    if len(paragraphs) != 8 or line_limit < 7:
        raise ValueError("License text cannot be processed")

    lines = []
    # Work the first three paragraphs
    limit = line_limit - 2
    for pg in paragraphs[0:3]:
        lines.extend([" " + el for el in textwrap.wrap(pg, width=limit)])
        lines.append("")
    # Work the next four paragraphs
    limit = line_limit - 6
    for i, pg in enumerate(paragraphs[3:7]):
        sublines = textwrap.wrap(pg[4:], width=limit)
        lines.append((" (%d) " % (i + 1)) + sublines[0])
        for el in sublines[1:]:
            lines.append("     " + el)
        lines.append("")
    # Work the last paragraph
    limit = line_limit - 2
    lines.extend([" " + el for el in textwrap.wrap(paragraphs[7], width=limit)])
    return "#" + "\n#".join(lines) + "\n"


def previous() -> str:
    """Return the previous Python license text (last year)."""
    # Modify the year in the text
    originalYear = "2015"
    currentYear = _previous_year
    txt = _original.replace(originalYear, currentYear)
    # Modify and delete some lines with LBNL IP permission
    # Keep in mind that the line numbering here starts with 0
    lines = txt.splitlines()
    # On line 37, replace LBNL with USDOE
    lines[37] = lines[37].replace("Lawrence Berkeley National Laboratory", "the U.S. Department of Energy")
    # Delete the last 9 lines
    lines = lines[:-9]
    # Delete lines 4-6
    lines = lines[:4] + lines[7:]
    # Modify the notice
    lines[0] = lines[0].replace(" and", ",")
    lines[2] = "// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge"
    lines.insert(3, "// National Laboratory, managed by UT-Battelle, Alliance for Energy Innovation, LLC, and other")
    lines[4] = "// contributors. All rights reserved."
    txt = "\n".join(lines) + "\n"
    return txt


def previous_python() -> str:
    """Return the previous Python license text (last year)."""
    cpp_license = previous()
    text = pythonize(cpp_license)
    return text


def current() -> str:
    """Return the current license text, as of today."""
    # Modify the year in the text
    originalYear = "2015"
    currentYear = "%d" % datetime.date.today().year
    txt = _original.replace(originalYear, currentYear)
    # Modify and delete some lines with LBNL IP permission
    # Keep in mind that the line numbering here starts with 0
    lines = txt.splitlines()
    # On line 37, replace LBNL with USDOE
    lines[37] = lines[37].replace("Lawrence Berkeley National Laboratory", "the U.S. Department of Energy")
    # Delete the last 9 lines
    lines = lines[:-9]
    # Delete lines 4-6
    lines = lines[:4] + lines[7:]
    # Modify the notice
    lines[0] = lines[0].replace(" and", ",")
    lines[2] = "// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge"
    lines.insert(3, "// National Laboratory, managed by UT-Battelle, Alliance for Energy Innovation, LLC, and other")
    lines[4] = "// contributors. All rights reserved."
    txt = "\n".join(lines) + "\n"
    return txt


def current_python() -> str:
    """Return the current Python license text, as of today."""
    cpp_license = current()
    text = pythonize(cpp_license)
    return text


def original():
    """Return the original BSD3+ license text from 2015."""
    return _original


def check_license(
    filepath: Path,
    possible: str,
    correct: str,
    offset: int = 0,
    toolname: str = "unspecified",
) -> LogMessage | None:
    """Check for a few of the usual issues with the license"""
    if possible == correct:
        return None
    try:
        possibleYear = possible[offset + 31 : offset + 35]
        correctYear = correct[offset + 31 : offset + 35]
    except IndexError:
        return ErrorMessage(
            tool=toolname,
            filepath=filepath,
            line_number=1,
            message="License text cannot be matched, check entire license",
        )
    try:
        # The original behavior was to report year differences and then possibly
        # report overall differences. Switch that around a bit so there are no
        # double reports. Now report incorrect license year only if that is all
        # that is wrong.
        corrected = possible[: offset + 31] + correctYear + possible[offset + 35 :]
        if corrected == correct:
            return ErrorMessage(
                tool=toolname,
                filepath=filepath,
                line_number=1,
                message="License year is incorrect",
            )
    except:
        return ErrorMessage(
            tool=toolname,
            filepath=filepath,
            line_number=1,
            message="License text cannot be matched, check entire license",
        )

    return ErrorMessage(
        tool=toolname,
        filepath=filepath,
        line_number=1,
        message="Differences in license text, check entire license",
    )


class FileVisitor(ABC):
    def __init__(self, toolname: str, extensions: list[str] | None = None):
        self.toolname = toolname
        self.visited_files: list[Path] = []
        if extensions is None:
            self.extensions = ["cc", "cpp", "c", "hh", "hpp", "h"]
        else:
            self.extensions = extensions
        self.log_messages: list[LogMessage] = []

    def error(self, filepath: Path, line_number: int, message: str):
        self.log_messages.append(
            ErrorMessage(
                tool=self.toolname,
                filepath=filepath,
                line_number=line_number,
                message=message,
            )
        )

    def files(self, path: Path, exclude_patterns=None) -> list[Path]:
        results = []
        for ext in self.extensions:
            results.extend(list(path.glob(f"**/*.{ext}")))
        if exclude_patterns is not None:
            for pattern in exclude_patterns:
                matcher = re.compile(pattern)
                results = [el for el in results if not matcher.match(str(relative_path_from_root(el)))]
        return results

    @abstractmethod
    def visit_file(self, filepath: Path) -> bool:
        """Visit a single file. Append to visited_files when overriding"""
        pass

    def visit(self, path: Path, exclude_patterns: list[str] | None = None) -> bool:
        overall_success = True
        for filepath in self.files(path, exclude_patterns=exclude_patterns):
            file_success = self.visit_file(filepath=filepath)
            if not file_success:
                overall_success = False
        return overall_success

    def readtext(self, filepath: Path):
        try:
            txt = filepath.read_text(encoding="utf-8")
        except UnicodeDecodeError as exc:
            self.error(filepath=filepath, line_number=0, message=f"UnicodeDecodeError: {exc}")
            txt = None
        except Exception as exc:
            self.error(filepath=filepath, line_number=0, message=f"Exception: {exc}")
            txt = None
        return txt


class Checker(FileVisitor):
    def __init__(
        self,
        boilerplate: str,
        offset: int = 3,
        toolname: str = "unspecified",
        extensions: list[str] | None = None,
        shebang: bool = False,
        empty_passes: bool = False,
    ):
        super().__init__(toolname=toolname, extensions=extensions)
        lines = boilerplate.splitlines()
        self.n = len(lines)
        self.text = boilerplate
        self.offset = offset
        self.shebang = shebang
        self.empty_passes = empty_passes

    def visit_file(self, filepath: Path) -> bool:
        self.visited_files.append(filepath)
        txt = self.readtext(filepath)
        if txt is None:
            return True

        if self.empty_passes:
            if txt.strip() == "":
                return True

        n = txt.count(self.text)
        if n == 0:
            lines = txt.splitlines()[: self.n]
            shortened = "\n".join(lines) + "\n"
            ret = check_license(
                filepath=filepath,
                possible=shortened,
                correct=self.text,
                offset=self.offset,
                toolname=self.toolname,
            )
            if ret is not None:
                self.log_messages.append(ret)
                return False
            return True
        else:
            if n > 1:
                self.error(filepath=filepath, line_number=1, message="Multiple instances of license text")
                return False
            if not txt.startswith(self.text):
                if self.shebang:
                    # Maybe the first line is a shebang?
                    lines = txt.splitlines()
                    if lines[0].startswith("#!"):
                        count = 1
                        if len(lines) > 1:
                            if re.match("^[ \t\f]*#.*?coding[:=][ \t]*" "([-_.a-zA-Z0-9]+)", lines[1]):
                                count += 1
                        txt = "\n".join(lines[count:])
                        if txt.startswith(self.text):
                            return True
                self.error(filepath=filepath, line_number=1, message="License text is not at top of file")
                return False
        return True


class Replacer(FileVisitor):
    def __init__(self, oldtext, newtext, extensions=None, dryrun=True):
        super().__init__(toolname="license-updater", extensions=extensions)
        self.oldtxt = oldtext
        self.newtxt = newtext
        self.dryrun = dryrun
        self.replaced = []

    def writetext(self, filepath: Path, txt: str):
        filepath.write_text(txt, encoding="utf-8")

    def visit_file(self, filepath) -> bool:
        self.visited_files.append(filepath)
        txt = self.readtext(filepath)
        if txt is None:
            return True

        is_current = self.newtxt in txt
        if is_current:
            return True

        is_outdated = self.oldtxt in txt
        if is_outdated:
            if self.dryrun:
                self.replaced.append(filepath)
            else:
                txt = txt.replace(self.oldtxt, self.newtxt)
                self.writetext(filepath, txt)
                self.replaced.append(filepath)
        else:
            self.error(filepath=filepath, line_number=1, message="File does not contain the old license text")

        return False

    def summary(self, full_report=False):
        txt = [f"Checked {len(self.visited_files)} files"]
        difference = list(set(self.visited_files) - set(self.replaced))
        prefix_repl = "Would have replaced" if self.dryrun else "Replaced"
        prefix_nothing = "Would have done nothing" if self.dryrun else "Did nothing"
        txt.append(f"{prefix_repl} text in {len(self.replaced)} file(s)")
        if full_report:
            txt += [f"\t{relative_path_from_root(file)}" for file in self.replaced]
        txt.append(f"{prefix_nothing} in {len(difference)} file(s)")
        if full_report:
            txt += [f"\t{relative_path_from_root(file)}" for file in difference]
        if self.log_messages:
            txt.append(f"Failures in {len(self.log_messages)} file(s)")
            txt += [f"\t{message}" for message in self.log_messages]
        return "\n".join(txt)


if __name__ == "__main__":
    text = current()
    print(text)
    for number, line in enumerate(text.splitlines()):
        print("%2d" % number, line)
