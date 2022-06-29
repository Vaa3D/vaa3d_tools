import numpy as np
import keras
from keras import backend as K
from keras.engine import Input, Model
from keras.layers import Conv3D, MaxPooling3D, AveragePooling3D, UpSampling3D, Activation, \
    BatchNormalization, PReLU, Deconvolution3D, LeakyReLU, Add, Dense, Flatten, GlobalAveragePooling3D,GlobalMaxPooling3D, Reshape, Multiply
from keras.optimizers import Adam,SGD
from keras.layers.core import Lambda

from metrics import dice_coefficient_loss, dice_coefficient, Mean_weighted_dice_coef, Mean_weighted_dice_loss

try:
    from keras_contrib.layers.normalization.instancenormalization import InstanceNormalization
except ImportError:
    raise ImportError("Install keras_contrib in order to use instance normalization."
                      "\nTry: pip install git+https://www.github.com/farizrahman4u/keras-contrib.git")


K.set_image_data_format("channels_last")
keras.initializers.TruncatedNormal(mean=0.0, stddev=0.05, seed=None)


try:
    from keras.engine import merge
except ImportError:
    from keras.layers.merge import concatenate




def ChannelSep(InputLayer,batch_normalization=False,instance_normalization=True,name = 'ASPP',nlayers = 5,nfilters = 30):
    DilationRates = [3, 5, 7, 11, 13,17, 19,23,29,31,37,39,41,43,45]
    CurrentLayer = InputLayer
    AllLayers = list()
    for i in range(nlayers):
        TmpLayer = create_convolution_block(input_layer=CurrentLayer,
                                          n_filters=nfilters,
                                          batch_normalization=batch_normalization,
                                          instance_normalization=instance_normalization,
                                          activation=LeakyReLU,
                                          name= name+'_conv_' + str(i),
                                         dilation_rate=(DilationRates[i], DilationRates[i], DilationRates[i]))
        AllLayers.append(TmpLayer)
    if nlayers == 1:
        CurrentLayer = AllLayers[0]
    elif nlayers == 2:
        CurrentLayer = concatenate([AllLayers[0], AllLayers[1]],
                                   axis=4, name=name + 'concat')
    elif nlayers == 3:
        CurrentLayer = concatenate([AllLayers[0], AllLayers[1],AllLayers[2]],
                                   axis=4, name=name + 'concat')
    elif nlayers == 4:
        CurrentLayer = concatenate([AllLayers[0], AllLayers[1],AllLayers[2],AllLayers[3]],
                                   axis=4, name=name + 'concat')
    elif nlayers == 5:
        CurrentLayer = concatenate([AllLayers[0], AllLayers[1],AllLayers[2],AllLayers[3],
                                    AllLayers[4]],
                                   axis=4, name=name + 'concat')
    elif nlayers == 6:
        CurrentLayer = concatenate([AllLayers[0], AllLayers[1],AllLayers[2],AllLayers[3],
                                    AllLayers[4],AllLayers[5]],
                                   axis=4, name=name + 'concat')
    elif nlayers == 7:
        CurrentLayer = concatenate([AllLayers[0], AllLayers[1],AllLayers[2],AllLayers[3],
                                    AllLayers[4],AllLayers[5],AllLayers[6]],
                                   axis=4, name=name + 'concat')
    elif nlayers == 8:
        CurrentLayer = concatenate([AllLayers[0], AllLayers[1],AllLayers[2],AllLayers[3],
                                    AllLayers[4],AllLayers[5],AllLayers[6],AllLayers[7]],
                                   axis=4, name=name + 'concat')
    elif nlayers == 9:
        CurrentLayer = concatenate([AllLayers[0], AllLayers[1],AllLayers[2],AllLayers[3],
                                    AllLayers[4],AllLayers[5],AllLayers[6],AllLayers[7],
                                    AllLayers[8]],
                                   axis=4, name=name + 'concat')
    elif nlayers == 10:
        CurrentLayer = concatenate([AllLayers[0], AllLayers[1],AllLayers[2],AllLayers[3],
                                    AllLayers[4],AllLayers[5],AllLayers[6],AllLayers[7],
                                    AllLayers[8],AllLayers[9]],
                                   axis=4, name=name + 'concat')

    return CurrentLayer









#   ------  Deep Supervised Learning   -----   #
def DS_Block(up_levels,BaseFilters = 1,batch_normalization=False, instance_normalization=True,n_labels=1,name = 'DS'):
    # layer_up0 = create_convolution_block(n_filters=BaseFilters,
    #                                  input_layer=up_levels[0],
    #                                  kernel = (1, 1, 1),
    #                                  batch_normalization=batch_normalization,
    #                                  instance_normalization=instance_normalization,
    #                                  activation=LeakyReLU,
    #                                      name = name+'_conv1')
    # layer_up1 = create_convolution_block(n_filters=BaseFilters,
    #                                  input_layer=up_levels[1],
    #                                  kernel = (1, 1, 1),
    #                                  batch_normalization=batch_normalization,
    #                                  instance_normalization=instance_normalization,
    #                                  activation=LeakyReLU,
    #                                      name = name+'_conv2')
    # layer_up2 = create_convolution_block(n_filters=BaseFilters,
    #                                  input_layer=up_levels[2],
    #                                  kernel = (1, 1, 1),
    #                                  batch_normalization=batch_normalization,
    #                                  instance_normalization=instance_normalization,
    #                                  activation=LeakyReLU,
    #                                      name = name+'_conv3')

    # layer_up0 = get_up_convolution(pool_size=(8,8,8),
    #                                     deconvolution=False,
    #                                     n_filters=n_labels,name = name+'_up1')(up_levels[0])
    layer_up1 = get_up_convolution(pool_size=(4,4,4),
                                        deconvolution=False,
                                        n_filters=n_labels,name = name+'_up2')(up_levels[0])
    layer_up2 = get_up_convolution(pool_size=(2,2,2),
                                        deconvolution=False,
                                        n_filters=n_labels,name = name+'_up3')(up_levels[1])
    return layer_up1,layer_up2







