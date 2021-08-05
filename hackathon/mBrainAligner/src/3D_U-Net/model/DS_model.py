import numpy as np
import keras
from keras import backend as K
from keras.engine import Input, Model
from keras.layers import Conv3D, MaxPooling3D, AveragePooling3D, UpSampling3D, Activation, \
    BatchNormalization, PReLU, Deconvolution3D, LeakyReLU, add, Dense, Flatten, GlobalAveragePooling3D, Reshape, Multiply
from keras.optimizers import Adam,SGD
from keras.layers.core import Lambda

from metrics import dice_coefficient_loss, dice_coefficient,dice_coefficient1,\
    Mean_weighted_dice_coef, Mean_weighted_dice_loss,dice_coefficient_loss1

from model.class_block import Refining_Block,DS_Block,NNet_Block,class_supervised_block,class_seg_block
from model.base_block import create_convolution_block,\
    get_up_convolution,ChannelSep
K.set_image_data_format("channels_last")
keras.initializers.TruncatedNormal(mean=0.0, stddev=0.05, seed=None)


try:
    from keras.engine import merge
except ImportError:
    from keras.layers.merge import concatenate





#   --------------------   DS   --------------------   #

def DS_model(input_shape, pool_size=(2, 2, 2), n_labels=1, initial_learning_rate=0.00001, deconvolution=False,
                  depth=4,batch_normalization=False, instance_normalization=True, activation_name="softmax",WeightLoss = 0):
    print('the shape of the input : ', input_shape)
    level_filters = [32,64,128,256,512]
    inputs = Input(input_shape)
    current_layer = inputs
    levels = list()
    up_layers = list()
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
            print("#", layer2._keras_shape)

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
        print("#", up_convolution._keras_shape)
        print("#", levels[layer_depth]._keras_shape)
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
        up_layers.append(current_layer)


    layer_up0,layer_up1 = DS_Block(up_layers, BaseFilters=n_labels,n_labels = n_labels,name='DS')  #     DS
    final_convolution = Conv3D(n_labels, (1, 1, 1),name = 'final')(current_layer)
    final_convolution1 = Conv3D(n_labels, (1, 1, 1),name = 'final1')(layer_up0)
    final_convolution2 = Conv3D(n_labels, (1, 1, 1),name = 'final2')(layer_up1)
    # final_convolution3 = Conv3D(n_labels, (1, 1, 1),name = 'final3')(layer_up2)
    # print("#", layer_up0._keras_shape)
    print(final_convolution1.shape,final_convolution2.shape)
    act = Activation(activation_name, name='cat')(final_convolution)
    act1 = Activation(activation_name, name='cat1')(final_convolution1)
    act2 = Activation(activation_name, name='cat2')(final_convolution2)
    # act3 = Activation(activation_name, name='cat3')(final_convolution3)

    model = Model(inputs=inputs, outputs=[act,act1,act2])

    metrics = dice_coefficient

    Loss = dice_coefficient_loss

    model.compile(optimizer=Adam(lr=initial_learning_rate),
                  loss=Loss,
                  loss_weights = {'cat':1.0,
                                  'cat1' : 0.6,
                                  'cat2' : 0.8},
                  metrics=[metrics])
    return model












#   --------------------   DS + C   --------------------   #

