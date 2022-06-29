import cv2
import os
import numpy as np
import tifffile

'''
Get MIP images with a tile for a folder.
'''

dir_dir_path = 'H:/Zhaohu_20220614'
# dir_dir_path = 'F:/pavelData/downsample_data/data_down32'
savePath = 'H:/Zhaohu_20220614_results/MIP_results'

# MIP_num = 20
overlap_X = 0.1
overlap_Y = 0.1


def YZ_projection_MIP_L(dir_path, savePath):
    # img_list = os.listdir(dir_path)
    img_list = sorted(os.listdir(dir_path), key=lambda x: int(str(x).split('.')[0].split('_')[1]))
    imgSzZ = len(img_list)
    img_name0 = img_list[0]
    img_path0 = dir_path + '/' + img_name0
    img0 = cv2.imread(img_path0, -1)
    [imgSzY, imgSzX] = img0.shape

    MIP_num = int(imgSzX * overlap_X / 2) - 1
    # proCenter = int(imgSzX - imgSzX*overlap_X/2)
    proCenter = int(imgSzX * overlap_X / 2)
    print('proCenter = ', proCenter)
    image_res = np.zeros((imgSzZ, imgSzY), dtype=np.int16)

    for i in range(imgSzZ):
        img_path_i = dir_path + '/' + img_list[i]
        image_i = cv2.imread(img_path_i, -1)

        for j in range(imgSzY):
            maxPro = 0
            for k in range(proCenter-MIP_num, proCenter+MIP_num):
                if maxPro < image_i[j][k]:
                    maxPro = image_i[j][k]
            image_res[i][j] = maxPro

    saveName = savePath + '/' + os.path.basename(dir_path) + '_YZ_MIP_L.tif'
    # saveName = savePath  + '_YZ_MIP_L.tif'
    image_res = image_res.astype(np.uint16)
    tifffile.imsave(saveName, image_res)

def YZ_projection_MIP_R(dir_path, savePath):
    # img_list = os.listdir(dir_path)
    img_list = sorted(os.listdir(dir_path), key=lambda x: int(str(x).split('.')[0].split('_')[1]))
    imgSzZ = len(img_list)
    img_name0 = img_list[0]
    img_path0 = dir_path + '/' + img_name0
    img0 = cv2.imread(img_path0, -1)
    [imgSzY, imgSzX] = img0.shape

    MIP_num = int(imgSzX * overlap_X / 2) - 1
    proCenter = int(imgSzX - imgSzX*overlap_X/2)
    # proCenter = int(imgSzX * overlap_X / 2)
    print('proCenter = ', proCenter)
    image_res = np.zeros((imgSzZ, imgSzY), dtype=np.int16)

    for i in range(imgSzZ):
        img_path_i = dir_path + '/' + img_list[i]
        image_i = cv2.imread(img_path_i, -1)

        for j in range(imgSzY):
            maxPro = 0
            for k in range(proCenter-MIP_num, proCenter+MIP_num):
                if maxPro < image_i[j][k]:
                    maxPro = image_i[j][k]
            image_res[i][j] = maxPro

    saveName = savePath + '/' + os.path.basename(dir_path) + '_YZ_MIP_R.tif'
    # saveName = savePath  + '_YZ_MIP_R.tif'
    image_res = image_res.astype(np.uint16)
    tifffile.imsave(saveName, image_res)

def XZ_projection_MIP_U(dir_path, savePath):
    # img_list = os.listdir(dir_path)
    img_list = sorted(os.listdir(dir_path), key=lambda x: int(str(x).split('.')[0].split('_')[1]))
    imgSzZ = len(img_list)
    img_name0 = img_list[0]
    img_path0 = dir_path + '/' + img_name0
    img0 = cv2.imread(img_path0, -1)
    [imgSzY, imgSzX] = img0.shape

    MIP_num = int(imgSzY * overlap_Y / 2) - 1
    # proCenter = int(imgSzY - imgSzY * overlap_Y / 2)
    proCenter = int(imgSzY * overlap_Y / 2)
    print('proCenter = ', proCenter)
    image_res = np.zeros((imgSzZ, imgSzX), dtype=np.int16)

    for i in range(imgSzZ):
        img_path_i = dir_path + '/' + img_list[i]
        image_i = cv2.imread(img_path_i, -1)

        for j in range(imgSzX):
            maxPro = 0
            for k in range(proCenter-MIP_num, proCenter+MIP_num):
                if maxPro < image_i[k][j]:
                    maxPro = image_i[k][j]
            image_res[i][j] = maxPro

    saveName = savePath + '/' + os.path.basename(dir_path) + '_XZ_MIP_U.tif'
    image_res = image_res.astype(np.uint16)
    tifffile.imsave(saveName, image_res)

