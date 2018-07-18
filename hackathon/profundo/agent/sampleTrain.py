#!/usr/bin/env python
# -*- coding: utf-8 -*-
# File: sampleTrain.py
# Author: Amir Alansary <amiralansary@gmail.com>

import warnings
warnings.simplefilter("ignore", category=ResourceWarning)

import numpy as np
import SimpleITK as sitk
from tensorpack import logger
from IPython.core.debugger import set_trace

__all__ = ['files', 'filesListBrainMRLandmark', 'filesListFetalUSLandmark',
           'filesListCardioMRLandmark', 'NiftiImage']

#######################################################################
## list file/directory names
import glob
import os
import re


def alphanum_key(s):
    """ Turn a string into a list of string and number chunks.
        "z23a" -> ["z", 23, "a"]
    """
    return [tryint(c) for c in re.split('([0-9]+)', s)]


def tryint(s):
    try:
        return int(s)
    except:
        return s


def listFiles(dirpath, dirnames):
    curpath = os.getcwd()
    os.chdir(os.path.abspath(dirpath))
    f = glob.glob(dirnames)
    f.sort(key=alphanum_key)
    os.chdir(curpath)
    return f


#######################################################################




#######################################################################

class files(object):
    """ A class for managing train files

        Attributes:
        directory: input data directo
    """

    def __init__(self, directory=None):

        assert directory, 'There is no directory containing training files given'

        self.dir = directory

        # self.images_list = self._listImages()
        # self.landmarks_list = self._listLandmarks()
        # self.all_landmarks_list = self._listLandmarks_all()
    #
    # def _listImages(self):
    #
    #     childDirs = listFiles(self.dir, '*')
    #
    #     image_files = []
    #
    #     for child in childDirs:
    #         dir_path = os.path.join(self.dir, child)
    #         if not (os.path.isdir(dir_path)): continue
    #         file_name = listFiles(dir_path, '*.nii.gz')
    #         file_path = os.path.join(dir_path, file_name[0])
    #         image_files.append(file_path)
    #
    #     return image_files

    @property
    def num_files(self):
        return len(self.images_list)


###############################################################################
#

#######################################################################
class FilesListCubeNPY(files):
    def __init__(self, filenames_list=None, filepaths_list=None):

        assert filenames_list, 'There is no directory containing training files given'
        assert filepaths_list, 'There is no directory containing files list'

        self.filenames = filenames_list
        self.files_list = filepaths_list

    @property
    def num_files(self):
        return len(self.files_list)


    def sample_circular(self, shuffle=False):
        """ return a random sampled ImageRecord from the list of files
        """
        if shuffle:
            pass  # we shuffle using train_test_split
            #indexes = rng.choice(x, len(x), replace=False)
        else:
            indexes = np.arange(self.num_files)

        while True:  #
            for idx in indexes:
                image_path = self.files_list[idx]
                image_filename =self.files_list[idx] 
                # x, y, file, ?
                yield image_path, image_filename

