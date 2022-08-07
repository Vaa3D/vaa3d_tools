import os
import numpy as np
import cv2
import tifffile
import math
import heapq
import copy
sigma_set = 2
n_set = 1








def maker_to_points(path):
    points = []
    with open(path) as f:
        lines = f.readlines()
        for line in lines:
            if '#' in line:
                continue
            else:
                ss = line.split(',')
                point_3D = [int(float(ss[2])-1), int(float(ss[1])-1), int(float(ss[0])-1)]
                points.append(point_3D)
    f.close()
    return points


def creat_ideal(sigma, m_path,n=1):
    sigma1 = int(n * sigma)
    points = maker_to_points(m_path)

    image = np.zeros((64,64,64))
    for point in points:
        for x in range(-sigma1, sigma1 + 1):
            for y in range(-sigma1, sigma1 + 1):
                for z in range(-sigma1, sigma1 + 1):
                    if x * x + y * y + z * z <= sigma1 * sigma1:
                        if 0 <= point[2] + x < image.shape[2] and 0 <= point[0] + z < image.shape[0] and 0 <= point[1] + y < image.shape[1]:
                            image[point[0] + z][point[1] + y][point[2] + x] = int(65535 / (1 * ((2 * np.pi) ** 0.5)) * math.exp(-(x * x + y * y + z * z) / (2*(1**2))))

    return image


path_ms = 'C:/Users/admin/Desktop/data/manual'
path_is = 'C:/Users/admin/Desktop/data/ideal_ims'
for root, dirs, files in os.walk(path_ms):
    for f in files:
        path_m = root + '/' + f
        path_i = path_is + '/' + f + '.tif'
        image1 = creat_ideal(sigma_set,path_m,n=n_set)
        image1 = image1.astype(np.uint16)
        tifffile.imwrite(path_i, image1)


