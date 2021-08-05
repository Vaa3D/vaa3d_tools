import numpy as np
import keras
from keras import backend as K
from keras.engine import Input, Model
from keras.layers import Conv3D, MaxPooling3D, AveragePooling3D, UpSampling3D, Add,Activation,BatchNormalization, \
    PReLU, Deconvolution3D, LeakyReLU, add, Dense, Flatten, GlobalAveragePooling3D, Reshape, Multiply,Reshape,Subtract
from keras.optimizers import Adam,SGD
from keras.layers.core import Lambda

from model.base_block import ASPP_Block,SpatialChannelAttentionBlcok
from model.class_block import class_supervised_block
from model.base_block import create_convolution_block,get_up_convolution,ChannelSep,Conv_shortcut_Block,\
    CurrentConvBlock,deeplab_atrous_Block
from metrics import dice_coefficient_loss, get_label_dice_coefficient_function, dice_coefficient,\
    focal_loss,dice_coefficient1,dice_coefficient_loss1

K.set_image_data_format("channels_last")
keras.initializers.TruncatedNormal(mean=0.0, stddev=0.05, seed=None)

try:
    from keras.engine import merge
except ImportError:
    from keras.layers.merge import concatenate


#   --------------------   current unet   --------------------   #

def current_unet_model(input_shape, pool_size=(2, 2, 2), n_labels=9, initial_learning_rate=0.00001, deconvolution=False,
                  TrainOP='Adam',
                  depth=4, batch_normalization=False, instance_normalization=True, activation_name="softmax",
                  include_label_wise_dice_coefficients=False, metrics=dice_coefficient):
    print('the shape of the input : ', input_shape)
    level_filters = [30, 60, 120, 240, 320, 480]
    inputs = Input(input_shape)
    current_layer = inputs
    levels = list()
    print("###", current_layer._keras_shape)
    current_layer = CurrentConvBlock(input_layer=current_layer,
                              n_filters=level_filters[0],
                              batch_normalization=batch_normalization,
                              instance_normalization=instance_normalization,
                              activation=LeakyReLU,
                              name='deconv_' + str(0))
    levels.append(current_layer)
    current_layer = MaxPooling3D(pool_size=pool_size, name='maxpool' + str(0))(current_layer)

    current_layer = CurrentConvBlock(input_layer=current_layer,
                              n_filters=level_filters[1],
                              batch_normalization=batch_normalization,
                              instance_normalization=instance_normalization,
                              activation=LeakyReLU,
                              name='deconv_' + str(1))
    levels.append(current_layer)
    current_layer = MaxPooling3D(pool_size=pool_size, name='maxpool' + str(1))(current_layer)

    current_layer = CurrentConvBlock(input_layer=current_layer,
                              n_filters=level_filters[2],
                              batch_normalization=batch_normalization,
                              instance_normalization=instance_normalization,
                              activation=LeakyReLU,
                              name='deconv_' + str(2))
    levels.append(current_layer)
    current_layer = MaxPooling3D(pool_size=pool_size, name='maxpool' + str(2))(current_layer)

    current_layer = CurrentConvBlock(input_layer=current_layer,
                              n_filters=level_filters[3],
                              batch_normalization=batch_normalization,
                              instance_normalization=instance_normalization,
                              activation=LeakyReLU,
                              name='deconv_' + str(3))

    current_layer = get_up_convolution(pool_size=pool_size,
                                        deconvolution=deconvolution,
                                        n_filters=level_filters[2],
                                        name='Upsample' + str(2))(current_layer)
    current_layer = concatenate([current_layer, levels[2]], axis=4)
    current_layer = CurrentConvBlock(input_layer=current_layer,
                                     n_filters=level_filters[2],
                                     batch_normalization=batch_normalization,
                                     instance_normalization=instance_normalization,
                                     activation=LeakyReLU,
                                     name='upconv_' + str(2))

    current_layer = get_up_convolution(pool_size=pool_size,
                                        deconvolution=deconvolution,
                                        n_filters=level_filters[1],
                                        name='Upsample' + str(1))(current_layer)
    current_layer = concatenate([current_layer, levels[1]], axis=4)
    current_layer = CurrentConvBlock(input_layer=current_layer,
                                     n_filters=level_filters[1],
                                     batch_normalization=batch_normalization,
                                     instance_normalization=instance_normalization,
                                     activation=LeakyReLU,
                                     name='upconv_' + str(1))

    current_layer = get_up_convolution(pool_size=pool_size,
                                        deconvolution=deconvolution,
                                        n_filters=level_filters[0],
                                        name='Upsample' + str(0))(current_layer)
    current_layer = concatenate([current_layer, levels[0]], axis=4)
    current_layer = CurrentConvBlock(input_layer=current_layer,
                                     n_filters=level_filters[0],
                                     batch_normalization=batch_normalization,
                                     instance_normalization=instance_normalization,
                                     activation=LeakyReLU,
                                     name='upconv_' + str(0))

    final_convolution = Conv3D(n_labels, (1, 1, 1), name='FinalConv1')(current_layer)

    print("###", final_convolution._keras_shape)
    act1 = Activation(activation_name, name='act1')(final_convolution)
    model = Model(inputs=inputs, outputs=act1)

    if TrainOP == 'Adam':
        model.compile(optimizer=Adam(lr=initial_learning_rate), loss=dice_coefficient_loss, metrics=[metrics])
    else:
        model.compile(optimizer=SGD(lr=initial_learning_rate), loss=dice_coefficient_loss, metrics=[metrics])
    return model


