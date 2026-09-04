# to get the list of input files, go to the root of the document folder, and:
# grep -r '\\input{' * | cut -d ':' -f 2 | cut -c 8- | rev | cut -c 2- | rev | sed 's|^|  ${PROJECT_SOURCE_DIR}/acknowledgments/|g' | sed 's/$/.tex/g'
set(INCLUDED_TEX
    ${PROJECT_SOURCE_DIR}/header.tex
    ${PROJECT_SOURCE_DIR}/getting-started/getting-started.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/title.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/energyplus-overview.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/energyplus-overview/what-is-energyplus.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/energyplus-overview/why-does-energyplus-exist-and-what-were-its.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/energyplus-overview/energyplus-documentation-library.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/getting-started-with-energyplus.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/running-energyplus.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/running-energyplus/introduction.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/running-energyplus/ep-launch-program.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/running-energyplus/energyplus-file-extensions.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/tutorial-example-for-running-energyplus.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/tutorial-example-for-running-energyplus/running-energyplus-building-envelope-internal.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/tutorial-example-for-running-energyplus/instructions.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/overall-scheme-methodology-for-running.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/overall-scheme-methodology-for-running/building-simulation.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/overall-scheme-methodology-for-running/a-methodology-for-using-energyplus.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/tutorial-exercise-2.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/tutorial-exercise-2/unitary-system-and-vav-using-hvactemplate.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/tutorial-exercise-2/instructions-001.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/idf-editor-brief-introduction.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/other-useful-programs-information.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/other-useful-programs-information/hvactemplate-objects.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/other-useful-programs-information/data-sets.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/other-useful-programs-information/slab-and-basement-programs.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/other-useful-programs-information/coefficient-curve-generation.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/other-useful-programs-information/useful-programs.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/other-useful-programs-information/weatherdata.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/other-useful-programs-information/results-processing.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/other-useful-programs-information/hvac-diagram.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/other-useful-programs-information/csvproc.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/other-useful-programs-information/convertesomtr.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/other-useful-programs-information/datafiles.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/other-useful-programs-information/library-files.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/energy-meters.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/energy-meters/standard-energy-meters.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/energy-meters/custom-meters.tex
    ${PROJECT_SOURCE_DIR}/getting-started/src/standard-energyplus-units.tex
    ${PROJECT_BINARY_DIR}/title.tex)

# to get the list of included images, go to the root of the document folder and:
# grep -r '\\includegraphics' * | cut -d ':' -f 2 | rev | cut -d '{' -f 1 | rev | cut -d '}' -f 1 | sed 's/^/  /g' | sort -n -t'.' -k 1.14
set(INCLUDED_IMAGES
    ${PROJECT_SOURCE_DIR}/getting-started/media/image001.png
    ${PROJECT_SOURCE_DIR}/getting-started/media/image002.jpg
    ${PROJECT_SOURCE_DIR}/getting-started/media/image004.png
    ${PROJECT_SOURCE_DIR}/getting-started/media/image005.jpg
    ${PROJECT_SOURCE_DIR}/getting-started/media/image006.png
    ${PROJECT_SOURCE_DIR}/getting-started/media/image007.png
    ${PROJECT_SOURCE_DIR}/getting-started/media/image008.png
    ${PROJECT_SOURCE_DIR}/getting-started/media/image009.png
    ${PROJECT_SOURCE_DIR}/getting-started/media/image010.jpg
    ${PROJECT_SOURCE_DIR}/getting-started/media/image011.jpg
    ${PROJECT_SOURCE_DIR}/getting-started/media/image012.png
    ${PROJECT_SOURCE_DIR}/getting-started/media/image013.png
    ${PROJECT_SOURCE_DIR}/getting-started/media/image014.png
    ${PROJECT_SOURCE_DIR}/getting-started/media/image015.png
    ${PROJECT_SOURCE_DIR}/getting-started/media/image016.png
    ${PROJECT_SOURCE_DIR}/getting-started/media/image017.jpg
    ${PROJECT_SOURCE_DIR}/getting-started/media/image018.png
    ${PROJECT_SOURCE_DIR}/getting-started/media/image019.jpg
    ${PROJECT_SOURCE_DIR}/getting-started/media/image020.png
    ${PROJECT_SOURCE_DIR}/getting-started/media/image021.jpg
    ${PROJECT_SOURCE_DIR}/getting-started/media/image022.png
    ${PROJECT_SOURCE_DIR}/getting-started/media/image023.png
    ${PROJECT_SOURCE_DIR}/getting-started/media/image024.jpg)

create_doc_target("getting-started" "GettingStarted")
