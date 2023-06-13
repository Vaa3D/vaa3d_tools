from pylib.img_io import *


# img=Readimg(r"D:\A_test\test1\140921c16.tif.v3dpbd.tiff.v3draw_964_700_15.tiff")
#
# mean=np.mean(img)
# std=np.std(img)
# value=mean+0.5*std
#
# img[img<value]=0
#
# img[img!=0]=255
# img=np.array(img,dtype='uint8')
# Writeimg(img,"1.tiff")

# image=sitk.ReadImage("1.tiff",sitk.sitkInt8)
# # 查找所有连通域并分配标签
# label_image = sitk.ConnectedComponent(image)
# label_image = sitk.RelabelComponent(label_image)
#
# # 保存每个连通域
# statistics_filter = sitk.LabelShapeStatisticsImageFilter()
# statistics_filter.Execute(label_image)
# number_of_labels = statistics_filter.GetNumberOfLabels() + 1
# print(number_of_labels)
# post=np.zeros(sitk.GetArrayFromImage(image).shape)
# print(post.shape)
# for label in range(1, number_of_labels):
#     # 创建标签二值图像
#     label_map = sitk.Threshold(label_image, label, label, 0)
#     label_data = sitk.GetArrayFromImage(label_map)
#
#     label_data[label_data != 0] = 1
#     if np.sum(label_data) < 125:
#         continue
#     post+=label_data
#     print(np.sum(label_data) )
# post[post!=0]=255
# post=np.array(post,dtype='uint8')
# post=sitk.GetImageFromArray(post)
# sitk.WriteImage(post,"2.tiff")


# data=np.zeros((3,3,3))
# data[0][0][:2]=255
# data[2][2][1:]=255
# post=np.array(data,dtype='uint8')
# post=sitk.GetImageFromArray(post)
# sitk.WriteImage(post,"3.tiff")

img=Readimg("4.tiff")
img=np.rot90(img,axes=(2,0))
img=np.flip(img,axis=2)
Writeimg(img,"5.tiff")