def DS_C_model(input_shape, pool_size=(2, 2, 2), n_labels=1, initial_learning_rate=0.00001, deconvolution=False,TrainOP = 'Adam',
                  depth=4, metrics=dice_coefficient,batch_normalization=False, instance_normalization=True, activation_name="softmax"):
    print('the shape of the input : ', input_shape)
    level_filters =[32,64,128,256,512]
    inputs = Input(input_shape)
    current_layer = inputs
    levels = list()
    up_layers = list()
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
            print("#", layer2._keras_shape)

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
        print("###", levels[layer_depth]._keras_shape)
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
        up_layers.append(current_layer)



    layer_up0,layer_up1 = DS_Block(up_layers, BaseFilters=n_labels,n_labels = n_labels,name='DS')  #     DS
    final_convolution = Conv3D(n_labels, (1, 1, 1),name = 'final')(current_layer)
    final_convolution1 = Conv3D(n_labels, (1, 1, 1),name = 'final1')(layer_up0)
    final_convolution2 = Conv3D(n_labels, (1, 1, 1),name = 'final2')(layer_up1)
    # final_convolution3 = Conv3D(n_labels, (1, 1, 1),name = 'final3')(layer_up2)
    print('final conv :',final_convolution1.shape, final_convolution2.shape)


    #   ----------   classification   ----------   #
    layer7_1, layer7_2, layer7_3 = class_supervised_block(final_convolution,nbasefilters=n_labels,name = 'class01')
    # layer8_1, layer8_2, layer8_3 = class_supervised_block(final_convolution1,nbasefilters=n_labels,name = 'class11')
    # layer9_1, layer9_2, layer9_3 = class_supervised_block(final_convolution2,nbasefilters=n_labels,name = 'class21')
    # layer10_1, layer10_2, layer10_3 = class_supervised_block(final_convolution3,nbasefilters=n_labels,name = 'class31')



    act = Activation(activation_name,name = 'cat')(final_convolution)
    act1 = Activation(activation_name,name = 'cat1')(layer7_1)
    act2 = Activation(activation_name,name = 'cat2')(layer7_2)
    act3 = Activation(activation_name,name = 'cat3')(layer7_3)
    # act4 = Activation(activation_name,name = 'cat4')(layer8_1)
    # act5 = Activation(activation_name,name = 'cat5')(layer8_2)
    # act6 = Activation(activation_name,name = 'cat6')(layer8_3)
    # act7 = Activation(activation_name,name = 'cat7')(layer9_1)
    # act8 = Activation(activation_name,name = 'cat8')(layer9_2)
    # act9 = Activation(activation_name,name = 'cat9')(layer9_3)
    # act10 = Activation(activation_name,name = 'cat10')(layer10_1)
    # act11 = Activation(activation_name,name = 'cat11')(layer10_2)
    # act12 = Activation(activation_name,name = 'cat12')(layer10_3)
    act13 = Activation(activation_name, name='cat13')(final_convolution1)
    act14 = Activation(activation_name, name='cat14')(final_convolution2)
    # act15 = Activation(activation_name, name='cat15')(final_convolution3)


    model = Model(inputs=inputs, outputs=[act,act13,act14,
                                          act1,act2,act3])
    # model = Model(inputs=inputs, outputs=[act,act13,act14,act15,
    #                                       act1,act2,act3,act4,act5,act6,act7,act8,act9,act10,act11,act12])
    beishu = 0.5
    if TrainOP == 'Adam':

        model.compile(optimizer=Adam(lr=initial_learning_rate),
                      loss={'cat': dice_coefficient_loss,
                                    'cat1': dice_coefficient_loss,
                                    'cat2': dice_coefficient_loss,
                                    'cat3': dice_coefficient_loss,
                                    'cat13': dice_coefficient_loss,
                                    'cat14': dice_coefficient_loss},
                      loss_weights={'cat': 1.,
                                    'cat1': 1.0*beishu,
                                    'cat2': 1.0*beishu,
                                    'cat3': 1.0*beishu,
                                    'cat13': 0.6,
                                    'cat14': 0.8},
                      metrics=[metrics])
    else:
        model.compile(optimizer=SGD(lr=initial_learning_rate),
                      loss={'cat': dice_coefficient_loss,
                                    'cat1': dice_coefficient_loss,
                                    'cat2': dice_coefficient_loss,
                                    'cat3': dice_coefficient_loss,
                                    'cat13': dice_coefficient_loss,
                                    'cat14': dice_coefficient_loss},
                      loss_weights={'cat': 1.,
                                    'cat1': 1.0*beishu,
                                    'cat2': 1.0*beishu,
                                    'cat3': 1.0*beishu,
                                    'cat13': 0.6,
                                    'cat14': 0.8},
                      metrics=[metrics])

        # if TrainOP == 'Adam':
        #
        #     model.compile(optimizer=Adam(lr=initial_learning_rate),
        #                   loss={'cat': dice_coefficient_loss,
        #                         'cat1': dice_coefficient_loss,
        #                         'cat2': dice_coefficient_loss,
        #                         'cat3': dice_coefficient_loss,
        #                         'cat4': dice_coefficient_loss,
        #                         'cat5': dice_coefficient_loss,
        #                         'cat6': dice_coefficient_loss,
        #                         'cat7': dice_coefficient_loss,
        #                         'cat8': dice_coefficient_loss,
        #                         'cat9': dice_coefficient_loss,
        #                         'cat10': dice_coefficient_loss,
        #                         'cat11': dice_coefficient_loss,
        #                         'cat12': dice_coefficient_loss,
        #                         'cat13': dice_coefficient_loss,
        #                         'cat14': dice_coefficient_loss,
        #                         'cat15': dice_coefficient_loss},
        #                   loss_weights={'cat': 1.,
        #                                 'cat1': 1.0 * beishu,
        #                                 'cat2': 1.0 * beishu,
        #                                 'cat3': 1.0 * beishu,
        #                                 'cat4': 0.4 * beishu,
        #                                 'cat5': 0.4 * beishu,
        #                                 'cat6': 0.4 * beishu,
        #                                 'cat7': 0.6 * beishu,
        #                                 'cat8': 0.6 * beishu,
        #                                 'cat9': 0.6 * beishu,
        #                                 'cat10': 0.8 * beishu,
        #                                 'cat11': 0.8 * beishu,
        #                                 'cat12': 0.8 * beishu,
        #                                 'cat13': 0.4,
        #                                 'cat14': 0.6,
        #                                 'cat15': 0.8},
        #                   metrics=[metrics])
        # else:
        #     model.compile(optimizer=SGD(lr=initial_learning_rate),
        #                   loss={'cat': dice_coefficient_loss,
        #                         'cat1': dice_coefficient_loss,
        #                         'cat2': dice_coefficient_loss,
        #                         'cat3': dice_coefficient_loss,
        #                         'cat4': dice_coefficient_loss,
        #                         'cat5': dice_coefficient_loss,
        #                         'cat6': dice_coefficient_loss,
        #                         'cat7': dice_coefficient_loss,
        #                         'cat8': dice_coefficient_loss,
        #                         'cat9': dice_coefficient_loss,
        #                         'cat10': dice_coefficient_loss,
        #                         'cat11': dice_coefficient_loss,
        #                         'cat12': dice_coefficient_loss,
        #                         'cat13': dice_coefficient_loss,
        #                         'cat14': dice_coefficient_loss,
        #                         'cat15': dice_coefficient_loss},
        #                   loss_weights={'cat': 1.,
        #                                 'cat1': 1.0 * beishu,
        #                                 'cat2': 1.0 * beishu,
        #                                 'cat3': 1.0 * beishu,
        #                                 'cat4': 0.4 * beishu,
        #                                 'cat5': 0.4 * beishu,
        #                                 'cat6': 0.4 * beishu,
        #                                 'cat7': 0.6 * beishu,
        #                                 'cat8': 0.6 * beishu,
        #                                 'cat9': 0.6 * beishu,
        #                                 'cat10': 0.8 * beishu,
        #                                 'cat11': 0.8 * beishu,
        #                                 'cat12': 0.8 * beishu,
        #                                 'cat13': 0.4,
        #                                 'cat14': 0.6,
        #                                 'cat15': 0.8},
        #                   metrics=[metrics])

    return model











