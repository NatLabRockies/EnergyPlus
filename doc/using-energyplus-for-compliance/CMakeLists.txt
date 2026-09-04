# to get the list of input files, go to the root of the document folder, and:
# grep -r '\\input{' * | cut -d ':' -f 2 | cut -c 8- | rev | cut -c 2- | rev | sed 's|^|  ${PROJECT_SOURCE_DIR}/acknowledgments/|g' | sed 's/$/.tex/g'
set(INCLUDED_TEX
    ${PROJECT_BINARY_DIR}/title.tex
    ${PROJECT_SOURCE_DIR}/using-energyplus-for-compliance/using-energyplus-for-compliance.tex
    ${PROJECT_SOURCE_DIR}/header.tex
    ${PROJECT_SOURCE_DIR}/using-energyplus-for-compliance/src/using-energyplus-for-compliance.tex
    ${PROJECT_SOURCE_DIR}/using-energyplus-for-compliance/src/introduction.tex
    ${PROJECT_SOURCE_DIR}/using-energyplus-for-compliance/src/introduction/background.tex
    ${PROJECT_SOURCE_DIR}/using-energyplus-for-compliance/src/introduction/compliance-objects.tex
    ${PROJECT_SOURCE_DIR}/using-energyplus-for-compliance/src/introduction/tabular-summary-reports.tex
    ${PROJECT_SOURCE_DIR}/using-energyplus-for-compliance/src/introduction/features-related-to-appendix-g-and-leed.tex
    ${PROJECT_SOURCE_DIR}/using-energyplus-for-compliance/src/introduction/features-related-to-california-title-24.tex)

# to get the list of included images, go to the root of the document folder and:
# grep -r '\\includegraphics' * | cut -d ':' -f 2 | rev | cut -d '{' -f 1 | rev | cut -d '}' -f 1 | sed 's/^/  /g' | sort -n -t'.' -k 1.14
set(INCLUDED_IMAGES)

create_doc_target("using-energyplus-for-compliance" "UsingEnergyPlusForCompliance")
