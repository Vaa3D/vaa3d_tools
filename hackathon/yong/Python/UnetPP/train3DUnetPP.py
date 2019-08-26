#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from __future__ import print_function

import os
import keras
import tensorflow as tf
from keras.models import Model
from keras.utils import plot_model
from keras import backend as K
from keras.callbacks import LambdaCallback, TensorBoard
from keras.layers import Input, merge, Conv3D, ZeroPadding2D, UpSampling3D, Dense, concatenate, Conv3DTranspose
from keras.layers.pooling import MaxPooling3D, GlobalAveragePooling3D, MaxPooling3D
from keras.layers.core import Dense, Dropout, Activation
from keras.layers import BatchNormalization, Dropout, Flatten, Lambda
from keras.layers.advanced_activations import ELU, LeakyReLU
from keras.optimizers import Adam, RMSprop, SGD
from keras.regularizers import l2
from keras.layers.noise import GaussianDropout
from skimage.transform import resize
from skimage.io import imsave
from libtiff import TIFF
import numpy as np
import matplotlib.pyplot as plt
import shutil
import cv2
from data3DUnetPP import load_train_data, load_test_data
from keras.models import load_model, model_from_json

model_path = 'C:\\Users\\admin\\Desktop\\zhangyong\\unetpp\\'
logs_path = 'C:\\Users\\admin\\Desktop\\zhangyong\\unetpp\\'
images_x = 32
images_y = 32
images_z = 16

EPOCHS = 1000
BS = 1

GPU_COUNT = 1
smooth = 1.
dropout_rate = 0.5
act = "relu"
patience = 30

def mean_iou(y_true, y_pred):
    prec = []
    for t in np.arange(0.5, 1.0, 0.05):
        y_pred_ = tf.to_int32(y_pred > t)
        score, up_opt = tf.metrics.mean_iou(y_true, y_pred_, 2)
        K.get_session().run(tf.local_variables_initializer())
        with tf.control_dependencies([up_opt]):
            score = tf.identity(score)
        prec.append(score)
    return K.mean(K.stack(prec), axis=0)

# Custom loss function
def dice_coef(y_true, y_pred):
    smooth = 1.
    y_true_f = K.flatten(y_true)
    y_pred_f = K.flatten(y_pred)
    intersection = K.sum(y_true_f * y_pred_f)
    return (2. * intersection + smooth) / (K.sum(y_true_f) + K.sum(y_pred_f) + smooth)

# 样本前景背景比例不平衡 加快速度
def bce_dice_loss(y_true, y_pred):
    # return -dice_coef(y_true, y_pred)
    return 0.5 * keras.losses.binary_crossentropy(y_true, y_pred) - dice_coef(y_true, y_pred)

def standard_unit(input_tensor, stage, nb_filter, kernel_size=3):
    x = Conv3D(nb_filter, (kernel_size, kernel_size, kernel_size), activation=act, name='conv'+stage+'_1', kernel_initializer = 'he_normal', padding='same', kernel_regularizer=l2(1e-4))(input_tensor)
    x = Dropout(dropout_rate, name='dp'+stage+'_1')(x)
    x = Conv3D(nb_filter, (kernel_size, kernel_size, kernel_size), activation=act, name='conv'+stage+'_2', kernel_initializer = 'he_normal', padding='same', kernel_regularizer=l2(1e-4))(x)
    x = Dropout(dropout_rate, name='dp'+stage+'_2')(x)

    return x