#   ------  classification Supervised multiply1   -----   #
def class_seg_block1(w_act_x ,w_act_y ,w_act_z ,name = 'a'):
    out_act = Multiply()([w_act_x, w_act_y])
    out_act = Multiply(name = name)([out_act, w_act_z])
    return out_act






#   ------  classification Supervised multiply2   -----   #
def class_seg_block(act ,w_act_x ,w_act_y ,w_act_z ,name = 'a'):
    out_act =Multiply()([act, w_act_x])
    out_act = Multiply()([out_act, w_act_y])
    out_act = Multiply(name=name)([out_act, w_act_z])
    return out_act









#
# #   ------  classification Supervised1   -----   #
# def class_supervised_block(input_layer, batch_normalization=False, instance_normalization=True,
#                            LeakyReLU=LeakyReLU,nbasefilters = 1,LayerDepth = 3,name = 'CSblock'):
#     '''
#     it can be used in any segmentation model
#     '''
#     # input_layer = create_convolution_block(n_filters=nbasefilters,
#     #                                     input_layer=input_layer,
#     #                                     strides=(1, 1, 1), kernel=(1, 1, 1),
#     #                                     batch_normalization=batch_normalization,
#     #                                     instance_normalization=instance_normalization,
#     #                                     activation=LeakyReLU,
#     #                                     name = name+'_x0000')
#     layer6_1 = MaxPooling3D(pool_size=(2, 2, 1),strides=(2, 2, 1), name=name+'_x1_1')(input_layer)
#     layer6_1 = create_convolution_block(n_filters=nbasefilters,
#                                         input_layer=layer6_1,
#                                         batch_normalization=batch_normalization,
#                                         instance_normalization=instance_normalization,
#                                         activation=None,
#                                         name = name+'_x1_2')
#     layer6_2 = MaxPooling3D(pool_size=(2, 1, 2), strides=(2, 1, 2), name=name + '_y1_1')(input_layer)
#     layer6_2 = create_convolution_block(n_filters=nbasefilters,
#                                         input_layer=layer6_2,
#                                         batch_normalization=batch_normalization,
#                                         instance_normalization=instance_normalization,
#                                         activation=None,
#                                         name = name+'_y1_2')
#     layer6_3 = MaxPooling3D(pool_size=(1, 2, 2), strides=(1, 2, 2), name=name + '_z1_1')(input_layer)
#     layer6_3 = create_convolution_block(n_filters=nbasefilters,
#                                         input_layer=layer6_3,
#                                         batch_normalization=batch_normalization,
#                                         instance_normalization=instance_normalization,
#                                         activation=None,
#                                         name = name+'_z1_2')
#     print(layer6_1.shape, layer6_2.shape, layer6_3._keras_shape)
#
#     for i in range(LayerDepth):
#         layer6_1 = MaxPooling3D(pool_size=(2, 2, 1), strides=(2, 2, 1), name=name+'_x' + str(i+2) + '_1')(layer6_1)
#         layer6_1 = create_convolution_block(n_filters=nbasefilters,
#                                             input_layer=layer6_1,
#                                             batch_normalization=batch_normalization,
#                                             instance_normalization=instance_normalization,
#                                             activation=None,
#                                             name = name+'_x' + str(i+2) + '_2')
#         print('x shape : ', layer6_1._keras_shape)
#
#     for i in range(LayerDepth):
#         layer6_2 = MaxPooling3D(pool_size=(2, 1, 2), strides=(2, 1, 2), name=name+'_y' + str(i+2) + '_1')(layer6_2)
#         layer6_2 = create_convolution_block(n_filters=nbasefilters,
#                                             input_layer=layer6_2,
#                                             batch_normalization=batch_normalization,
#                                             instance_normalization=instance_normalization,
#                                             activation=None,
#                                             name = name+'_y' + str(i+2) + '_2')
#         print('y shape : ', layer6_2._keras_shape)
#
#     for i in range(LayerDepth):
#         layer6_3 = MaxPooling3D(pool_size=(1, 2, 2), strides=(1, 2, 2), name=name+'_z' + str(i+2) + '_1')(layer6_3)
#         layer6_3 = create_convolution_block(n_filters=nbasefilters,
#                                             input_layer=layer6_3,
#                                             batch_normalization=batch_normalization,
#                                             instance_normalization=instance_normalization,
#                                             activation=None,
#                                             name = name+'_z' + str(i+2) + '_2')
#         print('z shape : ', layer6_3.shape)
#
#     layer6_1 = AveragePooling3D(pool_size=(5, 9, 1), strides=(1,1,1), padding='valid',name = name+'_poolx')(layer6_1)
#     layer6_2 = AveragePooling3D(pool_size=(5, 1, 7), strides=(1,1,1), padding='valid',name = name+'_pooly')(layer6_2)
#     layer6_3 = AveragePooling3D(pool_size=(1, 9, 7), strides=(1,1,1), padding='valid',name = name+'_poolz')(layer6_3)
#     print(layer6_1._keras_shape, layer6_2._keras_shape, layer6_3.shape)
#     # layer6_1 = Conv3D(1, (1, 1, 1),name = name+'final1')(layer6_1)
#     # layer6_2 = Conv3D(1, (1, 1, 1),name = name+'final2')(layer6_2)
#     # layer6_3 = Conv3D(1, (1, 1, 1),name = name+'final3')(layer6_3)
#     # print(layer6_1._keras_shape, layer6_2._keras_shape, layer6_3.shape)
#     return layer6_1, layer6_2, layer6_3



