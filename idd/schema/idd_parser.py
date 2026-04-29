# EnergyPlus, Copyright (c) 1996-present, The Board of Trustees of the
# University of Illinois, The Regents of the University of California, through
# Lawrence Berkeley National Laboratory (subject to receipt of any required
# approvals from the U.S. Dept. of Energy), Oak Ridge National Laboratory,
# managed by UT-Battelle, Alliance for Energy Innovation, LLC, and other
# contributors. All rights reserved.
#
# NOTICE: This Software was developed under funding from the U.S. Department of
# Energy and the U.S. Government consequently retains certain rights. As such,
# the U.S. Government has been granted for itself and others acting on its
# behalf a paid-up, nonexclusive, irrevocable, worldwide license in the
# Software to reproduce, distribute copies to the public, prepare derivative
# works, and perform publicly and display publicly, and to permit others to do
# so.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# (1) Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#
# (2) Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#
# (3) Neither the name of the University of California, Lawrence Berkeley
#     National Laboratory, the University of Illinois, U.S. Dept. of Energy nor
#     the names of its contributors may be used to endorse or promote products
#     derived from this software without specific prior written permission.
#
# (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in
#     stand-alone form without changes from the version obtained under this
#     License, or (ii) Licensee makes a reference solely to the software
#     portion of its product, Licensee must refer to the software as
#     "EnergyPlus version X" software, where "X" is the version number Licensee
#     obtained under this License and may not use a different name for the
#     software. Except as specifically required in this Section (4), Licensee
#     shall not use in a company name, a product name, in advertising,
#     publicity, or other promotional activities any name, trade name,
#     trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or
#     confusingly similar designation, without the U.S. Department of Energy's
#     prior written consent.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

import re


def _squiggle_context(data, problem_index, num_context_lines_around=5):
    lines = data.file.splitlines()
    guilty_line_index = 0
    count = 0
    squiggle_pos = None
    for i, line in enumerate(lines):
        count += len(line) + 1  # +1 for newline
        if count > problem_index:
            guilty_line_index = i
            squiggle_pos = problem_index - (count - len(line)) + 2
            break

    start_line = max(guilty_line_index - num_context_lines_around, 0)
    error_lines = lines[start_line : guilty_line_index + num_context_lines_around]
    squiggly_line = " " * squiggle_pos + "^" + "~" * (40 - squiggle_pos)
    error_lines.insert(guilty_line_index - start_line + 1, squiggly_line)
    return "\n".join(error_lines)


class IddParsingError(Exception):
    def __init__(self, message, data, problem_index=None, num_context_lines_around=5):
        """Constructor for an Exception with a context.

        @param message (str): the message to be printed
        @param: data (Data): for retrieving the context
        @param: problem_index (Optional[int]): if not provided, assumes this is the current data.index
        @param: num_context_lines_around (int): the number of lines to show before and after
        """
        if problem_index is None:
            problem_index = data.index
        message += "\nContext:\n" + _squiggle_context(
            data=data, problem_index=problem_index, num_context_lines_around=num_context_lines_around
        )
        # Call the base class constructor with the parameters it needs
        super().__init__(message)


class MissingSemiColonException(IddParsingError):
    pass


# Preston Shires

# tokens
TOKEN_NONE = 0
TOKEN_END = 1
TOKEN_EXCLAMATION = 2
TOKEN_COMMA = 3
TOKEN_SEMICOLON = 4
TOKEN_STRING = 5
TOKEN_NUMBER = 6
TOKEN_FIELD = 7
TOKEN_A = 8
TOKEN_N = 9
TOKEN_GROUP = 100

# object level tokens
TOKEN_MEMO = 10
TOKEN_MIN_FIELDS = 11
TOKEN_EXTENSIBLE = 12
TOKEN_FORMAT = 13
TOKEN_UNIQUE_OBJ = 14
TOKEN_REQUIRED_OBJ = 15
TOKEN_REFERENCE_CLASS_NAME = 17
TOKEN_OBSOLETE = 18

