import numpy as np
import os
import torch
from scipy import ndimage
from torch.utils.data import Dataset, DataLoader
from torchvision import transforms as T
from utilsa.common import *
from process.DataAugmentation import *

class Soma_DataSet(Dataset):
    def __init__(self, dataset_path,mode=None):

        self.dataset_path = dataset_path
        self.augment = InstanceAugmentation(p=0.2, phase=mode)
        if mode=='train':
            self.filename_list = load_file_name_list(os.path.join(dataset_path, 'train_name_list.txt'))
        elif mode =='val':
            self.filename_list = load_file_name_list(os.path.join(dataset_path, 'val_name_list.txt'))
        else:
            raise TypeError('Dataset mode error!!! ')


    def __getitem__(self, index):
        data, target = self.get_train_batch_by_index(index=index)
        data=np.expand_dims(data,axis=0)

        return torch.from_numpy(data.copy()), torch.from_numpy(target.copy())

    def __len__(self):
        return len(self.filename_list)

    def get_train_batch_by_index(self,index):
        img, label = self.get_np_data_3d(self.filename_list[index])
        return img, label

    def get_np_data_3d(self, filename):

        data_np = sitk_read_raw(self.dataset_path +'data/'+ filename)
        data_np=np.array(data_np,dtype='uint8')

        label_np = sitk_read_raw(self.dataset_path + 'label/seg_' + filename)
        label_np=np.array(label_np,dtype='uint8')
        label_np[label_np!=0]=1

        data_np, label_np = self.augment(data_np, label_np)
        data_np = norm_img(data_np)
        # label_np = norm_img(label_np)



        return data_np, label_np

# 测试代码
'''
import matplotlib.pyplot as plt
def main():
    fixd_path  = r'D:\DLresult'
    dataset = Soma_DataSet([128,128,128],1,fixd_path,mode='train')  #batch size
    data_loader=DataLoader(dataset=dataset,batch_size=2,num_workers=1, shuffle=True)
    for batch_idx, (data, target) in enumerate(data_loader):
        target = to_one_hot_3d(target.long())
        print(data.shape, target.shape)
        plt.subplot(121)
        plt.imshow(data[0, 0, 0])
        plt.subplot(122)
        plt.imshow(target[0, 1, 0])
        plt.show()
if __name__ == '__main__':
    main()'''
