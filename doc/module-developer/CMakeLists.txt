# to get the list of input files, go to the root of the document folder, and:
# grep -r '\\input{' * | cut -d ':' -f 2 | cut -c 8- | rev | cut -c 2- | rev | sed 's|^|  ${PROJECT_SOURCE_DIR}/acknowledgments/|g' | sed 's/$/.tex/g'
set(INCLUDED_TEX
    ${PROJECT_SOURCE_DIR}/header.tex
    ${PROJECT_SOURCE_DIR}/module-developer/module-developer.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/guide-for-module-developers.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/introduction.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/modules-in-energyplus.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/modules-in-energyplus/what-is-a-module-anyway.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/modules-in-energyplus/what-is-a-module-developer.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/modules-in-energyplus/adding-new-features.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/input-concepts.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/input-concepts/input-data-dictionary.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/input-concepts/data-dictionary-naming-conventions.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/input-concepts/input-data-file.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/input-concepts/input-considerations.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/input-concepts/advanced-input-considerations.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/input-concepts/datasets.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/module-structure.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/module-structure/module-outline.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/module-structure/module-example.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/module-structure/changing-existing-code.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/module-structure/considerations-for-legacy-codes.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/module-structure/code-readability-vs-speed-of-execution.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/module-structure/how-it-fits-together.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/energyplus-calling-structure.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/energyplus-calling-structure/top-level-calling-tree.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/energyplus-calling-structure/high-level-hvac-calling-tree-schematic-not.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/energyplus-calling-structure/air-system-calling-tree-schematic-not-all.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/energyplus-calling-structure/plant-calling-tree-schematic-not-all-routines.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/energyplus-calling-structure/zone-equipment-calling-tree-schematic-not-all.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/energyplus-calling-structure/inserting-the-new-module-into-the-program.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/hvac-network.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/hvac-network/branches-connectors-and-nodes.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/hvac-network/nodes-in-the-simulation.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/hvac-network/getting-nodes.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/hvac-network/data-flow-in-an-hvac-component-module.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/hvac-network/interfacing-with-plant.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/hvac-network/node-mass-flow-variables.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/energyplus-services.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/energyplus-services/utility-routines-functions.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/energyplus-services/input-services.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/energyplus-services/object-services.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/energyplus-services/branch-node-checking-and-services.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/energyplus-services/schedule-services.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/energyplus-services/data-services.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/energyplus-services/other-useful-utilities.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/energyplus-services/error-messages.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/energyplus-services/display-strings.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/energyplus-services/performance-curve-services.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/energyplus-services/fluid-property-services.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/energyplus-services/weather-services.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/energyplus-services/global-data-flags-and-parameters.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/energyplus-services/psychrometric-services.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/energyplus-services/tabular-output-utilities.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/energyplus-services/internal-zone-gains-services.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/output.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/output/how-do-i-output-my-variables.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/output/output-variable-dos-and-don-ts.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/energy-management-system.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/energy-management-system/ems-actuator-interface.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/running-testing-energyplus-for-developers.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/running-testing-energyplus-for-developers/environment-variables-to-assist-running.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/running-testing-energyplus-for-developers/air-loop-simulation-environment-variables.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/quick-procedure-outline-for-making-code.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/important-rules-for-module-developers.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/appendix-a-dataglobals-and-dataenvironments.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/appendix-a-dataglobals-and-dataenvironments/dataglobals.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/appendix-a-dataglobals-and-dataenvironments/dataenvironment.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/appendix-b-submissions-and-check-ins.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/appendix-c-documentation-specifics.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/appendix-d-module-subroutine-function.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/appendix-e-test-file-documentation.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/appendix-f-new-or-changed-proposal-feature.tex
    ${PROJECT_SOURCE_DIR}/module-developer/src/appendix-g.tex
    ${PROJECT_BINARY_DIR}/title.tex)

# to get the list of included images, go to the root of the document folder and:
# grep -r '\\includegraphics' * | cut -d ':' -f 2 | rev | cut -d '{' -f 1 | rev | cut -d '}' -f 1 | sed 's/^/  /g' | sort -n -t'.' -k 1.14
set(INCLUDED_IMAGES ${PROJECT_SOURCE_DIR}/module-developer/media/image001.png ${PROJECT_SOURCE_DIR}/module-developer/media/image002.png
                    ${PROJECT_SOURCE_DIR}/module-developer/media/image003.jpg)

create_doc_target("module-developer" "ModuleDeveloper")
