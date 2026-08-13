# Add custom command, target, and dependencies for documentation file
macro( CREATE_DOC_TARGET SOURCE_FILENAME OUTPUT_FILENAME )
  if (NOT UNIX)
    # Avoid issue #8568 - main memory size exceeded for I/O ref on Windows with a recent MikTex (eg 4.1)
    set(XELATEX_MEM_FLAGS ON)
  endif()
  add_custom_command( OUTPUT ${PROJECT_BINARY_DIR}/pdf/${OUTPUT_FILENAME}.pdf
    COMMAND ${CMAKE_COMMAND} -DXELATEX_COMPILER=${XELATEX_COMPILER} -DINNAME=${SOURCE_FILENAME} -DOUTNAME=${OUTPUT_FILENAME}
            -DORIGINAL_CMAKE_SOURCE_DIR=${PROJECT_SOURCE_DIR} -DORIGINAL_CMAKE_BINARY_DIR=${PROJECT_BINARY_DIR}
            -DTEX_INTERACTION=${TEX_INTERACTION} -DDOCS_TESTING=${DOCS_TESTING}
            -DXELATEX_MEM_FLAGS=${XELATEX_MEM_FLAGS}
            -P ${PROJECT_SOURCE_DIR}/cmake/BuildDocumentation.cmake
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/${SOURCE_FILENAME}
    DEPENDS ${INCLUDED_TEX} ${INCLUDED_IMAGES}
    )

  add_custom_target( zPDF_${OUTPUT_FILENAME}
    DEPENDS ${PROJECT_BINARY_DIR}/pdf/${OUTPUT_FILENAME}.pdf
    )

  add_dependencies(docs zPDF_${OUTPUT_FILENAME})

  set_target_properties(zPDF_${OUTPUT_FILENAME} PROPERTIES FOLDER Documentation)

  if (DOCS_TESTING)
    add_custom_command(TARGET zPDF_${OUTPUT_FILENAME}
      POST_BUILD
      COMMAND ${Python_EXECUTABLE} "${PROJECT_SOURCE_DIR}/tools/parse_latex_log.py" "${PROJECT_BINARY_DIR}/${SOURCE_FILENAME}/${SOURCE_FILENAME}.log" "${PROJECT_SOURCE_DIR}/${SOURCE_FILENAME}" "${PROJECT_BINARY_DIR}/${OUTPUT_FILENAME}_errors.json"
      )
  endif()
endmacro()
