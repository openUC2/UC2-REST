import logging


class Logger(object):
    def __init__(
        self,
        name=__name__,
        log_to_file=False,
        filename='uc2rest.log'
    ):
        # Create a logger with the given name
        self.logger = logging.getLogger(name)
        self.logger.setLevel(logging.DEBUG)
        formatter = logging.Formatter(
            '%(asctime)s [%(levelname)s] %(name)s: %(message)s'
        )

        # Console handler
        ch = logging.StreamHandler()
        ch.setLevel(logging.DEBUG)
        ch.setFormatter(formatter)
        self.logger.addHandler(ch)

        # Optional file handler
        if log_to_file:
            fh = logging.FileHandler(filename)
            fh.setLevel(logging.DEBUG)
            fh.setFormatter(formatter)
            self.logger.addHandler(fh)

    def error(self, message):
        # Log an error message
        self.logger.error(message)

    def debug(self, message):
        # Log a debug message
        self.logger.debug(message)

    def info(self, message):
        # Log an info message
        self.logger.info(message)

    def warning(self, message):
        # Log a warning message
        self.logger.warning(message)