#   ------  classification Supervised1   -----   #
def class_supervised_block(input_layer, batch_normalization=False, instance_normalization=True,
                           LeakyReLU=LeakyReLU,nbasefilters = 1,LayerDepth = 3,name = 'CSblock'):
    input_layer = create_convolution_block(n_filters=nbasefilters,
                                        input_layer=input_layer,
                                        strides=(1, 1, 1), kernel=(1, 1, 1),
                                        batch_normalization=batch_normalization,
                                        instance_normalization=instance_normalization,
                                        activation=LeakyReLU,
                                        name = name+'_x0000')
    layer6_1 = create_convolution_block(n_filters=nbasefilters,
                                        input_layer=input_layer,
                                        strides=(2, 2, 1),kernel=(2, 2, 1),
                                        batch_normalization=batch_normalization,
                                        instance_normalization=instance_normalization,
                                        activation=LeakyReLU,
                                        name = name+'_x1_1')
    # layer6_1 = create_convolution_block(n_filters=nbasefilters,
    #                                     input_layer=layer6_1,
    #                                     batch_normalization=batch_normalization,
    #                                     instance_normalization=instance_normalization,
    #                                     activation=LeakyReLU,
    #                                     name = name+'_x1_2')
    layer6_2 = create_convolution_block(n_filters=nbasefilters,
                                        input_layer=input_layer,
                                        strides=(2, 1, 2),kernel=(2, 1, 2),
                                        batch_normalization=batch_normalization,
                                        instance_normalization=instance_normalization,
                                        activation=LeakyReLU,
                                        name = name+'_y1_1')
    # layer6_2 = create_convolution_block(n_filters=nbasefilters,
    #                                     input_layer=layer6_2,
    #                                     batch_normalization=batch_normalization,
    #                                     instance_normalization=instance_normalization,
    #                                     activation=LeakyReLU,
    #                                     name = name+'_y1_2')
    layer6_3 = create_convolution_block(n_filters=nbasefilters,
                                        input_layer=input_layer,
                                        strides=(1, 2, 2),kernel=(1, 2, 2),
                                        batch_normalization=batch_normalization,
                                        instance_normalization=instance_normalization,
                                        activation=LeakyReLU,
                                        name = name+'_z1_1')
    # layer6_3 = create_convolution_block(n_filters=nbasefilters,
    #                                     input_layer=layer6_3,
    #                                     batch_normalization=batch_normalization,
    #                                     instance_normalization=instance_normalization,
    #                                     activation=LeakyReLU,
    #                                     name = name+'_z1_2')
    print(layer6_1.shape, layer6_2.shape, layer6_3._keras_shape)

    for i in range(LayerDepth):
        layer6_1 = create_convolution_block(n_filters=nbasefilters,
                                            input_layer=layer6_1,
                                            strides=(2, 2, 1),kernel=(2, 2, 1),
                                            batch_normalization=batch_normalization,
                                            instance_normalization=instance_normalization,
                                            activation=LeakyReLU,
                                            name = name+'_x' + str(i+2) + '_1')
        # layer6_1 = create_convolution_block(n_filters=nbasefilters,
        #                                     input_layer=layer6_1,
        #                                     batch_normalization=batch_normalization,
        #                                     instance_normalization=instance_normalization,
        #                                     activation=LeakyReLU,
        #                                     name = name+'_x' + str(i+2) + '_2')
        print('x shape : ', layer6_1._keras_shape)

    for i in range(LayerDepth):
        layer6_2 = create_convolution_block(n_filters=nbasefilters,
                                            input_layer=layer6_2,
                                            strides=(2, 1, 2),kernel=(2, 1, 2),
                                            batch_normalization=batch_normalization,
                                            instance_normalization=instance_normalization,
                                            activation=LeakyReLU,
                                            name = name+'_y' + str(i+2) + '_1')
        # layer6_2 = create_convolution_block(n_filters=nbasefilters,
        #                                     input_layer=layer6_2,
        #                                     batch_normalization=batch_normalization,
        #                                     instance_normalization=instance_normalization,
        #                                     activation=LeakyReLU,
        #                                     name = name+'_y' + str(i+2) + '_2')
        print('y shape : ', layer6_2._keras_shape)

    for i in range(LayerDepth):
        layer6_3 = create_convolution_block(n_filters=nbasefilters,
                                            input_layer=layer6_3,
                                            strides=(1, 2, 2),kernel=(1, 2, 2),
                                            batch_normalization=batch_normalization,
                                            instance_normalization=instance_normalization,
                                            activation=LeakyReLU,
                                            name = name+'_z' + str(i+2) + '_1')
        # layer6_3 = create_convolution_block(n_filters=nbasefilters,
        #                                     input_layer=layer6_3,
        #                                     batch_normalization=batch_normalization,
        #                                     instance_normalization=instance_normalization,
        #                                     activation=LeakyReLU,
        #                                     name = name+'_z' + str(i+2) + '_2')
        print('z shape : ', layer6_3.shape)

    layer6_1 = AveragePooling3D(pool_size=(5, 9, 1), strides=(1,1,1), padding='valid',name = name+'_poolx')(layer6_1)
    layer6_2 = AveragePooling3D(pool_size=(5, 1, 7), strides=(1,1,1), padding='valid',name = name+'_pooly')(layer6_2)
    layer6_3 = AveragePooling3D(pool_size=(1, 9, 7), strides=(1,1,1), padding='valid',name = name+'_poolz')(layer6_3)
    print(layer6_1._keras_shape, layer6_2._keras_shape, layer6_3.shape)
    # layer6_1 = Conv3D(1, (1, 1, 1),name = name+'final1')(layer6_1)
    # layer6_2 = Conv3D(1, (1, 1, 1),name = name+'final2')(layer6_2)
    # layer6_3 = Conv3D(1, (1, 1, 1),name = name+'final3')(layer6_3)
    # print(layer6_1._keras_shape, layer6_2._keras_shape, layer6_3.shape)
    return layer6_1, layer6_2, layer6_3



