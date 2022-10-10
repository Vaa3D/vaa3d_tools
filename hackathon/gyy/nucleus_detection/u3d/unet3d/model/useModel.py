# TensorFlow and tf.keras
import json
import os

import tensorflow as tf
import keras
import tifffile
# Helper libraries
import numpy as np


pic_path = r'F:\NuMorph\3DNucleiTracingData\cut_Training\testData'
results_path = r'F:\NuMorph\3DNucleiTracingData\cut_Training\evaluation_r9_2\nomorph'
model_path = r'C:\Users\admin\Desktop\program_Yanyan\NuMorph\unet3d\isensee_2017_model.h5'
model = keras.models.load_model(model_path)
model.summary()

raw_ims = []
ideal_ims = []

for root, dirs, files in os.walk(pic_path):
    for f in files:
        raw_im_path = root + '/' + f
        raw_im = tifffile.imread(raw_im_path) / 65535
        a = raw_im.reshape(1, raw_im.shape[0], raw_im.shape[1], raw_im.shape[2])
        a=np.asarray(a)
        a=np.expand_dims(a,axis=0)
        a=model.predict(a)
        raw_ims.append(a)
raw_ims = np.asarray(raw_ims)
#results = model.predict(raw_ims)
results = np.asarray(raw_ims)

img_list = os.listdir(pic_path)
b = 0
for r in results:
    r=np.asarray(r)
    img_name = img_list[b]

    a = np.zeros(shape=(64, 64, 64), dtype=np.float64)
    # print(r.shape)
    for i in range(64):
        for j in range(64):
            for k in range(64):
                if r[0][0][i][j][k] > 0:
                    a[i][j][k] = r[0][0][i][j][k]
    a = a * 65535
    a = a.astype(np.uint16)
    # tifffile.imwrite(results_path + '/' + str(b).zfill(3) + '_r.tif', a)
    result_name = results_path + '/' + img_name + '_r.tif'
    print(result_name)
    tifffile.imwrite(result_name, a)
    b=b+1

