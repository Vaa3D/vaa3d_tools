import SimpleITK as sitk
import numpy as np
from scipy import ndimage
import torch

#MIN_BOUND = -1000.0
#MAX_BOUND = 400.0

def norm_img(image): # 归一化像素值到（0，1）之间，且将溢出值取边界值
    '''
    image = (image - MIN_BOUND) / (MAX_BOUND - MIN_BOUND)
    image[image > 1] = 1.
    image[image < 0] = 0.
    '''
    img_max=np.max(image)
    img_min=np.min(image)
    # print("init",img_max,img_min)
    if img_max==img_min:
        return image
    image=(image-img_min)/(img_max-img_min)
    # print("after",np.max(image),np.min(image))
    return image


def sitk_read_raw(img_path): # 读取3D图像并resale（因为一般医学图像并不是标准的[1,1,1]scale）
    nda = sitk.ReadImage(img_path,sitk.sitkUInt8)
    if nda is None:
        raise TypeError("input img is None!!!")
    nda = sitk.GetArrayFromImage(nda)  # channel first

    return nda


def load_file_name_list(file_path):
    file_name_list = []
    with open(file_path, 'r') as file_to_read:
        while True:
            lines = file_to_read.readline().strip()  # 整行读取数据
            if not lines:
                break
                pass
            file_name_list.append(lines)
            pass
    return file_name_list

def adjust_learning_rate(optimizer, epoch, args):
    """Sets the learning rate to the initial LR decayed by 10 every 30 epochs"""
    lr = args.lr * (0.1 ** (epoch // 30))
    for param_group in optimizer.param_groups:
        param_group['lr'] = lr