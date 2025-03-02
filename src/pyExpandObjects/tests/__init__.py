import os
import inspect
import sys
from functools import wraps

this_script_path = os.path.dirname(
    os.path.abspath(__file__)
)

sys.path.append(os.path.join(this_script_path, '..', 'src'))

from src.logger import Logger


class BaseTest(object):

    @classmethod
    def _test_logger(cls, doc_text="General"):
        """
        Wrapper that writes a CSV formatted message to a log file for further processing
        :param doc_text: Colon separated text for document

        :return: None
        """
        def _test_logger_wrapper(func):
            @wraps(func)
            def wrapper(self, *args, **kwargs):
                func_name = func.__name__
                func_status = False
                output_msg = False
                try:
                    func(self, *args, **kwargs)
                    func_status = True
                    output_msg = True
                    self._write_logger(
                        doc_text=doc_text,
                        file_name=os.path.basename(inspect.getfile(func)),
                        func_name=func_name,
                        func_status=func_status)
                finally:
                    # prevent double logging output
                    if not output_msg:
                        self._write_logger(
                            doc_text=doc_text,
                            file_name=os.path.basename(inspect.getfile(func)),
                            func_name=func_name,
                            func_status=func_status)
            # make output the called function for unittest to work
            _test_logger_wrapper.__wrapped__ = func
            return wrapper
        return _test_logger_wrapper

    @staticmethod
    def _write_logger(
            doc_text,
            file_name,
            func_name,
            func_status,
            testing_logger=Logger(logger_name='testing_logger').logger):
        """
        Write a structured output to logging file.

        :param doc_text: Colon separated string indicating Document section and message
        :param file_name: Function file name
        :param func_name: Function name
        :param func_status: Final status of function
        :param testing_logger: logger object to write with
        :return: None
        """
        try:
            testing_logger.info(
                '%s,%s,%s,%s',
                doc_text,
                file_name,
                func_name,
                func_status)
        except AttributeError:
            pass
        return
