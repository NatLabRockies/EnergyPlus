# to get the list of input files, go to the root of the document folder, and:
# grep -r '\\input{' * | cut -d ':' -f 2 | cut -c 8- | rev | cut -c 2- | rev | sed 's|^|  ${PROJECT_SOURCE_DIR}/acknowledgments/|g' | sed 's/$/.tex/g'
set(INCLUDED_TEX
    ${PROJECT_SOURCE_DIR}/header.tex
    ${PROJECT_SOURCE_DIR}/external-interfaces-application-guide/external-interfaces-application-guide.tex
    ${PROJECT_SOURCE_DIR}/external-interfaces-application-guide/src/external-interfaces-application-guide.tex
    ${PROJECT_SOURCE_DIR}/external-interfaces-application-guide/src/external-interface-s.tex
    ${PROJECT_SOURCE_DIR}/external-interfaces-application-guide/src/external-interface-s/introduction.tex
    ${PROJECT_SOURCE_DIR}/external-interfaces-application-guide/src/external-interface-s/coupling-energyplus-with-the-building.tex
    ${PROJECT_SOURCE_DIR}/external-interfaces-application-guide/src/external-interface-s/bcvtb-examples.tex
    ${PROJECT_SOURCE_DIR}/external-interfaces-application-guide/src/external-interface-s/coupling-energyplus-with-functional-mock-up.tex
    ${PROJECT_SOURCE_DIR}/external-interfaces-application-guide/src/external-interface-s/fmu-examples.tex
    ${PROJECT_SOURCE_DIR}/external-interfaces-application-guide/src/external-interface-s/exporting-energyplus-as-a-functional-mock-up.tex
    ${PROJECT_SOURCE_DIR}/external-interfaces-application-guide/src/references.tex
    ${PROJECT_BINARY_DIR}/title.tex)

# to get the list of included images, go to the root of the document folder and:
# grep -r '\\includegraphics' * | cut -d ':' -f 2 | rev | cut -d '{' -f 1 | rev | cut -d '}' -f 1 | sed 's/^/  /g' | sort -n -t'.' -k 1.14
set(INCLUDED_IMAGES
    ${PROJECT_SOURCE_DIR}/external-interfaces-application-guide/media/image004.png
    ${PROJECT_SOURCE_DIR}/external-interfaces-application-guide/media/image005.png
    ${PROJECT_SOURCE_DIR}/external-interfaces-application-guide/media/image006.png
    ${PROJECT_SOURCE_DIR}/external-interfaces-application-guide/media/image007.png
    ${PROJECT_SOURCE_DIR}/external-interfaces-application-guide/media/image008.png
    ${PROJECT_SOURCE_DIR}/external-interfaces-application-guide/media/image016.png
    ${PROJECT_SOURCE_DIR}/external-interfaces-application-guide/media/image033.png
    ${PROJECT_SOURCE_DIR}/external-interfaces-application-guide/media/image034.png
    ${PROJECT_SOURCE_DIR}/external-interfaces-application-guide/media/image035.png
    ${PROJECT_SOURCE_DIR}/external-interfaces-application-guide/media/image036.png
    ${PROJECT_SOURCE_DIR}/external-interfaces-application-guide/media/image037.png)

create_doc_target("external-interfaces-application-guide" "ExternalInterfacesApplicationGuide")

if(BUILD_HTML_DOCS)
  create_html_doc_target("external-interfaces-application-guide" "external-interfaces-application-guide")
endif()
