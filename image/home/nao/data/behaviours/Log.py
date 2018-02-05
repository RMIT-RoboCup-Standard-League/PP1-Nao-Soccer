import logging
import os

logger = logging.getLogger('behaviour')
logger.setLevel(logging.INFO)  # Set this to logging.INFO for comp

log_folders = os.listdir('/var/volatile/runswift')
log_folders.sort()

# Make sure we get the last folder and not a file
for path in reversed(log_folders):
	if(os.path.isdir('/var/volatile/runswift/%s' % path)):
		log_folder_path = '/var/volatile/runswift/%s' % path 
		break

log = '%s/behaviour' % log_folder_path 



file_handler = logging.FileHandler(log)
logger.addHandler(file_handler)

# Comment this handler out for competition
console_handler = logging.StreamHandler()
logger.addHandler(console_handler)


# Not sure how useful it would be to timestamp *every* message,
# might leave this to the user when required
# formatter = logging.Formatter(fmt = '%(created)f:%(message)s')
# handler.setFormatter(formatter)

def debug(*args, **kwargs):
    logger.debug(*args, **kwargs)


def info(*args, **kwargs):
    logger.info(*args, **kwargs)


def warning(*args, **kwargs):
    logger.warning(*args, **kwargs)


def error(*args, **kwargs):
    logger.error(*args, **kwargs)


def critical(*args, **kwargs):
    logger.critical(*args, **kwargs)
