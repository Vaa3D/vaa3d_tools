import numpy as np
import os
import SimpleITK as sitk
import random
from scipy import ndimage

class soma_test_txt:
    def __init__(self, raw_dataset_path, fixed_dataset_path):
        self.raw_root_path = raw_dataset_path
        self.fixed_path = fixed_dataset_path

        if not os.path.exists(self.fixed_path):  # 创建保存目录
            os.makedirs(self.fixed_path + 'hres/')
            os.makedirs(self.fixed_path + 'lres/')
        #self.fix_data()
        self.write_test_name_list()  # 创建索引txt文件
        #self.flag=set()


    def fix_data(self):
        print('the raw dataset total numbers of samples is :', len(os.listdir(self.raw_root_path+ 'data')))
        for data_file in os.listdir(self.raw_root_path+ 'data/'):
            print(data_file)
            data = sitk.ReadImage(os.path.join(self.raw_root_path + 'data/', data_file), sitk.sitkInt8)
            data_array = sitk.GetArrayFromImage(data)

            seg = sitk.ReadImage(os.path.join(self.raw_root_path + 'label/', 'seg_'+data_file),
                                 sitk.sitkInt8)
            seg_array = sitk.GetArrayFromImage(seg)

            print(data_array.shape, seg_array.shape)

            seg_array = np.flip(seg_array, axis=1)

            new_data = sitk.GetImageFromArray(data_array)
            new_seg = sitk.GetImageFromArray(seg_array)

            sitk.WriteImage(new_data, os.path.join(self.fixed_path + 'data/', data_file))
            sitk.WriteImage(new_seg,
                            os.path.join(self.fixed_path + 'label/', 'seg_'+data_file))
    def write_test_name_list(self):
        data_name_list = os.listdir(self.raw_root_path + "data")
        data_num = len(data_name_list)
        print('the dataset total numbers of samples is :', data_num)

        test_name_list = data_name_list[0:int(data_num)]

        self.write_name_list(test_name_list, "test_name_list.txt")

    def write_name_list(self, name_list, file_name):
        f = open(self.fixed_path + file_name, 'w')
        for i in range(len(name_list)):
            f.write(str(name_list[i]) + "\n")
        f.close()

    def get_test_list(self):
        data_name_list = os.listdir(self.raw_root_path + "data")
        data_num = len(data_name_list)
        print('the dataset total numbers of samples is :', data_num)

        test_name_list = data_name_list[0:int(data_num)]

        self.write_name_list(test_name_list, "test_name_list.txt")

def main():
    raw_dataset_path = 'D:/DLaugraw/'
    fixed_dataset_path = 'D:/A_DLcsz/fixeddata'

    #tiff_fix(raw_dataset_path, fixed_dataset_path)
    soma_test_txt(raw_dataset_path, fixed_dataset_path)

if __name__ == '__main__':
    main()
