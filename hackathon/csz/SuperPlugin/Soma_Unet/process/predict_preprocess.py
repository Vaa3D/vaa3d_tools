import numpy as np
import os
import SimpleITK as sitk
import random
from scipy import ndimage
from utilsa.common import *
import glob as glob


class predict_pre:
    def __init__(self, raw_dataset_path):
        self.raw_root_path = raw_dataset_path
        self.write_train_val_test_name_list()  # 创建索引txt文件


    def write_train_val_test_name_list(self):
        data_name_list = os.listdir(self.raw_root_path)
        data_num = len(data_name_list)
        print('the fixed dataset total numbers of samples is :', data_num)

        predict_name_list = data_name_list[0:int(data_num)]

        self.write_name_list(predict_name_list, "predict_name_list.txt")

    def write_name_list(self, name_list, file_name):
        f = open(self.raw_root_path +'/'+ file_name, 'w')
        for i in range(len(name_list)):
            f.write(str(name_list[i]) + "\n")
        f.close()


def main():
    raw_dataset_path = r'D:/A_beshe/raw'
    files=glob.glob(raw_dataset_path)
    for file in files:
        predict_pre(file)


if __name__ == '__main__':
    #main()
    predict_pre(r"D:\A_DLcsz\DLtrain\fixed_data\data")