'''

#   ------  classification Supervised1   -----   #
def class_supervised_block(input_layer, batch_normalization=False, instance_normalization=True,
                           LeakyReLU=LeakyReLU,nbasefilters = 1,LayerDepth = 3,name = 'CSblock'):

    #it can be used in any segmentation model

    input_layer = create_convolution_block(n_filters=nbasefilters,
                                        input_layer=input_layer,
                                        strides=(1, 1, 1), kernel=(1, 1, 1),
                                        batch_normalization=batch_normalization,
                                        instance_normalization=instance_normalization,
                                        activation=LeakyReLU,
                                        name = name+'_x0000')
    layer6_1 = create_convolution_block(n_filters=nbasefilters,
                                        input_layer=input_layer,
                                        strides=(2, 2, 1),kernel=(2, 2, 1),
                                        batch_normalization=batch_normalization,
                                        instance_normalization=instance_normalization,
                                        activation=LeakyReLU,
                                        name = name+'_x1_1')
    # layer6_1 = create_convolution_block(n_filters=nbasefilters,
    #                                     input_layer=layer6_1,
    #                                     batch_normalization=batch_normalization,
    #                                     instance_normalization=instance_normalization,
    #                                     activation=LeakyReLU,
    #                                     name = name+'_x1_2')
    layer6_2 = create_convolution_block(n_filters=nbasefilters,
                                        input_layer=input_layer,
                                        strides=(2, 1, 2),kernel=(2, 1, 2),
                                        batch_normalization=batch_normalization,
                                        instance_normalization=instance_normalization,
                                        activation=LeakyReLU,
                                        name = name+'_y1_1')
    # layer6_2 = create_convolution_block(n_filters=nbasefilters,
    #                                     input_layer=layer6_2,
    #                                     batch_normalization=batch_normalization,
    #                                     instance_normalization=instance_normalization,
    #                                     activation=LeakyReLU,
    #                                     name = name+'_y1_2')
    layer6_3 = create_convolution_block(n_filters=nbasefilters,
                                        input_layer=input_layer,
                                        strides=(1, 2, 2),kernel=(1, 2, 2),
                                        batch_normalization=batch_normalization,
                                        instance_normalization=instance_normalization,
                                        activation=LeakyReLU,
                                        name = name+'_z1_1')
    # layer6_3 = create_convolution_block(n_filters=nbasefilters,
    #                                     input_layer=layer6_3,
    #                                     batch_normalization=batch_normalization,
    #                                     instance_normalization=instance_normalization,
    #                                     activation=LeakyReLU,
    #                                     name = name+'_z1_2')
    print(layer6_1.shape, layer6_2.shape, layer6_3._keras_shape)

    for i in range(LayerDepth):
        layer6_1 = create_convolution_block(n_filters=nbasefilters,
                                            input_layer=layer6_1,
                                            strides=(2, 2, 1),kernel=(2, 2, 1),
                                            batch_normalization=batch_normalization,
                                            instance_normalization=instance_normalization,
                                            activation=LeakyReLU,
                                            name = name+'_x' + str(i+2) + '_1')
        # layer6_1 = create_convolution_block(n_filters=nbasefilters,
        #                                     input_layer=layer6_1,
        #                                     batch_normalization=batch_normalization,
        #                                     instance_normalization=instance_normalization,
        #                                     activation=LeakyReLU,
        #                                     name = name+'_x' + str(i+2) + '_2')
        print('x shape : ', layer6_1._keras_shape)

    for i in range(LayerDepth):
        layer6_2 = create_convolution_block(n_filters=nbasefilters,
                                            input_layer=layer6_2,
                                            strides=(2, 1, 2),kernel=(2, 1, 2),
                                            batch_normalization=batch_normalization,
                                            instance_normalization=instance_normalization,
                                            activation=LeakyReLU,
                                            name = name+'_y' + str(i+2) + '_1')
        # layer6_2 = create_convolution_block(n_filters=nbasefilters,
        #                                     input_layer=layer6_2,
        #                                     batch_normalization=batch_normalization,
        #                                     instance_normalization=instance_normalization,
        #                                     activation=LeakyReLU,
        #                                     name = name+'_y' + str(i+2) + '_2')
        print('y shape : ', layer6_2._keras_shape)

    for i in range(LayerDepth):
        layer6_3 = create_convolution_block(n_filters=nbasefilters,
                                            input_layer=layer6_3,
                                            strides=(1, 2, 2),kernel=(1, 2, 2),
                                            batch_normalization=batch_normalization,
                                            instance_normalization=instance_normalization,
                                            activation=LeakyReLU,
                                            name = name+'_z' + str(i+2) + '_1')
        # layer6_3 = create_convolution_block(n_filters=nbasefilters,
        #                                     input_layer=layer6_3,
        #                                     batch_normalization=batch_normalization,
        #                                     instance_normalization=instance_normalization,
        #                                     activation=LeakyReLU,
        #                                     name = name+'_z' + str(i+2) + '_2')
        print('z shape : ', layer6_3.shape)

    layer6_1 = AveragePooling3D(pool_size=(5, 9, 1), strides=(1,1,1), padding='valid',name = name+'_poolx')(layer6_1)
    layer6_2 = AveragePooling3D(pool_size=(5, 1, 7), strides=(1,1,1), padding='valid',name = name+'_pooly')(layer6_2)
    layer6_3 = AveragePooling3D(pool_size=(1, 9, 7), strides=(1,1,1), padding='valid',name = name+'_poolz')(layer6_3)
    print(layer6_1._keras_shape, layer6_2._keras_shape, layer6_3.shape)
    # layer6_1 = Conv3D(1, (1, 1, 1),name = name+'final1')(layer6_1)
    # layer6_2 = Conv3D(1, (1, 1, 1),name = name+'final2')(layer6_2)
    # layer6_3 = Conv3D(1, (1, 1, 1),name = name+'final3')(layer6_3)
    # print(layer6_1._keras_shape, layer6_2._keras_shape, layer6_3.shape)
    return layer6_1, layer6_2, layer6_3

'''




