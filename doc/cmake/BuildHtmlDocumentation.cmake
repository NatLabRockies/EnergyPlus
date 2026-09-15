# Caller needs to set:
  # PANDOC, the path to the pandoc executable
  # INNAME, the name of the input tex file (without extension)
  # OUTNAME, the directory name to use under html/ in the build tree (eg "input-output-reference")
  # HTML_ASSETS_DIR, the doc/html directory holding the shared templates/css/lua filters
  # ORIGINAL_CMAKE_SOURCE_DIR, the root of the source repo (doc/)
  # ORIGINAL_CMAKE_BINARY_DIR, the root of the build tree (doc/'s binary dir)
  # Python_EXECUTABLE, used to build the search index from pandoc's sitemap.json

set(COMMAND_ECHO_MODE NONE)

if(WIN32)
  set(TEXINPUTS_SEPARATOR ";")
else()
  set(TEXINPUTS_SEPARATOR ":")
endif()
set(ENV{TEXINPUTS} "${ORIGINAL_CMAKE_BINARY_DIR}${TEXINPUTS_SEPARATOR}$ENV{TEXINPUTS}")

set(HTML_OUT_DIR "${ORIGINAL_CMAKE_BINARY_DIR}/html/${OUTNAME}")

file(REMOVE_RECURSE "${HTML_OUT_DIR}")

# Pandoc versions used by local development and Read the Docs do not expose the
# same MathML option. Prefer the current spelling when available, while retaining
# compatibility with older Pandoc releases.
execute_process(
  COMMAND "${PANDOC}" --help
  OUTPUT_VARIABLE PANDOC_HELP
  ERROR_QUIET
  RESULT_VARIABLE PANDOC_HELP_RESULT
)

if(PANDOC_HELP_RESULT EQUAL 0 AND PANDOC_HELP MATCHES "--math-method")
  set(PANDOC_MATHML_OPTION "--math-method=mathml")
else()
  set(PANDOC_MATHML_OPTION "--mathml")
endif()

execute_process(
  COMMAND "${PANDOC}"
          --to=chunkedhtml
          ${PANDOC_MATHML_OPTION}
          --standalone
          --table-of-contents
          --split-level=2
          --metadata=doc-class:${OUTNAME}
          --output=${HTML_OUT_DIR}
          --template=${HTML_ASSETS_DIR}/template_chunked.html
          --css=style.css
          --include-in-header=${HTML_ASSETS_DIR}/header.html
          --include-after-body=${HTML_ASSETS_DIR}/footer.html
          --lua-filter=${HTML_ASSETS_DIR}/bootstrap-tables.lua
          --lua-filter=${HTML_ASSETS_DIR}/numbered-cross-references.lua
          --lua-filter=${HTML_ASSETS_DIR}/object-index.lua
          ${INNAME}.tex
  RESULT_VARIABLE ERRCODE
  COMMAND_ECHO ${COMMAND_ECHO_MODE}
)

if(NOT ERRCODE EQUAL 0)
  message(FATAL_ERROR "pandoc failed to build the HTML documentation for ${INNAME} (error code ${ERRCODE})")
endif()

# Build search index from sitemap (levels 2, 3 & 5: groups, objects, field names)
execute_process(
  COMMAND "${Python_EXECUTABLE}" "${ORIGINAL_CMAKE_SOURCE_DIR}/cmake/build_search_index.py"
          "${HTML_OUT_DIR}/sitemap.json" "${HTML_OUT_DIR}/search-index.js"
  RESULT_VARIABLE ERRCODE
  COMMAND_ECHO ${COMMAND_ECHO_MODE}
)

if(NOT ERRCODE EQUAL 0)
  message(FATAL_ERROR "Failed to build the search index for ${INNAME} (error code ${ERRCODE})")
endif()

# Copy assets that pandoc doesn't copy for chunked output
file(COPY "${HTML_ASSETS_DIR}/style.css" DESTINATION "${HTML_OUT_DIR}")
file(MAKE_DIRECTORY "${HTML_OUT_DIR}/media")
file(COPY "${ORIGINAL_CMAKE_SOURCE_DIR}/../release/ep_nobg.svg" DESTINATION "${HTML_OUT_DIR}/media")
