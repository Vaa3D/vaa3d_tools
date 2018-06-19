import glob
from pyspark import SparkContext
import logging
import sys

sc = SparkContext(appName='SparkBranchSplitter')
sc.setLogLevel(logging.INFO)

def get_human_trajectories():
    file_list = glob.glob("../data/human/*.swc")
    return file_list


def remove_comments(text):
    """SWC files start with comments, remove before proceeding"""
    #for line in text.
    logger = getlogger('comment_remover')
    logger.critical(str(text))

def chop_branches(swc_file):
    pass

def center_branches(branch):
    pass

def render_volume_from_branch(branch):
    # TODO parrallelize
    pass

# https://stackoverflow.com/a/43750660/4212158
def getlogger(name, level=logging.INFO):
    logger = logging.getLogger(name)
    logger.setLevel(level)
    if logger.handlers:
        # or else, as I found out, we keep adding handlers and duplicate messages
        pass
    else:
        ch = logging.StreamHandler(sys.stderr)
        ch.setLevel(level)
        formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
        ch.setFormatter(formatter)
        logger.addHandler(ch)
    return logger


def tst_log():
    logger = getlogger('my-worker')
    logger.debug('a')
    logger.info('b')
    logger.warning('c')
    logger.error('d')
    logger.critical('e')

def main():
    # TODO don't repeat work
    swc_list = get_human_trajectories()
    trajectories = sc.textFile(','.join(swc_list))
    trajectories = trajectories.map(remove_comments)
    tst_log()
    #trajectories = trajectories.map(chop_branches)


if __name__ == "__main__":
    main()