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

"""Repair links and identifiers after Pandoc writes chunked HTML.

Pandoc cannot always rewrite a LaTeX cross-reference when a Lua filter creates
the destination identifier.  In that case the generated link still points to
``#label`` even when the destination is in another chunk.  The same conversion
can also emit duplicate identifiers (an automatic heading identifier followed
by an equivalent LaTeX label) and identifiers containing whitespace.

This script operates on the completed manual so it can resolve destinations
across every generated chunk without guessing Pandoc's output filenames. It
also adds subsection links to chapter landing pages, including chunks whose
source would otherwise contain only a heading.
"""

from __future__ import annotations

import argparse
import html
import json
import os
import re
from pathlib import Path
from urllib.parse import unquote, urlsplit

ID_RE = re.compile(r'\bid="([^"]*)"')
HREF_RE = re.compile(r'\bhref="([^"]*)"')
CHAPTER_HEADING_RE = re.compile(
    r'(?P<heading><div class="bd-content[^"]*">\s*<h1\b[^>]*>.*?</h1>)',
    re.DOTALL,
)


def add_chapter_contents(html_directory: Path, contents: dict[Path, str]) -> int:
    sitemap_path = html_directory / "sitemap.json"
    if not sitemap_path.exists():
        return 0

    sitemap = json.loads(sitemap_path.read_text(encoding="utf-8"))
    contents_added = 0
    for chapter in sitemap.get("subsections", []):
        section = chapter.get("section", {})
        subsections = chapter.get("subsections", [])
        chapter_url = section.get("path", "")
        if not chapter_url or not subsections:
            continue

        chapter_path = html_directory / urlsplit(chapter_url).path
        source = contents.get(chapter_path)
        if source is None or 'id="section-contents"' in source:
            continue

        items = []
        for subsection in subsections:
            subsection_data = subsection.get("section", {})
            target = subsection_data.get("path", "")
            title = subsection_data.get("title", "")
            if not target or not title:
                continue
            number = subsection_data.get("number")
            label = f"{number} {title}" if number else title
            items.append(f'        <li><a href="{html.escape(target, quote=True)}">' f"{html.escape(label)}</a></li>")

        if not items:
            continue

        chapter_contents = (
            '\n      <nav class="section-contents mt-4" aria-labelledby="section-contents">\n'
            '        <h2 id="section-contents">Contents</h2>\n'
            "        <ul>\n" + "\n".join(items) + "\n        </ul>\n"
            "      </nav>"
        )
        updated, replacements = CHAPTER_HEADING_RE.subn(
            lambda match: match.group("heading") + chapter_contents,
            source,
            count=1,
        )
        if replacements:
            contents[chapter_path] = updated
            contents_added += 1

    return contents_added


def normalized_identifier(identifier: str) -> str:
    return re.sub(r"\s+", "-", identifier.strip())


def fragment_from_href(href: str) -> tuple[str, str] | None:
    if "#" not in href:
        return None
    path, fragment = href.rsplit("#", 1)
    return path, unquote(html.unescape(fragment))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("html_directory", type=Path)
    args = parser.parse_args()

    files = sorted(args.html_directory.glob("*.html"))
    contents = {path: path.read_text(encoding="utf-8") for path in files}

    chapter_contents_added = add_chapter_contents(args.html_directory, contents)

    identifier_renames: dict[str, str] = {}
    for source in contents.values():
        for identifier in ID_RE.findall(source):
            normalized = normalized_identifier(identifier)
            if normalized != identifier:
                identifier_renames[identifier] = normalized

    def rename_id(match: re.Match[str]) -> str:
        identifier = match.group(1)
        return f'id="{identifier_renames.get(identifier, identifier)}"'

    normalized_contents = {path: ID_RE.sub(rename_id, source) for path, source in contents.items()}

    # Keep the first occurrence of an identifier in each chunk.  Explicit
    # LaTeX labels are commonly emitted as empty spans immediately after the
    # heading that already owns the same identifier.
    deduplicated_contents: dict[Path, str] = {}
    duplicates_removed = 0
    for path, source in normalized_contents.items():
        seen: set[str] = set()

        def remove_duplicate_id(match: re.Match[str]) -> str:
            nonlocal duplicates_removed
            identifier = match.group(1)
            if identifier in seen:
                duplicates_removed += 1
                return ""
            seen.add(identifier)
            return match.group(0)

        deduplicated_contents[path] = ID_RE.sub(remove_duplicate_id, source)

    locations: dict[str, list[Path]] = {}
    local_ids: dict[Path, set[str]] = {}
    for path, source in deduplicated_contents.items():
        ids = set(ID_RE.findall(source))
        local_ids[path] = ids
        for identifier in ids:
            locations.setdefault(identifier, []).append(path)

    links_repaired = 0
    ids_normalized = len(identifier_renames)
    for path, source in deduplicated_contents.items():

        def repair_href(match: re.Match[str]) -> str:
            nonlocal links_repaired
            href = match.group(1)
            parsed_url = urlsplit(html.unescape(href))
            if parsed_url.scheme or parsed_url.netloc:
                return match.group(0)

            parsed = fragment_from_href(href)
            if parsed is None:
                return match.group(0)

            target_path, fragment = parsed
            fragment = identifier_renames.get(fragment, fragment)

            if target_path:
                normalized_href = f"{target_path}#{fragment}"
                return f'href="{normalized_href}"'

            if fragment in local_ids[path]:
                return f'href="#{fragment}"'

            destinations = locations.get(fragment, [])
            if len(destinations) != 1:
                return f'href="#{fragment}"'

            relative = os.path.relpath(destinations[0], path.parent)
            links_repaired += 1
            return f'href="{relative}#{fragment}"'

        repaired = HREF_RE.sub(repair_href, source)
        path.write_text(repaired, encoding="utf-8")

    print(
        "Chunked HTML cleanup: "
        f"added contents to {chapter_contents_added} chapter pages, "
        f"normalized {ids_normalized} identifiers, "
        f"removed {duplicates_removed} duplicate identifiers, "
        f"and repaired {links_repaired} cross-chunk links"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
