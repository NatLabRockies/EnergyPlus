import argparse
import os
import pathlib
import re

from hvac_template import HVACTemplate
from epjson_handler import EPJSON
import logging
import json


def get_property(prop):
    """
    Get property value from __init__.py file in src directory

    :param prop: Property name
    :return: Return value for a given property
    """
    try:
        result = re.search(
            r'{}\s*=\s*[\'"]([^\'"]*)[\'"]'.format(prop),
            open(os.path.join(os.path.dirname(__file__), '__init__.py')).read())
        output = result.group(1)
    except AttributeError:
        output = '{} could not be found'.format(prop)
    return output


def build_parser():  # pragma: no cover
    """
    Build argument parser.
    """
    parser = argparse.ArgumentParser(
        prog='pyExpandObjects',
        description='Automated process that expands HVACTemplate objects into regular EnergyPlus objects.')
    parser.add_argument(
        '--no-schema',
        '-ns',
        action='store_true',
        help='Skip schema validations')
    parser.add_argument(
        "--file",
        '-f',
        nargs='?',
        help='Path of epJSON file to convert'
    )
    parser.add_argument(
        '--output_directory',
        '-o',
        nargs='?',
        help='Specify output directory.  If not provided, then input file directory is used'
    )
    parser.add_argument(
        '--no_backup',
        '-nb',
        action='store_true',
        help='Prevent backup files from being written'
    )
    parser.add_argument(
        '--logger_level',
        '-l',
        nargs='?',
        choices=['DEBUG', 'INFO', 'WARNING', 'ERROR', 'CRITICAL'],
        default='WARNING',
        help='Specify logger level.'
    )
    parser.add_argument(
        '--version',
        '-v',
        action='store_true',
        help='Display version information')
    parser.add_argument(
        '--write_logs',
        '-wl',
        action='store_true',
        help='Write logs to file')
    return parser


def output_preprocessor_message_formatter(output_stream):
    messages = {}
    for line in output_stream.split('\n'):
        counter = len(messages.keys()) + 1
        messages.update({
            'Output:PreprocessorMessage {}'.format(str(counter)): {
                'preprocessor_name': 'pyExpandObjects'
            }
        })
        if line.startswith('Error:'):
            messages['Output:PreprocessorMessage {}'.format(str(counter))]['error_severity'] = 'Severe'
        elif line.startswith('Warning:'):
            messages['Output:PreprocessorMessage {}'.format(str(counter))]['error_severity'] = 'Warning'
        else:
            messages['Output:PreprocessorMessage {}'.format(str(counter))]['error_severity'] = 'Information'
        words = line.split()
        word_groups = [words[i:i + 10] for i in range(0, len(words), 10)]
        word_group_counter = 1
        for wg in word_groups:
            messages['Output:PreprocessorMessage {}'.format(str(counter))][
                'message_line_{}'.format(word_group_counter)] = ' '.join(wg)
            word_group_counter += 1
        if not messages['Output:PreprocessorMessage {}'.format(str(counter))].get('message_line_1'):
            messages.pop('Output:PreprocessorMessage {}'.format(str(counter)))
    return messages