def FRnet(input_shape,input_shape1, pool_size=(2, 2, 2), n_labels=9, initial_learning_rate=0.00001, deconvolution=False,
          TrainOP='Adam',depth=4, batch_normalization=False, instance_normalization=True, activation_name="softmax",
          include_label_wise_dice_coefficients=False, metrics=dice_coefficient):
    print('the shape of the input : ', input_shape)
    level_filters = [32,64,128,256,512]
    inputs = Input(input_shape)
    inputs1 = Input(input_shape1)
    current_layer1 = inputs1
    current_layer = inputs
    levels = list()
    print("model2  : ", current_layer1._keras_shape)

    # add levels with max pooling
    for layer_depth in range(depth):  # 01234
        if layer_depth > -1:
            layer2 = create_convolution_block(input_layer=current_layer,
                                              n_filters=level_filters[layer_depth],
                                              batch_normalization=batch_normalization,
                                              instance_normalization=instance_normalization,
                                              activation=LeakyReLU,
                                              name = 'conv1_' + str(layer_depth))
            layer2 = create_convolution_block(input_layer=layer2,
                                              n_filters=level_filters[layer_depth],
                                              batch_normalization=batch_normalization,
                                              instance_normalization=instance_normalization,
                                              activation=LeakyReLU,
                                              name = 'conv2_' + str(layer_depth))
            print("###", layer2._keras_shape)

            levels.append(layer2)
            if layer_depth < depth - 1:
                current_layer = MaxPooling3D(pool_size=pool_size,name = 'maxpool' + str(layer_depth))(layer2)
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
                                            name = 'Upsample' + str(layer_depth))(current_layer)
        print("###",up_convolution._keras_shape)
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
                                                 name = 'Up_conv1_' + str(layer_depth))
        current_layer = create_convolution_block(n_filters=level_filters[layer_depth],
                                                 input_layer=current_layer,
                                                 batch_normalization=batch_normalization,
                                                 instance_normalization=instance_normalization,
                                                 activation=LeakyReLU,
                                                 name = 'Up_conv2_' + str(layer_depth))

    if include_label_wise_dice_coefficients and n_labels > 1:
        label_wise_dice_metrics = [get_label_dice_coefficient_function(index) for index in range(n_labels)]
        if metrics:
            metrics = metrics + label_wise_dice_metrics
        else:
            metrics = label_wise_dice_metrics

    if not isinstance(metrics, list):
        metrics = [metrics]


    # current_layer1 = ChannelSep(current_layer1,
    #                             batch_normalization=batch_normalization,
    #                             instance_normalization=instance_normalization,
    #                             name='CS1', nlayers=4,
    #                             nfilters=8)
    # print("###", current_layer1._keras_shape)
    # current_layer1 = ChannelSep(current_layer1,
    #                             batch_normalization=batch_normalization,
    #                             instance_normalization=instance_normalization,
    #                             name='CS3', nlayers=4,
    #                             nfilters=8)
    # print("###", current_layer1._keras_shape)
    #
    current_layer1 = create_convolution_block(input_layer=current_layer1,
                                      n_filters=level_filters[0],
                                      batch_normalization=batch_normalization,
                                      instance_normalization=instance_normalization,
                                      activation=LeakyReLU,
                                      name='convA1')
    print("###", current_layer1._keras_shape)
    current_layer1 = create_convolution_block(input_layer=current_layer1,
                                      n_filters=level_filters[0],
                                      batch_normalization=batch_normalization,
                                      instance_normalization=instance_normalization,
                                      activation=LeakyReLU,
                                      name='convA2')
    print("###", current_layer1._keras_shape)

    current_layer1 = concatenate([current_layer, current_layer1], axis=4)
    # current_layer1 = SpatialChannelAttentionBlcok(current_layer1, level_filters[0], name='SC2', layernumbers=3)
    # current_layer1 = ASPP_Block(current_layer1,name = 'ASPP1',n_filters = 30)

    final_convolution1 = Conv3D(n_labels, (1, 1, 1), name='FinalConv1')(current_layer)
    final_convolution2 = Conv3D(n_labels, (1, 1, 1), name='FinalConv3')(current_layer1)
    print("###", final_convolution2._keras_shape)
    act1 = Activation(activation_name, name='act1')(final_convolution1)
    act2 = Activation(activation_name, name='act2')(final_convolution2)
    model = Model(inputs=[inputs,inputs1], outputs=[act1,act2])

    if TrainOP == 'Adam':
        model.compile(optimizer=Adam(lr=initial_learning_rate),
                      loss={'act1': dice_coefficient_loss,
                            'act2': dice_coefficient_loss},
                      loss_weights={'act1': 1.,
                                    'act2': 1.0},
                      metrics=metrics)
    else:
        model.compile(optimizer=SGD(lr=initial_learning_rate),
                      loss={'act1': dice_coefficient_loss,
                            'act2': dice_coefficient_loss},
                      loss_weights={'act1': 1.,
                                    'act2': 1.0},
                      metrics=metrics)
    return model


