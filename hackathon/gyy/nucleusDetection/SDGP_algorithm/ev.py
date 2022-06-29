import numpy as np
import SimpleITK as sitk
import cv2
import skimage
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
from skimage import filters

plt.rcParams["figure.figsize"] = [16, 12]
from aicsimageio.aics_image import AICSImage
from aicssegmentation.core.seg_dot import dot_3d_wrapper
from aicssegmentation.cli.to_analysis import masked_builder
from aicssegmentation.core.pre_processing_utils import intensity_normalization, edge_preserving_smoothing_3d


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


def face_area(region):
    image = region.image
    r = 0
    z = image.shape[0]
    y = image.shape[1]
    x = image.shape[2]
    for ii in range(z):
        for jj in range(y):
            for kk in range(x):
                if image[ii][jj][kk]:
                    if ii == 0 or ii == z - 1 or jj == 0 or jj == y - 1 or kk == 0 or kk == x - 1:
                        r = r + 1
                    elif not (image[ii + 1][jj][kk] and image[ii - 1][jj][kk] and image[ii][jj + 1][kk] and
                              image[ii][jj - 1][kk] and image[ii][jj][kk + 1] and image[ii][jj][kk - 1]):
                        r = r + 1
    return r


def seg_by_density(ss, region, img):
    dc = region.minor_axis_length / 4 + region.major_axis_length / 4 - 0.5

    mmax = region.major_axis_length

    class point:
        def __init__(self, p, huidu):
            self.p = p
            self.prep = None
            self.h = huidu
            self.density = 0
            self.distance = 0
            self.label = None
            self.index = None

    def distance(p, p_s):
        r = mmax
        for iii in p_s:
            if p.density < iii.density:
                if (p.p[0] - iii.p[0]) ** 2 + (p.p[1] - iii.p[1]) ** 2 + (p.p[2] - iii.p[2]) ** 2 < r:
                    r = (p.p[0] - iii.p[0]) ** 2 + (p.p[1] - iii.p[1]) ** 2 + (p.p[2] - iii.p[2]) ** 2
                    p.prep = iii.index
        p.distance = r

    def density(p, p_s):
        r = 0
        for iii in p_s:
            if p.h > iii.h and (p.p[0] - iii.p[0]) ** 2 + (p.p[1] - iii.p[1]) ** 2 + (p.p[2] - iii.p[2]) ** 2 <= dc:
                r = r + 1
        p.density = r

    def set_label(p_s, p):
        pre = p.prep
        if p_s[pre].prep is None:
            p.label = p_s[pre].label
            return p.label
        else:
            p.label = set_label(p_s, p_s[pre])
            return p.label

    # def density(p, p_s):
    #     r = 0
    #     for iii in p_s:
    #         if p.h > iii.h and (p.p[0] - iii.p[0]) ** 2 + (p.p[1] - iii.p[1]) ** 2 + (p.p[2] - iii.p[2]) ** 2 <= dc:
    #             r = r + 1
    #     p.density = r * p.h

    # def density(p, p_s):
    #     r = 0
    #     for iii in p_s:
    #         tttt = (p.p[0] - iii.p[0]) ** 2 + (p.p[1] - iii.p[1]) ** 2 + (p.p[2] - iii.p[2]) ** 2
    #         if 0 < tttt <= dc:
    #             r = r + 1/(dc*(2*np.pi)**0.5) * np.exp(-ttt/dc**2)
    #             r = r + np.exp(-(ttt / dc) ** 2)
    #     p.density = r * p.h

    img = img
    num = np.max(ss)
    coords = region.coords
    point_s = []
    for ii in coords:
        point_s.append(point(ii, img[ii[0]][ii[1]][ii[2]]))

    for ii in range(len(point_s)):
        point_s[ii].index = ii

    for ii in point_s:
        density(ii, point_s)

    for ii in point_s:
        distance(ii, point_s)

    plus = 0

    centers = []
    for ii in point_s:
        if ii.prep is None:
            centers.append(ii.index)
            plus += 1
    if plus == 1:
        return ss
    for ii in range(len(centers)):
        point_s[centers[ii]].label = num + ii
    for ii in point_s:
        if ii.prep is not None and ii.label is None:
            ii.label = set_label(point_s, ii)

    for ii in point_s:
        ss[ii.p[0]][ii.p[1]][ii.p[2]] = ii.label
    print(region.label)
    print(plus)
    for ii in point_s:
        if ii.prep is None:
            print('’‘clustering center')
            print(ii.p)

    return ss


path_image = r'C:\Users\admin\Desktop\1_unet_marker_k3\15_r.tif'
path_makers = r'C:\Users\admin\Desktop1_unet_marker_k3\15_r.tif_localMax.marker'
path_makers_m = r'C:\Users\admin\Desktop\1_unet_marker_k3\15_manual.marker'
points = maker_to_points(path_makers)
points_m = maker_to_points(path_makers_m)
mask = maker_to_mask(points)
obj_label = label(mask == 1)
reader = AICSImage(path_image)
IMG = reader.data[0][0]

