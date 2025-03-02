import logging
import os
import sys
from pathlib import Path
from logging.config import fileConfig
from io import StringIO

loggers = {}
stream = StringIO()

this_script_path = Path(__file__).resolve()


class Logger:
    """
    General logger
    """

    def __init__(
            self,
            logging_file_name='logging.conf',
            logger_name='console_only_logger',
            log_file_name='base',
            logger_level='WARNING',
            reset_stream=False):
        # prevent re-calling same logger handlers once initialized
        # also prevent bad logger name from being called
        global loggers
        global stream
        # noinspection PyBroadException
        # Use a different file for testing logger
        # When packaged, the logs file is under the src/ directorey, so change the directory based on mode
        if getattr(sys, 'frozen', False):
            logging_dir = os.path.join(os.path.dirname(this_script_path), 'logs')
        else:
            logging_dir = str(this_script_path.parent.parent / 'logs')
        log_file_location = os.path.join(
            logging_dir,
            r'{}.log'.format(log_file_name)
        )
        testing_log_file_location = os.path.join(
            logging_dir,
            r'{}.log'.format('test')
        )
        for log_file in [log_file_location, testing_log_file_location]:
            if not os.path.isfile(log_file):  # pragma: no cover
                with open(log_file, 'w'):
                    pass
        fileConfig(
            os.path.join(
                logging_dir,
                logging_file_name
            ),
            defaults={
                "base_log_filename": log_file_location,
                "testing_log_filename": testing_log_file_location
            }
        )
        # if the code fails, fall back to root logger
        try:
            # if the logger exists, use it instead of creating a new one
            if not loggers.get(logger_name):
                # if logger_name is not in the config file, default to root
                if logger_name in logging.root.manager.loggerDict.keys():
                    self.logger = logging.getLogger(logger_name)
                    self.logger.setLevel(logger_level)
                else:  # pragma: no cover
                    self.logger = logging.getLogger('root')
                    self.logger.setLevel(logger_level)
                    self.logger.warning(
                        'Bad logger name passed (%s), continuing with only console logging',
                        logger_name
                    )
                # save logger to global dictionary
                loggers.update({logger_name: self.logger})
            else:
                self.logger = loggers[logger_name]
        except Exception as e:  # pragma: no cover
            self.logger = logging.getLogger('root')
            self.logger.setLevel(logger_level)
            loggers.update({logger_name: self.logger})
            self.logger.warning(
                'Logger failed to start %s, continuing with only console logging, error message: %s',
                logger_name, str(e)
            )
        finally:
            if reset_stream:
                stream = StringIO()
            # add stream handler for output
            self.stream = stream
            handler = logging.StreamHandler(self.stream)
            handler.setLevel(logger_level)
            self.logger.addHandler(handler)
            self.logger.setLevel(logger_level)
            self.logger.stream_flush = self.stream.flush()
        return