def XZ_projection_MIP_D(dir_path, savePath):
    # img_list = os.listdir(dir_path)
    img_list = sorted(os.listdir(dir_path), key=lambda x: int(str(x).split('.')[0].split('_')[1]))
    imgSzZ = len(img_list)
    img_name0 = img_list[0]
    img_path0 = dir_path + '/' + img_name0
    img0 = cv2.imread(img_path0, -1)
    [imgSzY, imgSzX] = img0.shape

    MIP_num = int(imgSzY * overlap_Y / 2) - 1
    proCenter = int(imgSzY - imgSzY * overlap_Y / 2)
    # proCenter = int(imgSzY * overlap_Y / 2)
    print('proCenter = ', proCenter)
    image_res = np.zeros((imgSzZ, imgSzX), dtype=np.int16)

    for i in range(imgSzZ):
        img_path_i = dir_path + '/' + img_list[i]
        image_i = cv2.imread(img_path_i, -1)

        for j in range(imgSzX):
            maxPro = 0
            for k in range(proCenter-MIP_num, proCenter+MIP_num):
                if maxPro < image_i[k][j]:
                    maxPro = image_i[k][j]
            image_res[i][j] = maxPro

    saveName = savePath + '/' + os.path.basename(dir_path) + '_XZ_MIP_D.tif'
    image_res = image_res.astype(np.uint16)
    tifffile.imsave(saveName, image_res)

def XY_MIP_function(dir_path, savePath):
    # img_list = os.listdir(dir_path)
    img_list = sorted(os.listdir(dir_path), key=lambda x: int(str(x).split('.')[0].split('_')[1]))
    imgSzZ = len(img_list)
    img_name0 = img_list[0]
    img_path0 = dir_path + '/' + img_name0
    img0 = cv2.imread(img_path0, -1)
    [imgSzY, imgSzX] = img0.shape

    image_res = np.zeros((imgSzY, imgSzX), dtype=np.int16)
#    big_image = np.zeros((imgSzZ, imgSzY, imgSzX), dtype=np.int)

    for i in range(imgSzZ):
        img_path_i = dir_path + '/' + img_list[i]
        image_i = cv2.imread(img_path_i, -1)

        for j in range(imgSzY):
            for k in range(imgSzX):
                if image_res[j][k] < image_i[j][k]:
                    image_res[j][k] = image_i[j][k]

    saveName = savePath + '/' + os.path.basename(dir_path) + '_XY_MIP.tif'
    image_res = image_res.astype(np.uint16)
    tifffile.imsave(saveName, image_res)



def YZ_MIP_function(dir_path, savePath):
    # img_list = os.listdir(dir_path)
    img_list = sorted(os.listdir(dir_path), key=lambda x: int(str(x).split('.')[0].split('_')[1]))
    imgSzZ = len(img_list)
    img_name0 = img_list[0]
    img_path0 = dir_path + '/' + img_name0
    img0 = cv2.imread(img_path0, -1)
    [imgSzY, imgSzX] = img0.shape
    saveName = savePath + '/' + os.path.basename(dir_path) + '_YZ_MIP.tif'
    print(saveName)

    # proCenter = int(imgSzX - imgSzX*overlap_X/2)
    # proCenter = int(imgSzX * overlap_X / 2)
    # print('proCenter = ', proCenter)
    image_res = np.zeros((imgSzZ, imgSzY), dtype=np.int16)

    for i in range(imgSzZ):
        img_path_i = dir_path + '/' + img_list[i]
        image_i = cv2.imread(img_path_i, -1)

        for j in range(imgSzY):
            maxPro =np.max(image_i[j])
            image_res[i][j] = maxPro


    # saveName = savePath  + '_YZ_MIP_R.tif'
    image_res = image_res.astype(np.uint16)
    tifffile.imsave(saveName, image_res)

def XZ_MIP_function(dir_path, savePath):
    # img_list = os.listdir(dir_path)
    # name = 'Image136_1.tif'
    img_list = sorted(os.listdir(dir_path),key=lambda x:int(str(x).split('.')[0].split('_')[1]))
    imgSzZ = len(img_list)
    img_name0 = img_list[0]
    img_path0 = dir_path + '/' + img_name0

    img0 = cv2.imread(img_path0, -1)
    [imgSzY, imgSzX] = img0.shape
    saveName = savePath + '/' + os.path.basename(dir_path) + '_XZ_MIP.tif'
    print(saveName)

    # proCenter = int(imgSzY - imgSzY * overlap_Y / 2)
    # proCenter = int(imgSzY * overlap_Y / 2)
    # print('proCenter = ', proCenter)
    image_res = np.zeros((imgSzZ, imgSzX), dtype=np.int16)

    for i in range(imgSzZ):
        img_path_i = dir_path + '/' + img_list[i]
        # print('img_path_i = ', img_path_i)
        image_i = cv2.imread(img_path_i, -1)

        for j in range(imgSzX):
            maxPro = np.max(image_i[:, j])
            image_res[i][j] = maxPro


    image_res = image_res.astype(np.uint16)
    tifffile.imsave(saveName, image_res)


dir_path_list = os.listdir(dir_dir_path)
# dir_path_list.sort()
dir_len = len(dir_path_list)
for i in range(15):
    dir_path = dir_dir_path + '/' + dir_path_list[i]
    YZ_projection_MIP_L(dir_path, savePath)
    YZ_projection_MIP_R(dir_path, savePath)
    XZ_projection_MIP_U(dir_path, savePath)
    XZ_projection_MIP_D(dir_path, savePath)
    # XZ_MIP_function(dir_path, savePath)
    # YZ_MIP_function(dir_path, savePath)
    # XY_MIP_function(dir_path, savePath)