thresh = filters.threshold_otsu(IMG)
ttt = np.mean(IMG) + 1.5 * np.std(IMG)
mask_b = IMG > (np.mean(IMG) + 0.5 * np.std(IMG))
# mask_b = IMG > thresh
seg = watershed(-IMG, obj_label, mask=mask_b, watershed_line=True)
seg = remove_small_objects(seg, min_size=20, connectivity=1, in_place=False)

output_ = regionprops(seg)
# a=0
# for i in output_:
#     if i.area>a:
#         a =i.area

need = []
need_r = []
for i in points:
    a = seg[i[0]][i[1]][i[2]]
    for j in output_:
        if a == j.label and ((i[0] - j.centroid[0]) ** 2 + (i[1] - j.centroid[1]) ** 2 + (
                i[2] - j.centroid[2]) ** 2 > 0.5) and j.major_axis_length / j.minor_axis_length > 1.5:
            need.append(j.label)

# for i in points:
#     a = seg[i[0]][i[1]][i[2]]
#     for j in output_:
#         if a == j.label and ((i[0]-j.centroid[0])**2 + (i[1]-j.centroid[1])**2 + (i[2]-j.centroid[2])**2 > 0.8) and j.area/j.bbox_area<0.35:
#             need.append(j.label)
# for j in output_:
#     if j.area/j.bbox_area<0.35:
#         need.append(j.label)
# for j in output_:
#     if j.major_axis_length / j.minor_axis_length > 1.5:
#         need.append(j.label)
# for j in output_:
#     t_ = face_area(j)
#     if t_ / j.area > 0.8 and j.major_axis_length / j.minor_axis_length > 1.5:
#         need.append(j.label)

for i in output_:
    for j in need:
        if j == i.label:
            need_r.append(i)

n_recall = 0
n_wrong = 0
n_touch = 0
label_touch = set()
label_touch_r = []
for i in range(len(points_m)):
    m = points_m[i]
    for j in range(i + 1, len(points_m)):
        n = points_m[j]
        if seg[m[0]][m[1]][m[2]] == seg[n[0]][n[1]][n[2]] and seg[m[0]][m[1]][m[2]] > 0:
            n_touch = n_touch + 1
            label_touch.add(seg[m[0]][m[1]][m[2]])
for i in output_:
    for j in label_touch:
        if j == i.label:
            label_touch_r.append(i)
seg_ = seg.copy()
seg__ = seg.copy()
seg___ = seg.copy()
for i in label_touch:
    seg_[seg_ == i] = 50000
seg_[seg_ != 50000] = 0
seg_ = seg_.astype(np.uint16)
for i in need:
    seg___[seg___ == i] = 50000
seg___[seg___ != 50000] = 0
seg___ = seg___.astype(np.uint16)

tifffile.imwrite(r'C:\Users\admin\Desktop\1_unet_marker_k2\15_seg_need_0.tif', seg___)
tifffile.imwrite(r'C:\Users\admin\Desktop\1_unet_marker_k2\15_seg_touch_0.tif', seg_)

seg_need_color = seg.copy()
seg_need_color = seg_need_color.astype(np.uint16)
seg_need_color[seg_need_color>0]=0

seg_needs = []
for i in need:
    ttt = seg.copy()
    ttt[ttt != i] = 0
    seg_needs.append(ttt)
for i in seg_needs:
    seg_need_color = seg_need_color + i

for i in need_r:
    seg = seg_by_density(seg, i, IMG)
    seg_need_color = seg_by_density(seg_need_color, i, IMG)

color_seg_need = mylabel2rgb(seg_need_color)

tifffile.imwrite(r'C:\Users\admin\Desktop\1_unet_marker_k2\15_seg_need_seg_color.tif', color_seg_need)


for i in points_m:
    if seg[i[0]][i[1]][i[2]] > 0:
        a = seg[i[0]][i[1]][i[2]]
        for j in output_:
            if a == j.label:
                n_recall = n_recall + 1
                output_.remove(j)
                break

for i in points:
    if i[0] < 3 or i[0] > 61 or i[1] < 3 or i[1] > 61 or i[2] < 3 or i[2] > 61:
        a = seg[i[0]][i[1]][i[2]]
        for j in output_:
            if a == j.label:
                output_.remove(j)
n_wrong = len(output_)

for i in output_:
    a = i.label
    seg__[seg__ == a] = 50000
seg__[seg__ != 50000] = 0
seg__ = seg__.astype(np.uint16)
tifffile.imwrite(r'C:\Users\admin\Desktop\1_unet_marker_k2\15_seg_wrong_0.tif', seg__)

print(path_image)
print("n_recall")
print(n_recall)
print('n_wrong')
print(n_wrong)
print('len(points_m)')
print(len(points_m))
print('Recall n_recall/len(points_m)')
print(n_recall/len(points_m))
print('n_wrong/len(points_m)')
print(n_wrong/len(points_m))
print('Precision n_recall/(n_recall+n_wrong')
print(n_recall/(n_recall+n_wrong))
print('F1 2*n_recall/len(points_m)*n_recall/(n_recall+n_wrong)/(n_recall/len(points_m)+n_recall/(n_recall+n_wrong))')
print(2*n_recall/len(points_m)*n_recall/(n_recall+n_wrong)/(n_recall/len(points_m)+n_recall/(n_recall+n_wrong)))

acas = 1
