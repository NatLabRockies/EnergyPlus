# -*- coding: utf-8 -*-

import os
import sys
from json import load
from pathlib import Path
from shutil import copytree, rmtree
from subprocess import DEVNULL, CalledProcessError, check_call

this_file_path = Path(__file__)
api_source_dir = this_file_path.parent.parent.parent.parent / "src" / "EnergyPlus" / "api"
print(f"* Adding api_source_dir: {api_source_dir}")
sys.path.insert(0, str(api_source_dir))
sys.path.insert(0, os.path.abspath("."))

# add a mock version of pyenergyplus
autodoc_mock_imports = ["pyenergyplus"]

# -- Project information -----------------------------------------------------
project = "EnergyPlus"
copyright = "2023, National Renewable Energy Laboratory for the United States Department of Energy"
author = "National Renewable Energy Laboratory for the United States Department of Energy"
version = "23.1"  # The short X.Y version
release = "23.1"  # The full version, including alpha/beta/rc tags

# -- General configuration ---------------------------------------------------
extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.coverage",
    "sphinx.ext.mathjax",
    "sphinx.ext.viewcode",
]
source_suffix = ".rst"
master_doc = "index"
language = "en"
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]
pygments_style = None

# -- Options for HTML output -------------------------------------------------
html_theme = "sphinx_rtd_theme"
html_logo = "../../../release/favicon_white_plus.png"
html_show_sphinx = False
html_show_copyright = True
html_extra_path = ["_build_c"]
html_static_path = ["_static"]
html_css_files = [
    "main.css",
]
html_js_files = [
    "https://code.jquery.com/jquery-1.11.2.min.js",
    "main.js",
]
extra_nav_links = {"Blah": "https://energyplus.net"}
templates_path = [".templates"]
html_theme_options = {
    "titles_only": True,
}
numfig = True
# html_sidebars = {
#     '**': [
#         'about.html',
#         'navigation.html',
#         'relations.html',
#         'searchbox.html',
#     ]
# }

# # # # # # # build the C docs and schema here # # # # # # #

# assuming doxygen is on PATH, but alter this locally if you need to point to a specific binary
DOXYGEN_BINARY = "doxygen"

# set up some file paths for convenience
this_file_path = Path(__file__)
repo_root = this_file_path.parent.parent.parent.parent
rtd_dir = this_file_path.parent.parent
doxygen_dir = rtd_dir / "doxygen"
doxygen_html_output_dir = doxygen_dir / "_build" / "html"
sphinx_dir = rtd_dir / "sphinx"
target_c_prebuilt_dir = sphinx_dir / "_build_c"

# test a file path to make sure we are in the right spot before trying to run
if doxygen_dir.exists():
    print("* Directory validation completed successfully")
else:
    raise Exception(f'Expected doxygen config dir to exist at "{doxygen_dir}" but it does not; aborting!')

# now try to run doxygen:
try:
    check_call([DOXYGEN_BINARY], cwd=doxygen_dir, stdout=DEVNULL, stderr=DEVNULL)
    print("* Doxygen completed successfully")
except CalledProcessError as e:
    raise Exception(f"Doxygen failed! Exception string: {str(e)}") from None
except FileNotFoundError as e:
    raise Exception(
        f"Doxygen binary not found, was it on path?  Looked for it at: {DOXYGEN_BINARY}; error = {str(e)}"
    ) from None

# ok, so doxygen should've run, validate the output directory exists
if doxygen_html_output_dir.exists():
    print("* Doxygen html output directory existence verified")
else:
    raise Exception(
        f"Although Doxygen appeared to run, the output directory is missing at {doxygen_html_output_dir}"
    ) from None

# alright, it exists, time to clean up the previous version, if it exists
if target_c_prebuilt_dir.exists():
    try:
        rmtree(target_c_prebuilt_dir)
        print("* Successfully deleted previous _build_c html directory")
    except Exception as e:
        raise Exception(f"Could not delete existing _build_c html directory") from None
else:
    print("* No _build_c directory to remove, skipping this step")

# ok, now just copy it
try:
    copytree(doxygen_html_output_dir, target_c_prebuilt_dir)
    print("* Successfully copied doxygen output directory to c_prebuilt directory")
except Exception as e:
    raise Exception(
        f"Could not copy doxygen output from '{doxygen_html_output_dir}' to '{target_c_prebuilt_dir}'"
    ) from None

# rename the root c file to index_c.html so it doesn't override the sphinx index.html file
os.rename(target_c_prebuilt_dir / "index.html", target_c_prebuilt_dir / "index_c.html")

print("* C Docs Complete!")

print("* Generating epJSON schema")
# # OK, now we need to make sure the epJSON schema is generated so we can process it
# Since this will primarily just be run by readthedocs, I'm just going to re-run the schema generator
try:
    check_call([sys.executable, "idd/schema/generate_epJSON_schema.py", "idd"], cwd=repo_root)
except CalledProcessError as e:
    raise Exception(f"Schema Generation failed! Exception string: {str(e)}") from None
except FileNotFoundError as e:
    raise Exception(f"python3 binary not found, what?  Looked for it at: `python3'; error = {str(e)}") from None

generated_schema_file = repo_root / "idd" / "Energy+.schema.epJSON"  # I know this will have CMake placeholders
if not generated_schema_file.exists():
    raise Exception("Generated schema file did not exist, aborting.")
print("* Generated schema existence confirmed")
with generated_schema_file.open() as f:
    o = load(f)

idf_objects: dict = o["properties"]
print("* Processing schema into RTD contents")
rtd_out = ""
object_entries_by_group = dict()

