import SimpleITK as sitk
import cv2
import tifffile
import numpy as np
import matplotlib.pyplot as plt
plt.rcParams["figure.figsize"] = [16, 12]
from skimage.morphology import remove_small_objects, watershed, dilation, ball     # function for post-processing (size filter)
from skimage.feature import peak_local_max
from skimage.measure import label,regionprops
from scipy.ndimage import distance_transform_edt
import matplotlib.pyplot as plt
plt.rcParams["figure.figsize"] = [16, 12]
from aicsimageio.aics_image import AICSImage
from aicssegmentation.core.seg_dot import dot_3d_wrapper
from aicssegmentation.cli.to_analysis import masked_builder
from aicssegmentation.core.pre_processing_utils import intensity_normalization, edge_preserving_smoothing_3d
def writeMarker(centroid):
    markerPath = r'C:/Users/admin/Desktop/testData/025300_042970_007200.tif_seed.marker'
    markerTitle = '##x,y,z,radius,shape,name,comment, color_r,color_g,color_b\n'
    len_centroid = len(centroid)
    file = open(markerPath, 'w')
    file.write(markerTitle)
    for centroid_i in range(0, len_centroid):
        centroid_x = centroid[centroid_i][2]
        centroid_y = centroid[centroid_i][1]
        centroid_z = centroid[centroid_i][0]
        # centroid_x = int(centroid[centroid_i][2])
        # centroid_y = int(centroid[centroid_i][1])
        # centroid_z = int(centroid[centroid_i][0])
        file.write(str(centroid_x+1)+','+str(centroid_y+1)+','+str(centroid_z+1)+', 0, 0, , , 255,0,0\n') # 记录点坐标+颜色，对应上面的markerTitle
    file.close()
    return centroid
def writeRegionprops(centroid,area):
    regionpropsPath=r'C:\Users\braintell\Desktop\result./025300_030360_007200.tif_cut.tif.regionpropsinformation'
    regionpropsTitle='##centroid(x,y,z),area\n'
    len_area=len(area)
    len_centroid = len(centroid)
    file = open(regionpropsPath, 'w')
    file.write(regionpropsTitle)
    for centroid_i in range(0, len_centroid):
        centroid_x = centroid[centroid_i][2]
        centroid_y = centroid[centroid_i][1]
        centroid_z = centroid[centroid_i][0]
        area_i=area[centroid_i]
        file.write(str(centroid_x+1)+','+str(centroid_y+1)+','+str(centroid_z+1)+','+str(area_i)+'\n')
    return centroid,area
def computedDistance(centroid):
    regionpropsPath = r'C:\Users\braintell\Desktop\result./025300_030360_007200.tif_cut.tif.centroiddistance'
    regionpropsTitle = '##centroiddistance\n'
    len_centroid=len(centroid)
    file = open(regionpropsPath, 'w')
    file.write(regionpropsTitle)
    distance_list=[]
    for i in range(len_centroid):
        for j in range(len_centroid):
            if i ==j:
                continue
            distance=np.sqrt((centroid[j][0]-centroid[i][0])**2+(centroid[j][1]-centroid[i][1])**2+(centroid[j][1]-centroid[i][1])**2)
            distance_list.append(distance)
        min_distance=min(distance_list)
        file.write(str(min_distance) + '\n')
        distance_list.clear()
    return centroid
def coor(file_path):
    file_name_list=[]
    with open(file_path, 'r') as file_to_read:
        while True:
            lines = file_to_read.readline().strip()  # 整行读取数据
            if not lines:
                break
                pass
            file_name_list.append(lines)
            pass
    del(file_name_list[0])
    x_max = len(file_name_list)
    y_max=max(file_name_list)
    plt.hist(file_name_list, bins=x_max,normed=1,weights=3, color='blue', alpha=0.5)
    plt.xlim(0, 1000)
    plt.xlabel('centroid')  # 横轴名
    plt.ylabel('distance')  # 纵轴名
    plt.title('centroidmindistance')
    plt.show()