def get_3DUnetPP(images_x, images_y, images_z, color_type=1, num_class=1,deep_supervision=False):
    nb_filter = [32,64,128,256,512]

    # Handle Dimension Ordering for different backends
    global bn_axis
    if K.image_dim_ordering() == 'tf':
      bn_axis = -1
      img_input = Input(shape=(images_x, images_y, images_z, color_type), name='main_input')
    else:
      bn_axis = 1
      img_input = Input(shape=(color_type, images_x, images_y, images_z), name='main_input')
    
    conv1_1 = standard_unit(img_input, stage='11', nb_filter=nb_filter[0])
    pool1 = MaxPooling3D((2, 2, 2), strides=(2, 2, 2), name='pool1')(conv1_1)

    conv2_1 = standard_unit(pool1, stage='21', nb_filter=nb_filter[1])
    pool2 = MaxPooling3D((2, 2, 2), strides=(2, 2, 2), name='pool2')(conv2_1)

    up1_2 = Conv3DTranspose(nb_filter[0], (2, 2, 2), strides=(2, 2, 2), name='up12', padding='same')(conv2_1)
    conv1_2 = concatenate([up1_2, conv1_1], name='merge12', axis=bn_axis)
    conv1_2 = standard_unit(conv1_2, stage='12', nb_filter=nb_filter[0])

    conv3_1 = standard_unit(pool2, stage='31', nb_filter=nb_filter[2])
    pool3 = MaxPooling3D((2, 2, 2), strides=(2, 2, 2), name='pool3')(conv3_1)

    up2_2 = Conv3DTranspose(nb_filter[1], (2, 2, 2), strides=(2, 2, 2), name='up22', padding='same')(conv3_1)
    conv2_2 = concatenate([up2_2, conv2_1], name='merge22', axis=bn_axis)
    conv2_2 = standard_unit(conv2_2, stage='22', nb_filter=nb_filter[1])

    up1_3 = Conv3DTranspose(nb_filter[0], (2, 2, 2), strides=(2, 2, 2), name='up13', padding='same')(conv2_2)
    conv1_3 = concatenate([up1_3, conv1_1, conv1_2], name='merge13', axis=bn_axis)
    conv1_3 = standard_unit(conv1_3, stage='13', nb_filter=nb_filter[0])

    conv4_1 = standard_unit(pool3, stage='41', nb_filter=nb_filter[3])
    pool4 = MaxPooling3D((2, 2, 2), strides=(2, 2, 2), name='pool4')(conv4_1)

    up3_2 = Conv3DTranspose(nb_filter[2], (2, 2, 2), strides=(2, 2, 2), name='up32', padding='same')(conv4_1)
    conv3_2 = concatenate([up3_2, conv3_1], name='merge32', axis=bn_axis)
    conv3_2 = standard_unit(conv3_2, stage='32', nb_filter=nb_filter[2])

    up2_3 = Conv3DTranspose(nb_filter[1], (2, 2, 2), strides=(2, 2, 2), name='up23', padding='same')(conv3_2)
    conv2_3 = concatenate([up2_3, conv2_1, conv2_2], name='merge23', axis=bn_axis)
    conv2_3 = standard_unit(conv2_3, stage='23', nb_filter=nb_filter[1])

    up1_4 = Conv3DTranspose(nb_filter[0], (2, 2, 2), strides=(2, 2, 2), name='up14', padding='same')(conv2_3)
    conv1_4 = concatenate([up1_4, conv1_1, conv1_2, conv1_3], name='merge14', axis=bn_axis)
    conv1_4 = standard_unit(conv1_4, stage='14', nb_filter=nb_filter[0])

    conv5_1 = standard_unit(pool4, stage='51', nb_filter=nb_filter[4])

    up4_2 = Conv3DTranspose(nb_filter[3], (2, 2, 2), strides=(2, 2, 2), name='up42', padding='same')(conv5_1)
    conv4_2 = concatenate([up4_2, conv4_1], name='merge42', axis=bn_axis)
    conv4_2 = standard_unit(conv4_2, stage='42', nb_filter=nb_filter[3])

    up3_3 = Conv3DTranspose(nb_filter[2], (2, 2, 2), strides=(2, 2, 2), name='up33', padding='same')(conv4_2)
    conv3_3 = concatenate([up3_3, conv3_1, conv3_2], name='merge33', axis=bn_axis)
    conv3_3 = standard_unit(conv3_3, stage='33', nb_filter=nb_filter[2])

    up2_4 = Conv3DTranspose(nb_filter[1], (2, 2, 2), strides=(2, 2, 2), name='up24', padding='same')(conv3_3)
    conv2_4 = concatenate([up2_4, conv2_1, conv2_2, conv2_3], name='merge24', axis=bn_axis)
    conv2_4 = standard_unit(conv2_4, stage='24', nb_filter=nb_filter[1])

    up1_5 = Conv3DTranspose(nb_filter[0], (2, 2, 2), strides=(2, 2, 2), name='up15', padding='same')(conv2_4)
    conv1_5 = concatenate([up1_5, conv1_1, conv1_2, conv1_3, conv1_4], name='merge15', axis=bn_axis)
    conv1_5 = standard_unit(conv1_5, stage='15', nb_filter=nb_filter[0])

    # conv_final1 = Conv3D(2, (1, 1, 1), activation=act, name='class', kernel_initializer='he_normal', padding='same', kernel_regularizer=l2(1e-4))(conv1_2)
    # conv_final2 = Conv3D(2, (1, 1, 1), activation=act, name='class', kernel_initializer='he_normal', padding='same', kernel_regularizer=l2(1e-4))(conv1_3)
    # conv_final3 = Conv3D(2, (1, 1, 1), activation=act, name='class', kernel_initializer='he_normal', padding='same', kernel_regularizer=l2(1e-4))(conv1_4)
    # conv_final4 = Conv3D(2, (1, 1, 1), activation=act, name='class', kernel_initializer = 'he_normal', padding='same', kernel_regularizer=l2(1e-4))(conv1_5)
    # nestnet_output_1 = Conv3D(num_class, (1, 1, 1), activation='sigmoid', name='output_1', kernel_initializer='he_normal', padding='same', kernel_regularizer=l2(1e-4))(conv_final1)
    # nestnet_output_2 = Conv3D(num_class, (1, 1, 1), activation='sigmoid', name='output_2', kernel_initializer='he_normal', padding='same', kernel_regularizer=l2(1e-4))(conv_final2)
    # nestnet_output_3 = Conv3D(num_class, (1, 1, 1), activation='sigmoid', name='output_3', kernel_initializer='he_normal', padding='same', kernel_regularizer=l2(1e-4))(conv_final3)
    # nestnet_output_4 = Conv3D(num_class, (1, 1, 1), activation='sigmoid', name='output_4', kernel_initializer='he_normal', padding='same', kernel_regularizer=l2(1e-4))(conv_final4)

    nestnet_output_1 = Conv3D(num_class, (1, 1, 1), activation='sigmoid', name='output_1', kernel_initializer = 'he_normal', padding='same', kernel_regularizer=l2(1e-4))(conv1_2)
    nestnet_output_2 = Conv3D(num_class, (1, 1, 1), activation='sigmoid', name='output_2', kernel_initializer = 'he_normal', padding='same', kernel_regularizer=l2(1e-4))(conv1_3)
    nestnet_output_3 = Conv3D(num_class, (1, 1, 1), activation='sigmoid', name='output_3', kernel_initializer = 'he_normal', padding='same', kernel_regularizer=l2(1e-4))(conv1_4)
    nestnet_output_4 = Conv3D(num_class, (1, 1, 1), activation='sigmoid', name='output_4', kernel_initializer = 'he_normal', padding='same', kernel_regularizer=l2(1e-4))(conv1_5)
 
    if deep_supervision: 
        model = Model(input=img_input, output=[nestnet_output_1,
                                                nestnet_output_2,
                                                nestnet_output_3,
                                                nestnet_output_4])
    else: 
        model = Model(input=img_input, output=[nestnet_output_4])

    return model