#   ------  classification Supervised3   -----   #
def class_supervised_block2(input_layer, batch_normalization=False, instance_normalization=True,
                           LeakyReLU=LeakyReLU,nbasefilters = 30,LayerDepth = 4,name = 'CSblock'):
    '''

    it can be used in the layer of max channels.

    '''
    CS_conv = Conv3D(nbasefilters, (1, 1, 1), name=name + '_CS_conv')(input_layer)
    for layer_depth in range(LayerDepth):
        print('##  ', CS_conv._keras_shape)
        CS_conv = MaxPooling3D(pool_size=(2,2,2), name=name + '_CS_pool' + str(layer_depth))(CS_conv)
        CS_conv = create_convolution_block(n_filters=nbasefilters,
                                            input_layer=CS_conv,
                                            strides=(1, 1, 1),
                                            batch_normalization=batch_normalization,
                                            instance_normalization=instance_normalization,
                                            activation=LeakyReLU,
                                            name=name + '_CS_conv_' + str(layer_depth))
        print('##  ',CS_conv._keras_shape)

    CS_conv = GlobalAveragePooling3D(name = name + '_layer1')(CS_conv)
    print('The shape of AveragePooling3D : ',CS_conv._keras_shape)
    CS_conv = Dense(240, activation='relu',name = name + 'active1')(CS_conv)
    CS_conv = Dense(1024, activation='relu', name=name + 'active2')(CS_conv)
    # CSLayer1 = Dropout(rate=0.3)(CSLayer1)
    CS_conv = Dense(80, activation='sigmoid', name = name+'_class')(CS_conv)
    return CS_conv


    
    
    
    
    
def channel_weight(InputLayer,name = 'WeightLayer'):
    supervise_channel_0_max = GlobalMaxPooling3D()(InputLayer)
#    supervise_channel_0_max = Reshape((1, 1, InputShape[1]))(supervise_channel_0_max)
    supervise_channel_0_avg = GlobalAveragePooling3D()(InputLayer)
#    supervise_channel_0_avg = Reshape((1, 1, InputShape[1]))(supervise_channel_0_avg)
    supervise_channel_0_max = Dense(30, activation='relu',name = name + 'active1')(supervise_channel_0_max)
    supervise_channel_0_avg = Dense(30, activation='relu',name = name + 'active2')(supervise_channel_0_avg)
    

#    supervise_channel_0_max = create_convolution_block(input_layer=supervise_channel_0_max,
#                                                       n_filters=30,
#                                                       kernel=(1, 1, 1),
#                                                       name=name+'_Gmax',
#                                                       batch_normalization=None,
#                                                       instance_normalization=None,
#                                                       activation=LeakyReLU)
#    supervise_channel_0_avg = create_convolution_block(input_layer=supervise_channel_0_avg,
#                                                       n_filters=30,
#                                                       kernel=(1, 1, 1),
#                                                       name=name+'_Gavg',
#                                                       batch_normalization=None,
#                                                       instance_normalization=None,
#                                                       activation=LeakyReLU)
    supervise_channel_0 = Add()([supervise_channel_0_max, supervise_channel_0_avg])
    supervise_channel_0 = Activation('sigmoid', name=name + '_act_0')(supervise_channel_0)
    return supervise_channel_0

    
    
    
    
    
    
