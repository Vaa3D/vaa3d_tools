import numpy as np
import tifffile
import cv2
import math
import os


def cutFolderImg_to_64_Voxel(folder_path, save_path):
    for filepath, dirnames, filenames in os.walk(folder_path):
        for filename in filenames:
            if filename.endswith('.tif'):
                fileFullName = os.path.join(filepath, filename)
                # print(fileFullName)
                save_name = save_path + '/' + filename + '_cut.tif'
                oriImage = tifffile.imread(fileFullName)
                cutImg_to_Central_64_Voxel(oriImage, save_name)


def cutImg_to_Central_64_Voxel(oriImage, save_name):
    [imgSzZ, imgSzY, imgSzX] = oriImage.shape
    cut_image = np.zeros((64, 64, 64), dtype=np.float)

    for x in range(64):
        for y in range(64):
            for z in range(64):
                cut_image[z][y][x] = oriImage[imgSzZ//2-32+z][imgSzY//2-32+y][imgSzX//2-32+x]

    cut_image = cut_image.astype(np.uint16)
    tifffile.imsave(save_name, cut_image)
    return cut_image


def cutImg_3DTo2D(oriImage, save_path):
    [imgSzZ, imgSzY, imgSzX] = oriImage.shape
    for z in range(imgSzZ):
        save_zname = save_path + '/'+str(z).rjust(2,'0')+'_Img.tif'
        zImg = np.zeros((imgSzY, imgSzX), dtype=np.float)
        for y in range(imgSzY):
            for x in range(imgSzX):
                zImg[y][x] = oriImage[z][y][x]
        zImg = zImg.astype(np.uint16)
        tifffile.imsave(save_zname, zImg)

def oneChannelTo3C(img_name, markerFile, save_path):
    save_name = save_path + '/ID47_3C.tif'
    grayImage = tifffile.imread(img_name)
    stacked_img = np.stack((grayImage,) * 3, axis=-1)

    with open(markerFile, 'r') as f:
        lines = f.readlines()
        for line in lines:
            if '#' in line:
                continue
            ss = line.split(',')
            xyz = []
            color = []
            xyz.append(int(ss[0]))
            xyz.append(int(ss[1]))
            xyz.append(int(ss[2]))
            color.append(int(ss[7]))
            color.append(int(ss[8]))
            color.append(int(ss[9]))
            stacked_img[xyz[2]-1,xyz[1]-1,xyz[0]-1] = (color[0], color[1], color[2])

    tifffile.imsave(save_name, stacked_img)


def markerDownSample(markerFile, save_path):
    save_name = save_path + '/123_25.marker'

    markerTitle = '##x,y,z,radius,shape,name,comment, color_r,color_g,color_b\n'
    file = open(save_name, 'w')
    file.write(markerTitle)

    with open(markerFile, 'r') as f:
        lines = f.readlines()
        for line in lines:
            if '#' in line:
                continue
            ss = line.split(',')

            xyz = []
            xyz.append(round(float(ss[0])/25, 2))
            xyz.append(round(float(ss[1])/25, 2))
            xyz.append(round(float(ss[2])/25, 2))
            if xyz[0]>0 and xyz[1]>0 and xyz[2]>0:
                file.write(str(xyz[0]) + ',' + str(xyz[1]) + ',' + str(xyz[2]) + ', 0, 0, , , 0,0,255\n')
            else:
                file.write(str(abs(xyz[0])) + ',' + str(abs(xyz[1])) + ',' + str(abs(xyz[2])) + ', 0, 0, , , 255,0,0\n')

    file.close()

def rotateMarker(markerFile, save_path):
    imgX0 = 7322/2
    imgY0 = 8588/2

    save_name = save_path + '/rotate90_marker000.marker'
    markerTitle = '##x,y,z,radius,shape,name,comment, color_r,color_g,color_b\n'
    file = open(save_name, 'w')
    file.write(markerTitle)

    with open(markerFile, 'r') as f:
        lines = f.readlines()
        for line in lines:
            if '#' in line:
                continue
            ss = line.split(',')

            xyz = []
            xyz.append(float(ss[0]))
            xyz.append(float(ss[1]))
            xyz.append(float(ss[2]))

            xyzn = []
            xyzn.append(-xyz[1]+imgY0+imgX0)
            xyzn.append(xyz[0]-imgX0+imgY0)
            xyzn.append(xyz[2])

            xyzn[0] = 2 * imgX0 - xyzn[0] + imgY0 - imgX0
            xyzn[1] = xyzn[1] + imgX0 - imgY0

            file.write(str(round(xyzn[0]/10,2)) + ',' + str(round(xyzn[1]/10,2)) + ',' + str(round(xyzn[2]/9,2)) + ', 0, 0, , , 0,0,255\n')

    file.close()

def renameImages():
    path = 'F:/QualityControlProject/20211201_16_04_24_PSF_a_A_destriped_DONE/testData'
    filelist = os.listdir(path)
    a=1
    for files in filelist:
        oldname = os.path.join(path, files)
        if os.path.isdir(oldname):
            continue
        # filename = os.path.splitext(files)[0]
        filetype = os.path.splitext(files)[1]
        newname = os.path.join(path, str(a)+filetype)
        os.rename(oldname, newname)
        a += 1

def dirDownSampled(dir_dir_path, save_path):
    dir_list = os.listdir(dir_dir_path)
    for dir_name in dir_list:
        dir_path = dir_dir_path + '/' + dir_name
        save_tile_path = save_path + '/' + dir_name
        os.mkdir(save_tile_path)
        downsample2DPavelData(dir_path, save_tile_path)


def downsample2DPavelData(dir_path, save_path):
    img_list = os.listdir(dir_path)
    imgSzZ = len(img_list)
    img_name0 = img_list[0]
    img_path0 = dir_path + '/' + img_name0
    img0 = cv2.imread(img_path0, -1)
    [imgSzY, imgSzX] = img0.shape

    for i in range(1541, math.ceil(imgSzZ/4)):
        img = img_list[i * 4]
        img_path = dir_path + '/' + img
        gray_img = cv2.imread(img_path, -1)
        save_img = save_path + '/' + img.split('_')[0] + '_' + str(i).zfill(4) + '.tif'
        image_downSampled = np.zeros((math.ceil(imgSzY / 2), math.ceil(imgSzX / 2)), dtype=float)

        for j in range(math.ceil(imgSzY/2)):
            for k in range(math.ceil(imgSzX/2)):
                image_downSampled[j][k] = gray_img[j*2][k*2]

        image_downSampled = image_downSampled.astype(np.uint16)
        tifffile.imsave(save_img, image_downSampled)


dir_path = 'E:/Pavel_data_for_tracing/lyb_do_rotate/AVP-IHC-A2_2D_stitched'
save_path = 'E:/Pavel_data_for_tracing/results_stitching'
downsample2DPavelData(dir_path, save_path)







