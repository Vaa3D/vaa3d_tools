import numpy as np
import os
import torch
from scipy import ndimage
from torch.utils.data import Dataset, DataLoader
from torchvision import transforms as T
import SimpleITK as sitk
import pandas as pd
from DataAugmentation import *

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

def sitk_read_raw(img_path): # 读取3D图像并resale（因为一般医学图像并不是标准的[1,1,1]scale）
    nda = sitk.ReadImage(img_path,sitk.sitkUInt8)
    if nda is None:
        raise TypeError("input img is None!!!")
    nda = sitk.GetArrayFromImage(nda)  # channel first

    return nda

class AlexDataSet(Dataset):
    def __init__(self, dataset_path,mode=None):
        # self.ground_truth= np.array(pd.read_csv('./datasetlist/one_hot.csv'))[:,:-1]
        self.dataset_path = dataset_path
        self.augment = InstanceAugmentation(p=0.2, phase=mode)
        if mode=='train':
            self.filename_list = load_file_name_list(os.path.join(dataset_path, 'train_name_list.txt'))
        elif mode =='val':
            self.filename_list = load_file_name_list(os.path.join(dataset_path, 'val_name_list.txt'))
        else:
            raise TypeError('Dataset mode error!!! ')
        d=dict()
        for line in np.array(pd.read_csv('./datasetlist/one_hot_win_washed.csv'))[:,:-1]:
            imgname=line[0].split('\\')[2]
            position=line[0].split('\\')[-1].split('_gold')[0]
            # print(imgname+position)
            d[imgname+position]=line[1:-1]
            # print(imgname+position)
        self.ground_truth=d


    def __getitem__(self, index):
        data,label= self.get_train_batch_by_index(index=index)
        data=np.expand_dims(data,axis=0)

        return torch.from_numpy(data.copy()),torch.from_numpy(label.copy())

    def __len__(self):
        return len(self.filename_list)

    def get_train_batch_by_index(self,index):
        img,label = self.get_np_data_3d(self.filename_list[index])
        return img,label

    def get_np_data_3d(self, filename):

        data_np = sitk_read_raw(filename)
        data_np=np.array(data_np,dtype='uint8')
        if data_np.shape!=(128,128,128):
            temp=np.zeros((128,128,128))
            temp[0:data_np.shape[0],0:data_np.shape[1],0:data_np.shape[2]]=data_np
            data_np=temp

        fakelabel=np.zeros((128,128,128))
        data_np,fakelabel=self.augment(data_np,fakelabel)
        data_np = norm_img(data_np)
        data_np=np.array(data_np,dtype='float')
        imgname=filename.split('\\')[1]
        labelname=filename.split('\\')[-1].split('_raw')[0]
        key=imgname+labelname

        label_onehot=np.array(self.ground_truth[key],dtype='int')
        label=np.array(label_onehot)
        label=np.argmax(label_onehot)
        label=np.array(label,dtype='int')
        return data_np,label