def FRnet_C(input_shape,input_shape1, pool_size=(2, 2, 2), n_labels=9, initial_learning_rate=0.00001, deconvolution=False,
          TrainOP='Adam',depth=4, batch_normalization=False, instance_normalization=True, activation_name="softmax",
          include_label_wise_dice_coefficients=False, metrics=dice_coefficient):
    print('the shape of the input : ', input_shape)
    level_filters = [32,64,128,256,512]
    inputs = Input(input_shape)
    inputs1 = Input(input_shape1)
    current_layer1 = inputs1
    current_layer = inputs
    levels = list()
    print("model2  : ", current_layer1._keras_shape)

    # add levels with max pooling
    for layer_depth in range(depth):  # 01234
        if layer_depth > -1:
            layer2 = create_convolution_block(input_layer=current_layer,
                                              n_filters=level_filters[layer_depth],
                                              batch_normalization=batch_normalization,
                                              instance_normalization=instance_normalization,
                                              activation=LeakyReLU,
                                              name = 'conv1_' + str(layer_depth))
            layer2 = create_convolution_block(input_layer=layer2,
                                              n_filters=level_filters[layer_depth],
                                              batch_normalization=batch_normalization,
                                              instance_normalization=instance_normalization,
                                              activation=LeakyReLU,
                                              name = 'conv2_' + str(layer_depth))
            print("###", layer2._keras_shape)

            levels.append(layer2)
            if layer_depth < depth - 1:
                current_layer = MaxPooling3D(pool_size=pool_size,name = 'maxpool' + str(layer_depth))(layer2)
            else:
                current_layer = layer2

    # add levels with up-convolution or up-sampling
    for layer_depth in range(depth - 2, -1, -1):
        up_convolution = get_up_convolution(pool_size=pool_size,
                                            deconvolution=deconvolution,
                                            n_filters=level_filters[layer_depth],
                                            name = 'Upsample' + str(layer_depth))(current_layer)
        print("###",up_convolution._keras_shape)
        concat = concatenate([up_convolution, levels[layer_depth]], axis=4)
        current_layer = create_convolution_block(n_filters=level_filters[layer_depth],
                                                 input_layer=concat,
                                                 batch_normalization=batch_normalization,
                                                 instance_normalization=instance_normalization,
                                                 activation=LeakyReLU,
                                                 name = 'Up_conv1_' + str(layer_depth))
        current_layer = create_convolution_block(n_filters=level_filters[layer_depth],
                                                 input_layer=current_layer,
                                                 batch_normalization=batch_normalization,
                                                 instance_normalization=instance_normalization,
                                                 activation=LeakyReLU,
                                                 name = 'Up_conv2_' + str(layer_depth))

    if include_label_wise_dice_coefficients and n_labels > 1:
        label_wise_dice_metrics = [get_label_dice_coefficient_function(index) for index in range(n_labels)]
        if metrics:
            metrics = metrics + label_wise_dice_metrics
        else:
            metrics = label_wise_dice_metrics

    if not isinstance(metrics, list):
        metrics = [metrics]


    current_layer1 = create_convolution_block(input_layer=current_layer1,
                                      n_filters=level_filters[0],
                                      batch_normalization=batch_normalization,
                                      instance_normalization=instance_normalization,
                                      activation=LeakyReLU,
                                      name='convA1')
    print("###", current_layer1._keras_shape)
    current_layer1 = create_convolution_block(input_layer=current_layer1,
                                      n_filters=level_filters[0],
                                      batch_normalization=batch_normalization,
                                      instance_normalization=instance_normalization,
                                      activation=LeakyReLU,
                                      name='convA2')
    print("###", current_layer1._keras_shape)

    current_layer1 = concatenate([current_layer, current_layer1], axis=4)
    # current_layer1 = SpatialChannelAttentionBlcok(current_layer1, level_filters[0], name='SC2', layernumbers=3)
    # current_layer1 = ASPP_Block(current_layer1,name = 'ASPP1',n_filters = 30)

    final_convolution1 = Conv3D(n_labels, (1, 1, 1), name='FinalConv1')(current_layer)
    final_convolution2 = Conv3D(n_labels, (1, 1, 1), name='FinalConv3')(current_layer1)

    # final_convolution3, final_convolution4, final_convolution5 = class_supervised_block(final_convolution1,
    #                                                                                     nbasefilters=n_labels,name='CS')
    final_convolution6, final_convolution7, final_convolution8 = class_supervised_block(final_convolution2,
                                                                                        nbasefilters=n_labels, name='CS1')

    print("###", final_convolution2._keras_shape)
    act1 = Activation(activation_name, name='act1')(final_convolution1)
    act2 = Activation(activation_name, name='act2')(final_convolution2)
    # w_act_x1 = Activation(activation_name, name='act_x1')(final_convolution3)
    # w_act_y1 = Activation(activation_name, name='act_y1')(final_convolution4)
    # w_act_z1 = Activation(activation_name, name='act_z1')(final_convolution5)
    w_act_x2 = Activation(activation_name, name='act_x2')(final_convolution6)
    w_act_y2 = Activation(activation_name, name='act_y2')(final_convolution7)
    w_act_z2 = Activation(activation_name, name='act_z2')(final_convolution8)

    model = Model(inputs=[inputs,inputs1], outputs=[act1,act2, w_act_x2, w_act_y2, w_act_z2])
    # model = Model(inputs=[inputs, inputs1],
    #               outputs=[act1, act2, w_act_x1, w_act_y1, w_act_z1, w_act_x2, w_act_y2, w_act_z2])
    beishu = 0.5

    if TrainOP == 'Adam':
        model.compile(optimizer=Adam(lr=initial_learning_rate),
                      loss={'act1': dice_coefficient_loss,
                            'act2': dice_coefficient_loss,
                            'act_x2': dice_coefficient_loss1,
                            'act_y2': dice_coefficient_loss1,
                            'act_z2': dice_coefficient_loss1},
                      loss_weights={'act1': 1.,
                                    'act2': 1.,
                                    'act_x2': 1.0*beishu,
                                    'act_y2': 1.0*beishu,
                                    'act_z2': 1.0*beishu},
                      metrics=metrics)
    else:
        model.compile(optimizer=SGD(lr=initial_learning_rate),
                      loss={'act1': dice_coefficient_loss,
                            'act2': dice_coefficient_loss,
                            'act_x2': dice_coefficient_loss1,
                            'act_y2': dice_coefficient_loss1,
                            'act_z2': dice_coefficient_loss1},
                      loss_weights={'act1': 1.,
                                    'act2': 1.,
                                    'act_x2': 1.0*beishu,
                                    'act_y2': 1.0*beishu,
                                    'act_z2': 1.0*beishu},
                      metrics=metrics)

        # if TrainOP == 'Adam':
        #     model.compile(optimizer=Adam(lr=initial_learning_rate),
        #                   loss={'act1': dice_coefficient_loss,
        #                         'act2': dice_coefficient_loss,
        #                         'act_x1': dice_coefficient_loss1,
        #                         'act_y1': dice_coefficient_loss1,
        #                         'act_z1': dice_coefficient_loss1,
        #                         'act_x2': dice_coefficient_loss1,
        #                         'act_y2': dice_coefficient_loss1,
        #                         'act_z2': dice_coefficient_loss1},
        #                   loss_weights={'act1': 1.,
        #                                 'act2': 1.,
        #                                 'act_x1': 1.0 * beishu,
        #                                 'act_y1': 1.0 * beishu,
        #                                 'act_z1': 1.0 * beishu,
        #                                 'act_x2': 1.0 * beishu,
        #                                 'act_y2': 1.0 * beishu,
        #                                 'act_z2': 1.0 * beishu},
        #                   metrics=metrics)
        # else:
        #     model.compile(optimizer=SGD(lr=initial_learning_rate),
        #                   loss={'act1': dice_coefficient_loss,
        #                         'act2': dice_coefficient_loss,
        #                         'act_x1': dice_coefficient_loss1,
        #                         'act_y1': dice_coefficient_loss1,
        #                         'act_z1': dice_coefficient_loss1,
        #                         'act_x2': dice_coefficient_loss1,
        #                         'act_y2': dice_coefficient_loss1,
        #                         'act_z2': dice_coefficient_loss1},
        #                   loss_weights={'act1': 1.,
        #                                 'act2': 1.,
        #                                 'act_x1': 1.0 * beishu,
        #                                 'act_y1': 1.0 * beishu,
        #                                 'act_z1': 1.0 * beishu,
        #                                 'act_x2': 1.0 * beishu,
        #                                 'act_y2': 1.0 * beishu,
        #                                 'act_z2': 1.0 * beishu},
        #                   metrics=metrics)

    return model


