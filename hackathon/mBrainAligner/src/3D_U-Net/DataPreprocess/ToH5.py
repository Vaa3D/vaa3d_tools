
import os
import SimpleITK as sitk
import numpy as np
from random import shuffle
from Formatcov import load_v3d_raw_img_file1, save_v3d_raw_img_file1
import tables
import warnings
warnings.filterwarnings('ignore')
from random import random


def create_data_file(out_file, image_shape):
    hdf5_file = tables.open_file(out_file, mode='w')
    filters = tables.Filters(complevel=5, complib='blosc')
    data_shape = tuple([0] + list(image_shape))
    label_shape = tuple([0] + list(image_shape))
    data_storage = hdf5_file.create_earray(hdf5_file.root, 'data', tables.Float32Atom(), shape=data_shape,
                                           filters=filters)
    truth_storage = hdf5_file.create_earray(hdf5_file.root, 'label', tables.Float32Atom(), shape=label_shape,
                                            filters=filters)
    return hdf5_file, data_storage, truth_storage


def LoadImage(imagePath):
    image = sitk.ReadImage(imagePath)
    array = sitk.GetArrayFromImage(image)
    return array


LabelDir = './data/orig/label/'
ImageDir = './data/orig/img/'

out_file = "./DataPreprocess/train.h5"
out_file1 = "./DataPreprocess/val.h5"


input_shape = (200, 324, 268)
channels = 1

try:
    hdf5_file, data_storage, truth_storage = create_data_file(out_file, image_shape=input_shape)
except Exception as e:
    os.remove(out_file)
    raise e


try:
    hdf5_file1, data_storage1, truth_storage1 = create_data_file(out_file1, image_shape=input_shape)
except Exception as e:
    os.remove(out_file1)
    raise e


Names = os.listdir(ImageDir)
shuffle(Names)
count = 0
for name in Names:
    count+=1
    ImagePath = ImageDir + name
    LabelPath = LabelDir + name
    Im = load_v3d_raw_img_file1(ImagePath)
    ImageArray = Im['data']
    ImageArray = ImageArray[:, :, :, 0]
    Im = load_v3d_raw_img_file1(LabelPath)
    LabelArray = Im['data']
    LabelArray.flags.writeable = True
    LabelArray = LabelArray[:, :, :, 0]

    LabelArray[LabelArray == 10] = 0
    LabelArray[LabelArray == 62] = 1
    LabelArray[LabelArray == 75] = 2
    LabelArray[LabelArray == 80] = 3
    LabelArray[LabelArray == 100] = 4
    LabelArray[LabelArray == 145] = 5
    LabelArray[LabelArray == 159] = 6
    LabelArray[LabelArray == 168] = 7
    LabelArray[LabelArray == 237] = 8
    LabelArray[LabelArray == 249] = 9

    LabelArray = np.asarray(LabelArray)[np.newaxis]
    ImageArray = np.asarray(ImageArray)[np.newaxis]
    print(count, ImageArray.shape, LabelArray.shape)
    if count % 7 == 0:
        print(name)
        truth_storage1.append(LabelArray)
        data_storage1.append(ImageArray)
    else:
        truth_storage.append(LabelArray)
        data_storage.append(ImageArray)

hdf5_file.close()
hdf5_file1.close()