# field level tokens
TOKEN_KEY = 19
TOKEN_NOTE = 20
TOKEN_OBJ_LIST = 21
TOKEN_DEFAULT = 22
TOKEN_AUTOCALCULATABLE = 23
TOKEN_AUTOSIZABLE = 24
TOKEN_MIN = 25
TOKEN_MIN_EXCLUSIVE = 26
TOKEN_MAX = 27
TOKEN_MAX_EXCLUSIVE = 28
TOKEN_BEGIN_EXTENSIBLE = 29
TOKEN_UNITS = 30
TOKEN_TYPE = 31
TOKEN_REQUIRED_FIELD = 32
TOKEN_REFERENCE = 33
TOKEN_IP_UNITS = 34
TOKEN_UNITS_BASED_ON_FIELD = 35
TOKEN_DEPRECATED = 36
TOKEN_EXTERNAL_LIST = 37
TOKEN_RETAIN_CASE = 38

# above object level group string
GROUP_STR = "group"

# object level strings
EXTENSIBLE_STR = "extensible:"
MEMO_STR = "memo"
MIN_FIELDS_STR = "min-fields"
FORMAT_STR = "format"
FIELD_STR = "field"
UNIQUE_OBJ_STR = "unique-object"
REQUIRED_OBJ_STR = "required-object"
OBSOLETE_STR = "obsolete"

# field-level strings
REQUIRED_FIELD_STR = "required-field"
TYPE_STR = "type"
REFERENCE_STR = "reference"
REFERENCE_CLASS_NAME_STR = "-class-name"
KEY_STR = "key"
NOTE_STR = "note"
DEFAULT_STR = "default"
AUTOCALCULATABLE_STR = "autocalculatable"
AUTOSIZABLE_STR = "autosizable"
MINIMUM_STR = "minimum"
MAXIMUM_STR = "maximum"
BEGIN_EXTENSIBLE_STR = "begin-extensible"
UNITS_STR = "units"
IP_UNITS_STR = "ip-units"
BASED_ON_FIELD_STR = "BasedOnField"
DEPRECATED_STR = "deprecated"
RETAIN_CASE_STR = "retaincase"

# field-level type strings
ALPHA_STR = "alpha"
CHOICE_STR = "choice"
OBJECT_LIST_STR = "object-list"
EXTERNAL_LIST_STR = "external-list"
REAL_STR = "real"
INTEGER_STR = "integer"
NODE_STR = "node"


class Data:
    index = 0
    file = None
    file_size = 0
    schema = None