def Conv_Up_Blcok(input_layer, n_filters, batch_normalization=False, instance_normalization=True,
                  pool_size = (2,2,2),deconvolution = False,name = 'conv'):
    ConvLayer = create_convolution_block(input_layer=input_layer,
                                          n_filters=n_filters,
                                          batch_normalization=batch_normalization,
                                          instance_normalization=instance_normalization,
                                          activation=LeakyReLU,
                                          name=name + 'conv')
    UpLayer = get_up_convolution(pool_size=pool_size,
                                        deconvolution=deconvolution,
                                        n_filters=n_filters,
                                        name=name + 'conv')(ConvLayer)
    return UpLayer
    
    
    
    
    
    
    

def SpatialChannelAttentionBlcok(input_layer, Outfilters,batch_normalization=False, instance_normalization=True,
                  pool_size = (2,2,2),deconvolution = False,name = 'conv',activation=LeakyReLU,layernumbers = 4):
    n_labels = 1
    layer1 = input_layer
    for i in range(layernumbers):
        layer1 = create_convolution_block(input_layer=layer1,
                                          n_filters=Outfilters,
                                          batch_normalization=batch_normalization,
                                          instance_normalization=instance_normalization,
                                          activation=activation,
                                          strides=pool_size,
                                          name=name + '_SAconv' + str(i))

    layer2 = create_convolution_block(input_layer=layer1,
                                      n_filters=Outfilters,
                                      batch_normalization=batch_normalization,
                                      instance_normalization=instance_normalization,
                                      activation=activation,
                                      strides=pool_size,
                                      name=name + '_SAconv' + str(layernumbers))
    layer2 = create_convolution_block(input_layer=layer2,
                                      n_filters=Outfilters,
                                      batch_normalization=batch_normalization,
                                      instance_normalization=instance_normalization,
                                      activation=activation,
                                      strides=pool_size,
                                      name=name + '_SAconv1' + str(layernumbers))
    layer2 = GlobalAveragePooling3D(name = name + '_GPool')(layer2)
    layer2 = Dense(int(Outfilters/2), activation='relu', use_bias = True, name=name + 'Dense1')(layer2)
    layer2 = Dense(Outfilters, activation=None, use_bias = True,name=name + 'Dense2')(layer2)
    Cact = Activation('sigmoid', name=name + 'CAact')(layer2)

    for i in range(layernumbers):
        layer1 = Conv_Up_Blcok(layer1, n_filters = 32, batch_normalization=batch_normalization, instance_normalization=instance_normalization,
                      pool_size=pool_size, deconvolution=deconvolution, name=name + 'SAUpLayer' + str(i))
    layer1 = Conv3D(n_labels, (1, 1, 1), name=name + 'finalConv')(layer1)
    Sact = Activation('sigmoid', name=name + 'SAact')(layer1)

    SCLayer = create_convolution_block(input_layer=input_layer,
                                             n_filters=Outfilters,
                                             batch_normalization=batch_normalization,
                                             instance_normalization=instance_normalization,
                                             activation=LeakyReLU,
                                             name=name + 'LSLayer_conv')
    current_layer = Multiply(name=name + 'SCLayer' + '_multiply1')([SCLayer, Sact])
    current_layer = Multiply(name=name + 'SCLayer' + '_multiply2')([current_layer, Cact])
    current_layer = Add(name=name + 'SCLayer_add')([current_layer, SCLayer])
    return current_layer

    
    
    
    
   
    

def ASPP_Block(_Input,batch_normalization=False,instance_normalization=True,name = 'ASPP',n_filters = 512):
    DilationRates = [3,5,7,11]
    axis = 1
    conv1 = create_convolution_block(input_layer=_Input,
                                     n_filters=n_filters,
                                     batch_normalization=batch_normalization,
                                     instance_normalization=instance_normalization,
                                     activation=LeakyReLU,
                                     name=name + '_conv1',
                                     dilation_rate=(DilationRates[0], DilationRates[0], DilationRates[0]))
    conv2 = create_convolution_block(input_layer=conv1,
                                     n_filters=n_filters,
                                     batch_normalization=batch_normalization,
                                     instance_normalization=instance_normalization,
                                     activation=LeakyReLU,
                                     name=name + '_conv2',
                                     dilation_rate=(DilationRates[1], DilationRates[1], DilationRates[1]))
    concat3 = concatenate([conv1, conv2], axis=axis, name=name + 'concat3')
    conv3 = create_convolution_block(input_layer=concat3,
                                     n_filters=n_filters,
                                     batch_normalization=batch_normalization,
                                     instance_normalization=instance_normalization,
                                     activation=LeakyReLU,
                                     name=name + '_conv3',
                                     dilation_rate=(DilationRates[2], DilationRates[2], DilationRates[2]))
    concat4 = concatenate([ conv1, conv2, conv3], axis=axis, name=name + 'concat4')
    conv4 = create_convolution_block(input_layer=concat4,
                                     n_filters=n_filters,
                                     batch_normalization=batch_normalization,
                                     instance_normalization=instance_normalization,
                                     activation=LeakyReLU,
                                     name=name + '_conv4',
                                     dilation_rate=(DilationRates[3], DilationRates[3], DilationRates[3]))
    concat5 = concatenate([ conv1, conv2, conv3,conv4], axis=axis, name=name + 'concat5')
    return concat5