all_group_names = list()
for obj_name, data in idf_objects.items():
    group_name = data["group"]
    # wanting to persist order here, so just using list() not set()
    if group_name not in all_group_names:
        all_group_names.append(group_name)
    details = ""
    req_prefix = f":abbr:`🅁 (Required Object)` " if obj_name in o["required"] else f""
    formed_obj_name = f"{req_prefix}{obj_name}"
    details += f"{formed_obj_name}\n{'=' * len(formed_obj_name)}\n\n"
    if "memo" in data:
        memo = data["memo"].replace("*", "\\*").replace("|", "\\|")
        details += f"{memo}\n"
    pattern_props = data["patternProperties"]
    value_with_unknown_key = next(iter(pattern_props.values()))  # only key could be '.*' or something else
    fields: dict = value_with_unknown_key["properties"]
    required_fields = value_with_unknown_key["required"] if "required" in value_with_unknown_key else []
    details += "\n\n"
    field_type_icon = {
        "number": ":abbr:`Ⓝ (Numeric)`",
        "real": ":abbr:`Ⓝ (Numeric)`",
        "integer": ":abbr:`Ⓘ (Integer)`",
        "string": ":abbr:`Ⓢ (String)`",
        "array": ":abbr:`Ⓧ (Array)`",
        "unknown field type": ":abbr:`⍰ (Unknown type)`",
        "auto_size_numeric": ":abbr:`ⒶⓃ (Auto-sizable Numeric)`",
        "auto_size_integer": ":abbr:`ⒶⒾ (Auto-sizable Integer)`",
        "auto_calculate_numeric": ":abbr:`ⒶⓃ (Auto-calculable Numeric)`",
        "": ":abbr:`⍰ (Unknown type)`",
    }
    for field_name, field_data in fields.items():
        default_string = ""
        if "default" in field_data:
            if obj_name == "Version":
                default_string = " (Current Version)"
            else:
                default_data = str(field_data["default"]).replace("*", "\\*")
                default_string = f" (Default: {default_data})"
        field_type = field_data.get("type", "unknown field type")
        if field_type == "array" and "items" in field_data:
            array_required = field_data["items"].get("required", [])
            this_field_type = "Array of {"
            for i, variable_name in enumerate(field_data["items"]["properties"]):
                req_icon = f":abbr:`🅁 (Required Field)` " if variable_name in array_required else ""
                variable_value = field_data["items"]["properties"][variable_name]
                type_icon = field_type_icon[variable_value["type"]] if "type" in variable_value else ""
                nice_var_name = f"{req_icon}{type_icon} {variable_name}"
                this_field_type += ", " + nice_var_name if i > 0 else nice_var_name
            this_field_type += "}"
        else:  # could be a known type, auto-sizable numeric, or nothing
            if "anyOf" in field_data:
                auto_size_found = False
                auto_calculate_found = False
                numeric_found = False
                integer_found = False
                for option in field_data["anyOf"]:
                    if "type" in option:
                        if option["type"] == "number":
                            numeric_found = True
                        elif option["type"] == "integer":
                            integer_found = True
                        elif option["type"] == "string":
                            if "enum" in option:
                                if "Autosize" in option["enum"]:
                                    auto_size_found = True
                                elif "Autocalculate" in option["enum"]:
                                    auto_calculate_found = True
                if auto_size_found and numeric_found:
                    field_type = "auto_size_numeric"
                elif auto_calculate_found and numeric_found:
                    field_type = "auto_calculate_numeric"
                elif auto_size_found and integer_found:
                    field_type = "auto_size_integer"
                elif numeric_found:  # this handles goofy things like RefrigerationSystem:NumCompStages
                    field_type = "number"
            this_field_type = field_type_icon[field_type]
        req_prefix = f":abbr:`🅁 (Required Field)` " if field_name in required_fields else ""
        type_prefix = this_field_type
        details += f"* {req_prefix}{type_prefix} `{field_name}` {default_string}\n"
    final_object_snippet = details + "\n"
    if group_name in object_entries_by_group:
        object_entries_by_group[group_name].append(final_object_snippet)
    else:
        object_entries_by_group[group_name] = [final_object_snippet]

for group_name in all_group_names:
    # rtd_out += f"{group_name}\n{'*' * len(group_name)}\n\n"
    for obj_snippet in object_entries_by_group[group_name]:
        rtd_out += ".. rst-class:: special\n\n"
        rtd_out += obj_snippet

print("* Writing schema contents into template, saving at schema.rst")
schema_template = sphinx_dir / "in.schema.rst.in"
with open(schema_template) as template:
    template_text = template.read()

output_schema_file = sphinx_dir / "schema.rst"
with open(output_schema_file, "w") as out:
    out.write(template_text.replace("{REPLACE_ME}", rtd_out))

print("* Schema docs complete!")


# Options that I don't currently use but someone might need if they build the docs for a different target
htmlhelp_basename = "EnergyPlusPythonAPIDocumentationdoc"
latex_elements = {}
latex_documents = [
    (
        master_doc,
        "EnergyPlusPythonAPIDocumentation.tex",
        "EnergyPlus Python API Documentation Documentation",
        "National Renewable Energy Laboratory for the United States Department of Energy",
        "manual",
    ),
]
man_pages = [
    (master_doc, "energypluspythonapidocumentation", "EnergyPlus Python API Documentation Documentation", [author], 1)
]
texinfo_documents = [
    (
        master_doc,
        "EnergyPlusPythonAPIDocumentation",
        "EnergyPlus Python API Documentation Documentation",
        author,
        "EnergyPlusPythonAPIDocumentation",
        "One line description of project.",
        "Miscellaneous",
    ),
]
epub_title = project
epub_exclude_files = ["search.html"]