def parse_idd(data):
    root = {"$schema": "https://json-schema.org/draft-07/schema#", "properties": {}}
    data.file_size = len(data.file)
    current_group_name = "**ungrouped**"

    while data.index < data.file_size:
        token = look_ahead(data)
        if token == TOKEN_END:
            break
        elif token == TOKEN_NONE:
            print("TOKEN_NONE returned in parse_idd at index ", data.index, data.file[data.index - 50 : data.index])
            break
        elif token == TOKEN_EXCLAMATION:
            eat_comment(data)
        elif token == TOKEN_GROUP:
            next_token(data)
            current_group_name = parse_line(data)
        else:
            obj_name = parse_string(data)
            if obj_name is None or obj_name == "":
                return root
            obj_data = parse_obj(data, obj_name=obj_name)
            root["properties"][obj_name] = {}
            root["properties"][obj_name]["patternProperties"] = {}
            root["properties"][obj_name]["group"] = current_group_name

            name_pattern_properties = ".*"
            if "name" in obj_data:
                name_data = obj_data.pop("name")
                root["properties"][obj_name]["name"] = name_data
                required_name = name_data.get("is_required", False)
                if required_name:
                    name_pattern_properties = R"^.*\S.*$"
                    root["properties"][obj_name]["additionalProperties"] = False

            root["properties"][obj_name]["patternProperties"][name_pattern_properties] = obj_data

            root["properties"][obj_name]["legacy_idd"] = obj_data.pop("legacy_idd")
            root["properties"][obj_name]["type"] = "object"
            if "is_required_obj" in obj_data:
                obj_data.pop("is_required_obj")
                root["properties"][obj_name]["minProperties"] = 1
                if "required" not in root:
                    root["required"] = []
                root["required"].append(obj_name)
            if "is_unique_obj" in obj_data:
                obj_data.pop("is_unique_obj")
                root["properties"][obj_name]["maxProperties"] = 1
            if "memo" in obj_data:
                root["properties"][obj_name]["memo"] = obj_data.pop("memo")
            if "min_fields" in obj_data:
                num_fields_with_name = len(obj_data["properties"]) + 1
                if int(obj_data["min_fields"]) > num_fields_with_name:
                    if "extensions" not in obj_data["properties"]:
                        raise RuntimeError("Object with min-fields > num_fields. Object name = " + obj_name)
                root["properties"][obj_name]["min_fields"] = obj_data.pop("min_fields")
            if "extensible_size" in obj_data:
                root["properties"][obj_name]["extensible_size"] = obj_data.pop("extensible_size")
            if "format" in obj_data:
                root["properties"][obj_name]["format"] = obj_data.pop("format")

            for name, field in obj_data["properties"].items():
                if "unitsBasedOnField" in field:
                    based_on_field = field["unitsBasedOnField"]
                    index = int(based_on_field[1]) - 1
                    if based_on_field[0].lower() == "a":
                        field["unitsBasedOnField"] = root["properties"][obj_name]["legacy_idd"]["alphas"]["fields"][
                            index
                        ]
                    else:
                        field["unitsBasedOnField"] = root["properties"][obj_name]["legacy_idd"]["numerics"]["fields"][
                            index
                        ]

    data.schema = root


