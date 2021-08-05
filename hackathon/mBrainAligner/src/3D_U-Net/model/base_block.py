import numpy as np
import keras
from keras import backend as K
from keras.engine import Input, Model
from keras.layers import Conv3D, MaxPooling3D, AveragePooling3D, UpSampling3D, Activation, \
    BatchNormalization, PReLU, Deconvolution3D, LeakyReLU, Add, Dense, Flatten, GlobalAveragePooling3D,GlobalMaxPooling3D, Reshape, Multiply
from keras.optimizers import Adam,SGD
from keras.layers.core import Lambda
import copy


K.set_image_data_format("channels_last")
keras.initializers.TruncatedNormal(mean=0.0, stddev=0.05, seed=None)

try:
    from keras.engine import merge
except ImportError:
    from keras.layers.merge import concatenate


def deeplab_atrous_Block(_Input,batch_normalization=False,instance_normalization=True,name = 'ASPP',n_filters = 30):
    DilationRates = [6,12,18,24]
    axis = 4
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
    conv3 = create_convolution_block(input_layer=conv2,
                                     n_filters=n_filters,
                                     batch_normalization=batch_normalization,
                                     instance_normalization=instance_normalization,
                                     activation=LeakyReLU,
                                     name=name + '_conv3',
                                     dilation_rate=(DilationRates[2], DilationRates[2], DilationRates[2]))
    conv4 = create_convolution_block(input_layer=conv3,
                                     n_filters=n_filters,
                                     batch_normalization=batch_normalization,
                                     instance_normalization=instance_normalization,
                                     activation=LeakyReLU,
                                     name=name + '_conv4',
                                     dilation_rate=(DilationRates[3], DilationRates[3], DilationRates[3]))
    # concat4 = concatenate([conv1, conv2, conv3], axis=axis, name=name + 'concat4')
    return conv4



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











def SpatialChannelAttentionBlcok(input_layer, Outfilters,batch_normalization=False, instance_normalization=True,
                  pool_size = (2,2,2),name = 'conv',activation=LeakyReLU,layernumbers = 3):
    n_labels = 1
    SCLayer = Conv3D(Outfilters, (1, 1, 1), name=name + 'LSLayer_conv')(input_layer)
    # SCLayer = create_convolution_block(input_layer=input_layer,
    #                                          n_filters=Outfilters,
    #                                          batch_normalization=batch_normalization,
    #                                          instance_normalization=instance_normalization,
    #                                          activation=LeakyReLU,
    #                                          name=name + 'LSLayer_conv')
    layer1 = SCLayer
    for i in range(layernumbers):
        layer1 = create_convolution_block(input_layer=layer1,
                                          n_filters=Outfilters,
                                          batch_normalization=batch_normalization,
                                          instance_normalization=instance_normalization,
                                          activation=activation,
                                          strides=pool_size,
                                          kernel=(2, 2, 2),
                                          name=name + '_SAconv' + str(i))

    layer1 = GlobalAveragePooling3D(name = name + '_GPool')(layer1)
    # layer1 = Dense(Outfilters*4, activation=None, use_bias = True,name=name + 'Dense1')(layer1)
    layer1 = Dense(Outfilters, activation=None, use_bias=True, name=name + 'Dense2')(layer1)
    Cact = Activation('sigmoid', name=name + 'CAact')(layer1)
    Cact = Reshape((1,1,1,Outfilters))(Cact)
    current_layer = Multiply(name=name + 'SCLayer' + '_multiply1')([SCLayer, Cact])
    layers2 = current_layer
    layers2 = Conv3D(n_labels, (1, 1, 1), name=name + 'finalConv')(layers2)
    Sact = Activation('sigmoid', name=name + 'SAact')(layers2)
    current_layer = Multiply(name=name + 'SCLayer' + '_multiply2')([current_layer, Sact])
    current_layer = Add(name=name + 'SCLayer_add')([current_layer, SCLayer])
    return current_layer