#   --------------------   改进DS   --------------------   #
def UpdateDS_model_block( pool_size=(2, 2, 2), n_labels=1,deconvolution=False,
                  depth=5,batch_normalization=False, instance_normalization=True):

    inputs0 = Input(shape = (64, 64, 64, 30),name = 'twoinput')
    print('the shape of the input : ', inputs0._keras_shape)
    level_filters = [32,64,128,256,512]
    levels = list()
    up_layers = list()
    # add levels with max pooling
    current_layer = inputs0
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
            print("#", layer2._keras_shape)

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
        print("#", up_convolution._keras_shape)
        print("#", levels[layer_depth]._keras_shape)
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
        up_layers.append(current_layer)

    layer_up0, layer_up1, layer_up2 = DS_Block(up_layers, BaseFilters=level_filters[0], n_labels=n_labels,
                                               name='DS')  # DS
    feature = Conv3D(n_labels, (1, 1, 1), name='final')(current_layer)
    feature1 = Conv3D(n_labels, (1, 1, 1), name='final1')(layer_up0)
    feature2 = Conv3D(n_labels, (1, 1, 1), name='final2')(layer_up1)
    feature3 = Conv3D(n_labels, (1, 1, 1), name='final3')(layer_up2)
    model = Model(inputs=inputs0, outputs=[feature,feature1,feature2,feature3])
    return model