def parse_obj(data, obj_name):
    """Parse an object info.

    @param obj_name (str): is used for reporting errors
    """
    root = {
        "type": "object",
        "properties": {},
        "legacy_idd": {"field_info": {}, "fields": [], "alphas": {"fields": []}, "numerics": {"fields": []}},
    }
    extensible_count = 0
    duplicate_field_count = 0

    a_field_numbers = []
    n_field_numbers = []

    while True:
        token = look_ahead(data)
        if token == TOKEN_NONE:
            raise RuntimeError("TOKEN_NONE returned")

        elif token == TOKEN_END:
            return root

        elif token == TOKEN_MEMO:
            next_token(data)
            memo = parse_line(data)
            if "memo" in root:
                root["memo"] += " " + memo
            else:
                root["memo"] = memo

        elif token == TOKEN_EXCLAMATION:
            eat_comment(data)

        elif token == TOKEN_REQUIRED_OBJ:
            next_token(data)
            root["is_required_obj"] = True

        elif token == TOKEN_UNIQUE_OBJ:
            next_token(data)
            root["is_unique_obj"] = True

        elif token == TOKEN_EXTENSIBLE:
            next_token(data)
            if look_ahead(data) != TOKEN_NUMBER:
                raise RuntimeError("expected number after extensible:")
            num = parse_number(data)
            if num is None:
                raise RuntimeError("parse number returned None")
            root["extensible_size"] = num
            eat_comment(data)  # eat useless extensible string

        elif token == TOKEN_MIN_FIELDS:
            next_token(data)
            if look_ahead(data) != TOKEN_NUMBER:
                raise RuntimeError("expected number after /min-fields")
            num = parse_number(data)
            if num is None:
                raise RuntimeError("parse number returned None")
            if "min_fields" in root:
                raise KeyError("min_fields already exists, must define only one per object")
            root["min_fields"] = num

        elif token == TOKEN_FORMAT:
            next_token(data)
            if look_ahead(data) != TOKEN_STRING:
                raise RuntimeError("expected string after /format")
            if "format" in root:
                raise RuntimeError("cannot have duplicate formats")
            root["format"] = parse_line(data)

        elif token == TOKEN_OBSOLETE:
            next_token(data)
            if look_ahead(data) != TOKEN_STRING:
                raise IddParsingError(
                    message="In object '{obj_name}', expected string after /obsolete".format(
                        obj_name=obj_name,
                    ),
                    data=data,
                )
            if "obsolete" in root:
                raise IddParsingError("cannot have duplicate obsolete")
            root["obsolete"] = parse_line(data)

        elif token == TOKEN_A or token == TOKEN_N:  # past all object level comments, should be field now
            token_a_or_n = token
            eat_whitespace(data)
            field_number = data.file[data.index]
            data.index += 1
            field_index = data.index
            field_number += str(int(parse_number(data)))
            if token == TOKEN_A:
                if field_number in a_field_numbers:
                    raise IddParsingError(
                        message="In object '{obj_name}', duplicate field number {field_number}".format(
                            obj_name=obj_name, field_number=field_number
                        ),
                        data=data,
                    )
                a_field_numbers.append(field_number)
            if token == TOKEN_N:
                if field_number in n_field_numbers:
                    raise IddParsingError(
                        message="In object '{obj_name}', duplicate field number {field_number}".format(
                            obj_name=obj_name, field_number=field_number
                        ),
                        data=data,
                    )
                n_field_numbers.append(field_number)
            comma_or_semicolon = look_ahead(data)
            if comma_or_semicolon != TOKEN_COMMA and comma_or_semicolon != TOKEN_SEMICOLON:
                raise IddParsingError(
                    message="In object '{obj_name}', No comma or semicolon after field {field_number}".format(
                        obj_name=obj_name,
                        field_number=field_number,
                    ),
                    data=data,
                    problem_index=field_index,
                )
            next_token(data)
            token = look_ahead(data)

            if token == TOKEN_A or token == TOKEN_N:
                next_token(data)
                comma_or_semicolon = next_token(data)
                token = look_ahead(data)
                if comma_or_semicolon == TOKEN_SEMICOLON:
                    eat_comment(data)
                    return root
                if token != TOKEN_A and token != TOKEN_N:
                    eat_comment(data)
                continue

            if token == TOKEN_NOTE:
                eat_comment(data)
                if comma_or_semicolon == TOKEN_SEMICOLON:
                    return root
                continue

            if token != TOKEN_FIELD:
                eat_comment(data)
                if comma_or_semicolon == TOKEN_SEMICOLON:
                    return root
                raise RuntimeError("expected /field after , or ;")
            next_token(data)
            field_name = parse_line(data)
            original_field_name = field_name
            field_name = field_name.lower()
            field_data = parse_field(data, token_a_or_n)

            if "begin-extensible" in field_data:
                field_data.pop("begin-extensible")
                field_name = re.sub("[ ]+[0-9]+", "", field_name)
                original_field_name = re.sub("[ ]+[0-9]+", "", original_field_name)
                field_name = re.sub("[^0-9a-zA-Z]+", "_", field_name)
                append_field_number(root, field_number, field_name, True, original_field_name)
                root["properties"]["extensions"] = {"type": "array", "items": {"properties": {}, "type": "object"}}
                root["legacy_idd"]["extensibles"] = [field_name]
                parse_extensibles(root["properties"]["extensions"]["items"], field_data, field_name)
                extensible_count += 1
                if comma_or_semicolon == TOKEN_SEMICOLON:
                    return root
                continue
            elif (
                "extensible_size" in root
                and extensible_count >= root["extensible_size"]
                and comma_or_semicolon != TOKEN_SEMICOLON
            ):
                continue
            elif extensible_count > 0:
                if extensible_count < root["extensible_size"]:
                    field_name = re.sub("[ ]+[0-9]+", "", field_name)
                    original_field_name = re.sub("[ ]+[0-9]+", "", original_field_name)
                    field_name = re.sub("[^0-9a-zA-Z]+", "_", field_name)
                    append_field_number(root, field_number, field_name, True, original_field_name)
                    root["legacy_idd"]["extensibles"].append(field_name)
                    parse_extensibles(root["properties"]["extensions"]["items"], field_data, field_name)
                    extensible_count += 1
                if comma_or_semicolon == TOKEN_SEMICOLON:
                    return root
                continue

            field_name = re.sub("[^0-9a-zA-Z]+", "_", field_name)
            if field_name in root["properties"]:
                duplicate_field_count += 1
                field_name += "_" + str(duplicate_field_count)
            append_field_number(root, field_number, field_name, False, original_field_name)
            root["legacy_idd"]["fields"].append(field_name)
            if field_name != "name":
                root["properties"][field_name] = field_data
                is_required = field_data.get("is_required", False)
                if is_required:
                    field_data.pop("is_required")
                    if "default" not in field_data:
                        if "required" not in root:
                            root["required"] = []
                        root["required"].append(field_name)

            else:
                root["name"] = field_data

            if comma_or_semicolon == TOKEN_SEMICOLON:
                return root

            if data.file[data.index : data.index + 2] == "\n\n":
                raise MissingSemiColonException(
                    message="In object '{obj_name}', Missing semi-colon in field {field_number}".format(
                        obj_name=obj_name, field_number=field_number
                    ),
                    data=data,
                    problem_index=field_index,
                )