# def SpatialChannelAttentionBlcok(input_layer, Outfilters,batch_normalization=False, instance_normalization=True,
#                   pool_size = (2,2,2),deconvolution = False,name = 'conv',activation=LeakyReLU,layernumbers = 3):
#     n_labels = 1
#     SCLayer = create_convolution_block(input_layer=input_layer,
#                                              n_filters=Outfilters,
#                                              batch_normalization=batch_normalization,
#                                              instance_normalization=instance_normalization,
#                                              activation=LeakyReLU,
#                                              name=name + 'LSLayer_conv')
#     layer1 = SCLayer
#     for i in range(layernumbers):
#         layer1 = create_convolution_block(input_layer=layer1,
#                                           n_filters=Outfilters,
#                                           batch_normalization=batch_normalization,
#                                           instance_normalization=instance_normalization,
#                                           activation=activation,
#                                           strides=pool_size,
#                                           name=name + '_SAconv' + str(i))
#
#     layer2 = create_convolution_block(input_layer=layer1,
#                                       n_filters=Outfilters,
#                                       batch_normalization=batch_normalization,
#                                       instance_normalization=instance_normalization,
#                                       activation=activation,
#                                       strides=pool_size,
#                                       name=name + '_SAconv' + str(layernumbers))
#     layer2 = GlobalMaxPooling3D(name = name + '_GPool')(layer2)
#     layer2 = Dense(Outfilters, activation=None, use_bias = True,name=name + 'Dense2')(layer2)
#     Cact = Activation('sigmoid', name=name + 'CAact')(layer2)
#     Cact = Reshape((1,1,1,Outfilters))(Cact)
#     # layer1 = get_up_convolution(pool_size=(2 ** layernumbers, 2 ** layernumbers, 2 ** layernumbers), deconvolution=False,
#     #                             n_filters=n_labels,
#     #                             name=name + 'up5')(layer1)
#     # layer1 = Conv3D(n_labels, (1, 1, 1), name=name + 'finalConv')(layer1)
#     # Sact = Activation('sigmoid', name=name + 'SAact')(layer1)
#     current_layer = Multiply(name=name + 'SCLayer' + '_multiply1')([SCLayer, Cact])
#     # current_layer = Multiply(name=name + 'SCLayer' + '_multiply2')([current_layer, Sact])
#     # current_layer = Add(name=name + 'SCLayer_add')([current_layer, SCLayer])
#     return current_layer


# def SpatialChannelAttentionBlcok(input_layer, Outfilters,batch_normalization=False, instance_normalization=True,
#                   pool_size = (2,2,2),deconvolution = False,name = 'conv',activation=LeakyReLU,layernumbers = 3):
#     n_labels = 1
#     SCLayer = create_convolution_block(input_layer=input_layer,
#                                              n_filters=Outfilters,
#                                              batch_normalization=batch_normalization,
#                                              instance_normalization=instance_normalization,
#                                              activation=LeakyReLU,
#                                              name=name + 'LSLayer_conv')
#     layer1 = SCLayer
#     for i in range(layernumbers):
#         layer1 = create_convolution_block(input_layer=layer1,
#                                           n_filters=Outfilters,
#                                           batch_normalization=batch_normalization,
#                                           instance_normalization=instance_normalization,
#                                           activation=activation,
#                                           strides=pool_size,
#                                           name=name + '_SAconv' + str(i))
#
#     layer2 = create_convolution_block(input_layer=layer1,
#                                       n_filters=Outfilters,
#                                       batch_normalization=batch_normalization,
#                                       instance_normalization=instance_normalization,
#                                       activation=activation,
#                                       strides=pool_size,
#                                       name=name + '_SAconv' + str(layernumbers))
#     layer2 = GlobalAveragePooling3D(name = name + '_GPool')(layer2)
#     # layer2 = Dense(2*Outfilters, activation='relu', use_bias = True, name=name + 'Dense1')(layer2)
#     layer2 = Dense(Outfilters, activation=None, use_bias = True,name=name + 'Dense2')(layer2)
#     Cact = Activation('sigmoid', name=name + 'CAact')(layer2)
#     Cact = Reshape((1,1,1,Outfilters))(Cact)
#     layer1 = get_up_convolution(pool_size=(2 ** layernumbers, 2 ** layernumbers, 2 ** layernumbers), deconvolution=False,
#                                 n_filters=n_labels,
#                                 name=name + 'up5')(layer1)
#     layer1 = Conv3D(n_labels, (1, 1, 1), name=name + 'finalConv')(layer1)
#     Sact = Activation('sigmoid', name=name + 'SAact')(layer1)
#     current_layer = Multiply(name=name + 'SCLayer' + '_multiply1')([SCLayer, Cact])
#     current_layer = Multiply(name=name + 'SCLayer' + '_multiply2')([current_layer, Sact])
#     # current_layer = Add(name=name + 'SCLayer_add')([current_layer, SCLayer])
#     return current_layer









