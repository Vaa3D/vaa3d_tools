import numpy as np
import os
import SimpleITK as sitk
import random
from scipy import ndimage
from utilsa.common import *
from scipy.ndimage import zoom
import shutil

def random_crop_3D_image(img, crop_size):
    if type(crop_size) not in (tuple, list, np.ndarray):
        crop_size = [crop_size] * len(img.shape)
    else:
        assert len(crop_size) == len(
            img.shape), "If you provide a list/tuple as center crop make sure it has the same len as your data has dims (3d)"

    lb_x,lb_y,lb_z=0,0,0
    if crop_size[0] < img.shape[0]:
        lb_x = np.random.randint(0, img.shape[0] - crop_size[0])
    elif crop_size[0] >= img.shape[0]:
        lb_x = 0
    # else:
    #     raise ValueError("crop_size[0] must be smaller or equal to the images x dimension")

    if crop_size[1] < img.shape[1]:
        lb_y = np.random.randint(0, img.shape[1] - crop_size[1])
    elif crop_size[1] >= img.shape[1]:
        lb_y = 0
    # else:
    #     raise ValueError("crop_size[1] must be smaller or equal to the images y dimension")

    if crop_size[2] < img.shape[2]:
        lb_z = np.random.randint(0, img.shape[2] - crop_size[2])
    elif crop_size[2] >= img.shape[2]:
        lb_z = 0
    # else:
    #     raise ValueError("crop_size[2] must be smaller or equal to the images z dimension")

    return lb_x, lb_y, lb_z

class Soma_fix:
    def __init__(self, raw_dataset_path, fixed_dataset_path,test_path):
        self.raw_root_path = raw_dataset_path
        self.fixed_path = fixed_dataset_path

        if not os.path.exists(self.fixed_path):  # 创建保存目录
            os.makedirs(self.fixed_path + 'data/')
            os.makedirs(self.fixed_path + 'label/')


        self.write_train_val_test_name_list()  # 创建索引txt文件



    def write_train_val_test_name_list(self):
        data_name_list = os.listdir(self.fixed_path + "data")
        data_num = len(data_name_list)
        print('the fixed dataset total numbers of samples is :', data_num)
        random.shuffle(data_name_list)

        train_rate = 1
        val_rate = 0

        assert val_rate + train_rate == 1.0
        train_name_list = data_name_list[0:int(data_num * train_rate)]
        val_name_list = data_name_list[int(data_num * train_rate):int(data_num * (train_rate + val_rate))]

        self.write_name_list(train_name_list, "train_name_list.txt")
        self.write_name_list(val_name_list, "val_name_list.txt")

    def write_name_list(self, name_list, file_name):
        f = open(self.fixed_path + file_name, 'w')
        for i in range(len(name_list)):
            f.write(str(name_list[i]) + "\n")
        f.close()

    def get_test_list(self):
        data_name_list = os.listdir(self.fixed_path + "data")
        for i in range(0,len(data_name_list),15):
            print(self.fixed_path+"data/"+data_name_list[i])
            shutil.move(self.fixed_path+"data/"+data_name_list[i],self.test_path+"data/"+data_name_list[i])
            shutil.move(self.fixed_path + "label/seg_" + data_name_list[i], self.test_path + "label/seg_" + data_name_list[i])



def main():
    raw_dataset_path = 'D:/A_beshe/'
    fixed_dataset_path = 'D:/A_beshe/'
    testpath=r"D:/A_DLcsz/ld/test/"

    Soma_fix(raw_dataset_path, fixed_dataset_path,testpath)


if __name__ == '__main__':
    main()
