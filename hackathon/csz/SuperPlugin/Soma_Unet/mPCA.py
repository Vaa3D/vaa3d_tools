import numpy as np
import SimpleITK as sitk

# 从tiff文件中读取三维图像数据
img = sitk.ReadImage(r'D:\A_test\test1\220217_F_060_0_1_01_05_RSGb_1298_gyc.tif')

# 将图像转换为NumPy数组
image_array = sitk.GetArrayFromImage(img)

# 将三维数组展平为二维数组
flat_array = np.reshape(image_array, (-1, image_array.shape[-1]))

# 计算协方差矩阵
covariance_matrix = np.cov(flat_array, rowvar=False)

# 计算特征值和特征向量
eigen_values, eigen_vectors = np.linalg.eigh(covariance_matrix)

# 将特征向量按特征值从大到小排序
sorted_indices = np.argsort(eigen_values)[::-1]
sorted_eigen_vectors = eigen_vectors[:, sorted_indices]

# 提取前三个主成分
pc = sorted_eigen_vectors[:, :]
print(pc.shape)
# 将图像投影到主成分空间
projected_array = np.dot(flat_array, pc)
print(projected_array.shape)
# 将投影后的数组还原为三维数组
projected_image_array = np.reshape(projected_array, image_array.shape)

projected_image_array=np.array(projected_image_array,dtype='uint8')
# 将投影后的数组转换为SimpleITK图像
projected_image = sitk.GetImageFromArray(projected_image_array)

# 将投影后的图像保存为tiff格式
sitk.WriteImage(projected_image, 'output_image.tiff')

# 输出变量的shape
print('image_array shape:', image_array.shape)
print('flat_array shape:', flat_array.shape)
print('covariance_matrix shape:', covariance_matrix.shape)
print('eigen_values shape:', eigen_values.shape)
print('eigen_vectors shape:', eigen_vectors.shape)
print('sorted_eigen_vectors shape:', sorted_eigen_vectors.shape)
print('pc shape:', pc.shape)
print('projected_array shape:', projected_array.shape)
print('projected_image_array shape:', projected_image_array.shape)