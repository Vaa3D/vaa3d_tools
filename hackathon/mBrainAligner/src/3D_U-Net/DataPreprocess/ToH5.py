"""
brain with same id should not split into both train and val
revised at 2022.3.16
"""

import os
import numpy as np
import SimpleITK as sitk
from random import random
from random import shuffle
from Formatcov import load_v3d_raw_img_file1, save_v3d_raw_img_file1
import tables
import warnings
warnings.filterwarnings('ignore')


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


def main():
    # ------------------- attention: define path and validation set here --------------------------
    data_root = './data/orig/'
    val_id = [17788, 18457, 18458, 182725, 191813]

    # --------------------------------------------------------------------------
    img_dir = os.path.join(data_root, 'img')
    label_dir = os.path.join(data_root, 'label')
    train_file = "./DataPreprocess/train.h5"
    val_file = "./DataPreprocess/val.h5"
    img_names = os.listdir(img_dir)

    # check data
    label_names = os.listdir(label_dir)
    if not img_names == label_names:
        raise ValueError('img names should same to label names')

    # split data
    img_id = np.unique([name.split('_')[0] for name in img_names])
    train_id = []
    val_id = [str(id) for id in val_id]
    if any(id not in img_id for id in val_id):
        raise ValueError('val id not in dataset!')
    for id in img_id:
        if id not in val_id:
            train_id.append(id)
    train_img_path = [os.path.join(img_dir, name) for name in img_names if any(id in name for id in train_id)]
    val_img_path = [os.path.join(img_dir, name) for name in img_names if any(id in name for id in val_id)]
    dataset_log = './DataPreprocess/data_split.txt'
    with open(dataset_log, 'w') as f:
        f.write("Train ID:\n")
        f.write("-----------------\n")
        for id in train_id:
            f.write('{}\n'.format(id))
        f.write("Validate ID:\n")
        f.write("-----------------\n")
        for id in val_id:
            f.write('{}\n'.format(id))

    # define img shape
    temp_img = load_v3d_raw_img_file1(os.path.join(img_dir, img_names[0]))
    input_shape = temp_img['data'].squeeze().shape

    # create h5 file
    try:
        train_h5, train_img, train_label = create_data_file(train_file, image_shape=input_shape)
        val_h5, val_img, val_label = create_data_file(val_file, image_shape=input_shape)
    except Exception as e:
        os.remove(out_file)
        raise e

    shuffle(train_img_path)
    for img_path in train_img_path:
        print('processing ', img_path)
        img_np = load_v3d_raw_img_file1(img_path)['data']
        img_np = np.moveaxis(img_np, -1, 0)
        label_np = load_v3d_raw_img_file1(img_path.replace('img', 'label'))['data']
        label_np = np.moveaxis(label_np, -1, 0)
        label_np.flags.writeable = True
        pixels = np.unique(label_np)
        ind = 0
        for pixel in pixels:
            label_np[label_np == pixel] = ind
            ind += 1
        train_img.append(img_np)
        train_label.append(label_np)
    train_h5.close()
    for img_path in val_img_path:
        print('processing ', img_path)
        img_np = load_v3d_raw_img_file1(img_path)['data']
        img_np = np.moveaxis(img_np, -1, 0)
        label_np = load_v3d_raw_img_file1(img_path.replace('img', 'label'))['data']
        label_np = np.moveaxis(label_np, -1, 0)
        label_np.flags.writeable = True
        pixels = np.unique(label_np)
        ind = 0
        for pixel in pixels:
            label_np[label_np == pixel] = ind
            ind += 1
        val_img.append(img_np)
        val_label.append(label_np)
    val_h5.close()


if __name__ == "__main__":
    main()