FILE_NAME = r"C:\Users\seusmy\Desktop\安徽科技学院教学\decon\decon\image1\042840_037500_025280.tif_cut_decon1.tif"
# IMG = tifffile.imread(FILE_NAME)
reader = AICSImage(FILE_NAME)
IMG = reader.data
print(IMG.shape)
N_CHANNELS = IMG.shape[1]
MID_SLICE = int(0.5*IMG.shape[2])
#MID_SLICE = np.int(IMG.shape[2])
fig, ax = plt.subplots(1, N_CHANNELS, figsize=(18,16), dpi=72, facecolor='w', edgecolor='k')
if N_CHANNELS == 1:
    ax.axis('off')
    #ax.imshow(IMG[0, 0, 0, :, :], cmap=plt.cm.gray)
    ax.imshow(IMG[0, 0, MID_SLICE, :, :], cmap=plt.cm.gray)
    plt.show()
else:
    for channel in range(N_CHANNELS):
        ax[channel].axis('off')
        ax[channel].imshow(IMG[0, channel, MID_SLICE, :, :], cmap=plt.cm.gray)
#####################
structure_channel = 0
struct_img0 = IMG[0, structure_channel, :, :, :].astype(np.float32)
###############################
## intensity ##
intensity_scaling_param = [2, 11]
# intensity_scaling_param = [11]
struct_img = intensity_normalization(struct_img0, scaling_param=intensity_scaling_param)
structure_img_smooth = edge_preserving_smoothing_3d(struct_img)
################################
## PARAMETERS for this step ##
s3_param = [[1, 0.03]]
bw = dot_3d_wrapper(structure_img_smooth, s3_param)
labels=label(bw)
################################
## watershed ##

# labels=label(Mask)
obj_label, obj_df=masked_builder(bw,labels)

output=regionprops(labels)
total_region_number=np.max(labels)
area=[]
centroid=[]
for i in range(total_region_number):
    area.append(output[i].area)
    centroid.append(output[i].centroid) #d第一个轴为Z，第二个轴为Y，第三个轴为X
minArea = 10
Mask = remove_small_objects(bw > 0, min_size=minArea, connectivity=1, in_place=False)
# labels_=label(Mask);
# local_max=peak_local_max(struct_img,labels=label(Mask), min_distance=2, indices=False)
Seed = dilation(peak_local_max(struct_img,labels=label(Mask), min_distance=2, indices=False), selem=ball(1))
labels_=label(Seed)
# labels_=label(local_max)
output_=regionprops(labels_)
total_region_number_=np.max(labels_)
area_=[]
centroid_=[]
for i in range(total_region_number_):
    area_.append(output_[i].area)
    centroid_.append(output_[i].centroid)#d第一个轴为Z，第二个轴为Y，第三个轴为X
Watershed_Map = -1*distance_transform_edt(bw)
seg = watershed(Watershed_Map, obj_label, mask=Mask, watershed_line=True)
# seg = watershed(Watershed_Map, labels_, mask=Mask, watershed_line=True)
labels1=label(seg)
# labels1=seg
output1=regionprops(labels1)
total_region_number1=np.max(labels1)
area1=[]
centroid1=[]
for i in range(total_region_number1):
    area1.append(output1[i].area)
    centroid1.append(output1
                     [i].centroid)#d第一个轴为Z，第二个轴为Y，第三个轴为X
# ###############################
# # PARAMETERS for this step ##
minArea = 5
seg1 = remove_small_objects(seg>0 , min_size=minArea, connectivity=1, in_place=False)
labels2 = label(seg1)
output2 = regionprops(labels2)
total_region_number2=np.max(labels2)
area2 = []
centroid2 = []
for i in range(total_region_number2):
    area2.append(output2[i].area)
    centroid2.append(output2[i].centroid)
# seg = remove_small_objects(bw>0, min_size=minArea, connectivity=1, in_place=False)
centroid2 = writeMarker(centroid2)
# (centroid2, area2) = writeRegionprops(centroid2,area2)
# centroid2=computedDistance(centroid2)
# coor(r'C:\Users\braintell\Desktop\result\025300_015180_016800.tif_cut.tif.centroiddistance')
seg1 = seg1 > 0
out = seg1.astype(np.uint8)
out[out > 0] = 255
img = sitk.GetImageFromArray(out)
(H, W, D) = out.shape
new_out = np.zeros(shape=(H, W, D), dtype=np.uint8)
for h in range(H):
    new_out[h, :, :]=cv2.flip(out[h, :, :], 0)#2D no need
img = sitk.GetImageFromArray(new_out)
sitk.WriteImage(img, r'C:\Users\braintell\Desktop\result\test_025300_030360_007200.tif_cut.tif')
#OmeTiffWriter.save(out, "one.tif", dim_order="ZYX")