def main(args=None):
    if hasattr(args, 'version') and args.version:
        version = get_property('__version__')
        print('pyExpandObjects Version: {}'.format(version))
        return
    # set the arg defaults for testing when Namespace is used
    if not hasattr(args, 'logger_level'):
        args.logger_level = 'WARNING'
    if not hasattr(args, 'no_backup'):
        args.no_backup = False
    if not hasattr(args, 'no_schema'):
        args.no_schema = False
    if getattr(args, 'write_logs', None):
        logger_name = 'expand_objects_logger'
    else:
        logger_name = 'console_only_logger'
    hvt = HVACTemplate(
        no_schema=args.no_schema,
        logger_level=args.logger_level,
        logger_name=logger_name)
    if isinstance(args.file, str):
        file_suffix_check = args.file.endswith('.epJSON')
    elif isinstance(args.file, (pathlib.PosixPath, pathlib.WindowsPath)):
        file_suffix_check = args.file.suffix == '.epJSON'
    else:
        hvt.logger.error('Error: Invalid input file reference')
        return
    # get or set output directory
    if hasattr(args, 'output_directory') and args.output_directory:
        if not os.path.exists(args.output_directory):
            hvt.logger.error('Specified output directory %s does not exist. '
                             'Files will be written to default directory %s.',
                             args.output_directory,
                             os.path.dirname(os.path.abspath(args.file)))
            output_directory = os.path.dirname(os.path.abspath(args.file))
        else:
            output_directory = args.output_directory
    else:
        output_directory = os.path.dirname(os.path.abspath(args.file))
    # start blank dictionary for processing
    output = {
        'epJSON': {},
        'Output:PreprocessorMessage': ''}
    if file_suffix_check:
        # write output and keep list of written files
        output_file_dictionary = {}
        # create file names and raise error if modified name is the same as the base name
        input_file_name = os.path.basename(args.file)
        expanded_file_name = input_file_name.replace('.epJSON', '_expanded.epJSON')
        hvac_templates_file_name = input_file_name.replace('.epJSON', '_hvac_templates.epJSON') \
            if not args.no_backup else None
        base_file_name = input_file_name.replace('.epJSON', '_base.epJSON') \
            if not args.no_backup else None
        # check that file names are not the same as the original
        if input_file_name in [expanded_file_name, hvac_templates_file_name, base_file_name]:
            hvt.logger.error('Error: file could not be renamed')  # pragma: no cover - unlikely to be hit
            return
        if os.path.exists(args.file):
            hvt.logger.info('Processing %s', args.file)
            # QA skipped since any unanticipated condition should still get caught and returned to user.
            try:
                output.update(hvt.run(input_epjson=args.file))
                output_file_dictionary['expanded'] = os.path.join(output_directory, str(expanded_file_name))
            except:  # noqa: E722
                output.update({'Output:PreprocessorMessage': hvt.stream.getvalue()})
            if output.get('epJSON'):
                # verify expanded epJSON is valid if schema validation is turned on.
                if not args.no_schema:
                    ej = EPJSON(
                        no_schema=False,
                        logger_level=args.logger_level,
                        logger_name=logger_name)
                    try:
                        ej.epjson_process(epjson_ref=output['epJSON'])
                    except:  # noqa: E722
                        output['Output:PreprocessorMessage'] = '\n'.join([
                            output['Output:PreprocessorMessage'],
                            'Error: Output epJSON schema validation failed. See output files for details.\n',
                            ej.stream.getvalue()])
            if not args.no_backup and output.get('epJSON_hvac_templates'):
                with open(os.path.join(output_directory, hvac_templates_file_name), 'w') as hvac_template_file:
                    json.dump(output['epJSON_hvac_templates'], hvac_template_file, indent=4, sort_keys=True)
                    output_file_dictionary['hvac_templates'] = \
                        os.path.join(output_directory, str(hvac_templates_file_name))
            if not args.no_backup and output.get('epJSON_base'):
                with open(os.path.join(output_directory, base_file_name), 'w') as base_file:
                    json.dump(output['epJSON_base'], base_file, indent=4, sort_keys=True)
                    output_file_dictionary['base'] = os.path.join(output_directory, str(base_file_name))
            if output_file_dictionary and output['epJSON']:
                output_file_dictionary['expanded'] = os.path.join(output_directory, str(expanded_file_name))
                hvt.logger.info('Output files written %s', output_file_dictionary)
            else:
                output['Output:PreprocessorMessage'] = '\n'.join([
                    output['Output:PreprocessorMessage'],
                    'Error: No expanded epJSON object created, check Output:Preprocessor object at {} for details'
                    .format(os.path.join(output_directory, str(expanded_file_name)))])
                hvt.logger.error('Error: No expanded epJSON object created, check '
                                 'Output:Preprocessor object at %s for details',
                                 os.path.join(output_directory, str(expanded_file_name)))
            output['output_files'] = output_file_dictionary
        else:
            output['Output:PreprocessorMessage'] = r'\n'.join([
                output['Output:PreprocessorMessage'],
                'Error: File does not exist: {}.  File not processed'.format(args.file)])
            hvt.logger.error('Error: File does not exist: %s.  File not processed', args.file)
        output['epJSON']['Output:PreprocessorMessage'] = \
            output_preprocessor_message_formatter(output['Output:PreprocessorMessage'])
        # Write out epJSON file.
        with open(os.path.join(output_directory, expanded_file_name), 'w') as expanded_file:
            json.dump(output['epJSON'], expanded_file, indent=4, sort_keys=True)
        # write out successful file creation to base preprocessor object
        if output_file_dictionary and output['epJSON']:
            output['Output:PreprocessorMessage'] = '\n'.join([
                output['Output:PreprocessorMessage'],
                'Output files written: {}'.format(output_file_dictionary)])
    else:
        output['Output:PreprocessorMessage'] = r'\n'.join([
            output['Output:PreprocessorMessage'],
            'Error: Bad file extension for {}.  File not processed'.format(args.file)])
        hvt.logger.error('Error: Bad file extension for %s.  File not processed', args.file)
    return output


if __name__ == "__main__":
    epJSON_parser = build_parser()
    epJSON_args, unknown_args = epJSON_parser.parse_known_args()
    # If unknown arguments are passed, and no file specified, then put the arguments
    #  in the file namespace.
    if not epJSON_args.file and unknown_args:
        epJSON_args.file = unknown_args[0]
    main(epJSON_args)
    logging.shutdown()
