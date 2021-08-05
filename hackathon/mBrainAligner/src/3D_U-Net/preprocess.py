# -*- coding: utf-8 -*-

import os
import numpy as np
import SimpleITK as sitk
from random import random
from scipy.ndimage.interpolation import zoom,rotate
import copy
import itertools


def generate_permutation_keys():
    return set(itertools.product(
        itertools.combinations_with_replacement(range(2), 2), range(2), range(2), range(2), range(2)))


def random_permutation_key():
    return random.choice(list(generate_permutation_keys()))


def permute_data(data, key):
    data = np.copy(data)
    (rotate_y, rotate_z), flip_x, flip_y, flip_z, transpose = key

    if rotate_y != 0:
        data = np.rot90(data, rotate_y, axes=(1, 3))
    if rotate_z != 0:
        data = np.rot90(data, rotate_z, axes=(2, 3))
    if flip_x:
        data = data[:, ::-1]
    if flip_y:
        data = data[:, :, ::-1]
    if flip_z:
        data = data[:, :, :, ::-1]
    if transpose:
        for i in range(data.shape[0]):
            data[i] = data[i].T
    return data


def random_permutation_x_y(x_data, y_data):
    key = random_permutation_key()
    return permute_data(x_data, key), permute_data(y_data, key)


def nii2tensorarray1(data):
    [c, z, y, x] = data.shape
    new_data = np.reshape(data, [c, z, y, x])
    new_data = new_data.astype("float32")
    return new_data


def nii2tensorarray2(label):
    new_data = label.astype("float32")
    new_data[new_data > 0] = 1
    return new_data


def JudgeBoundary(array,w,h,l):
    if ((array[w-1,h,l] + array[w+1,h,l] == 1.0 and array[w-1,h,l] * array[w+1,h,l] == 0.0)
    or (array[w,h-1,l] + array[w,h+1,l] == 1.0 and array[w,h-1,l] * array[w,h+1,l] == 0.0)
    or (array[w,h,l-1] + array[w,h,l+1] == 1.0 and array[w,h,l-1] * array[w,h,l+1] == 0.0)):
        return True
    else:
        return False


def BoundLabel(Array):
    Array[Array > 0] = 1.0
    (W, H, L) = Array.shape
    out_array = np.zeros((W, H, L))
    BoundD = 2
    for w in range(W):
        for h in range(H):
            for l in range(L):
                if Array[w, h, l] > 0 and JudgeBoundary(Array, w, h, l):
                    out_array[w, h, l] = 1
                    if Array[w - 1, h, l] < 1:
                        out_array[w - BoundD:w, h, l] = 1
                    if Array[w, h - 1, l] < 1:
                        out_array[w, h - BoundD:h, l] = 1
                    if Array[w, h, l - 1] < 1:
                        out_array[w, h, l - BoundD:l] = 1
                    if Array[w + 1, h, l] < 1:
                        out_array[w:w + BoundD, h, l] = 1
                    if Array[w, h + 1, l] < 1:
                        out_array[w, h:h + BoundD, l] = 1
                    if Array[w, h, l + 1] < 1:
                        out_array[w, h, l:l + BoundD] = 1
    return Array + out_array


def Classification_label(LabelArray,nlables = 1):
    input_shape = LabelArray.shape
    LabelArray1_1 = np.zeros((1, 1, input_shape[2],nlables))
    LabelArray1_2 = np.zeros((1, input_shape[1], 1,nlables))
    LabelArray1_3 = np.zeros((input_shape[0], 1, 1,nlables))

    for j in range(nlables):
        for i in range(input_shape[2]):
            if j in LabelArray[:, :, i]:
                LabelArray1_1[:, :, i,j] = 1

        for i in range(input_shape[1]):
            if j in LabelArray[:, i, :]:
                LabelArray1_2[:, i, :,j] = 1
        for i in range(input_shape[0]):
            if j in LabelArray[i, :, :]:
                LabelArray1_3[i, :, :,j] = 1
    return LabelArray1_1,LabelArray1_2,LabelArray1_3


def Classification_label1(LabelArray):
    input_shape = LabelArray.shape
    LabelArray1_1 = np.zeros((1, 1, input_shape[2]))
    LabelArray1_2 = np.zeros((1, input_shape[1], 1))
    LabelArray1_3 = np.zeros((input_shape[0], 1, 1))
    for i in range(input_shape[2]):
        LabelArray1_1[:, :, i] = np.max(LabelArray[:, :, i])
    for i in range(input_shape[1]):
        LabelArray1_2[:, i, :] = np.max(LabelArray[:, i, :])
    for i in range(input_shape[0]):
        LabelArray1_3[i, :, :] = np.max(LabelArray[i, :, :])
    LabelArray1_1 = np.reshape(LabelArray1_1,(input_shape[2]))
    LabelArray1_2 = np.reshape(LabelArray1_2, (input_shape[1]))
    LabelArray1_3 = np.reshape(LabelArray1_3, (input_shape[0]))
    ClassArray = np.array([LabelArray1_1,LabelArray1_2,LabelArray1_3])
    return ClassArray.flatten()


