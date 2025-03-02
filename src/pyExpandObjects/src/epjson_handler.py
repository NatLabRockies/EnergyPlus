import pathlib
import re
import json
import jsonschema
import copy
from pathlib import Path
from custom_exceptions import PyExpandObjectsFileNotFoundError, PyExpandObjectsSchemaError, \
    PyExpandObjectsTypeError, UniqueNameException, InvalidEpJSONException
from logger import Logger

this_script_path = Path(__file__).resolve()


class EPJSON(Logger):
    """
    Handle epJSON and JSON specific tasks

    Attributes:
        Validator: schema validator from jsonschema

        schema: loaded schema.  Only validated schemas will be loaded.

        input_epjson: input epjson file

        schema_is_valid: initialized as None.  False if failed, True if passed.

        input_epjson_is_valid: initialized as None.  False if failed, True if passed.
    """

    def __init__(self, no_schema=False, logger_level="WARNING", logger_name='console_only_logger', reset_stream=False):
        super().__init__(logger_level=logger_level, logger_name=logger_name, reset_stream=reset_stream)
        self.no_schema = no_schema
        self.schema = None
        self.Validator = jsonschema.Draft4Validator
        self.schema_is_valid = None
        self.input_epjson = None
        self.input_epjson_is_valid = None
        if self.no_schema:
            self.logger.warning(
                'Warning: Schema validation has been turned off.  There will be no invalid choice error messages '
                'generated, which may lead to unexplained errors')
        return

    @staticmethod
    def merge_epjson(
            super_dictionary: dict,
            object_dictionary: dict,
            unique_name_override: bool = False,
            unique_name_fail: bool = True):
        """
        Merge a high level formatted dictionary with a sub-dictionary, both in epJSON format

        :param super_dictionary: high level dictionary used as the base object
        :param object_dictionary: dictionary to merge into base object
        :param unique_name_override: allow a duplicate unique name to overwrite an existing object
        :param unique_name_fail: if override is set to False, choose whether to skip object or fail
        :return: merged output of the two input dictionaries.  Note, the super_dictionary is modified in this operation.
            A copy operation was not performed intentionally.  If the user wants the original super_dictionary
            to remain unchanged then a copy.deepcopy() should be performed before running the function.
        """
        if object_dictionary:
            for object_type, object_structure in object_dictionary.items():
                if not super_dictionary.get(object_type):
                    super_dictionary[object_type] = {}
                if isinstance(object_structure, dict):
                    for object_name, object_fields in object_structure.items():
                        if not unique_name_override and object_name in super_dictionary[object_type].keys():
                            # Raise exception if the object name already exists in the target dictionary.  One exception
                            # to this rule is to ignore Schedule:Compact objects with template names.  This is allowed
                            # to make it easier to specify schedules while building objects without needing to turn
                            # off the unique name check entirely.
                            if unique_name_fail and not (
                                    re.match(
                                        r'HVACTemplate-Always',
                                        object_name,
                                        re.IGNORECASE) and object_type.lower() == 'schedule:compact'
                            ):
                                raise UniqueNameException("Unique name {} already exists in object {}".format(
                                    object_name,
                                    object_type
                                ))
                            else:  # pragma: loop bug
                                # if two unique names are present and the user has opted to not fail on this condition
                                # and not override the existing object, then skip it.
                                continue
                        super_dictionary[object_type][object_name] = object_fields
                else:
                    raise PyExpandObjectsTypeError(
                        'An Invalid object {} failed to merge'.format(object_structure))
        return

    @staticmethod
    def summarize_epjson(epjson):
        """
        Provide summary of epJSON dictionary for comparisons and metrics.

        :param epjson: epJSON formatted dictionary
        :return: dictionary of count summaries
        """
        output = {}
        for object_type, epjson_objects in epjson.items():
            for _, _ in epjson_objects.items():
                if not output.get(object_type):
                    output[object_type] = 1
                else:
                    output[object_type] = output[object_type] + 1
        return output

    @staticmethod
    def purge_epjson(epjson, purge_dictionary=None):
        """
        Remove objects in an input epJSON object.
        :param epjson: input epJSON
        :param purge_dictionary: key-value pair of object_type and list of regular expressions to remove items
            (.* removes all objects)
        :return: epJSON with items referenced in purge_dictionary removed.
        """
        tmp_d = copy.deepcopy(epjson)
        if purge_dictionary:
            for object_type, object_structure in epjson.items():
                if object_type in purge_dictionary.keys():
                    for object_name, object_fields in object_structure.items():
                        # if the purge_dictionary value is a string, then it is a single regex to be processed, so
                        #  convert it to a list and continue
                        if isinstance(purge_dictionary[object_type], str):
                            purge_dictionary[object_type] = [purge_dictionary[object_type], ]
                        # iterate over the list of regex strings in purge_dictionary value and pop the object if there
                        # is a match.
                        for rgx_match in purge_dictionary[object_type]:
                            if re.match(rgx_match, object_name):
                                tmp_d[object_type].pop(object_name)
                # if the object_type is now empty, delete it as well.
                if not tmp_d[object_type].keys():
                    tmp_d.pop(object_type)
        return tmp_d

    @staticmethod
    def epjson_genexp(epjson):
        """
        Create generator of individual epJSON objects in epJSON format from a dictionary of objects in epJSON format.

        {object_type: {object_name: object_fields}, {...}} -> {object_type: {object_name: object_fields}}, {...}

        :param epjson: epJSON object
        :return: generator which returns one unique object in epJSON format for each object in an object_type.
        """
        for object_type, epjson_objects in epjson.items():
            for object_name, object_structure in epjson_objects.items():
                yield {object_type: {object_name: object_structure}}
        return None

    @staticmethod
    def _get_json_file(json_location=None):
        """
        Load json file and return an error and None if fails

        :param json_location: file location for json object
        :return: loaded json object
        """
        if not isinstance(json_location, (str, pathlib.PosixPath, pathlib.WindowsPath)):
            raise PyExpandObjectsFileNotFoundError("JSON file location input is not a string: {}".format(json_location))
        try:
            with open(json_location) as f:
                json_obj = json.load(f)
            return json_obj
        except FileNotFoundError:
            raise PyExpandObjectsFileNotFoundError("file does not exist: {}".format(json_location))
        except json.decoder.JSONDecodeError as e:
            raise PyExpandObjectsTypeError("file is not a valid json: {}\n{}".format(json_location, str(e)))

    def get_epjson_objects(
            self, epjson: dict,
            object_type_regexp: str = '.*',
            object_name_regexp: str = '.*') -> dict:
        """
        Get objects from epJSON dictionary after filtering by object type and name.

        :param epjson: epJSON formatted Dictionary to scan
        :param object_type_regexp: regular expression to match with object type
        :param object_name_regexp: regular expression to match with object_name
        :return: epJSON dictionary of matched objects.
        """
        matched_epjson = {}
        try:
            for object_type, objects_structure in epjson.items():
                if re.match(object_type_regexp, object_type, re.IGNORECASE):
                    for object_name, object_structure in objects_structure.items():
                        if re.match(object_name_regexp, object_name, re.IGNORECASE):
                            self.merge_epjson(
                                super_dictionary=matched_epjson,
                                object_dictionary={object_type: {object_name: object_structure}}
                            )
            return matched_epjson
        except (ValueError, AttributeError, KeyError):
            raise InvalidEpJSONException('Invalid epJSON formatted object: {}'.format(epjson))

    def _validate_schema(self, schema):
        """
        Validate schema based on the loaded
        jsonschema pre-built validator (self.Validator)

        :param schema: loaded schema object
        :return: validated schema object.  object and boolean are added to class attributes.
        """
        try:
            self.Validator.check_schema(schema)
            validated_schema = self.Validator(schema)
            self.logger.info('schema version: %s', schema['epJSON_schema_version'])
            self.logger.info('schema build: %s', schema['epJSON_schema_build'])
            setattr(self, 'schema_is_valid', True)
            setattr(self, 'schema', validated_schema)
            return validated_schema
        except jsonschema.exceptions.SchemaError as e:
            raise PyExpandObjectsSchemaError(e.message)
        except Exception as e:
            raise PyExpandObjectsSchemaError("Schema Validator Failed: {}".format(str(e)))

    def _load_schema(self, schema_ref=None):
        """
        Load schema to class object.

        :param schema_ref: (Optional) location of json schema or dictionary object.  If not provided
            then the default relative path and file (Energy+.schema.epJSON) will be used.

        :return: Validated schema and boolean flag as class attributes
        """
        if self.no_schema:
            self.schema = False
            self.schema_is_valid = False
        else:
            if isinstance(schema_ref, dict):
                schema = schema_ref
            else:
                # load schema from default if location is not provided.
                if not schema_ref:
                    try:
                        schema_ref = str(this_script_path.parent / 'resources' / 'Energy+.schema.epJSON')
                    except FileNotFoundError:
                        raise PyExpandObjectsFileNotFoundError('Schema default file path is not valid; \n%s')
                schema = self._get_json_file(schema_ref)
            self._validate_schema(schema)
            self.logger.info('Schema loaded')
        return

    def validate_epjson(self, epjson):
        """
        Validate json object as epJSON.  Return object if valid

        :param epjson: epJSON object
        :return: validated epJSON object
        """
        building_object = epjson.get('Building')
        global_geometry_rules_object = epjson.get('GlobalGeometryRules')
        if not building_object or not global_geometry_rules_object:
            raise PyExpandObjectsSchemaError('Building or GlobalGeometryRules object missing')
        for object_type, object_structure in epjson.items():
            if object_type not in ['Building', 'GlobalGeometryRules']:
                epjson_object = {'Building': building_object, 'GlobalGeometryRules': global_geometry_rules_object}
                self.merge_epjson(
                    super_dictionary=epjson_object,
                    object_dictionary={object_type: object_structure})
                try:
                    file_validation = self.schema.is_valid(epjson_object)
                    if not file_validation:
                        (object_name, _), = object_structure.items()
                        # if the schema validation fails for the epJSON object, write out specific errors that occurred.
                        self.logger.error("Error: Input file does not meet schema format")
                        for err in self.schema.iter_errors(epjson_object):
                            if re.match(r'.*{.*}.*', err.message):
                                msg = '. '.join([
                                    err.message,
                                    '.  It appears a complex YAML reference was not resolved.'])
                            else:
                                msg = err.message
                            self.logger.error('Error: Invalid choice in {} ({}). {}'
                                              .format(object_type, object_name, msg))
                    else:
                        continue
                except Exception as e:
                    raise PyExpandObjectsSchemaError("epJSON validation failed while processing {}: {}"
                                                     .format(object_type, str(e)))
        return epjson

    def _validate_epjson(self, input_epjson):
        """
        Validate json file based on loaded schema.  I schema validation is off, then will return True for any
        dictionary.

        :param input_epjson: epJSON object
        :return: validated epJSON object.  object and boolean flag added to class attributes.
        """
        if self.no_schema:
            if isinstance(input_epjson, dict):
                setattr(self, 'input_epjson_is_valid', True)
                setattr(self, 'input_epjson', input_epjson)
                return input_epjson
            else:
                raise PyExpandObjectsTypeError("input epJSON is not a dictionary object")
        try:
            # Building ang GlobalGeometryRules are required objects. Retrieve those objects and one by one verify
            # epJSON object types such that when an error is kicked, the logging statement can identify
            # where it came from
            building_object = input_epjson.get('Building')
            global_geometry_rules_object = input_epjson.get('GlobalGeometryRules')
            if not building_object or not global_geometry_rules_object:
                raise PyExpandObjectsSchemaError('Building or GlobalGeometryRules object missing')
            for object_type, object_structure in input_epjson.items():
                if object_type not in ['Building', 'GlobalGeometryRules']:
                    epjson_object = {'Building': building_object, 'GlobalGeometryRules': global_geometry_rules_object}
                    self.merge_epjson(
                        super_dictionary=epjson_object,
                        object_dictionary={object_type: object_structure})
                    file_validation = self.schema.is_valid(epjson_object)
                    if not file_validation:
                        if len(object_structure.keys()) > 1:
                            self.logger.error("Error: Input file does not meet schema format")
                            for err in self.schema.iter_errors(input_epjson):
                                self.logger.error('Error: Invalid choice in {} {}'
                                                  .format(object_type, err.message))
                            raise PyExpandObjectsSchemaError("Error: Schema Format is invalid")
                        else:
                            (object_name, _), = object_structure.items()
                            # if the schema validation fails for the epJSON object, write out specific errors that occurred.
                            self.logger.error("Error: Input file does not meet schema format")
                            for err in self.schema.iter_errors(input_epjson):
                                if re.match(r'.*{.*}.*', err.message):
                                    msg = '. '.join([
                                        err.message,
                                        '.  It appears a complex YAML reference was not resolved.'])
                                else:
                                    msg = err.message
                                self.logger.error('Error: Invalid choice in {} ({}). {}'
                                                  .format(object_type, object_name, msg))
                            raise PyExpandObjectsSchemaError("Error: Schema Format is invalid")
                    else:
                        continue
            setattr(self, 'input_epjson_is_valid', True)
            setattr(self, 'input_epjson', input_epjson)
            return input_epjson
        except Exception as e:
            raise PyExpandObjectsSchemaError("Error: epJSON validation failed: {}".format(str(e)))

    def _load_epjson(self, epjson_ref):
        """
        Load schema to class object.

        :param epjson_ref: Location of epJSON file to read or object itself

        :return: boolean flag for valid epJSON and epJSON object as class attributes
        """
        if isinstance(epjson_ref, dict):
            input_epjson = epjson_ref
        else:
            input_epjson = self._get_json_file(epjson_ref)
        self._validate_epjson(input_epjson)
        self.logger.info(
            'input EPJSON file loaded, %s EnergyPlus object types',
            len(self.input_epjson.keys())
        )
        return self.input_epjson

    def epjson_process(self, epjson_ref):
        """
        Default loading and verification of epJSON file
        :param epjson_ref: epJSON in dictionary format or file location.
        :return: initialized class attributes and input_epJSON object
        """
        self._load_schema()
        self._load_epjson(epjson_ref=epjson_ref)
        return
