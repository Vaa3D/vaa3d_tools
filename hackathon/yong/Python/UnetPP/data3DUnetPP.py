#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from __future__ import print_function

import os
from skimage.io import imsave, imread
from libtiff import TIFF
from scipy import misc
import cv2
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

data_path = 'C:\\Users\\admin\\Desktop\\zhangyong\\unetpp\\raw'
image_x = 32
image_y = 32
image_z = 16

def create_train_data():
    train_data_path = os.path.join(data_path, 'train')
    images = os.listdir(train_data_path)
    total = int(len(images)/2)

    imgs_train = np.ndarray((total, image_x, image_y, image_z), dtype=np.uint8)
    imgs_mask_train = np.ndarray((total, image_x, image_y, image_z), dtype=np.uint8)

    print('Creating training images...')
    
    i = 0
    for image_name in images:
        if 'mask' in image_name:
            continue
        image_mask_name = image_name.split('.')[0] + "_mask.tif"
        #print(train_data_path)
        #print(image_name)
        tif = TIFF.open(train_data_path + '/' + image_name, mode = 'r')
        tif_mask = TIFF.open(train_data_path + '/' + image_mask_name, mode = 'r')
        
        count = 0  
        imageAll = np.array([0])         
        for image in tif.iter_images():
            if count == 0:
                imageAll = image
            else:
                imageAll = np.dstack((imageAll,image))
            count = count +1

        count_mask = 0
        imageAll_mask = np.array([0])
        for image_mask in tif_mask.iter_images():
            if count_mask == 0:
                imageAll_mask = image_mask
            else:
                imageAll_mask = np.dstack((imageAll_mask,image_mask))
            count_mask = count_mask + 1

        imgs_train[i] = imageAll
        imgs_mask_train[i] = imageAll_mask
        
        i = i+1

    print(imgs_train.shape)
    print(imgs_mask_train.shape)
    np.save('images_train.npy',imgs_train)
    np.save('images_mask_train.npy',imgs_mask_train)
  
    print('Creating training images done!')

    # 加载.npy文件，验证保存是否成功
    # matrix = np.load('images_trian.npy')
    # print(matrix)
    # plt.matshow(matrix[1,:,:,1],cmap='Greys_r')
    # plt.show() 

    # matrix = np.load('images_trian.npy')
    # n = matrix.shape[3]
    # for i in range(total):
    #     for j in range(n):
    #         plt.matshow(matrix[i,:,:,j],cmap='Greys_r')
    #         plt.show()

    # matrix_mask = np.load("images_mask_train.npy")
    # print(matrix_mask)
    # m = matrix_mask.shape[2]
    # for i in range(total):
    #     for j in range(m):
    #         plt.matshow(matrix_mask[i,:,:,j],cmap='Greys_r')
    #         plt.show()

def load_train_data():
    images_train = np.load('images_train.npy')
    images_mask_train = np.load('images_mask_train.npy')
    return images_train, images_mask_train


def create_test_data():
    test_data_path = os.path.join(data_path,'test')
    images = os.listdir(test_data_path)
    total = len(images)

    imgs_test = np.ndarray((total, image_x, image_y, image_z), dtype=np.uint8)
    #imgs_test_id = np.ndarray((total,), dtype = np.str)
    imgs_test_id = list()
    
    print('Creating testing images')

    i = 0
    for image_name in images:   
        tif = TIFF.open(test_data_path + '/' + image_name, mode = 'r')
        img_test_id = str(image_name.split('.')[0])

        count_test = 0  
        imageAll_test = np.array([0])         
        for image in tif.iter_images():
            if count_test == 0:
                imageAll_test = image
            else:
                imageAll_test = np.dstack((imageAll_test,image))
            count_test = count_test +1

        imgs_test[i] = imageAll_test
        imgs_test_id.append(img_test_id)
        i = i+1

    print(imgs_test.shape)
    np.save('images_test.npy',imgs_test)
    np.save('images_test_id.npy',imgs_test_id)

    print('Creating testing images done!')
    
def load_test_data():
    images_test = np.load('images_test.npy')
    images_test_id = np.load('images_test_id.npy')
    return images_test, images_test_id


if __name__ == '__main__':
    create_train_data()
    create_test_data()
