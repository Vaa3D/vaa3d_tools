import numpy as np
import keras
from keras import backend as K
from keras.engine import Input, Model
from keras.layers import Conv3D, MaxPooling3D, AveragePooling3D, UpSampling3D, Add,Activation, \
    BatchNormalization, PReLU, Deconvolution3D, LeakyReLU, add, Dense, Flatten, GlobalAveragePooling3D, Reshape, Multiply,Reshape,Subtract
from keras.optimizers import Adam,SGD
from keras.layers.core import Lambda

from model.base_block import ASPP_Block,SpatialChannelAttentionBlcok
from model.class_block import class_supervised_block

from metrics import dice_coefficient_loss, get_label_dice_coefficient_function, dice_coefficient,\
    focal_loss,dice_coefficient1,dice_coefficient_loss1
from model.base_block import create_convolution_block,\
    get_up_convolution,ChannelSep
K.set_image_data_format("channels_last")
keras.initializers.TruncatedNormal(mean=0.0, stddev=0.05, seed=None)


try:
    from keras.engine import merge
except ImportError:
    from keras.layers.merge import concatenate


#   --------------------   unet   --------------------   #

def unet_classification_model_3d(input_shape, pool_size=(2, 2, 2), n_labels=1, initial_learning_rate=0.00001,
                                 deconvolution=False, TrainOP='Adam',
                                 depth=4, batch_normalization=False, instance_normalization=True,
                                 activation_name="softmax",
                                 include_label_wise_dice_coefficients=False, metrics=dice_coefficient):
    print('the shape of the input : ', input_shape)
    level_filters = [32,64,128,256,512,480]
    inputs = Input(input_shape)
    current_layer = inputs
    levels = list()
    print("###", current_layer._keras_shape)

    # add levels with max pooling
    for layer_depth in range(depth):  # 01234
        if layer_depth > -1:
            layer2 = create_convolution_block(input_layer=current_layer,
                                              n_filters=level_filters[layer_depth],
                                              batch_normalization=batch_normalization,
                                              instance_normalization=instance_normalization,
                                              activation=LeakyReLU,
                                              name='conv1_' + str(layer_depth))
            layer2 = create_convolution_block(input_layer=layer2,
                                              n_filters=level_filters[layer_depth],
                                              batch_normalization=batch_normalization,
                                              instance_normalization=instance_normalization,
                                              activation=LeakyReLU,
                                              name='conv2_' + str(layer_depth))
            print("###", layer2._keras_shape)

            levels.append(layer2)
            if layer_depth < depth - 1:
                current_layer = MaxPooling3D(pool_size=pool_size, name='maxpool' + str(layer_depth))(layer2)
            else:
                current_layer = layer2

    # add levels with up-convolution or up-sampling
    for layer_depth in range(depth - 2, -1, -1):
        up_convolution = get_up_convolution(pool_size=pool_size,
                                            deconvolution=deconvolution,
                                            n_filters=level_filters[layer_depth],
                                            name='Upsample' + str(layer_depth))(current_layer)
        print("###", up_convolution._keras_shape)
        # current_layer = create_convolution_block(n_filters=level_filters[layer_depth],
        #                                          input_layer=levels[layer_depth],
        #                                          batch_normalization=batch_normalization,
        #                                          instance_normalization=instance_normalization,
        #                                          activation=LeakyReLU,
        #                                          name='Up_conv3_' + str(layer_depth))
        concat = concatenate([up_convolution, levels[layer_depth]], axis=4)
        current_layer = create_convolution_block(n_filters=level_filters[layer_depth],
                                                 input_layer=concat,
                                                 batch_normalization=batch_normalization,
                                                 instance_normalization=instance_normalization,
                                                 activation=LeakyReLU,
                                                 name='Up_conv1_' + str(layer_depth))
        current_layer = create_convolution_block(n_filters=level_filters[layer_depth],
                                                 input_layer=current_layer,
                                                 batch_normalization=batch_normalization,
                                                 instance_normalization=instance_normalization,
                                                 activation=LeakyReLU,
                                                 name='Up_conv2_' + str(layer_depth))


    final_convolution = Conv3D(n_labels, (1, 1, 1), name='FinalConv1')(current_layer)
    print("###", final_convolution.shape)


    final_convolution1, final_convolution2, final_convolution3 = class_supervised_block(final_convolution,nbasefilters=n_labels,name = 'CS2')
    act1 = Activation(activation_name, name='act1')(final_convolution)
    w_act_x = Activation(activation_name, name='act_x')(final_convolution1)
    w_act_y = Activation(activation_name, name='act_y')(final_convolution2)
    w_act_z = Activation(activation_name, name='act_z')(final_convolution3)
    model = Model(inputs=inputs, outputs=[act1, w_act_x, w_act_y, w_act_z])
    beishu = 0.33
    if TrainOP == 'Adam':
        # loss = {'act1': dice_coefficient_loss,
        #         'act_x': 'binary_crossentropy',
        #         'act_y': "binary_crossentropy",
        #         'act_z': "binary_crossentropy"},
        model.compile(optimizer=Adam(lr=initial_learning_rate),
                      loss={'act1': dice_coefficient_loss,
                            'act_x': dice_coefficient_loss,
                            'act_y': dice_coefficient_loss,
                            'act_z': dice_coefficient_loss},
                      loss_weights={'act1': 1.,
                                    'act_x': 1.0 * beishu,
                                    'act_y': 1.0 * beishu,
                                    'act_z': 1.0 * beishu},
                      metrics=[metrics])
        # model.compile(optimizer=Adam(lr=initial_learning_rate), loss=dice_coefficient_loss, metrics=metrics)
    else:
        model.compile(optimizer=SGD(lr=initial_learning_rate),
                      loss={'act1': dice_coefficient_loss,
                            'act_x': dice_coefficient_loss,
                            'act_y': dice_coefficient_loss,
                            'act_z': dice_coefficient_loss},
                      loss_weights={'act1': 1.,
                                    'act_x': 1.0 * beishu,
                                    'act_y': 1.0 * beishu,
                                    'act_z': 1.0 * beishu},
                      metrics=[metrics])
        # model.compile(optimizer=SGD(lr=initial_learning_rate), loss=dice_coefficient_loss, metrics=metrics)
    return model








