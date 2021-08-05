import os
import numpy as np
from train_unet import config
import tables
import SimpleITK as sitk
from scipy.ndimage.interpolation import zoom
import copy
import sys
sys.path.append("..")
import tables
from model import unet_model_3d
from keras.utils.np_utils import to_categorical
from Formatcov import load_v3d_raw_img_file1,save_v3d_raw_img_file1

import argparse
parser = argparse.ArgumentParser()
parser.add_argument("--opt", type=str, default='Adam')
parser.add_argument("--WeightLoss", type=int, default=0)
parser.add_argument("--lr", type=float, default=0.001)
args = parser.parse_args()

os.environ["CUDA_VISIBLE_DEVICES"] = "0"


def itensity_normalize_one_volume(volume):
    pixels = volume[volume > 0]
    mean = pixels.mean()
    std = pixels.std()
    out = (volume - mean) / std
    out_random = np.random.normal(0, 1, size=volume.shape)
    out[volume == 0] = out_random[volume == 0]
    # out[volume == 0] = 0
    return out


def dice_coefficient(y_true, y_pred, smooth=1e-5):
    y_true_d = np.sum(y_true*y_true)
    y_pred_d = np.sum(y_pred*y_pred)
    intersection = np.sum(y_true * y_pred)
    return round((2. * intersection + smooth) / (y_true_d + y_pred_d + smooth),4)


def saveimage(array,imagepath):
    image = sitk.GetImageFromArray(array)
    sitk.WriteImage(image,imagepath)


def LoadImage(imagePath):
    image = sitk.ReadImage(imagePath)
    array = sitk.GetArrayFromImage(image)
    return array


def ResizeData(data,InputShape):
    [W,H,D] = data.shape
    scale = [InputShape[0]*1.0/W,InputShape[1]*1.0/H,InputShape[2]*1.0/D]
    data = zoom(data,scale,order = 0)
    return data






SaveDir = './data/predict/'
ImageDir = './data/test/'



model = unet_model_3d(input_shape=config["input_shape"],
              pool_size=config["pool_size"],
              n_labels=config["n_labels"],
              deconvolution=config["deconvolution"])
if os.path.exists(config["model_file"]):
    model.load_weights(config["model_file"],by_name = True)
    print(' ------------  load model !')

input_shape = config["image_shape"]

Names = os.listdir(ImageDir)
count = 0
for name in Names:
    print('predicting ',name,'.......')
    count+=1
    ImagePath = ImageDir + name
    Im = load_v3d_raw_img_file1(ImagePath)
    ImageArray = Im['data']
    shape = ImageArray.shape    # this shape is for save v3d image (has channel information)
    ImageArray = ImageArray[:,:,:,0]
    ori_shape=ImageArray.shape

    ImageArray = ResizeData(ImageArray, input_shape)
    ImageArray = itensity_normalize_one_volume(ImageArray)
    ImageArray = np.asarray(ImageArray)[np.newaxis][np.newaxis]
    ImageArray = np.moveaxis(ImageArray, 1, 4)

    prediction = model.predict(ImageArray)
    prediction = prediction[0]

    Im={}
    Im['size']=shape
    Im['datatype']=4 #float32

    # remove no.9 class cause it is no longer needed
    # save solo class image i.e. save 10 images in total
    for i in range(10):
        if i<=7:
            prediction1 = ResizeData(prediction[:, :, :, i], ori_shape)
            Im['data'] = prediction1[:, :, :, np.newaxis]
            Im['data'] .flags['WRITEABLE'] = True
            savepath=SaveDir + name.split('.v')[0] + '/' + str(i) + '.v3draw'
            if not os.path.exists(savepath.split(str(i)+'.v')[0]):
                os.mkdir(savepath.split(str(i)+'.v')[0])
            save_v3d_raw_img_file1(Im, savepath)
        elif i==9:
            prediction1 = ResizeData(prediction[:, :, :, i], ori_shape)
            Im['data'] = prediction1[:, :, :, np.newaxis]
            Im['data'] .flags['WRITEABLE'] = True
            savepath=SaveDir + name.split('.v')[0] + '/8.v3draw'
            save_v3d_raw_img_file1(Im, savepath)



    # save the segmentation image
    # attention: prediction1 get int64 dtype
    prediction1 = np.argmax(prediction, axis=3)
    prediction1 = ResizeData(prediction1, ori_shape)
    prediction2 = copy.deepcopy(prediction1)
    prediction2=prediction2.astype('float32')

    prediction2[prediction2 == 1] = 62
    prediction2[prediction2 == 2] = 75
    prediction2[prediction2 == 3] = 80
    prediction2[prediction2 == 4] = 100
    prediction2[prediction2 == 5] = 145
    prediction2[prediction2 == 6] = 159
    prediction2[prediction2 == 7] = 168
    # prediction2[prediction2 == 8] = 237
    prediction2[prediction2 == 8] = 0
    prediction2[prediction2 == 9] = 249

    Im['data'] = prediction2[:, :, :, np.newaxis]
    savepath = SaveDir + name.split('.v')[0] + '/' + 'seg.v3draw'
    if not os.path.exists(savepath.split('seg')[0]):
        os.mkdir(savepath.split('seg')[0])
    save_v3d_raw_img_file1(Im, savepath)