def Refine_ASPP_Block2(UnetOutLayer,InputLayer,name = 'ASPP'):
        F_concat = InputLayer
        for i in [2, 3]:
            F_concat = SpatialChannelAttentionBlcok(F_concat,30 * (2 ** i),layernumbers = 4,name=name + 'F_conv' + str(i) + '_1')
            F_concat = SpatialChannelAttentionBlcok(F_concat,30 * (2 ** i),layernumbers = 4,name=name + 'F_conv' + str(i) + '_2')

        UnetOutLayer = SpatialChannelAttentionBlcok(UnetOutLayer, 240, layernumbers=4,
                                                name=name + 'OUT_conv_' + str(i))

        concat1 = concatenate([F_concat, UnetOutLayer], axis=1, name= name+'concat1')
        F_concat = SpatialChannelAttentionBlcok(concat1, 240, layernumbers=4,
                                                name=name + '_finalconv1')
        F_concat = SpatialChannelAttentionBlcok(F_concat, 240, layernumbers=4,
                                                name=name + '_finalconv2')
        # F_concat = SpatialChannelAttentionBlcok(F_concat, 240, layernumbers=4,
        #                                         name=name + '_finalconv3')
        # F_concat = SpatialChannelAttentionBlcok(F_concat, 240, layernumbers=4,
        #                                         name=name + '_finalconv4')
        return F_concat



    

def Refine_ASPP_Block(UnetOutLayer,InputLayer,supervise_channel_0,n_filters = 1,batch_normalization=False,instance_normalization=True,name = 'ASPP',WeghtLayer = False):
    axis = 1
    if WeghtLayer:
        DilationRates = [3,6,12,18]
        concat1 = concatenate([InputLayer, UnetOutLayer], axis=axis, name= name+'concat1')
        conv1 = create_convolution_block(input_layer=concat1,
                                          n_filters=n_filters,
                                          batch_normalization=batch_normalization,
                                          instance_normalization=instance_normalization,
                                          activation=LeakyReLU,
                                          name= name+'_conv1',
                                         dilation_rate=(DilationRates[0], DilationRates[0], DilationRates[0]))
        conv1 = Multiply([conv1, supervise_channel_0],name = name+'multy2')
        concat2 = concatenate([InputLayer, UnetOutLayer,conv1], axis=axis, name= name+'concat2')
        conv2 = create_convolution_block(input_layer=concat2,
                                          n_filters=n_filters,
                                          batch_normalization=batch_normalization,
                                          instance_normalization=instance_normalization,
                                          activation=LeakyReLU,
                                          name= name+'_conv2',
                                         dilation_rate=(DilationRates[1], DilationRates[1], DilationRates[1]))
        conv2 = Multiply([conv2, supervise_channel_0],name = name+'multy3')
        concat3 = concatenate([InputLayer, UnetOutLayer,conv1,conv2], axis=axis, name= name+'concat3')
        conv3 = create_convolution_block(input_layer=concat3,
                                          n_filters=n_filters,
                                          batch_normalization=batch_normalization,
                                          instance_normalization=instance_normalization,
                                          activation=LeakyReLU,
                                          name= name+'_conv3',
                                         dilation_rate=(DilationRates[2], DilationRates[2], DilationRates[2]))
        conv3 = Multiply([conv3, supervise_channel_0],name = name+'multy4')
        concat4 = concatenate([InputLayer, UnetOutLayer,conv1,conv2,conv3], axis=axis, name= name+'concat4')
        conv4 = create_convolution_block(input_layer=concat4,
                                          n_filters=n_filters,
                                          batch_normalization=batch_normalization,
                                          instance_normalization=instance_normalization,
                                          activation=LeakyReLU,
                                          name= name+'_conv4',
                                         dilation_rate=(DilationRates[3], DilationRates[3], DilationRates[3]))
        conv4 = Multiply([conv4, supervise_channel_0],name = name+'multy5')
        return conv1,conv2,conv3,conv4
    else:
        
        DilationRates = [3,6,12,18,24,30,36]
        concat1 = concatenate([InputLayer, UnetOutLayer], axis=axis, name= name+'concat1')
        conv1 = create_convolution_block(input_layer=concat1,
                                          n_filters=n_filters,
                                          batch_normalization=batch_normalization,
                                          instance_normalization=instance_normalization,
                                          activation=LeakyReLU,
                                          name= name+'_conv1',
                                         dilation_rate=(DilationRates[0], DilationRates[0], DilationRates[0]))
        concat2 = concatenate([InputLayer,conv1], axis=axis, name= name+'concat2')
        conv2 = create_convolution_block(input_layer=concat2,
                                          n_filters=n_filters,
                                          batch_normalization=batch_normalization,
                                          instance_normalization=instance_normalization,
                                          activation=LeakyReLU,
                                          name= name+'_conv2',
                                         dilation_rate=(DilationRates[1], DilationRates[1], DilationRates[1]))
        concat3 = concatenate([InputLayer,conv1,conv2], axis=axis, name= name+'concat3')
        conv3 = create_convolution_block(input_layer=concat3,
                                          n_filters=n_filters,
                                          batch_normalization=batch_normalization,
                                          instance_normalization=instance_normalization,
                                          activation=LeakyReLU,
                                          name= name+'_conv3',
                                         dilation_rate=(DilationRates[2], DilationRates[2], DilationRates[2]))
        concat4 = concatenate([InputLayer,conv1,conv2,conv3], axis=axis, name= name+'concat4')
        conv4 = create_convolution_block(input_layer=concat4,
                                          n_filters=n_filters,
                                          batch_normalization=batch_normalization,
                                          instance_normalization=instance_normalization,
                                          activation=LeakyReLU,
                                          name= name+'_conv4',
                                         dilation_rate=(DilationRates[3], DilationRates[3], DilationRates[3]))
        concat5 = concatenate([InputLayer,conv1,conv2,conv3,conv4], axis=axis, name= name+'concat5')
        conv5 = create_convolution_block(input_layer=concat5,
                                          n_filters=n_filters,
                                          batch_normalization=batch_normalization,
                                          instance_normalization=instance_normalization,
                                          activation=LeakyReLU,
                                          name= name+'_conv5',
                                         dilation_rate=(DilationRates[4], DilationRates[4], DilationRates[4]))
        concat6 = concatenate([InputLayer,conv1,conv2,conv3,conv4,conv5], axis=axis, name= name+'concat6')
        conv6 = create_convolution_block(input_layer=concat6,
                                          n_filters=n_filters,
                                          batch_normalization=batch_normalization,
                                          instance_normalization=instance_normalization,
                                          activation=LeakyReLU,
                                          name= name+'_conv6',
                                         dilation_rate=(DilationRates[5], DilationRates[5], DilationRates[5]))
        # concat7 = concatenate([InputLayer,conv1,conv2,conv3,conv4,conv5,conv6], axis=axis, name= name+'concat7')
        # conv7 = create_convolution_block(input_layer=concat7,
                                          # n_filters=n_filters,
                                          # batch_normalization=batch_normalization,
                                          # instance_normalization=instance_normalization,
                                          # activation=LeakyReLU,
                                          # name= name+'_conv7',
                                         # dilation_rate=(DilationRates[6], DilationRates[6], DilationRates[6]))
        return conv1,conv2,conv3,conv4,conv5,conv6
    






