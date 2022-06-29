# TensorFlow and tf.keras
import json
import os

import tensorflow as tf
import keras
import tifffile
import cv2
# Helper libraries
import numpy as np
import matplotlib.pyplot as plt

pic_path = 'C:/Users/admin/Desktop/testData/test'
results_path = 'C:/Users/admin/Desktop/testData/results'
model_path = 'C:/Users/admin/Desktop/data/relu_center_10_150_model_4.h5'
model = keras.models.load_model(model_path)
model.summary()

raw_ims = []
ideal_ims = []
for root, dirs, files in os.walk(pic_path):
    for f in files:
        raw_im_path = root + '/' + f
        raw_im = tifffile.imread(raw_im_path) / 65535
        a = raw_im.reshape(1, 64, 64, 64)
        raw_ims.append(a)
raw_ims = np.asarray(raw_ims)
results = model.predict(raw_ims)
b = 0
for r in results:
    a = np.zeros(shape=(64, 64, 64), dtype=np.float64)
    for i in range(64):
        for j in range(64):
            for k in range(64):
                if r[0][i][j][k] > 0:
                    a[i][j][k] = r[0][i][j][k]
    a = a * 65535
    a = a.astype(np.uint16)
    tifffile.imwrite(results_path + '/' + str(b).zfill(2) + '_r.tif', a)
    b=b+1

