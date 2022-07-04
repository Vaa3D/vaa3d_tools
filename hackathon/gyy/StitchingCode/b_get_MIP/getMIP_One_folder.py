import cv2
import os
import numpy as np
import tifffile
from multiprocessing import Process

'''
Get MIP images with all tiles in a folder.
'''

# input = 'F:/ZhaoHuImages/high_res_20220219'
# output = 'F:/ZhaoHuImages/MIP_results'
input = 'G:/Zhaohu0418/stack3_after_deeps'
output = 'F:/ZhaoHuImages/AI_denoise/MIP_results'

MIP_num = 20
overlap_X = 51*1.0/512
overlap_Y = 51*1.0/512
imgSzY = 512
imgSzX = 512
imgSzZ = 201

def XY_MIP_function(dir_path, savePath, tilename):
    image_res = np.zeros((imgSzY, imgSzX), dtype=np.int8)
#    big_image = np.zeros((imgSzZ, imgSzY, imgSzX), dtype=np.int)

    for i in range(imgSzZ):
        img_path_i = dir_path + '/' + 'TileScan 1_s' + tilename + '_z' + str(i).zfill(3) + '_RAW_ch00_dst.tif'
        image_i = cv2.imread(img_path_i, -1)

        for j in range(imgSzY):
            for k in range(imgSzX):
                if image_res[j][k] < image_i[j][k]:
                    image_res[j][k] = image_i[j][k]

    saveName = savePath + '/' + 's' + tilename + '_XY_MIP.tif'
    tifffile.imsave(saveName, image_res)

def YZ_MIP_function(dir_path, savePath, tilename):
    image_res = np.zeros((imgSzZ, imgSzY), dtype=np.int8)

    for i in range(imgSzZ):
        img_path_i = dir_path + '/' + 'TileScan 1_s' + tilename + '_z' + str(i).zfill(3) + '_RAW_ch00_dst.tif'
        image_i = cv2.imread(img_path_i, -1)

        for j in range(imgSzY):
            maxPro = np.max(image_i[j])
            image_res[i][j] = maxPro

    saveName = savePath + '/' + 's' + tilename + '_YZ_MIP.tif'
    tifffile.imsave(saveName, image_res)

def XZ_MIP_function(dir_path, savePath, tilename):
    image_res = np.zeros((imgSzZ, imgSzX), dtype=np.int8)

    for i in range(imgSzZ):
        img_path_i = dir_path + '/' + 'TileScan 1_s' + tilename + '_z' + str(i).zfill(3) + '_RAW_ch00_dst.tif'
        image_i = cv2.imread(img_path_i, -1)

        for j in range(imgSzX):
            maxPro = np.max(image_i[j])
            image_res[i][j] = maxPro

    saveName = savePath + '/' + 's' + tilename + '_XZ_MIP.tif'
    tifffile.imsave(saveName, image_res)

def YZ_projection_MIP_L(dir_path, savePath, tilename):
    proCenter = int(imgSzX * overlap_X / 2)
    print('proCenter = ', proCenter)
    image_res = np.zeros((imgSzZ, imgSzY), dtype=np.int8)

    for i in range(imgSzZ):
        img_path_i = dir_path + '/' + 'TileScan 1_s' + tilename + '_z' + str(i).zfill(3) + '_RAW_ch00_dst.tif'
        image_i = cv2.imread(img_path_i, -1)

        for j in range(imgSzY):
            maxPro = 0
            for k in range(proCenter-MIP_num, proCenter+MIP_num):
                if maxPro < image_i[j][k]:
                    maxPro = image_i[j][k]
            image_res[i][j] = maxPro

    saveName = savePath + '/' + 's' + tilename + '_YZ_MIP_L.tif'
    # saveName = savePath  + '_YZ_MIP_L.tif'
    # image_res = image_res.astype(np.uint16)
    tifffile.imsave(saveName, image_res)

def YZ_projection_MIP_R(dir_path, savePath, tilename):
    proCenter = int(imgSzX - imgSzX*overlap_X/2)
    # proCenter = int(imgSzX * overlap_X / 2)
    print('proCenter = ', proCenter)
    image_res = np.zeros((imgSzZ, imgSzY), dtype=np.int8)

    for i in range(imgSzZ):
        img_path_i = dir_path + '/' + 'TileScan 1_s' + tilename + '_z' + str(i).zfill(3) + '_RAW_ch00_dst.tif'
        image_i = cv2.imread(img_path_i, -1)

        for j in range(imgSzY):
            maxPro = 0
            for k in range(proCenter-MIP_num, proCenter+MIP_num):
                if maxPro < image_i[j][k]:
                    maxPro = image_i[j][k]
            image_res[i][j] = maxPro

    saveName = savePath + '/' + 's' + tilename + '_YZ_MIP_R.tif'
    # saveName = savePath  + '_YZ_MIP_R.tif'
    # image_res = image_res.astype(np.uint16)
    tifffile.imsave(saveName, image_res)

def XZ_projection_MIP_U(dir_path, savePath, tilename):
    proCenter = int(imgSzY * overlap_Y / 2)
    print('proCenter = ', proCenter)
    image_res = np.zeros((imgSzZ, imgSzX), dtype=np.int8)

    for i in range(imgSzZ):
        img_path_i = dir_path + '/' + 'TileScan 1_s' + tilename + '_z' + str(i).zfill(3) + '_RAW_ch00_dst.tif'
        image_i = cv2.imread(img_path_i, -1)

        for j in range(imgSzX):
            maxPro = 0
            for k in range(proCenter-MIP_num, proCenter+MIP_num):
                if maxPro < image_i[k][j]:
                    maxPro = image_i[k][j]
            image_res[i][j] = maxPro

    saveName = savePath + '/' + 's' + tilename + '_XZ_MIP_U.tif'
    # image_res = image_res.astype(np.uint16)
    tifffile.imsave(saveName, image_res)

def XZ_projection_MIP_D(dir_path, savePath, tilename):
    proCenter = int(imgSzY - imgSzY * overlap_Y / 2)
    # proCenter = int(imgSzY * overlap_Y / 2)
    print('proCenter = ', proCenter)
    image_res = np.zeros((imgSzZ, imgSzX), dtype=np.int8)

    for i in range(imgSzZ):
        img_path_i = dir_path + '/' + 'TileScan 1_s' + tilename + '_z' + str(i).zfill(3) + '_RAW_ch00_dst.tif'
        image_i = cv2.imread(img_path_i, -1)

        for j in range(imgSzX):
            maxPro = 0
            for k in range(proCenter-MIP_num, proCenter+MIP_num):
                if maxPro < image_i[k][j]:
                    maxPro = image_i[k][j]
            image_res[i][j] = maxPro

    saveName = savePath + '/' + 's' + tilename + '_XZ_MIP_D.tif'
    # image_res = image_res.astype(np.uint16)
    tifffile.imsave(saveName, image_res)


for i in range(370, 570):
    XY_MIP_function(input, output, str(i).zfill(4))
    # YZ_MIP_function(input, output, str(i).zfill(4))
    # XZ_MIP_function(input, output, str(i).zfill(4))
    YZ_projection_MIP_L(input, output, str(i).zfill(4))
    YZ_projection_MIP_R(input, output, str(i).zfill(4))
    XZ_projection_MIP_U(input, output, str(i).zfill(4))
    XZ_projection_MIP_D(input, output, str(i).zfill(4))