def preprocess(imgs):
    imgs_p = np.ndarray((imgs.shape[0],images_x,images_y,images_z), dtype = np.uint8)
    for i in range(imgs.shape[0]):
        imgAll = np.array([0])
        count = 0
        for j in range(imgs.shape[3]):
            img = resize(imgs[i, :, :, j], (images_x, images_y), preserve_range=True)
            if count == 0:
                imgAll = img
            else:
                imgAll = np.dstack((imgAll, img))
            count = count + 1
        imgs_p[i] = imgAll
    imgs_p = imgs_p[..., np.newaxis]
    return imgs_p

def train_predict():
    print('Loading and preprocessing train data!')

    images_train, images_mask_train = load_train_data()
    images_train = preprocess(images_train)
    images_mask_train = preprocess(images_mask_train)
    
    images_train = images_train.astype('float32')
    print(images_train.shape)
    mean = np.mean(images_train)
    std = np.std(images_train)
    print('mean = ', mean)
    print('std = ', std)

    images_train -= mean
    images_train /= std
    #print(images_train)
    
    images_mask_train = images_mask_train.astype('float32')
    images_mask_train /= 255.
    #print(images_mask_train)

    print('Creating and compiling model...')
    model = get_3DUnetPP(images_x, images_y, images_z, 1)
    # 断点续存
    # model=keras.models.load_model("UNetPP-old.h5",custom_objects={'bce_dice_loss': bce_dice_loss,'mean_iou':mean_iou,'dice_coef':dice_coef})
    model.compile(optimizer="Adam", loss=bce_dice_loss, metrics=["binary_crossentropy", mean_iou, dice_coef])

    # plot model
    exp_name = "UNetPP"
    plot_model(model, to_file=os.path.join(model_path, exp_name + ".png"))
    if os.path.exists(os.path.join(model_path,  exp_name + ".txt")):
        os.remove(os.path.join(model_path, exp_name + ".txt"))
    with open(os.path.join(model_path, exp_name + ".txt"), 'w') as fh:
        model.summary(positions=[.3, .55, .67, 1.], print_fn=lambda x: fh.write(x + '\n'))

    shutil.rmtree(os.path.join(logs_path, exp_name), ignore_errors=True)
    if not os.path.exists(os.path.join(logs_path, exp_name)):
        os.makedirs(os.path.join(logs_path, exp_name))
    tbCallBack = TensorBoard(log_dir=os.path.join(logs_path, exp_name),
                             histogram_freq=0,
                             write_graph=True,
                             write_images=True,
                             )
    tbCallBack.set_model(model)

    early_stopping = keras.callbacks.EarlyStopping(monitor='val_loss',
                                                   patience= patience,
                                                   verbose=0,
                                                   mode='min',
                                                   )
    check_point = keras.callbacks.ModelCheckpoint(os.path.join(model_path, exp_name + ".h5"),
                                                  monitor='val_loss',
                                                  verbose=1,
                                                  save_best_only=True,
                                                  mode='min',
                                                  )
    callbacks = [check_point, early_stopping, tbCallBack]

    print('Fitting model...')
    H = model.fit(images_train, images_mask_train, batch_size=BS, nb_epoch=EPOCHS, verbose=1, shuffle=True,
            validation_split=0.2, callbacks=callbacks)
    
    # save model
    model_json = model.to_json()
    with open(model_path + "model.json", "w") as json_file:
        json_file.write(model_json)

    # plot the training loss and accuracy
    plt.style.use("ggplot")
    plt.figure()
    N = EPOCHS
    plt.plot(np.arange(0, len(H.history["loss"])), H.history["loss"], label="train_loss")
    plt.plot(np.arange(0, len(H.history["loss"])), H.history["val_loss"], label="val_loss")
    # plt.plot(np.arange(0, N), H.history["acc"], label="train_acc")
    # plt.plot(np.arange(0, N), H.history["val_acc"], label="val_acc")
    plt.title("Training Loss and Accuracy on traffic-sign classifier")
    plt.xlabel("Epoch #")
    plt.ylabel("Loss/Accuracy")
    plt.legend(loc="lower left")
    plt.savefig(".//")

    print('Loading and preprocessing test data...')
    images_test, images_test_id = load_test_data()
    images_test = preprocess(images_test)
    images_test = images_test.astype('float32')

    print('Predicting masks on test data...')

    # load model
    # json_file = open('model.json', 'r')
    # loaded_model_json = json_file.read()
    # json_file.close()
    # model = model_from_json(loaded_model_json)

    model.load_weights(os.path.join(model_path, exp_name + ".h5"))
    model.compile(optimizer="Adam",
                  loss=bce_dice_loss,
                  metrics=["binary_crossentropy", mean_iou, dice_coef])
    images_mask_test = model.predict(images_test, verbose=1)
    np.save('images_mask_test.npy', images_mask_test)

    print('Saving predicted masks to files...')
    pred_dir = 'predicts'
    if not os.path.exists(pred_dir):
        os.mkdir(pred_dir)
    for image, image_id in zip(images_mask_test, images_test_id):
        image = (image[:, :, :, 0] * 255.).astype(np.uint8)
        # print(image.shape)
        # print(image)

        tif = TIFF.open(os.path.join(pred_dir, str(image_id) + '_unetpp.tif'), mode='w')
        for i in range(image.shape[2]):
            img = image[:, :, i]
            img = cv2.flip(img,0,dst=None) #垂直镜像
            tif.write_image(img, compression=None)
        # imsave(os.path.join(pred_dir, str(image_id) + '_unetpp.tif'), image)



def predict():
    images_test, images_test_id = load_test_data()
    images_test = preprocess(images_test)
    images_test = images_test.astype('float32')
    model = get_3DUnetPP(images_x, images_y, images_z, 1)
    model.load_weights(os.path.join(model_path,  "UNetPP.h5"))
    model.compile(optimizer="Adam",
                  loss=bce_dice_loss,
                  metrics=["binary_crossentropy", mean_iou, dice_coef])
    images_mask_test = model.predict(images_test, verbose=1)
    np.save('images_mask_test.npy', images_mask_test)

    print('Saving predicted masks to files...')
    pred_dir = 'predicts'
    if not os.path.exists(pred_dir):
        os.mkdir(pred_dir)
    for image, image_id in zip(images_mask_test, images_test_id):
        image = (image[:, :, :, 0] * 255.).astype(np.uint8)
        # print(image.shape)
        # print(image)

        tif = TIFF.open(os.path.join(pred_dir, str(image_id) + '_unetpp.tif'), mode='w')
        for i in range(image.shape[2]):
            img = image[:, :, i]
            img = cv2.flip(img, 0, dst=None)  # 垂直镜像
            tif.write_image(img, compression=None)


if __name__ == "__main__":
    train_predict()
    # predict2()