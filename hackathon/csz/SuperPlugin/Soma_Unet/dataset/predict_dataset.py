from utilsa.common import *
from scipy import ndimage
import numpy as np
from torchvision import transforms as T
import torch, os
from torch.utils.data import Dataset, DataLoader
from glob import glob
import math
import SimpleITK as sitk

class predict_dataset(Dataset):
    def __init__(self, dataset_path):
        self.dataset_path = dataset_path
        self.filename_list = load_file_name_list(os.path.join(dataset_path, 'predict_name_list.txt'))

    def __getitem__(self, index):
        data= self.get_test_batch_by_index(index=index)
        return torch.from_numpy(data)

    def __len__(self):
        return len(self.filename_list)

    def get_test_batch_by_index(self, index):
        img= self.get_np_data_3d(self.filename_list[index])
        return np.expand_dims(img,axis=0)

    def get_np_data_3d(self, filename):
        data_np = sitk_read_raw(self.dataset_path + filename)
        # print(data_np.shape)
        data_np = norm_img(data_np)
        return data_np