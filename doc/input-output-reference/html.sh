#!/usr/bin/env bash
# Convenience wrapper for manually building the chunked HTML doc without going through CMake.
# The actual build (used by `cmake --build . --target zHTML_input-output-reference` when
# BUILD_HTML_DOCS=ON) lives in ../cmake/BuildHtmlDocumentation.cmake and ../cmake/build_search_index.py.
set -euo pipefail
cd "$(dirname "$0")"

ASSETS=../html

rm -Rf chunked/

pandoc --to=chunkedhtml \
  --mathml \
  --standalone=true \
  --table-of-contents=true \
  --split-level=2 \
  --output=chunked \
  --template "$ASSETS/template_chunked.html" \
  --css=style.css \
  --number-sections=false \
  --include-in-header "$ASSETS/header.html" \
  --include-after-body "$ASSETS/footer.html" \
  --lua-filter="$ASSETS/bootstrap-tables.lua" \
  --lua-filter="$ASSETS/object-index.lua" \
  input-output-reference.tex

# Build search index from sitemap (levels 2, 3 & 5)
python3 "../cmake/build_search_index.py" chunked/sitemap.json chunked/search-index.js

# Copy assets that pandoc doesn't copy for chunked output
cp "$ASSETS/style.css" chunked/
mkdir -p chunked/media
cp ../../release/ep_nobg.png chunked/media/