def parse_field(data, token):
    root = {}
    is_autocalculatable = False
    is_autosizable = False

    if token == TOKEN_A:
        root["type"] = "string"
    else:
        root["type"] = "number"

    while True:
        token = look_ahead(data)
        if token == TOKEN_NONE:
            raise RuntimeError("token none returned in parse field")

        elif token == TOKEN_EXCLAMATION:
            next_token(data)
            eat_comment(data)

        elif token == TOKEN_REQUIRED_FIELD:
            next_token(data)
            root["is_required"] = True

        elif token == TOKEN_EXCLAMATION:
            next_token(data)
            eat_comment(data)

        elif token == TOKEN_TYPE:  # default is string, so only change if type: array etc?
            next_token(data)
            if match_string(data, ALPHA_STR) or match_string(data, CHOICE_STR):
                if "type" not in root or root["type"] != "string":
                    root["type"] = "string"
            elif match_string(data, OBJECT_LIST_STR):
                if "data_type" not in root:
                    root["data_type"] = "object_list"
                else:
                    raise RuntimeError("two object-lists?")
            elif match_string(data, EXTERNAL_LIST_STR):
                if "data_type" not in root:
                    root["data_type"] = "external_list"
                else:
                    raise RuntimeError("Two external-lists?")
            elif match_string(data, REAL_STR):
                if "type" not in root or root["type"] != "number":
                    root["type"] = "number"
            elif match_string(data, INTEGER_STR):
                if "type" not in root or root["type"] != "integer":
                    root["type"] = "integer"
            elif match_string(data, NODE_STR):
                root["type"] = "string"
            else:
                bad_type = parse_line(data)
                raise RuntimeError('Invalid \\type: "%s"' % bad_type)

        elif token == TOKEN_OBJ_LIST:
            next_token(data)
            if "object_list" not in root:
                root["object_list"] = []
            root["object_list"].append(parse_line(data))
            root["object_list"].sort()

        elif token == TOKEN_EXTERNAL_LIST:
            next_token(data)
            if "external_list" not in root:
                root["external_list"] = []
            root["external_list"].append(parse_line(data))
            root["external_list"].sort()

        elif token == TOKEN_DEFAULT:
            next_token(data)
            default = parse_number(data)
            if default is None:
                default = parse_line(data)
                if default.lower() == "autocalculate":
                    default = "Autocalculate"
                elif default.lower() == "autosize":
                    default = "Autosize"
            if "default" in root:
                raise RuntimeError("cannot have two defaults at " + default)
            else:
                root["default"] = default

        elif token == TOKEN_NOTE or token == TOKEN_MEMO:
            next_token(data)
            note = parse_line(data)
            if "note" in root:
                root["note"] += " " + note
            else:
                root["note"] = note

        elif token == TOKEN_KEY:
            next_token(data)
            if "enum" not in root:
                root["enum"] = []
            root["enum"].append(parse_line(data))
            root["enum"].sort()

        elif token == TOKEN_REFERENCE or token == TOKEN_REFERENCE_CLASS_NAME:
            token_str = "reference" if token == TOKEN_REFERENCE else "reference-class-name"
            next_token(data)
            token = look_ahead(data)
            if token == TOKEN_STRING:
                reference = parse_line(data)
            elif token == TOKEN_NUMBER:
                reference = parse_number(data)
            else:
                raise RuntimeError("Expected string or number after \\" + token_str)
            if token_str not in root:
                root[token_str] = []
            root[token_str].append(reference)
            root[token_str].sort()

        elif token == TOKEN_AUTOCALCULATABLE:
            next_token(data)
            is_autocalculatable = True

        elif token == TOKEN_AUTOSIZABLE:
            next_token(data)
            is_autosizable = True

        elif token == TOKEN_MAX or token == TOKEN_MAX_EXCLUSIVE:
            next_token(data)
            if token == TOKEN_MAX_EXCLUSIVE:
                root["exclusiveMaximum"] = parse_number(data)
            else:
                root["maximum"] = parse_number(data)

        elif token == TOKEN_MIN or token == TOKEN_MIN_EXCLUSIVE:
            next_token(data)
            if token == TOKEN_MIN_EXCLUSIVE:
                root["exclusiveMinimum"] = parse_number(data)
            else:
                root["minimum"] = parse_number(data)

        elif token == TOKEN_BEGIN_EXTENSIBLE:
            next_token(data)
            root["begin-extensible"] = True

        elif token == TOKEN_UNITS:
            next_token(data)
            root["units"] = parse_line(data)  # WHITESPACE TRAILING!!!

        elif token == TOKEN_UNITS_BASED_ON_FIELD:
            next_token(data)
            root["unitsBasedOnField"] = parse_line(data)

        elif token == TOKEN_IP_UNITS:
            next_token(data)
            root["ip-units"] = parse_line(data)  # WHITESPACE TRAILING!!!

        elif token == TOKEN_DEPRECATED:
            next_token(data)
            root["deprecated"] = True

        elif token == TOKEN_RETAIN_CASE:
            next_token(data)
            root["retaincase"] = True

        elif token in [TOKEN_A, TOKEN_N, TOKEN_END, TOKEN_STRING, TOKEN_GROUP]:
            has_default = "default" in root
            if is_autocalculatable:
                create_any_of(root, TOKEN_AUTOCALCULATABLE, has_default)
            elif is_autosizable:
                create_any_of(root, TOKEN_AUTOSIZABLE, has_default)
            if "enum" in root and has_default:
                root["enum"].insert(0, "")
                root["enum"].sort()
            if "type" in root and root["type"] == "integer":
                # Go back and make sure default, min/max are integers for integer fields
                # Handle default slightly differently
                if "default" in root:
                    if root["default"] in ["Autocalculate", "Autosize"]:
                        continue
                    if not root["default"].is_integer():
                        raise RuntimeError("found float default for integer field")
                    root["default"] = int(root["default"])
                # Now for the min/max items
                for fld in ["minimum", "exclusiveMinimum", "maximum", "exclusiveMaximum"]:
                    if fld in root:
                        if not root[fld].is_integer():
                            raise RuntimeError("found float %s for integer field" % fld)
                        root[fld] = int(root[fld])
            return root