#   --------------------   unet   --------------------   #
def unet_model_3d(input_shape, pool_size=(2, 2, 2), n_labels=9, initial_learning_rate=0.00001, deconvolution=False,TrainOP = 'Adam',
                  depth=4,batch_normalization=False, instance_normalization=True, activation_name="softmax",
                  include_label_wise_dice_coefficients=False, metrics=dice_coefficient):
    print('the shape of the input : ', input_shape)
    level_filters = [32,64,128,256,512]

    # create and compile model
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
                                              name = 'conv1_' + str(layer_depth))
            layer2 = create_convolution_block(input_layer=layer2,
                                              n_filters=level_filters[layer_depth],
                                              batch_normalization=batch_normalization,
                                              instance_normalization=instance_normalization,
                                              activation=LeakyReLU,
                                              name = 'conv2_' + str(layer_depth))
            print("###", layer2._keras_shape)

            levels.append(layer2)
            if layer_depth < depth - 1:
                current_layer = MaxPooling3D(pool_size=pool_size,name = 'maxpool' + str(layer_depth))(layer2)
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
                                            name = 'Upsample' + str(layer_depth))(current_layer)
        print("###",up_convolution._keras_shape)
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
                                                 name = 'Up_conv1_' + str(layer_depth))
        current_layer = create_convolution_block(n_filters=level_filters[layer_depth],
                                                 input_layer=current_layer,
                                                 batch_normalization=batch_normalization,
                                                 instance_normalization=instance_normalization,
                                                 activation=LeakyReLU,
                                                 name = 'Up_conv2_' + str(layer_depth))
        
    if include_label_wise_dice_coefficients and n_labels > 1:
        label_wise_dice_metrics = [get_label_dice_coefficient_function(index) for index in range(n_labels)]
        if metrics:
            metrics = metrics + label_wise_dice_metrics
        else:
            metrics = label_wise_dice_metrics
        
    if not isinstance(metrics, list):
            metrics = [metrics]

    final_convolution = Conv3D(n_labels, (1, 1, 1),name = 'FinalConv1')(current_layer)

    print("###", final_convolution._keras_shape)
    act1 = Activation(activation_name,name = 'act1')(final_convolution)
    model = Model(inputs=inputs, outputs=act1)

    if TrainOP == 'Adam':
        model.compile(optimizer=Adam(lr=initial_learning_rate), loss=dice_coefficient_loss, metrics=metrics)
    else:
        model.compile(optimizer=SGD(lr=initial_learning_rate), loss=dice_coefficient_loss, metrics=metrics)

    #print(model.summary())
    return model






