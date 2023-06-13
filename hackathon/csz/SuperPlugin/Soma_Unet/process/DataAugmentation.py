#================================================================
#   Copyright (C) 2023 Shize Chen (Braintell, Southeast University). All rights reserved.
#
#   Filename     : DataAugmentation.py
#   Author       : Shize Chen
#   Date         : 2023-03-21
#
#================================================================

import shutil
import numpy as np
from process.transform3d import *
from skimage.transform import resize
from scipy.ndimage import gaussian_filter
from skimage import exposure
import SimpleITK as sitk
from batchgenerators.augmentations.utils import create_zero_centered_coordinate_mesh, elastic_deform_coordinates, interpolate_img, rotate_coords_2d, rotate_coords_3d, scale_coords, elastic_deform_coordinates_2, resize_multichannel_image

from utilsa import image_util

class Compose(object):
    def __init__(self, transforms):
        self.transforms = transforms

    def __call__(self, img, tree):
        for t in self.transforms:
            img, tree = t(img, tree)
        return img, tree

class AbstractTransform(object):
    def __init__(self, p=0.5):
        self.p = p


class ConvertToFloat(object):
    """
    Most augmentation assumes the input image of float type, so it is always recommended to
    call this class before all augmentations.
    """

    def __init__(self, dtype=np.float32):
        self.dtype = dtype

    def __call__(self, img, label):
        if not img.dtype.name.startswith('float'):
            img = img.astype(self.dtype)
        if not label.dtype.name.startswith('float'):
            label = label.astype(self.dtype)
        # print("convert",np.max(img),np.min(img))
        # print("convert", np.max(label), np.min(label))
        return img, label

class InstanceAugmentation(object):
    def __init__(self, p=0.2, imgshape=(64,64,64), phase='train', divid=2 ** 5):
        if phase == 'train':
            pre_crop_size = []
            pre_crop_ratio = 1.5

            self.augment = Compose([
                ConvertToFloat(),
                RandomCrop(1.0, imgshape),
                RandomFlip(p,np.random.RandomState()),
                RandomRotate(p,np.random.RandomState()),
                ElasticDeformation(p,np.random.RandomState(),spline_order=3),
                RandomRotate90(p,np.random.RandomState()),
                # AdditivePoissonNoise(p,np.random.RandomState()),
            ])
        elif phase == 'val':
            self.augment = Compose([
                RandomCrop(1.0, imgshape),
            ])
        elif phase == 'test' or phase == 'par':
            self.augment = Compose([

            ])
        else:
            raise NotImplementedError

    def __call__(self, img, tree):
        return self.augment(img, tree)

class RandomCrop(AbstractTransform):
    def __init__(self, p=0.5, imgshape=None,  per_axis=True):
        super(RandomCrop, self).__init__(p)
        self.imgshape = imgshape
        self.per_axis = per_axis
        self.p = p

    def __call__(self, img, label):
        # print(img.shape,label.shape)
        if self.imgshape[0] < img.shape[0]:
            lb_x = np.random.randint(0, img.shape[0] - self.imgshape[0])
        elif self.imgshape[0] == img.shape[0]:
            lb_x = 0
        else:
            lb_x = -1

        if self.imgshape[1] < img.shape[1]:
            lb_y = np.random.randint(0, img.shape[1] - self.imgshape[1])
        elif self.imgshape[1] == img.shape[1]:
            lb_y = 0

        if self.imgshape[2] < img.shape[2]:
            lb_z = np.random.randint(0, img.shape[2] - self.imgshape[2])
        elif self.imgshape[2] == img.shape[2]:
            lb_z = 0


        #print(lb_x,lb_y,lb_z)
        if lb_x != -1:
            #print(np.sum(label[lb_x:lb_x + self.imgshape[0], lb_y:lb_y + self.imgshape[1], lb_z:lb_z + self.imgshape[2]]))
            return img[lb_x:lb_x + self.imgshape[0], lb_y:lb_y + self.imgshape[1], lb_z:lb_z + self.imgshape[2]], label[lb_x:lb_x + self.imgshape[0], lb_y:lb_y + self.imgshape[1], lb_z:lb_z + self.imgshape[2]]
        else:
            cropimg = img[:, lb_y:lb_y + self.imgshape[1], lb_z:lb_z + self.imgshape[2]]
            cropimg = np.pad(cropimg, ((0, self.imgshape[0] - cropimg.shape[0]), (0, 0), (0, 0)), mode='constant',
                             constant_values=(0, 0))
            croplabel = label[:, lb_y:lb_y + self.imgshape[1], lb_z:lb_z + self.imgshape[2]]
            croplabel = np.pad(croplabel, ((0, self.imgshape[0] - cropimg.shape[0]), (0, 0), (0, 0)), mode='constant',
                             constant_values=(0, 0))
        return cropimg, croplabel
