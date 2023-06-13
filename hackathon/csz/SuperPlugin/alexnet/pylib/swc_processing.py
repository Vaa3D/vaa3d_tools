#!/usr/bin/env python

#================================================================
#   Copyright (C) 2021 Yufeng Liu (Braintell, Southeast University). All rights reserved.
#   
#   Filename     : preprocess.py
#   Author       : Yufeng Liu
#   Date         : 2021-03-31
#   Description  : This package tries to standardize the input image, 
#                  for lowerize the burden when training, including: 
#                  - resampling
#                  - normalization
#                  - format conversion
#                  - dataset splitting
#                  
#================================================================

import os, glob
import numpy as np
from skimage.io import imread, imsave
from skimage.transform import resize
from scipy.ndimage.interpolation import map_coordinates
from copy import deepcopy
import SimpleITK as sitk
from multiprocessing.pool import Pool
import pickle
from skimage.draw import line_nd
import skimage.morphology as morphology

from pylib.swc_handler import parse_swc, write_swc
from pylib.path_util import get_file_prefix


def soma_labelling(image, z_ratio=0.3, r=9, thresh=220, label=255, soma_pos=None):
    dz, dy, dx = image.shape
    if soma_pos is None:
        cx, cy, cz = dx//2, dy//2, dz//2
    else:
        cx, cy, cz = soma_pos   # in x-y-z order
        cx = int(round(cx))
        cy = int(round(cy))
        cz = int(round(cz))

    img_thresh = image.copy()
    img_thresh[img_thresh > thresh] = thresh
    
    rz = int(round(z_ratio * r)) 
    img_labels = []
    zs, ze = cz - rz, cz + rz
    ys, ye = cy - r, cy + r 
    xs, xe = cx - r, cx + r 

    zs = max(0, zs)
    ze = min(ze, dz)
    ys = max(0, ys)
    ye = min(ye, dy)
    xs = max(0, xs)
    xe = min(xe, dx)
    img_thresh[zs:ze, ys:ye, xs:xe] = label
    
    return img_thresh

def is_in_box(x, y, z, imgshape):
    """
    imgshape must be in (z,y,x) order
    """
    if x < 0 or y < 0 or z < 0 or \
        x > imgshape[2] - 1 or \
        y > imgshape[1] - 1 or \
        z > imgshape[0] - 1:
        return False
    return True

def trim_swc(tree_orig, imgshape, keep_candidate_points=True):
    """
    Trim the out-of-box and non_connecting leaves
    """

    def traverse_leaves(idx, child_dict, good_points, cand_pints, pos_dict):
        leaf = pos_dict[idx]
        p_idx, ib = leaf[-2:]

        if (p_idx in good_points) or (p_idx == -1):
            if ib: 
                good_points.add(idx)    # current node
            else:
                cand_points.add(idx)
                return

        if idx not in child_dict:
            return

        for new_idx in child_dict[idx]:
            traverse_leaves(new_idx, child_dict, good_points, cand_pints, pos_dict) 


    # execute trimming
    pos_dict = {}
    tree = deepcopy(tree_orig)
    for i, leaf in enumerate(tree_orig):
        idx, type_, x, y, z, r, p = leaf
        leaf = (idx, type_, x, y, z, r, p, is_in_box(x,y,z,imgshape))
        pos_dict[idx] = leaf
        tree[i] = leaf

    good_points = set() # points and all its upstream parents are in-box
    cand_points = set() # all upstream parents are in-box, itself not
    # initialize the visited set with soma, whose parent index is -1
    soma_idx = None
    for leaf in tree:
        if leaf[-2] == -1:
            soma_idx = leaf[0]
            break

    child_dict = {}
    for leaf in tree:
        if leaf[-2] in child_dict:
            child_dict[leaf[-2]].append(leaf[0])
        else:
            child_dict[leaf[-2]] = [leaf[0]]
    # do DFS searching
    #print(soma_idx)
    traverse_leaves(soma_idx, child_dict, good_points, cand_points, pos_dict)
    #print("#good/#cand/#total:", len(good_points), len(cand_points), len(pos_dict))  
    
    # return the tree, (NOTE: without order)
    tree_trim = []
    if keep_candidate_points:
        keep_points = good_points | cand_points
    else:
        keep_points = good_points

    for i, leaf in enumerate(tree):
        idx = leaf[0]
        if idx in keep_points:
            tree_trim.append(leaf[:-1])

    return tree_trim

