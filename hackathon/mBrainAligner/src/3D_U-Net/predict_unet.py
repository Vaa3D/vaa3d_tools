import os
import sys
import copy
import numpy as np
import SimpleITK as sitk
from scipy.ndimage.interpolation import zoom

from train_unet import config
from model import unet_model_3d
from Formatcov import load_v3d_raw_img_file1, save_v3d_raw_img_file1
from metrics import dice_coefficient
import argparse
sys.path.append("..")

parser = argparse.ArgumentParser()
parser.add_argument("--nii", action='store_true')
parser.add_argument("--gpu", '-g', default='1', type=str)
args = parser.parse_args()

os.environ["CUDA_VISIBLE_DEVICES"] = args.gpu


def itensity_normalize_one_volume(volume):
    pixels = volume[volume > 0]
    mean = pixels.mean()
    std = pixels.std()
    out = (volume - mean) / std
    out_random = np.random.normal(0, 1, size=volume.shape)
    out[volume == 0] = out_random[volume == 0]
    # out[volume == 0] = 0
    return out


def ResizeData(data, InputShape):
    [W, H, D] = data.shape
    scale = [InputShape[0] * 1.0 / W, InputShape[1] * 1.0 / H, InputShape[2] * 1.0 / D]
    data = zoom(data, scale, order=1)
    return data


def ResizeMap(data, InputShape):
    [W, H, D, C] = data.shape
    scale = [InputShape[0] * 1.0 / W, InputShape[1] * 1.0 / H, InputShape[2] * 1.0 / D, 1]
    data = zoom(data, scale, order=1)
    return data


def main():
    save_dir = './data/predict/'
    image_dir = './data/test/'
    model_dir = config["model_file"]

    model = unet_model_3d(input_shape=config["input_shape"],
                          pool_size=config["pool_size"],
                          n_labels=config["n_labels"],
                          deconvolution=config["deconvolution"])
    if os.path.exists(model_dir):
        model.load_weights(model_dir, by_name=True)
        print(' ------------  load model !')
    else:
        print('model do not existing in ', model_dir)
        return
    input_shape = config["image_shape"]

    class2inten = {0: 0, 1: 62, 2: 75, 3: 80, 4: 100, 5: 145, 6: 159, 7: 168, 8: 0, 9: 249}

    names = os.listdir(image_dir)
    for name in names:
        image_path = os.path.join(image_dir, name)
        if name.endswith('.v3draw'):
            v3d_flag = True
            im_v3d = load_v3d_raw_img_file1(image_path)['data']
            shape_v3d = im_v3d.shape
            im_np = im_v3d[..., 0]
            ori_shape = im_np.shape
            im_np = ResizeData(im_np, input_shape)
            im_np = itensity_normalize_one_volume(im_np)
            im_np = im_np[np.newaxis, ..., np.newaxis]
            im_save = {}
            im_save['size'] = shape_v3d
            im_save['datatype'] = 4
        elif name.endswith('.nii') or name.endswith('.nii.gz'):
            # nii should have size: 320 568 456
            # TODO: only handle 456 320 568 like issue
            v3d_flag = False
            gz_flag = False
            if name.endswith('.nii.gz'):
                gz_flag = True
            im_np = sitk.GetArrayFromImage(sitk.ReadImage(image_path))
            ori_shape = im_np.shape
            if ori_shape[-1] > ori_shape[0] and ori_shape[-1] > ori_shape[1]:
                print('transfer nii shape ', ori_shape)
                im_np = im_np.transpose(1, -1, 0)
                ori_shape = im_np.shape
                print('to ', ori_shape)
            im_np = ResizeData(im_np, input_shape)
            im_np = itensity_normalize_one_volume(im_np)
            im_np = im_np[np.newaxis, ..., np.newaxis]
        else:
            print('wrong image format')
            return

        print('input image has shape: ', ori_shape, ' and will resize to shape: ', input_shape, ' as model input')

        print('--------------------- predicting: ', name, '... ----------------------------')
        pre_hot = model.predict(im_np)[0]

        # saving segmentation map to nii or v3draw
        print('resize predict map...')
        pre_hot = ResizeMap(pre_hot, ori_shape + (10,))
        print(pre_hot.shape)
        pre_class = np.float32(np.argmax(pre_hot, axis=3))

        for i in range(10):
            print('processing class ', i, ' result...')
            # processing classes prediction
            pre_class[pre_class == i] = class2inten[i]
            # saving one hot
            if i != 8:
                if v3d_flag:
                    im_save['data'] = pre_hot[:, :, :, i]
                    im_save['data'] = im_save['data'][..., np.newaxis]
                    im_save['data'].flags['WRITEABLE'] = True
                    save_path = save_dir + name.split('.v3draw')[0] + '/'
                    if not os.path.exists(save_path):
                        os.mkdir(save_path)
                    if i != 9:
                        save_path = save_path + str(i) + '.v3draw'
                    elif i == 9:
                        save_path = save_path + '/8.v3draw'
                    save_v3d_raw_img_file1(im_save, save_path)
                else:
                    im_save = sitk.GetImageFromArray(pre_hot[:, :, :, i].transpose(-1, 0, 1))
                    save_path = os.path.join(save_dir, name.split('.nii.gz' if gz_flag else '.nii')[0])
                    if not os.path.exists(save_path):
                        os.mkdir(save_path)
                    if i != 9:
                        save_path = os.path.join(save_path, str(i) + '.nii.gz' if gz_flag else str(i) + '.nii')
                    elif i == 9:
                        save_path = os.path.join(save_path, '8.nii.gz' if gz_flag else '8.nii')

                    sitk.WriteImage(im_save, save_path)

        # saving classes prediction
        print('saving seg result...')
        if v3d_flag:
            im_save['data'] = pre_class[..., np.newaxis]
            save_path = save_dir + name.split('.v3draw')[0] + '/seg.v3draw'
            save_v3d_raw_img_file1(im_save, save_path)
        else:
            im_save = sitk.GetImageFromArray(pre_class.transpose(-1, 0, 1))
            save_path = os.path.join(save_dir, name.split('.nii.gz' if gz_flag else '.nii')[0], 'seg.nii.gz' if gz_flag else 'seg.nii')
            sitk.WriteImage(im_save, save_path)


if __name__ == "__main__":
    main()
