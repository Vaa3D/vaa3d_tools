import numpy as np
import SimpleITK as sitk
import cv2
import tifffile
import numpy as np
import matplotlib.pyplot as plt

plt.rcParams["figure.figsize"] = [16, 12]
from skimage.morphology import remove_small_objects, watershed, dilation, \
    ball  # function for post-processing (size filter)
from skimage.color import label2rgb
from skimage.feature import peak_local_max
from skimage.measure import label, regionprops
from scipy.ndimage import distance_transform_edt
import matplotlib.pyplot as plt

plt.rcParams["figure.figsize"] = [16, 12]



def mylabel2rgb(mylabel):
    num = np.max(mylabel)
    a = np.linspace(150, 65400, 436)
    b = np.linspace(65400, 150, 436)
    c = np.linspace(150, 65400, 436)
    np.random.shuffle(c)
    d = np.zeros(shape=(3, 436), dtype=np.uint16)
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


def maker_to_points(path):
    points = []
    with open(path) as f:
        lines = f.readlines()
        for line in lines:
            if '#' in line:
                continue
            else:
                if ',' in line:
                    ss = line.split(',')
                point_3D = [int(float(ss[2]) - 1), int(float(ss[1]) - 1), int(float(ss[0]) - 1)]
                points.append(point_3D)
    f.close()
    return points


def maker_to_mask(poins):
    r = np.zeros(shape=(64, 64, 64), dtype=np.int16)
    for point in poins:
        r[point[0]][point[1]][point[2]] = 1
    return r


path_image = r'C:\Users\admin\Desktop11_r_results2.tif'
path_makers = r'C:\Users\admin\Desktop11_r.tif_localMax_results2.marker'
points = maker_to_points(path_makers)
mask = maker_to_mask(points)
obj_label = label(mask == 1)
reader = AICSImage(path_image)
IMG = reader.data[0][0]
mask_b = IMG > (np.mean(IMG) + np.std(IMG))
seg = watershed(-IMG, obj_label, mask=mask_b, watershed_line=True)

output_=regionprops(seg)
for i in output_:
    a = i.coords
    b=0
seg = seg.astype(np.uint16)

# seg [seg>0]=1
# rr = seg * IMG
# tifffile.imwrite(r'C:\Users\admin\Desktop\17_seg_rr.tif',rr)
# labels = label(rr>0)
r = mylabel2rgb(seg)
# test = np.zeros(shape=(64,64,64),dtype=np.uint16)
# for i in range(64):
#     for j in range(64):
#         for k in range(64):
#             if r[i][j][k][0] != 0:
#                 test[i][j][k] = 30000

tifffile.imwrite(r'C:\Users\admin\Desktop\11_seg_color_22.tif', r)
