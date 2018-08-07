#!/usr/bin/env python
# -*- coding: utf-8 -*-
# File: dataAPI.py
# Author: Amir Alansary <amiralansary@gmail.com>

import warnings
warnings.simplefilter("ignore", category=ResourceWarning)

import numpy as np
import SimpleITK as sitk
from tensorpack import logger
from IPython.core.debugger import set_trace

__all__ = ['files', 'FilesListCubeNPY']

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
    def __init__(self, directory=None, filepaths_list=None):

        assert directory, 'There is no directory containing training files given'
        assert filepaths_list, 'There is no directory containing files list'

        # self.filenames = directory
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
                image_path = self.files_list[idx]  # FIXME shouldn't these be different
                image_filename = os.path.basename(image_path)
                # split the last 2 periods off, then grab the basename
                swc_filename = image_filename.rsplit('.', 2)[0]
                # get relevant swc file
                # go up two directories
                data_dir = os.path.dirname(os.path.dirname(image_path))
                # FIXME shouldn't use abs name for swc folder
                swc_path = os.path.join(data_dir, "06_origin_cubes", swc_filename)
                print("img path ", image_path)
                # image_filename = self.filenames[idx]
                # x, y, file, ?
                begin, end = self.first_last_swc_nodes(swc_path)
                yield image_path, image_filename, begin, end

    def sample_first(self):
        """determinist for unit tests"""
        yield self.files_list[0], os.path.basename(self.files_list[0])

    def first_last_swc_nodes(self, swc_file):
        good_first = False
        # https://stackoverflow.com/a/18603065/4212158
        with open(swc_file, "rb") as f:
            while not good_first:
                first = f.readline()  # Read the first line.
                print("first ", first)
                if not first.startswith(b"#"):
                    good_first = True
            f.seek(-2, os.SEEK_END)  # Jump to the second last byte.
            while f.read(1) != b"\n":  # Until EOL is found...
                f.seek(-2, os.SEEK_CUR)  # ...jump back the read byte plus one more.
            last = f.readline()  # Read last line.

        # convert from bytes to regular string
        first, last = first.decode(), last.decode()
        # convert to lists
        first = first.strip().split(" ")
        last = last.strip().split(" ")

        #SWC convention:
        # node_id type x_coordinate y_coordinate z_coordinate radius parent_node
        # we want [2,3,4]
        first, last = first[2,3,4], last[2,3,4]

        return first, last


