import os
import numpy as np
import cv2
import tifffile
import math
import heapq
import copy
import numpy as np
import SimpleITK as sitk
import cv2
import skimage
import tifffile
import numpy as np
import matplotlib.pyplot as plt
import os
plt.rcParams["figure.figsize"] = [16, 12]
from skimage.morphology import remove_small_objects, watershed, dilation, \
    ball  # function for post-processing (size filter)
from skimage.color import label2rgb
from skimage.feature import peak_local_max
from skimage.measure import label, regionprops
from scipy.ndimage import distance_transform_edt
import matplotlib.pyplot as plt
from skimage import filters
sigma_set = 4
n_set = 1


def marker_to_points(img):
    points = []
    # img = tifffile.imread(path)
    # img *= 255
    img_regs = regionprops(label(img))
    print('size = ', len(img_regs))
    for j in img_regs:
        # points.append(j.centroid)
        point_3D = [round(j.centroid[0]), round(j.centroid[1]), round(j.centroid[2])]
        # print(point_3D)
        points.append(point_3D)

    return points


def create_ideal(sigma, m_path,n=1):
    sigma1 = int(n * sigma)
    img = tifffile.imread(m_path)
    points = marker_to_points(img)

    image = np.zeros((img.shape[0], img.shape[1], img.shape[2]))
    for point in points:
        for x in range(-sigma1, sigma1 + 1):
            for y in range(-sigma1, sigma1 + 1):
                for z in range(-sigma1, sigma1 + 1):
                    if x * x + y * y + z * z <= sigma1 * sigma1:
                        if 0 <= point[2] + x < image.shape[2] and 0 <= point[0] + z < image.shape[0] and 0 <= point[1] + y < image.shape[1]:
                            image[point[0] + z][point[1] + y][point[2] + x] = int(65535 / (1 * ((2 * np.pi) ** 0.5)) * math.exp(-(x * x + y * y + 4*z * z) / (2*(1**2))))

    return image


path_ms = r'F:\NuMorph\3DNucleiTracingData\cut_Training\manual_training'
path_is = r'F:\NuMorph\3DNucleiTracingData\cut_Training\ideal_ims_semi'
if not os.path.exists(path_is):
    os.mkdir(path_is)
for root, dirs, files in os.walk(path_ms):
    for f in files:
        path_m = root + '/' + f
        path_i = path_is + '/' + f + '.tif'
        image1 = create_ideal(sigma_set,path_m,n=n_set)
        image1 = image1.astype(np.uint16)
        tifffile.imwrite(path_i, image1)


