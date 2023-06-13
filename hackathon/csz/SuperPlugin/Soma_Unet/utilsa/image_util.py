#!/usr/bin/env python

#================================================================
#   Copyright (C) 2021 Yufeng Liu (Braintell, Southeast University). All rights reserved.
#   
#   Filename     : image_util.py
#   Author       : Yufeng Liu
#   Date         : 2021-04-05
#   Description  : 
#
#================================================================
import numpy as np

def normalize_normal(image4d, mask=None):
    assert image4d.ndim == 4, "image must in 4 dimension: c,z,y,x"
    assert image4d.dtype.name.startswith('float')
    for c in range(image4d.shape[0]):
        if mask is None:
            image4d[c] = (image4d[c] - image4d[c].mean()) / (image4d[c].std() + 1e-8)
        else:
            image4d[c][mask] = (image4d[c][mask] - image4d[c][mask].mean()) / (image4d[c][mask].std() + 1e-8) 
            image4d[c][mask==0] = 0 
    return image4d

def unnormalize_normal(image4d, output_range=(0,255)):
    assert image4d.ndim == 4
    assert image4d.dtype.name.startswith('float')
    for c in range(image4d.shape[0]):
        or1, or2 = output_range
        m1, m2 = image4d[c].min(), image4d[c].max()
        image4d[c] = (image4d[c] - m1) * (or2 - or1) / (m2 - m1 + 1e-8) + or1
    return image4d

def random_crop_3D_image(img, crop_size):
    if type(crop_size) not in (tuple, list, np.ndarray):
        crop_size = [crop_size] * len(img.shape)
    else:
        assert len(crop_size) == len(
            img.shape), "If you provide a list/tuple as center crop make sure it has the same len as your data has dims (3d)"

    if crop_size[0] < img.shape[0]:
        lb_x = np.random.randint(0, img.shape[0] - crop_size[0])
    elif crop_size[0] == img.shape[0]:
        lb_x = 0
    else:
        lb_x=-1
        # raise ValueError("crop_size[0] must be smaller or equal to the images x dimension")

    if crop_size[1] < img.shape[1]:
        lb_y = np.random.randint(0, img.shape[1] - crop_size[1])
    elif crop_size[1] == img.shape[1]:
        lb_y = 0
    else:
        raise ValueError("crop_size[1] must be smaller or equal to the images y dimension")

    if crop_size[2] < img.shape[2]:
        lb_z = np.random.randint(0, img.shape[2] - crop_size[2])
    elif crop_size[2] == img.shape[2]:
        lb_z = 0
    else:
        raise ValueError("crop_size[2] must be smaller or equal to the images z dimension")

    if lb_x!=-1:
        return img[lb_x:lb_x + crop_size[0], lb_y:lb_y + crop_size[1], lb_z:lb_z + crop_size[2]], lb_x, lb_y, lb_z
    else:
        cropimg=img[:,lb_y:lb_y + crop_size[1], lb_z:lb_z + crop_size[2]]
        cropimg=np.pad(cropimg,((0,crop_size[0]-cropimg.shape[0]),(0,0),(0,0)),mode='constant',constant_values=(0, 0))
        return cropimg,0,lb_y,lb_z


def augment_gamma(data_sample, gamma_range=(0.5, 2), invert_image=False, epsilon=1e-7, per_channel=False,
                  retain_stats=False):
    """Function directly copied from batchgenerators"""
    if invert_image:
        data_sample = - data_sample
    if not per_channel:
        if retain_stats:
            mn = data_sample.mean()
            sd = data_sample.std()
        if np.random.random() < 0.5 and gamma_range[0] < 1:
            gamma = np.random.uniform(gamma_range[0], 1)
        else:
            gamma = np.random.uniform(max(gamma_range[0], 1), gamma_range[1])
        minm = data_sample.min()
        rnge = data_sample.max() - minm
        data_sample = np.power(((data_sample - minm) / float(rnge + epsilon)), gamma) * rnge + minm
        if retain_stats:
            data_sample = data_sample - data_sample.mean()
            data_sample = data_sample / (data_sample.std() + 1e-8) * sd
            data_sample = data_sample + mn
    else:
        for c in range(data_sample.shape[0]):
            if retain_stats:
                mn = data_sample[c].mean()
                sd = data_sample[c].std()
            if np.random.random() < 0.5 and gamma_range[0] < 1:
                gamma = np.random.uniform(gamma_range[0], 1)
            else:
                gamma = np.random.uniform(max(gamma_range[0], 1), gamma_range[1])
            minm = data_sample[c].min()
            rnge = data_sample[c].max() - minm
            data_sample[c] = np.power(((data_sample[c] - minm) / float(rnge + epsilon)), gamma) * float(rnge + epsilon) + minm
            if retain_stats:
                data_sample[c] = data_sample[c] - data_sample[c].mean()
                data_sample[c] = data_sample[c] / (data_sample[c].std() + 1e-8) * sd
                data_sample[c] = data_sample[c] + mn
    if invert_image:
        data_sample = - data_sample
    return data_sample

def do_gamma(data_sample, gamma, trunc_thresh=0, invert_image=False, epsilon=1e-7, per_channel=False,
                  retain_stats=False):
    """Function directly copied from batchgenerators"""
    if invert_image:
        data_sample = - data_sample
    if not per_channel:
        if retain_stats:
            mn = data_sample.mean()
            sd = data_sample.std()

        minm = data_sample.min()
        rnge = data_sample.max() - minm
        if trunc_thresh <= 0 or trunc_thresh >= 1:
            data_sample = np.power(((data_sample - minm) / float(rnge + epsilon)), gamma) * rnge + minm
        else:
            data_sample = (data_sample - minm) / float(rnge + epsilon)
            mask = data_sample < trunc_thresh
            data_sample[mask] = np.power(data_sample[mask], gamma)
            data_sample[~mask] = data_sample[~mask] - trunc_thresh + np.power(trunc_thresh, gamma)
            data_sample = data_sample * rnge + minm
        if retain_stats:
            data_sample = data_sample - data_sample.mean()
            data_sample = data_sample / (data_sample.std() + 1e-8) * sd
            data_sample = data_sample + mn
    else:
        for c in range(data_sample.shape[0]):
            if retain_stats:
                mn = data_sample[c].mean()
                sd = data_sample[c].std()

            minm = data_sample[c].min()
            rnge = data_sample[c].max() - minm
            if trunc_thresh <= 0 or trunc_thresh >= 1:
                data_sample[c] = np.power(((data_sample[c] - minm) / float(rnge + epsilon)), gamma) * float(rnge + epsilon) + minm
            else:
                data_sample[c] = (data_sample[c] - minm) / float(rnge + epsilon)
                mask = data_sample[c] < trunc_thresh
                data_sample[c][mask] = np.power(data_sample[c][mask], gamma)
                data_sample[c][~mask] = data_sample[c][~mask] - trunc_thresh + np.power(trunc_thresh, gamma)
                data_sample[c] = data_sample[c] * rnge + minm

            if retain_stats:
                data_sample[c] = data_sample[c] - data_sample[c].mean()
                data_sample[c] = data_sample[c] / (data_sample[c].std() + 1e-8) * sd
                data_sample[c] = data_sample[c] + mn
    if invert_image:
        data_sample = - data_sample
    return data_sample