#   --------------------   unet   --------------------   #

def DeepUnet_classification_model_3d(input_shape, pool_size=(2, 2, 2), n_labels=9, initial_learning_rate=0.00001,
                                 deconvolution=False, TrainOP='Adam',
                                 depth=5, batch_normalization=False, instance_normalization=True,
                                 activation_name="softmax",
                                 include_label_wise_dice_coefficients=False, metrics=dice_coefficient):
    print('the shape of the input : ', input_shape)
    level_filters = [30, 60, 120, 240, 320, 480]
    inputs = Input(input_shape)
    current_layer = inputs
    levels = list()
    print("###", current_layer._keras_shape)

    # add levels with max pooling
    for layer_depth in range(depth):  # 01234
        if layer_depth > -1:
            layer2 = create_convolution_block(input_layer=current_layer,
                                              n_filters=level_filters[layer_depth],
                                              batch_normalization=batch_normalization,
                                              instance_normalization=instance_normalization,
                                              activation=LeakyReLU,
                                              name='conv1_' + str(layer_depth))
            layer2 = create_convolution_block(input_layer=layer2,
                                              n_filters=level_filters[layer_depth],
                                              batch_normalization=batch_normalization,
                                              instance_normalization=instance_normalization,
                                              activation=LeakyReLU,
                                              name='conv2_' + str(layer_depth))
            print("###", layer2._keras_shape)

            levels.append(layer2)
            if layer_depth < depth - 1:
                current_layer = MaxPooling3D(pool_size=pool_size, name='maxpool' + str(layer_depth))(layer2)
            else:
                # current_layer = create_convolution_block(input_layer=layer2,
                #                                   n_filters=level_filters[4],
                #                                   batch_normalization=batch_normalization,
                #                                   instance_normalization=instance_normalization,
                #                                   activation=LeakyReLU,
                #                                   name='conv3')
                # print("###", current_layer._keras_shape)
                # current_layer = create_convolution_block(input_layer=current_layer,
                #                                   n_filters=level_filters[5],
                #                                   batch_normalization=batch_normalization,
                #                                   instance_normalization=instance_normalization,
                #                                   activation=LeakyReLU,
                #                                   name='conv4')
                # print("###", current_layer._keras_shape)
                current_layer = layer2

    # add levels with up-convolution or up-sampling
    for layer_depth in range(depth - 2, -1, -1):
        up_convolution = get_up_convolution(pool_size=pool_size,
                                            deconvolution=deconvolution,
                                            n_filters=level_filters[layer_depth],
                                            name='Upsample' + str(layer_depth))(current_layer)
        print("###", up_convolution._keras_shape)
        concat = concatenate([up_convolution, levels[layer_depth]], axis=4)
        current_layer = create_convolution_block(n_filters=level_filters[layer_depth],
                                                 input_layer=concat,
                                                 batch_normalization=batch_normalization,
                                                 instance_normalization=instance_normalization,
                                                 activation=LeakyReLU,
                                                 name='Up_conv1_' + str(layer_depth))
        current_layer = create_convolution_block(n_filters=level_filters[layer_depth],
                                                 input_layer=current_layer,
                                                 batch_normalization=batch_normalization,
                                                 instance_normalization=instance_normalization,
                                                 activation=LeakyReLU,
                                                 name='Up_conv2_' + str(layer_depth))

    final_convolution = Conv3D(n_labels, (1, 1, 1), name='FinalConv1')(current_layer)
    print("###", final_convolution.shape)


    final_convolution1, final_convolution2, final_convolution3 = class_supervised_block(final_convolution)
    act1 = Activation(activation_name, name='act1')(final_convolution)
    w_act_x = Activation(activation_name, name='act_x')(final_convolution1)
    w_act_y = Activation(activation_name, name='act_y')(final_convolution2)
    w_act_z = Activation(activation_name, name='act_z')(final_convolution3)
    model = Model(inputs=inputs, outputs=[act1, w_act_x, w_act_y, w_act_z])
    if TrainOP == 'Adam':
        # loss = {'act1': dice_coefficient_loss,
        #         'act_x': 'binary_crossentropy',
        #         'act_y': "binary_crossentropy",
        #         'act_z': "binary_crossentropy"},
        model.compile(optimizer=Adam(lr=initial_learning_rate),
                      loss={'act1': dice_coefficient_loss,
                            'act_x': dice_coefficient_loss,
                            'act_y': dice_coefficient_loss,
                            'act_z': dice_coefficient_loss},
                      loss_weights={'act1': 1.,
                                    'act_x': 1.0,
                                    'act_y': 1.0,
                                    'act_z': 1.0},
                      metrics=[metrics])
        # model.compile(optimizer=Adam(lr=initial_learning_rate), loss=dice_coefficient_loss, metrics=metrics)
    else:
        model.compile(optimizer=SGD(lr=initial_learning_rate),
                      loss={'act1': dice_coefficient_loss,
                            'act_x': dice_coefficient_loss,
                            'act_y': dice_coefficient_loss,
                            'act_z': dice_coefficient_loss},
                      loss_weights={'act1': 1.,
                                    'act_x': 1.0,
                                    'act_y': 1.0,
                                    'act_z': 1.0},
                      metrics=[metrics])
        # model.compile(optimizer=SGD(lr=initial_learning_rate), loss=dice_coefficient_loss, metrics=metrics)
    return model
























