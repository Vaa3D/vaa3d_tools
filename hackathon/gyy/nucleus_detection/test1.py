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

plt.rcParams["figure.figsize"] = [16, 12]

def mylabel2rgb(mylabel):
    num = np.max(mylabel)
    a = np.linspace(150, 64500, 3000)
    b = np.linspace(64500, 150, 3000)
    c = np.linspace(150, 64500, 3000)
    np.random.shuffle(c)
    d = np.zeros(shape=(3, 3000), dtype=np.uint16)
    d[0] = a
    d[1] = b
    d[2] = c
    d = np.transpose(d)
    np.random.shuffle(d)
    r = np.zeros(shape=(64, 64, 64, 3), dtype=np.uint16)
    for i in range(mylabel.shape[0]):
        for j in range(mylabel.shape[1]):
            for k in range(mylabel.shape[2]):
                if mylabel[i][j][k] != 0:
                    r[i][63 - j][k] = d[mylabel[i][j][k]]
    return r


def marker_to_points(path):
    points = []
    with open(path) as f:
        lines = f.readlines()
        for line in lines:
            if line == '\n' or '#' in line:
                continue
            else:
                if ',' in line:
                    ss = line.split(',')
                point_3D = [int(float(ss[2]) - 1), int(float(ss[1]) - 1), int(float(ss[0]) - 1)]
                points.append(point_3D)
    f.close()
    return points


def marker_to_mask(poins):
    r = np.zeros(shape=(64, 64, 64), dtype=np.int16)
    for point in poins:
        r[point[0]][point[1]][point[2]] = 1
    return r


path_image = r'F:\QualityControlProject\Data\syntheitc_data\test\00_snr_img.tif'
path_markers = r'F:\QualityControlProject\Data\syntheitc_data\test\00_lm_marker.marker.marker'
path_markers_m = r'F:\QualityControlProject\Data\syntheitc_data\test\00_original_localMax.marker'
save_path = r'F:\QualityControlProject\Data\syntheitc_data\test'
regionPath = save_path + '/' + 'Recall.txt'
regionTitle = '##Name, Area, MajorAxisLength, MinorAxisLength, Orientation\n'
file_region = open(regionPath, 'w')
file_region.write(regionTitle)

points = marker_to_points(path_markers)
points_m = marker_to_points(path_markers_m)
mask = marker_to_mask(points)
obj_label = label(mask == 1)
# reader = AICSImage(path_image)
# IMG = reader.data[0][0]
IMG = tifffile.imread(path_image)

thresh = filters.threshold_otsu(IMG)
ttt = np.mean(IMG) + 0.5* np.std(IMG)
mask_b = IMG > (np.mean(IMG) + 0.5* np.std(IMG))
# mask_b = IMG > thresh
seg = watershed(-IMG, obj_label, mask=mask_b, watershed_line=True)

seg = remove_small_objects(seg, min_size=20, connectivity=1, in_place=False)
flag1 = 0


seg_color = mylabel2rgb(seg).astype(np.uint16)
tifffile.imwrite(save_path + '/00_seg.tif', seg_color)
output_ = regionprops(seg)

num = 0
for j in output_:
    num += 1
    file_region.write(str(num) + ',' + str(j.area) + ',' + str(j.major_axis_length) + ','
                      + str(j.minor_axis_length) + ',' + str(j.orientation) + '\n'
                      )

file_region.close()
