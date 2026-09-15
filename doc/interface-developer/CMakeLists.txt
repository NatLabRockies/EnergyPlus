# to get the list of input files, go to the root of the document folder, and:
# grep -r '\\input{' * | cut -d ':' -f 2 | cut -c 8- | rev | cut -c 2- | rev | sed 's|^|  ${PROJECT_SOURCE_DIR}/acknowledgments/|g' | sed 's/$/.tex/g'
set(INCLUDED_TEX
    ${PROJECT_SOURCE_DIR}/header.tex
    ${PROJECT_SOURCE_DIR}/interface-developer/interface-developer.tex
    ${PROJECT_SOURCE_DIR}/interface-developer/src/guide-for-interface-developers.tex
    ${PROJECT_SOURCE_DIR}/interface-developer/src/introduction.tex
    ${PROJECT_SOURCE_DIR}/interface-developer/src/introduction/interface-expectations.tex
    ${PROJECT_SOURCE_DIR}/interface-developer/src/energyplus-install-contents.tex
    ${PROJECT_SOURCE_DIR}/interface-developer/src/input-overview.tex
    ${PROJECT_SOURCE_DIR}/interface-developer/src/input-overview/general-input-rules.tex
    ${PROJECT_SOURCE_DIR}/interface-developer/src/input-overview/input-data-dictionary.tex
    ${PROJECT_SOURCE_DIR}/interface-developer/src/input-overview/input-data-file.tex
    ${PROJECT_SOURCE_DIR}/interface-developer/src/input-details.tex
    ${PROJECT_SOURCE_DIR}/interface-developer/src/input-details/idd-conventions.tex
    ${PROJECT_SOURCE_DIR}/interface-developer/src/input-details/standard-energyplus-units.tex
    ${PROJECT_SOURCE_DIR}/interface-developer/src/input-details/energyplus-reports.tex
    ${PROJECT_SOURCE_DIR}/interface-developer/src/output.tex
    ${PROJECT_SOURCE_DIR}/interface-developer/src/weather-data.tex
    ${PROJECT_SOURCE_DIR}/interface-developer/src/running-energyplus.tex
    ${PROJECT_SOURCE_DIR}/interface-developer/src/licensing.tex
    ${PROJECT_SOURCE_DIR}/interface-developer/src/appendix-a-simple-idf-file.tex
    ${PROJECT_BINARY_DIR}/title.tex)

# to get the list of included images, go to the root of the document folder and:
# grep -r '\\includegraphics' * | cut -d ':' -f 2 | rev | cut -d '{' -f 1 | rev | cut -d '}' -f 1 | sed 's/^/  /g' | sort -n -t'.' -k 1.14
set(INCLUDED_IMAGES ${PROJECT_SOURCE_DIR}/interface-developer/media/image001.png ${PROJECT_SOURCE_DIR}/interface-developer/media/image002.jpg
                    ${PROJECT_SOURCE_DIR}/interface-developer/media/image003.png)

create_doc_target("interface-developer" "InterfaceDeveloper")

if(BUILD_HTML_DOCS)
  create_html_doc_target("interface-developer" "interface-developer")
endif()