def UpdateDS_model(input_shape,input_shape1, pool_size=(2, 2, 2), n_labels=1, initial_learning_rate=0.00001, deconvolution=False,
                  depth=5,batch_normalization=False, instance_normalization=True, activation_name="softmax",WeightLoss = 0):
    level_filters = [32,64,128,256,512]
    inputs = Input(input_shape)
    inputs1 = Input(input_shape1)
    print(' input1 : ',inputs._keras_shape)
    print(' input2 : ', inputs1._keras_shape)
    inputs_0 = create_convolution_block(input_layer=inputs,
                                      n_filters=level_filters[0],
                                      batch_normalization=batch_normalization,
                                      instance_normalization=instance_normalization,
                                      activation=LeakyReLU,
                                      name='newconv1_' + str(0))
    inputs1_1 = create_convolution_block(input_layer=inputs1,
                                      n_filters=level_filters[0],
                                      batch_normalization=batch_normalization,
                                      instance_normalization=instance_normalization,
                                      activation=LeakyReLU,
                                      name='newconv2_' + str(0))
    print(' conv1 of input1 : ',inputs_0._keras_shape)
    print(' conv2 of input2 : ',inputs1_1._keras_shape)
    model = UpdateDS_model_block(pool_size=pool_size, n_labels=n_labels,deconvolution=deconvolution,
                  depth=depth,batch_normalization=batch_normalization, instance_normalization=instance_normalization)
    final_convolution,final_convolution1,\
    final_convolution2,final_convolution3 = model(inputs_0)
    final_convolution4,final_convolution5,\
    final_convolution6,final_convolution7 = model(inputs1_1)
    print(final_convolution.shape,final_convolution1.shape,final_convolution2.shape,final_convolution3.shape)
    print(final_convolution4.shape, final_convolution5.shape, final_convolution6.shape, final_convolution7.shape)

    act = Activation(activation_name, name='cat')(final_convolution)
    act1 = Activation(activation_name, name='cat1')(final_convolution1)
    act2 = Activation(activation_name, name='cat2')(final_convolution2)
    act3 = Activation(activation_name, name='cat3')(final_convolution3)
    act4 = Activation(activation_name, name='cat4')(final_convolution4)
    act5 = Activation(activation_name, name='cat5')(final_convolution5)
    act6 = Activation(activation_name, name='cat6')(final_convolution6)
    act7 = Activation(activation_name, name='cat7')(final_convolution7)

    concat = concatenate([act,act4], axis=4,name = 'concate')
    concat1 = concatenate([act1, act5], axis=4, name='concate1')
    concat2 = concatenate([act2, act6], axis=4, name='concate2')
    concat3 = concatenate([act3, act7], axis=4, name='concate3')

    model = Model(inputs=[inputs,inputs1], outputs=[act,act1,act2,act3,concat,concat1,concat2,concat3])

    model.compile(optimizer=Adam(lr=initial_learning_rate),
                  loss={'cat':dice_coefficient_loss,
                        'cat1' : dice_coefficient_loss,
                        'cat2' : dice_coefficient_loss,
                        'cat3' : dice_coefficient_loss,
                        'concate': dice_coefficient_loss1,
                        'concate1': dice_coefficient_loss1,
                        'concate2': dice_coefficient_loss1,
                        'concate3': dice_coefficient_loss1},
                  loss_weights = {'cat':1.0,
                                  'cat1' : 0.8,
                                  'cat2' : 0.8,
                                  'cat3' : 0.8,
                                  'concate': 1.0,
                                  'concate1': 0.8,
                                  'concate2': 0.8,
                                  'concate3': 0.8
                                  },
                  metrics={'cat':dice_coefficient,
                                  'cat1' : dice_coefficient,
                                  'cat2' : dice_coefficient,
                                  'cat3' : dice_coefficient,
                                  'concate': dice_coefficient1,
                                  'concate1': dice_coefficient1,
                                  'concate2': dice_coefficient1,
                                  'concate3': dice_coefficient1
                                  })
    return model












