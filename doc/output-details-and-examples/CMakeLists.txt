# to get the list of input files, go to the root of the document folder, and:
# grep -r '\\input{' * | cut -d ':' -f 2 | cut -c 8- | rev | cut -c 2- | rev | sed 's|^|  ${PROJECT_SOURCE_DIR}/acknowledgments/|g' | sed 's/$/.tex/g'
set(INCLUDED_TEX
    ${PROJECT_SOURCE_DIR}/header.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/output-details-and-examples.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/output-details-and-examples.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/introduction.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/output-files.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/output-files/output-file-list.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/output-files/eplusout-audit.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/output-files/eplusout-bnd.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/output-files/eplusout-dbg.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/output-files/eplusout-dxf.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/output-files/eplusout-edd.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/output-files/eplusout-eio.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/output-files/eplusout-perflogcsv.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/output-files/eplusssz-lt-ext-gt.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/output-files/epluszsz-lt-ext-gt.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/output-files/eplusout-lt-ext-gt.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/output-files/eplusmtr-lt-ext-gt.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/output-files/eplusmap-lt-ext-gt.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/output-files/eplusout-dfs.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/output-files/eplusscreen-csv.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/output-files/eplustbl-lt-ext-gt.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/output-files/readvars-rvaudit.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/output-files/eplusout-svg.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/output-files/eplusout-sci.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/output-files/eplusout-wrl.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/output-files/delight-output-files.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/example-input-files.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/data-sets.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/data-sets/simple-list-data-sets.tex
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/src/data-sets/macro-data-sets.tex
    ${PROJECT_BINARY_DIR}/title.tex)

# to get the list of included images, go to the root of the document folder and:
# grep -r '\\includegraphics' * | cut -d ':' -f 2 | rev | cut -d '{' -f 1 | rev | cut -d '}' -f 1 | sed 's/^/  /g' | sort -n -t'.' -k 1.14
set(INCLUDED_IMAGES
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/media/image001.jpg
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/media/image002.jpg
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/media/image017.png
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/media/image018.jpg
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/media/image019.png
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/media/image020.png
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/media/image021.png
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/media/image022.jpg
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/media/image023.jpg
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/media/image024.jpg
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/media/image025.jpg
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/media/image026.jpg
    ${PROJECT_SOURCE_DIR}/output-details-and-examples/media/image027.jpg)

create_doc_target("output-details-and-examples" "OutputDetailsAndExamples")