def ASPP_Block(_Input,batch_normalization=False,instance_normalization=True,name = 'ASPP',n_filters = 30):
    DilationRates = [3,5,7,11]
    axis = 4
    conv1 = create_convolution_block(input_layer=_Input,
                                     n_filters=n_filters,
                                     batch_normalization=batch_normalization,
                                     instance_normalization=instance_normalization,
                                     activation=LeakyReLU,
                                     name=name + '_conv1',
                                     dilation_rate=(DilationRates[0], DilationRates[0], DilationRates[0]))
    # concat2 = concatenate([_Input,conv1], axis=axis, name=name + 'concat2')
    # concat2 = Conv3D(n_filters, (1, 1, 1), name=name + 'ConcateConv0')(conv1)
    conv2 = create_convolution_block(input_layer=conv1,
                                     n_filters=n_filters,
                                     batch_normalization=batch_normalization,
                                     instance_normalization=instance_normalization,
                                     activation=LeakyReLU,
                                     name=name + '_conv2',
                                     dilation_rate=(DilationRates[1], DilationRates[1], DilationRates[1]))
    concat3 = concatenate([conv1, conv2], axis=axis, name=name + 'concat3')
    # concat3 = Conv3D(n_filters, (1, 1, 1), name=name + 'ConcateConv1')(concat3)
    conv3 = create_convolution_block(input_layer=concat3,
                                     n_filters=n_filters,
                                     batch_normalization=batch_normalization,
                                     instance_normalization=instance_normalization,
                                     activation=LeakyReLU,
                                     name=name + '_conv3',
                                     dilation_rate=(DilationRates[2], DilationRates[2], DilationRates[2]))
    concat4 = concatenate([conv1, conv2, conv3], axis=axis, name=name + 'concat4')
    # conv4 = create_convolution_block(input_layer=concat4,
    #                                  n_filters=n_filters,
    #                                  batch_normalization=batch_normalization,
    #                                  instance_normalization=instance_normalization,
    #                                  activation=LeakyReLU,
    #                                  name=name + '_conv4',
    #                                  dilation_rate=(DilationRates[3], DilationRates[3], DilationRates[3]))
    # concat5 = concatenate([conv1, conv2, conv3,conv4], axis=axis, name=name + 'concat5')
    return concat4


#   ------  convlution block   -----   #
def create_convolution_block(input_layer, n_filters, batch_normalization=False, kernel=(3, 3, 3), activation=None,
                             padding='same', strides=(1, 1, 1), instance_normalization=True,dilation_rate=(1,1,1),name = 'conv'):
    try:
        from keras_contrib.layers.normalization.instancenormalization import InstanceNormalization
    except ImportError:
        raise ImportError("Install keras_contrib in order to use instance normalization."
                          "\nTry: pip install git+https://www.github.com/farizrahman4u/keras-contrib.git")
    layer = Conv3D(n_filters, kernel, padding=padding, strides=strides,dilation_rate=dilation_rate,name = name+'_conv')(input_layer)
    # print(layer._keras_shape)
    if batch_normalization:
        layer = BatchNormalization(axis=4,name = name+'_BN')(layer)
    elif instance_normalization:
        layer = InstanceNormalization(axis=4,name = name+'IN')(layer)
    if activation is None:
        return layer
    else:
        return activation()(layer)



def Conv_shortcut_Block(input_layer, n_filters, batch_normalization=False, kernel=(3, 3, 3), activation=LeakyReLU,
                             padding='same', strides=(1, 1, 1), instance_normalization=True, with_conv_shortcut=False,name = 'conv'):
    x = create_convolution_block(input_layer=input_layer,
                                      n_filters=n_filters,
                                      kernel=kernel,
                                      batch_normalization=batch_normalization,
                                      instance_normalization=instance_normalization,
                                      activation=activation,
                                      padding=padding,
                                      name='conv1_' + name)
    if with_conv_shortcut:
        shortcut = create_convolution_block(input_layer=input_layer,
                                      n_filters=n_filters,
                                      kernel=kernel,
                                      batch_normalization=batch_normalization,
                                      instance_normalization=instance_normalization,
                                      activation=activation,
                                      padding=padding,
                                      name='conv2_' + name)
        x = Add([x, shortcut])
        return x
    else:
        x = Add([x, input_layer])
        return x




def CurrentConvBlock(input_layer, n_filters, batch_normalization=False, kernel=(3, 3, 3), activation=LeakyReLU,
                             padding='same', instance_normalization=True, with_conv_shortcut=False,name = 'conv'):
    x = Conv_shortcut_Block(input_layer=input_layer,
                                 n_filters=n_filters,
                                 batch_normalization=batch_normalization,
                                 instance_normalization=instance_normalization,
                                 activation=activation,
                                 name='conv1_' + name)
    x = Conv_shortcut_Block(input_layer=x,
                                 n_filters=n_filters,
                                 batch_normalization=batch_normalization,
                                 instance_normalization=instance_normalization,
                                 activation=activation,
                                 name='conv1_' + name)
    if with_conv_shortcut:
        shortcut = create_convolution_block(input_layer=input_layer,
                                      n_filters=n_filters,
                                      kernel=kernel,
                                      batch_normalization=batch_normalization,
                                      instance_normalization=instance_normalization,
                                      activation=activation,
                                      padding=padding,
                                      name='conv2_' + name)
        x = Add([x, shortcut])
        return x
    else:
        x = Add([x, input_layer])
        return x





#   ------  upsampling block   -----   #
def get_up_convolution(n_filters, pool_size, kernel_size=(2, 2, 2), strides=(2, 2, 2),
                       deconvolution=False,name = 'UPsample'):
    if deconvolution:
        return Deconvolution3D(filters=n_filters, kernel_size=kernel_size,
                               strides=strides,name = name+'_deconv')
    else:
        return UpSampling3D(size=pool_size,name = name + 'Upsample')