def load_spacing(spacing_file, zyx_order=True):
    spacing_dict = {}
    with open(spacing_file) as fp:
        fp.readline()    # skip the first line
        for line in fp.readlines():
            line = line.strip()
            if not line: continue
            if line[0] == '#': continue

            brain_id, xs, ys, zs = line.split(',')
            if zyx_order:
                spacing_dict[int(brain_id)] = np.array([float(zs), float(ys), float(xs)])
            else:
                spacing_dict[int(brain_id)] = np.array([float(xs), float(ys), float(zs)])
    
    return spacing_dict

def trim_out_of_box(tree_orig, imgshape, keep_candidate_points=True):
    """ 
    Trim the out-of-box leaves
    """
    # execute trimming
    child_dict = {}
    for leaf in tree_orig:
        if leaf[-1] in child_dict:
            child_dict[leaf[-1]].append(leaf[0])
        else:
            child_dict[leaf[-1]] = [leaf[0]]
    
    pos_dict = {}
    for i, leaf in enumerate(tree_orig):
        pos_dict[leaf[0]] = leaf

    tree = []
    for i, leaf in enumerate(tree_orig):
        idx, type_, x, y, z, r, p = leaf
        ib = is_in_box(x,y,z,imgshape)
        if ib: 
            tree.append(leaf)
        elif keep_candidate_points:
            if p in pos_dict and is_in_box(*pos_dict[p][2:5], imgshape):
                tree.append(leaf)
            elif idx in child_dict:
                for ch_leaf in child_dict[idx]:
                    if is_in_box(*pos_dict[ch_leaf][2:5], imgshape):
                        tree.append(leaf)
                        break
    return tree

def swc_to_image(tree, r_exp=3, z_ratio=0.4, imgshape=(128,128,128), flipy=False):
    # Note imgshape in (z,y,x) order
    # initialize empty image
    img = np.zeros(shape=imgshape, dtype=np.uint8)
    # get the position tree and parent tree
    pos_dict = {}
    soma_node = None
    for i, leaf in enumerate(tree):
        idx, type_, x, y, z, r, p = leaf
        if p == -1:
            soma_node = leaf
        
        leaf = (idx, type_, x, y, z, r, p, is_in_box(x,y,z,imgshape))
        pos_dict[idx] = leaf
        tree[i] = leaf
        
    xl, yl, zl = [], [], []
    for _, leaf in pos_dict.items():
        idx, type_, x, y, z, r, p, ib = leaf
        if idx == 1: continue   # soma
       
        if p not in pos_dict: 
            continue
        parent_leaf = pos_dict[p]
        if (not ib) and (not parent_leaf[ib]):
            print('All points are out of box! do trim_swc before!')
            raise ValueError
        
        # draw line connect each pair
        cur_pos = leaf[2:5]
        par_pos = parent_leaf[2:5]
        lin = line_nd(cur_pos[::-1], par_pos[::-1], endpoint=True)

        xl.extend(list(lin[0]))
        yl.extend(list(lin[1]))
        zl.extend(list(lin[2]))

    xn, yn, zn = [], [], []
    for (xi,yi,zi) in zip(xl,yl,zl):
        if is_in_box(xi,yi,zi,imgshape):
            xn.append(xi)
            yn.append(yi)
            zn.append(zi)
    img[zn,yn,xn] = 1

    # do morphology expansion
    r_z = int(round(z_ratio * r_exp))
    selem = np.ones((r_z, r_exp, r_exp), dtype=np.uint8)
    img = morphology.dilation(img, selem)
    # soma-labelling
    if soma_node is not None:
        lab_img = soma_labelling(img, r=r_exp*2+1, thresh=220, label=1, soma_pos=soma_node[2:5])
    else:
        lab_img = img
        
    if flipy:
        lab_img = lab_img[:,::-1]   # flip in y-axis, as the image is flipped


    return lab_img
    
if __name__ == '__main__':
    prefix = '8315_19523_2299'
    swc_file = f'/home/lyf/Research/auto_trace/neuronet/data/task0001_17302/{prefix}.swc'
    tree = parse_swc(swc_file)
    lab_img = swc_to_image(tree, 3, 0.4, (256,512,512))
    sitk.WriteImage(sitk.GetImageFromArray(lab_img), f'{prefix}_label.tiff')