def append_field_number(root, field_number, field_name, is_in_extensibles, original_name):
    if not is_in_extensibles:
        if field_number[0].lower() == "a":
            root["legacy_idd"]["alphas"]["fields"].append(field_name)
            root["legacy_idd"]["field_info"][field_name] = {"field_name": original_name, "field_type": "a"}
        elif field_number[0].lower() == "n":
            root["legacy_idd"]["numerics"]["fields"].append(field_name)
            root["legacy_idd"]["field_info"][field_name] = {"field_name": original_name, "field_type": "n"}
    else:
        if field_number[0].lower() == "a":
            if "extensions" not in root["legacy_idd"]["alphas"]:
                root["legacy_idd"]["alphas"]["extensions"] = []
            root["legacy_idd"]["alphas"]["extensions"].append(field_name)
            if field_name not in root["legacy_idd"]["field_info"]:
                root["legacy_idd"]["field_info"][field_name] = {"field_name": original_name, "field_type": "a"}
        elif field_number[0].lower() == "n":
            if "extensions" not in root["legacy_idd"]["numerics"]:
                root["legacy_idd"]["numerics"]["extensions"] = []
            root["legacy_idd"]["numerics"]["extensions"].append(field_name)
            if field_name not in root["legacy_idd"]["field_info"]:
                root["legacy_idd"]["field_info"][field_name] = {"field_name": original_name, "field_type": "n"}