#   ------  convlution block   -----   #
def create_convolution_block(input_layer, n_filters, batch_normalization=False, kernel=(3, 3, 3), activation=None,
                             padding='same', strides=(1, 1, 1), instance_normalization=True,dilation_rate=(1,1,1),name = 'conv'):

    layer = Conv3D(n_filters, kernel, padding=padding, strides=strides,dilation_rate=dilation_rate,name = name+'_conv')(input_layer)
    # print(layer._keras_shape)
    if batch_normalization:
        layer = BatchNormalization(axis=1,name = name+'_BN')(layer)
    elif instance_normalization:
        layer = InstanceNormalization(axis=1,name = name+'IN')(layer)
    if activation is None:
        return layer
    else:
        return activation()(layer)










#   ------  Res convlution block   -----   #
def create_convolution_res_block(input_layer, n_filters, batch_normalization=False, kernel=(3, 3, 3), activation=None,
                                 padding='same', strides=(1, 1, 1), instance_normalization=False):
    # from keras_contrib.layers.normalization import InstanceNormalization
    layer = Conv3D(n_filters, kernel, padding=padding, strides=strides)(input_layer)
    print(layer._keras_shape)
    if batch_normalization:
        layer = BatchNormalization(axis=1)(layer)
    if activation is None:
        return layer
    else:
        return activation()(layer)










#   ------  upsampling block   -----   #
def get_up_convolution(n_filters, pool_size, kernel_size=(2, 2, 2), strides=(2, 2, 2),
                       deconvolution=False,name = 'UPsample'):
    if deconvolution:
        return Deconvolution3D(filters=n_filters, kernel_size=kernel_size,
                               strides=strides,name = name+'_deconv')
    else:
        return UpSampling3D(size=pool_size,name = name + 'Upsample')






#   ------  no   -----   #
def Refining_Block(input_layer, n_filters, batch_normalization=False, kernel=(3, 3, 3), activation=None,
                                 padding='same', strides=(1, 1, 1), instance_normalization=False):
    # from keras_contrib.layers.normalization import InstanceNormalization
    layer = Conv3D(n_filters, kernel, padding=padding, strides=strides)(input_layer)
    print(layer._keras_shape)
    if batch_normalization:
        layer = BatchNormalization(axis=1)(layer)
    if activation is None:
        return layer
    else:
        return activation()(layer)


def NNet_Block(input_layer, n_filters, batch_normalization=False, kernel=(3, 3, 3), activation=None,
                                 padding='same', strides=(1, 1, 1), instance_normalization=False):
    # from keras_contrib.layers.normalization import InstanceNormalization
    layer = Conv3D(n_filters, kernel, padding=padding, strides=strides)(input_layer)
    print(layer._keras_shape)
    if batch_normalization:
        layer = BatchNormalization(axis=1)(layer)
    if activation is None:
        return layer
    else:
        return activation()(layer)

