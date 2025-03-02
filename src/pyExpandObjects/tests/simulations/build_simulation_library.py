import os
from argparse import Namespace
from tests.simulations import BaseSimulationTest
from src.main import main as py_main

base_project_path = os.path.dirname(
    os.path.dirname(
        os.path.dirname(
            os.path.abspath(__file__))))


def main():
    """
    Convert IDF Template files to various formats
    :return: None
    """
    file_directory = os.path.join(base_project_path, 'simulation', 'ExampleFiles')
    sample_output_directory = os.path.join(base_project_path, 'simulation', 'ExampleOutputs')
    template_files = [
        i for i in os.listdir(file_directory)
        if i.startswith('HVACTemplate') and 'expanded' not in i.lower() and i.endswith('.idf')]
    for tf in template_files:
        print('Transforming template file: {}'.format(tf))
        expanded_file = BaseSimulationTest.expand_idf(
            file_location=os.path.join(file_directory, tf),
            working_dir=file_directory)
        print('Expanded idf file: {}'.format(expanded_file))
        test_file = BaseSimulationTest.convert_file(
            file_location=os.path.join(file_directory, tf),
            working_dir=file_directory)
        print('Converted epJSON template file: {}'.format(test_file))
        expanded_epjson_file = BaseSimulationTest.convert_file(
            file_location=os.path.join(file_directory, expanded_file),
            working_dir=file_directory)
        print('Expanded epJSON file: {}'.format(expanded_epjson_file))
        py_main(
            Namespace(
                file=os.path.join(file_directory, tf.replace('.idf', '.epJSON')),
                no_schema=False,
                output_directory=sample_output_directory,
                no_backup=False))
        print('pyExpandObjects Sample file {}'.format(tf.replace('.idf', '.epJSON')))
    return


if __name__ == "__main__":
    main()