def parse_extensibles(items, field_data, field_name):
    if "is_required" in field_data:
        field_data.pop("is_required")
        if "required" not in items:
            items["required"] = []
        items["required"].append(field_name)
        items["required"].sort()
    items["properties"][field_name] = field_data


def create_any_of(root, token, has_default):
    root["anyOf"] = [{}, {}]

    if "type" in root:
        root["anyOf"][0]["type"] = root.pop("type")
    else:
        raise RuntimeError("shouldn't be in create_any_of, type was string")
    if "minimum" in root:
        root["anyOf"][0]["minimum"] = root.pop("minimum")
    if "exclusiveMinimum" in root:
        root["anyOf"][0]["exclusiveMinimum"] = root.pop("exclusiveMinimum")
    if "maximum" in root:
        root["anyOf"][0]["maximum"] = root.pop("maximum")
    if "exclusiveMaximum" in root:
        root["anyOf"][0]["exclusiveMaximum"] = root.pop("exclusiveMaximum")

    enum_list = []
    if has_default:
        enum_list.append("")
    if token == TOKEN_AUTOCALCULATABLE:
        enum_list.append("Autocalculate")
    else:
        enum_list.append("Autosize")
    root["anyOf"][1] = {"type": "string", "enum": enum_list}


def look_ahead(data):
    save_index = data.index
    token = next_token(data)
    data.index = save_index
    return token


