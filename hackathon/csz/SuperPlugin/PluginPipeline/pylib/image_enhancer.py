#!/usr/bin/env python

#================================================================
#   Copyright (C) 2021 Yufeng Liu (Braintell, Southeast University). All rights reserved.
#   
#   Filename     : image_enhancer.py
#   Author       : Yufeng Liu
#   Date         : 2021-05-17
#   Description  : 
#
#================================================================
import numpy as np
from skimage import exposure

def do_gamma(img, gamma, trunc_thresh=0, normalize=True, epsilon=1e-7):
    img = img.astype(np.float32)
    minm = img.min()
    rnge = img.max() - minm

    img = (img - minm) / float(rnge + epsilon)
    if trunc_thresh <= 0 or trunc_thresh >= 1:
        img = np.power(img, gamma) * rnge + minm
    else:
        mask = img < trunc_thresh
        img[mask] = np.power(img[mask], gamma)
        img[~mask] = img[~mask] - trunc_thresh + np.power(trunc_thresh, gamma)
        img = img * rnge + minm

    if normalize:
        img = (img - img.min()) / (img.max() - img.min() + epsilon) * 255.
        
    return img

def do_CLAHE(img, kernel_size, clip_limit=0.01, nbins=256, normalize=True, epsilon=1e-7):
    assert img.dtype == np.uint8
    img = exposure.equalize_adapthist(img, kernel_size=kernel_size, clip_limit=clip_limit, nbins=nbins)
    if normalize:
        img = (img - img.min()) / (img.max() - img.min() + epsilon) * 255.
    return img
    

