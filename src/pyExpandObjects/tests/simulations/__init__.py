import unittest
import json
from pathlib import Path
import subprocess
import re
import sys
import os
from argparse import Namespace
import traceback
import pandas as pd
import numpy as np
import time

from src.epjson_handler import EPJSON
from src.main import main
from tests import BaseTest

test_dir = Path(__file__).parent.parent
weather_file = 'USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw'


class BaseSimulationTest(BaseTest, unittest.TestCase):
    """
    Setup, extraction, and comparison functions for simulation testing.
    """

    @staticmethod
    def setup_file(file_path):
        """
        Read file path and run epJSON through processing to ensure it is a valid json before testing.
        This is to ensure that if there is a problem with the base file, it will be more obvious by returning an
        error from this function rather than pyExpandObjects.  Also, it allows for any future manipulations that may
        be useful

        :param file_path: epJSON file path
        :return: formatted epJSON
        """
        with open(file_path, 'r') as f:
            test_data = f.read()
        formatted_epjson = json.loads(test_data)
        # Do manipulations if necessary by using the following commands
        # epj = EPJSON()
        # epj.epjson_process(epjson_ref=base_raw_epjson)
        # purged_epjson = epj.purge_epjson(
        #     epjson=base_raw_epjson,
        #     purge_dictionary={})
        # formatted_epjson = copy.deepcopy(purged_epjson)
        # epj.merge_epjson(
        #     super_dictionary=formatted_epjson,
        #     object_dictionary={})
        return formatted_epjson

    @staticmethod
    def write_file_for_testing(epjson, file_name, sub_directory=('simulation', 'test')):
        """
        write file to simulation testing sudirectory
        :param epjson: epJSON dictionary
        :param sub_directory: project subdirectory
        :param file_name: file name to write
        :return: file_path for written epJSON
        """
        input_file_path = test_dir.joinpath('..', *sub_directory, file_name)
        with open(input_file_path, 'w') as f:
            json.dump(epjson, f, indent=4, sort_keys=True)
        return input_file_path

    def get_epjson_object_from_idf_file(self, idf_file_path):
        """
        Return an epJSON object from idf filfe path
        :param idf_file_path: idf file path
        :return: epJSON object
        """
        epjson_file_path = self.convert_file(idf_file_path)
        ej = EPJSON()
        return ej._get_json_file(epjson_file_path)

    def create_idf_file_from_epjson(self, epjson, file_name, sub_directory=('simulation', 'test')):
        """
        Create an epJSON and idf file from an epJSON object
        :param epjson: input epJSON object
        :param file_name: destination file name
        :param sub_directory: destination directory
        :return: idf file name
        """
        epjson_file_path = self.write_file_for_testing(epjson=epjson, file_name=file_name, sub_directory=sub_directory)
        idf_file_path = self.convert_file(file_location=epjson_file_path)
        return idf_file_path

    @staticmethod
    def expand_idf(file_location, working_dir=str(test_dir / '..' / 'simulation' / 'test')):
        """
        Expand idf file and save with a unique file name

        :param file_location: location for idf base file
        :param working_dir: directory to use as working directory for subprocess
        :return: file path to expanded file
        """
        # convert to Path if a string is passed
        if isinstance(file_location, str):
            file_location = Path(file_location)
        # calling the arguments with pathlib causes issues with the conversion exe, so direct strings are passed here.
        subprocess.run(
            [
                'wine',
                '../energyplus.exe',
                '-x',
                '--convert-only',
                os.path.basename(file_location)
            ],
            cwd=working_dir,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        time.sleep(0.1)
        os.rename(
            os.path.join(
                os.path.dirname(file_location),
                'eplusout.expidf'),
            os.path.join(
                os.path.dirname(file_location),
                os.path.basename(file_location).replace('.idf', 'Expanded.idf')))
        time.sleep(0.1)
        return os.path.basename(file_location).replace('.idf', 'Expanded.idf')

    @staticmethod
    def convert_file(file_location, working_dir=None):
        """
        Convert idf to epJSON and vice versa.
        converted file in alternative format is created in same directory as base file
        :param file_location: Path or string reference to file location
        :param working_dir: working directory
        :return: file path to converted file
        """
        if not working_dir:
            working_dir = os.path.dirname(file_location)
        # convert to Path if a string is passed
        if isinstance(file_location, str):
            file_location = Path(file_location)
        # calling the arguments with pathlib causes issues with the conversion exe, so direct strings are passed here.
        if 'expanded' in os.path.basename(file_location).lower():
            result = subprocess.run(
                [
                    'wine',
                    '../ConvertInputFormat.exe',
                    os.path.basename(file_location)
                ],
                cwd=working_dir,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )
        else:
            result = subprocess.run(
                [
                    'wine',
                    '../ConvertInputFormatIgnoreVersion.exe',
                    os.path.basename(file_location)
                ],
                cwd=working_dir,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )
        time.sleep(0.1)
        print('Conversion message output: {}'.format(result.stdout))
        print('Conversion error output: {}'.format(result.stderr))
        if file_location.suffix == '.epJSON':
            return file_location.with_suffix('.idf')
        elif file_location.suffix == '.idf':
            return file_location.with_suffix('.epJSON')
        else:
            print('File has incorrect extension {}'.format(file_location))
            sys.exit()

    @staticmethod
    def _copy_to_test_directory(base_file_path):
        base_copy_file_path = test_dir.joinpath('..', 'simulation', 'test', 'hvactemplate_copy_input.idf')
        with open(base_file_path, 'r') as f1, \
                open(base_copy_file_path, 'w') as f2:
            for line in f1:
                f2.write(line)
        return base_copy_file_path

    def perform_full_comparison(self, base_idf_file_path, warning_check=True, compare_epjson_files=True):
        """
        Simulate and compare two files where only the objects controlling output data are manipulated.
        Due to conversion issues within EnergyPlus, the baseline file must be simulated as an idf.
        :param base_idf_file_path: idf file location containing HVACTemplate:.* objects
        :param warning_check: boolean to indicate whether to check warnings or not.
        :param compare_epjson_files: boolean to indicate if epJSON summaries should be compared
        :return: None.  Assertions performed within function.
        """
        # Move old file to prevent calling an outdated file.
        try:
            os.rename(
                str(test_dir / '..' / 'simulation' / 'test' / 'base_input.idf'),
                str(test_dir / '..' / 'simulation' / 'test' / 'old_base_input.idf')
            )
        except FileNotFoundError:
            pass
        try:
            os.rename(
                str(test_dir / '..' / 'simulation' / 'test' / 'base_inputExpanded.idf'),
                str(test_dir / '..' / 'simulation' / 'test' / 'old_base_inputExpanded.idf')
            )
        except FileNotFoundError:
            pass
        try:
            os.rename(
                str(test_dir / '..' / 'simulation' / 'test' / 'base_inputExpanded.epJSON'),
                str(test_dir / '..' / 'simulation' / 'test' / 'old_base_inputExpanded.epJSON')
            )
        except FileNotFoundError:
            pass
        time.sleep(0.1)
        # move file to testing directory manually, shutil does not work reliably for some reason
        base_idf_test_file_path = test_dir.joinpath('..', 'simulation', 'test', 'base_input.idf')
        with open(base_idf_file_path, 'r') as f1, \
                open(base_idf_test_file_path, 'w') as f2:
            for line in f1:
                f2.write(line)
        # convert to epJSON for test simulation input
        baseline_file_location = self.convert_file(base_idf_test_file_path)
        # setup outputs for perform_comparison to function and write base file
        base_formatted_epjson = self.setup_file(baseline_file_location)
        # move file so that it isn't accidentally read from a previous run
        try:
            os.rename(
                str(test_dir / '..' / 'simulation' / 'test' / 'test_input_epjson.epJSON'),
                str(test_dir / '..' / 'simulation' / 'test' / 'old_test_input_epjson.epJSON')
            )
        except FileNotFoundError:
            pass
        try:
            os.rename(
                str(test_dir / '..' / 'simulation' / 'test' / 'test_pre_input_epjson.epJSON'),
                str(test_dir / '..' / 'simulation' / 'test' / 'old_test_pre_input_epjson.epJSON')
            )
        except FileNotFoundError:
            pass
        time.sleep(0.1)
        # write the preformatted base file for main to call
        test_pre_input_file_path = self.write_file_for_testing(
            epjson=base_formatted_epjson,
            file_name='test_pre_input_epjson.epJSON')
        # Expand and perform comparisons between the files
        # try expansion.  If it fails, raise an error
        output_epjson = None
        output = None
        try:
            output = main(
                Namespace(
                    no_schema=False,
                    file=str(test_dir / '..' / 'simulation' / 'ExampleFiles' / 'test' / test_pre_input_file_path),
                    logger_level='INFO',
                    write_logs=True))
            output_epjson = output['epJSON']
            test_input_file_path = self.write_file_for_testing(
                epjson=output_epjson,
                file_name='test_input_epjson.epJSON')
            # compare epjsons by summary count
            if compare_epjson_files:
                expanded_base_file = self.expand_idf(base_idf_test_file_path)
                comparison_base_epjson_file_path = self.convert_file(
                    str(test_dir / '..' / 'simulation' / 'test' / expanded_base_file))
                with open(comparison_base_epjson_file_path, 'r') as f:
                    comparison_base_epjson = json.load(f)
                with open(test_input_file_path, 'r') as f:
                    comparison_test_epjson = json.load(f)
                self.compare_epjsons(comparison_base_epjson, comparison_test_epjson)
            # todo_eo: pre-counting pyeo_warnings disabled due to new output formatting being captured in energyplus.
            #  Remove permanently after output is stable
            # pyeo_warnings = output['Output:PreprocessorMessage'].lower().count('warning')
            # check outputs
            self.perform_comparison(
                [base_idf_test_file_path, test_input_file_path],
                warning_check=warning_check,
                pyeo_warnings=0)
            del output
        except:
            traceback.print_exc()
            self.assertEqual(1, 0, 'pyExpandObjects process failed to complete')
            del output
        return

    def perform_comparison(self, simulation_files, warning_check=True, pyeo_warnings=0):
        """
        Simulate and compare epJSON files
        :param simulation_files: input simulation files to compare
        :param warning_check: boolean to run warning checks or not
        :param pyeo_warnings: number of warnings from PyExpandObjects pre-processor
        :return: dictionary of status outputs
        """
        total_energy_outputs = []
        warning_outputs = []
        error_outputs = []
        finished_statuses = []
        for file_path in simulation_files:
            # move files from previous runs, rm is too dangerous
            try:
                os.rename(
                    str(test_dir / '..' / 'simulation' / 'test' / 'eplusout.eso'),
                    str(test_dir / '..' / 'simulation' / 'test' / 'eplusout_previous.eso')
                )
            except FileNotFoundError:
                pass
            try:
                os.rename(
                    str(test_dir / '..' / 'simulation' / 'test' / 'eplustbl.htm'),
                    str(test_dir / '..' / 'simulation' / 'test' / 'eplustbl_previous.htm')
                )
            except FileNotFoundError:
                pass
            try:
                os.rename(
                    str(test_dir / '..' / 'simulation' / 'test' / 'eplusout.mtr'),
                    str(test_dir / '..' / 'simulation' / 'test' / 'eplusout_previous.mtr')
                )
            except FileNotFoundError:
                pass
            try:
                os.rename(
                    str(test_dir / '..' / 'simulation' / 'test' / 'eplusout.csv'),
                    str(test_dir / '..' / 'simulation' / 'test' / 'eplusout_previous.csv')
                )
            except FileNotFoundError:
                pass
            try:
                os.rename(
                    str(test_dir / '..' / 'simulation' / 'test' / 'eplusout.err'),
                    str(test_dir / '..' / 'simulation' / 'test' / 'eplusout_previous.err')
                )
            except FileNotFoundError:
                pass
            try:
                os.rename(
                    str(test_dir / '..' / 'simulation' / 'test' / 'eplusout.end'),
                    str(test_dir / '..' / 'simulation' / 'test' / 'eplusout_previous.end')
                )
            except FileNotFoundError:
                pass
            time.sleep(0.1)
            if sys.platform.startswith('win'):
                # enable ExpandObjects for idf files (-x).
                if file_path.suffix == '.idf':
                    subprocess.run(
                        [
                            str(test_dir / '..' / 'simulation' / 'energyplus.exe'),
                            '-d',
                            str(test_dir / '..' / 'simulation' / 'test'),
                            '-x',
                            '-w',
                            str(test_dir / '..' / 'simulation' / 'WeatherData' / weather_file),
                            str(file_path)
                        ]
                    )
                else:
                    subprocess.run(
                        [
                            str(test_dir / '..' / 'simulation' / 'energyplus.exe'),
                            '-d',
                            str(test_dir / '..' / 'simulation' / 'test'),
                            '-w',
                            str(test_dir / '..' / 'simulation' / 'WeatherData' / weather_file),
                            str(file_path)
                        ]
                    )
                subprocess.run(
                    [
                        str(test_dir / '..' / 'simulation' / 'PostProcess' / 'ReadVarsESO.exe'),
                        'convert.rvi'
                    ],
                    cwd=str(test_dir / '..' / 'simulation' / 'test')
                )
            else:
                if file_path.suffix == '.idf':
                    subprocess.run(
                        [
                            'wine',
                            str(test_dir / '..' / 'simulation' / 'energyplus.exe'),
                            '-d',
                            str(test_dir / '..' / 'simulation' / 'test'),
                            '-x',
                            '-w',
                            str(test_dir / '..' / 'simulation' / 'WeatherData' / weather_file),
                            file_path
                        ]
                    )
                else:
                    subprocess.run(
                        [
                            'wine',
                            str(test_dir / '..' / 'simulation' / 'energyplus.exe'),
                            '-d',
                            str(test_dir / '..' / 'simulation' / 'test'),
                            '-w',
                            str(test_dir / '..' / 'simulation' / 'WeatherData' / weather_file),
                            file_path
                        ]
                    )
                subprocess.run(
                    [
                        'wine',
                        str(test_dir / '..' / 'simulation' / 'PostProcess' / 'ReadVarsESO.exe'),
                        'convert.rvi'
                    ],
                    cwd=str(test_dir / '..' / 'simulation' / 'test')
                )
            time.sleep(0.1)
            # get sum of output csv rows to use as comparison
            energy_df = pd.read_csv(str(test_dir / '..' / 'simulation' / 'test' / 'eplusout.csv'))
            melt_columns = [c for c in energy_df.columns if not c == 'Date/Time']
            energy_df = energy_df.melt(id_vars=['Date/Time', ], value_vars=melt_columns)
            with open(str(test_dir / '..' / 'simulation' / 'test' / 'eplusout.end'), 'r') as f:
                lines = f.readlines()
            # check end file for matches and success message
            warning_rgx = r'.*\s+(\d+)\s+Warning;'
            error_rgx = r'.*\s+(\d+) Severe Errors'
            status_rgx = r'^EnergyPlus\s+Completed\s+Successfully'
            status_val = 0
            for line in lines:
                warning_match = re.match(warning_rgx, line)
                error_match = re.match(error_rgx, line)
                status_match = re.match(status_rgx, line)
                if warning_match:
                    try:
                        # pass the number of warnings from PyExpandObjects to the counter.
                        if 'test_input' in str(file_path):
                            warning_outputs.append(float(warning_match.group(1)) + pyeo_warnings)
                        else:
                            warning_outputs.append(float(warning_match.group(1)))
                    except TypeError:
                        # special flag for type error
                        warning_outputs.append(-1)
                if error_match:
                    try:
                        error_outputs.append(float(error_match.group(1)))
                    except TypeError:
                        error_outputs.append(-1)
                if status_match:
                    status_val = 1
            finished_statuses.append(status_val)
            total_energy_outputs.append(energy_df)
        status_checks = {
            "total_energy_outputs": total_energy_outputs,
            "warning_outputs": warning_outputs,
            "error_outputs": error_outputs,
            "finished_statuses": finished_statuses}
        # merge each meter output against the others and check if there is a discrepancy
        for energy_idx in range(len(simulation_files)):
            index_check = [i for i in range(len(simulation_files)) if i > energy_idx]
            for i in index_check:
                test_df = total_energy_outputs[energy_idx].merge(
                    total_energy_outputs[i],
                    how='left',
                    on=['Date/Time', 'variable'])
                test_df['diff'] = abs(test_df['value_x'] - test_df['value_y']) / np.maximum(1, test_df['value_x'])
                # Filter out low percentage differences
                test_filtered_df = test_df.loc[test_df['diff'] > 0.0001].copy()
                test_filtered_df.rename(columns={
                    "value_x": os.path.basename(simulation_files[energy_idx]),
                    "value_y": os.path.basename(simulation_files[i])
                }, inplace=True)
                self.assertEqual(
                    test_filtered_df.shape[0],
                    0,
                    "Meter Outputs are not approximately equal:\n{}".format(
                        test_filtered_df.sort_values(['diff', ], ascending=False)))
                print('Meter Outputs:\n{}'.format(test_df.sort_values(['diff', ])))
        if warning_check:
            for warning in status_checks['warning_outputs']:
                self.assertEqual(warning, max(status_checks['warning_outputs']), 'Unbalanced number of warnings')
        for error in status_checks['error_outputs']:
            self.assertEqual(error, max(status_checks['error_outputs']), 'Unbalanced number of errors')
            self.assertGreaterEqual(error, 0)
        for status in status_checks['finished_statuses']:
            self.assertEqual(1, status, 'Varying status outputs')
        return

    def compare_epjsons(
            self,
            epjson_1,
            epjson_2,
            exclude_list=('Schedule:Compact', 'Output:PreprocessorMessage',
                          'AvailabilityManagerAssignmentList', 'ScheduleTypeLimits',
                          'AvailabilityManager:NightCycle', 'OutdoorAir:NodeList', 'OutdoorAir:Node',
                          'NodeList')):
        """
        Summarize and compare two epJSONs based on object counts.

        :param epjson_1: epJSON object
        :param epjson_2: epJSON object
        :param exclude_list: epJSON object types to ignore
        :return: message if failures occur, otherwise None
        """
        eo = EPJSON()
        epjson_summary_1 = eo.summarize_epjson(epjson_1)
        epjson_summary_2 = eo.summarize_epjson(epjson_2)
        # remove schedule compact
        for el in exclude_list:
            epjson_summary_1.pop(el, None)
            epjson_summary_2.pop(el, None)
        msg = ''
        for k, v in epjson_summary_1.items():
            if k not in epjson_summary_2.keys():
                msg += '{} not in {}\n'.format(k, epjson_summary_2)
            else:
                if epjson_summary_1[k] != epjson_summary_2[k]:
                    msg += '{} is not the same number of objects\n'.format(k)
        for k, v in epjson_summary_2.items():
            if k not in epjson_summary_1.keys():
                msg += '{} not in {}\n'.format(k, epjson_summary_2)
        self.assertEqual('', msg, msg)
        return