def next_token(data):
    eat_whitespace(data)
    if data.index == data.file_size:
        return TOKEN_END

    c = data.file[data.index]
    data.index += 1
    if c == "!":
        return TOKEN_EXCLAMATION
    elif c == ",":
        return TOKEN_COMMA
    elif c == ";":
        return TOKEN_SEMICOLON
    elif c == "\\":
        if match_string(data, MEMO_STR):
            return TOKEN_MEMO
        if match_string(data, FIELD_STR):
            return TOKEN_FIELD
        if match_string(data, FORMAT_STR):
            return TOKEN_FORMAT
        if match_string(data, MIN_FIELDS_STR):
            return TOKEN_MIN_FIELDS
        if match_string(data, REQUIRED_OBJ_STR):
            return TOKEN_REQUIRED_OBJ
        if match_string(data, UNIQUE_OBJ_STR):
            return TOKEN_UNIQUE_OBJ
        if match_string(data, EXTENSIBLE_STR):
            return TOKEN_EXTENSIBLE
        if match_string(data, TYPE_STR):
            return TOKEN_TYPE
        if match_string(data, REFERENCE_STR):
            if match_string(data, REFERENCE_CLASS_NAME_STR):
                return TOKEN_REFERENCE_CLASS_NAME
            return TOKEN_REFERENCE
        if match_string(data, REQUIRED_FIELD_STR):
            return TOKEN_REQUIRED_FIELD
        if match_string(data, KEY_STR):
            return TOKEN_KEY
        if match_string(data, NOTE_STR):
            return TOKEN_NOTE
        if match_string(data, OBJECT_LIST_STR):
            return TOKEN_OBJ_LIST
        if match_string(data, EXTERNAL_LIST_STR):
            return TOKEN_EXTERNAL_LIST
        if match_string(data, DEFAULT_STR):
            return TOKEN_DEFAULT
        if match_string(data, AUTOCALCULATABLE_STR):
            return TOKEN_AUTOCALCULATABLE
        if match_string(data, AUTOSIZABLE_STR):
            return TOKEN_AUTOSIZABLE
        if match_string(data, BEGIN_EXTENSIBLE_STR):
            return TOKEN_BEGIN_EXTENSIBLE
        if match_string(data, DEPRECATED_STR):
            return TOKEN_DEPRECATED
        if match_string(data, OBSOLETE_STR):
            return TOKEN_OBSOLETE
        if match_string(data, RETAIN_CASE_STR):
            return TOKEN_RETAIN_CASE
        if match_string(data, GROUP_STR):
            return TOKEN_GROUP
        if match_string(data, MINIMUM_STR):
            if data.file[data.index] == ">":
                data.index += 1
                return TOKEN_MIN_EXCLUSIVE
            elif data.file[data.index] == " " and data.file[data.index + 1] == ">":
                data.index += 2
                return TOKEN_MIN_EXCLUSIVE
            else:
                return TOKEN_MIN
        if match_string(data, MAXIMUM_STR):
            if data.file[data.index] == "<":
                data.index += 1
                return TOKEN_MAX_EXCLUSIVE
            elif data.file[data.index] == " " and data.file[data.index + 1] == "<":
                data.index += 2
                return TOKEN_MAX_EXCLUSIVE
            else:
                return TOKEN_MAX
        if match_string(data, IP_UNITS_STR):
            return TOKEN_IP_UNITS
        if match_string(data, UNITS_STR):
            if match_string(data, BASED_ON_FIELD_STR):
                return TOKEN_UNITS_BASED_ON_FIELD
            return TOKEN_UNITS

        return TOKEN_NONE
    elif c.isalpha() or c in r'-:.#/\[]{}_@$%^&*()|+=<>?\'"~':
        if c == "A":
            if parse_number(data) is not None:
                return TOKEN_A
        elif c == "N":
            if parse_number(data) is not None:
                return TOKEN_N
        return TOKEN_STRING
    elif c.isdigit():
        return TOKEN_NUMBER
    else:
        data.index -= 1
        return TOKEN_NONE


def parse_line(data):
    string = ""
    if data.file[data.index] == "\n":
        return string
    eat_whitespace(data)
    while data.index < data.file_size and data.file[data.index] not in "\n\r":
        string += data.file[data.index]
        data.index += 1
    return string


def parse_number(data):
    eat_whitespace(data)
    save_index = data.index
    if data.file[save_index] not in "-+0123456789.":
        return None
    num = ""
    while data.file[save_index] not in " \n\t\r,;":
        if data.file[save_index] not in "-+0123456789.eE":
            return None
        num += data.file[save_index]
        save_index += 1

    data.index += len(num)
    return float(num)


def match_string(data, string):
    eat_whitespace(data)
    save_index = data.index
    if save_index + len(string) > data.file_size:
        return False
    for j in range(len(string)):
        if data.file[save_index].lower() != string[j].lower():
            return False
        save_index += 1

    data.index += len(string)  # if matched, move data.index to end of string
    return True


def parse_string(data):
    eat_whitespace(data)
    s = ""
    while data.index < data.file_size:
        c = data.file[data.index]
        if c == "," or c == ";" or c == "!":
            data.index += 1
            return s
        s += c
        data.index += 1


def eat_whitespace(data):
    while data.index < data.file_size:
        if data.file[data.index] in " \t\n\r":
            data.index += 1
        else:
            break


def eat_comment(data):
    if data.file[data.index] == "\n":
        data.index += 1
        return
    while True:
        data.index += 1
        if data.index == data.file_size or data.file[data.index] == "\n":
            return