def Classification_label2(LabelArray):
    input_shape = LabelArray.shape
    LabelArray1_3 = np.zeros((1, input_shape[1], 1))
    for i in range(input_shape[1]):
        LabelArray1_3[:, i, :] = np.max(LabelArray[:, i, :])
    LabelArray1_3 = np.reshape(LabelArray1_3, (input_shape[1]))
    return LabelArray1_3


def drop_invalid_range(volume, label=None):
    zero_value = volume[0, 0, 0]
    non_zeros_idx = np.where(volume != zero_value)
    [max_z, max_h, max_w] = np.max(np.array(non_zeros_idx), axis=1)
    [min_z, min_h, min_w] = np.min(np.array(non_zeros_idx), axis=1)

    if label is not None:
        return volume[min_z:max_z, min_h:max_h, min_w:max_w], \
               label[min_z:max_z, min_h:max_h, min_w:max_w]
    else:
        return volume[min_z:max_z, min_h:max_h, min_w:max_w]


def random_center_crop(data, label):
    target_indexs = np.where(label > 0)
    [img_d, img_h, img_w] = data.shape
    [max_D, max_H, max_W] = np.max(np.array(target_indexs), axis=1)
    [min_D, min_H, min_W] = np.min(np.array(target_indexs), axis=1)
    [target_depth, target_height, target_width] = np.array([max_D, max_H, max_W]) - np.array([min_D, min_H, min_W])
    Z_min = int((min_D - target_depth * 1.0 / 2) * random())
    Y_min = int((min_H - target_height * 1.0 / 2) * random())
    X_min = int((min_W - target_width * 1.0 / 2) * random())
    Z_max = int(img_d - ((img_d - (max_D + target_depth * 1.0 / 2)) * random()))
    Y_max = int(img_h - ((img_h - (max_H + target_height * 1.0 / 2)) * random()))
    X_max = int(img_w - ((img_w - (max_W + target_width * 1.0 / 2)) * random()))

    Z_min = np.max([0, Z_min])
    Y_min = np.max([0, Y_min])
    X_min = np.max([0, X_min])

    Z_max = np.min([img_d, Z_max])
    Y_max = np.min([img_h, Y_max])
    X_max = np.min([img_w, X_max])

    Z_min = int(Z_min)
    Y_min = int(Y_min)
    X_min = int(X_min)

    Z_max = int(Z_max)
    Y_max = int(Y_max)
    X_max = int(X_max)
    return data[Z_min: Z_max, Y_min: Y_max, X_min: X_max], \
           label[Z_min: Z_max, Y_min: Y_max, X_min: X_max]


def itensity_normalize_one_volume(volume):
    pixels = volume[volume > 0]
    mean = pixels.mean()
    std = pixels.std()
    out = (volume - mean) / std
    out_random = np.random.normal(0, 1, size=volume.shape)
    out[volume == 0] = out_random[volume == 0]
    # out[volume == 0] = 0
    return out


def resize_data(data, input_shape):
    (input_D, input_H, input_W) = input_shape
    [depth, height, width] = data.shape
    scale = [input_D * 1.0 / depth, input_H * 1.0 / height, input_W * 1.0 / width]
    data = zoom(data, scale, order=0)
    return data


def crop_data(data, label):
    # random center crop
    data, label = random_center_crop(data, label)
    return data, label


def flip(array, axis=0):
    if axis==0:
        return array[::-1,:,:]
    elif axis==1:
        return array[:,::-1,:]
    else:
        return array


# rotate(data, angle, axes=(0, 1), reshape=True, output=None, order=3, mode='constant', cval=0.0, prefilter=True)
def data_process(data, label, input_shape, Train_Flag=False):
    augment = False
    if(augment):
        # if np.random.randint(0, 10) > 4:
        #     axis = np.random.randint(0, 3)
        #     data = flip(data, axis=axis)
        #     label = flip(label, axis=axis)

        if np.random.randint(0, 10) > 6:
            angle = np.random.randint(0, 5)
            data = rotate(data, angle, axes=(0, 1), order=3, reshape=False)
            label = rotate(label, angle, axes=(0, 1), order=3, reshape=False)

        if np.random.randint(0, 10) > 6:
            angle = np.random.randint(0, 5)
            data = rotate(data, angle, axes=(0,2), order=3, reshape=False)
            label = rotate(label, angle, axes=(0, 2), order=3, reshape=False)

        if np.random.randint(0, 10) > 6:
            angle = np.random.randint(0, 5)
            data = rotate(data, angle, axes=(1, 2), order=3, reshape=False)
            label = rotate(label, angle, axes=(1, 2), order=3, reshape=False)


    data = resize_data(data,input_shape)
    label = resize_data(label,input_shape)
    # normalization datas
    data = itensity_normalize_one_volume(data)
    return data, label


def LoadImage(imagePath):
    image = sitk.ReadImage(imagePath)
    array = sitk.GetArrayFromImage(image)
    return array
