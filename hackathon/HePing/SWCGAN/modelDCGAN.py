from keras.layers.core import Dense, Reshape, Activation, Lambda
from keras.layers import Input, merge,Flatten
from keras.layers.normalization import BatchNormalization
from keras.models import Model,Sequential
from keras.layers.convolutional import UpSampling2D, Conv2D, Deconv2D
import layers as layers
from keras.layers import LeakyReLU as lrelu
import numpy as np

def generator(n_nodes=256, noise_dim=100, batch_size=32):
    # generate noise data
    noise_input = Input(shape=(1, noise_dim), name='noise_input')

    # ------------
    # Geometry model
    # -------------

    geometry_hidden1 = Dense(units=1024, input_dim=100, activation='tanh', name='g_dens_1')(noise_input)
    geometry_hidden2 = Dense(4*4*64, activation='tanh', name='g_dens_2')(geometry_hidden1)
    geometry_hidden2_reshape = Reshape(target_shape=(4, 4, 64), name='g_dens_reshape')(geometry_hidden2)

    geometry_hidden3 = Deconv2D(32, kernel_size=(3, 3), strides=(2, 2), name='g_deconv_1', padding='same' ,data_format='channels_last',kernel_initializer='normal')(geometry_hidden2_reshape)
    # geometry_hidden3 = UpSampling2D(size=(2, 2), name='g_up_1', data_format='channels_last')(geometry_hidden2_reshape)
    # geometry_hidden3 = Conv2D(32, kernel_size=(5, 5), padding='same',  name='g_conv_1')(geometry_hidden3)
    geometry_hidden3 = BatchNormalization(name='g_bn_1')(Activation(activation='tanh')(geometry_hidden3))

    geometry_hidden4 = Deconv2D(3, kernel_size=(3, 3), strides=(2, 2), name='g_deconv_2', padding='same',data_format='channels_last', kernel_initializer='normal')(geometry_hidden3)
    # geometry_hidden4 = UpSampling2D(size=(2, 2), name='g_up_2', data_format='channels_last')(geometry_hidden3)
    # geometry_hidden4 = Conv2D(3, kernel_size=(5, 5), padding='same', name='g_conv_2')(geometry_hidden4)
    geometry_hidden4 = BatchNormalization(name='g_bn_2')(Activation(activation='tanh')(geometry_hidden4))

    # geometry_hidden5 = UpSampling2D(size=(2, 2), name='g_up_3', data_format='channels_last')(geometry_hidden3)
    # geometry_hidden5 = Conv2D(3, kernel_size=(5, 5), padding='same', name='g_conv_3')(
    #     geometry_hidden5)
    # geometry_hidden5 = BatchNormalization(name='g_bn_3')(Activation(activation='tanh')(geometry_hidden5))

    geometry_hidden_reshape = Reshape(target_shape=(n_nodes, 3))(geometry_hidden4)
    geometry_output = geometry_hidden_reshape
    geometry_model = Model(inputs=[noise_input],
                           outputs=[geometry_output],
                           name='geometry')

    # -----------
    # morphology
    # ----------

    morphology_hidden1 = Dense(units=512, activation='relu', name='m_den_1', input_dim=100)(noise_input)
    morphology_hidden1 = Dense(8*8*128, activation='relu', name='m_den_2')(morphology_hidden1)
    morphology_hidden1 = Reshape(target_shape=(8, 8, 128), name='m_reshape')(morphology_hidden1)

    morphology_hidden2 = Deconv2D(64,kernel_size=(3, 3), padding='same', strides=(2,2), name='m_deconv_1', data_format='channels_last', kernel_initializer='uniform')(morphology_hidden1)
    # morphology_hidden2 = UpSampling2D(size=(2, 2), name='m_up_1')(morphology_hidden1)
    # morphology_hidden2 = Conv2D(64, kernel_size=(5, 5), padding='same',name='m_conv_1',data_format='channels_last')(morphology_hidden2)
    morphology_hidden2 = BatchNormalization(name='m_bn_1')(Activation(activation='relu')(morphology_hidden2))

    morphology_hidden3 = Deconv2D(32, kernel_size=(3, 3), strides=(2, 2), padding='same', name='m_deconv_2', data_format='channels_last', kernel_initializer='uniform')(morphology_hidden2)
    # morphology_hidden3 = UpSampling2D(size=(2, 2), name='m_up_2')(morphology_hidden2)
    # morphology_hidden3 = Conv2D(32, kernel_size=(5, 5), padding='same', name='m_conv_2')(morphology_hidden3)
    morphology_hidden3 = BatchNormalization(name='m_bn_2')(Activation(activation='relu')(morphology_hidden3))

    morphology_hidden4 = Deconv2D(16, kernel_size=(3, 3), strides=(2, 2), padding='same', name='m_deconv_3', data_format='channels_last', kernel_initializer='uniform')(morphology_hidden3)
    # morphology_hidden4 = UpSampling2D(size=(2, 2), name='m_up_3')(morphology_hidden3)
    # morphology_hidden4 = Conv2D(16, kernel_size=(5, 5), padding='same', name='m_conv_3')(morphology_hidden4)
    morphology_hidden4 = BatchNormalization(name='m_bn_3')(Activation(activation='relu')(morphology_hidden4))

    morphology_hidden5 = Deconv2D(8, kernel_size=(3, 3), strides=(2, 2), padding='same', name='m_deconv_4', data_format='channels_last', kernel_initializer='uniform')(morphology_hidden4)
    # morphology_hidden5 = UpSampling2D(size=(2, 2), name='m_up_4')(morphology_hidden4)
    # morphology_hidden5 = Conv2D(8, kernel_size=(5, 5), padding='same', name='m_conv_4')(morphology_hidden5)
    morphology_hidden5 = BatchNormalization(name='m_bn_4')(Activation(activation='relu')(morphology_hidden5))

    morphology_hidden6 = Deconv2D(1, kernel_size=(3, 3), strides=(2, 2), padding='same', name='m_deconv_5', data_format='channels_last', kernel_initializer='uniform')(morphology_hidden5)
    # morphology_hidden6 = UpSampling2D(size=(2, 2), name='m_up_5')(morphology_hidden5)
    # morphology_hidden6 = Conv2D(1, kernel_size=(5, 5), padding='same', name='m_conv_5')(
    #     morphology_hidden6)
    morphology_hidden6 = BatchNormalization(name='m_bn_5')(Activation(activation='sigmoid')(morphology_hidden6))

    morphology_reshape = Reshape(target_shape=(n_nodes, -1))(morphology_hidden6)


    lambda_args={'n_nodes':n_nodes, 'batch_size':batch_size}

    morphology_output =Lambda(layers.masked_softmax,
                              output_shape=(n_nodes, n_nodes),
                              arguments=lambda_args)(morphology_reshape)  # input mismatch because softmax output is n_nodes-1,n_nodes
    morphology_model = Model(inputs=[noise_input],
                             outputs=[morphology_output],
                             name='morphology')
    geometry_model.summary()  # show geometry network structure
    morphology_model.summary()  # show morphology network structure

    return geometry_model, morphology_model


def discriminator(n_nodes=256, batch_size=32):
    geometry_input = Input(shape=(n_nodes, 3))
    morphology_input = Input(shape=(n_nodes,n_nodes))
    lambda_args = {'n_nodes': n_nodes, 'batch_size': batch_size}
    n_features = 5*n_nodes+3  # feature embedding size
    both_inputs = merge.concatenate([geometry_input, morphology_input], axis=2)

    embedding = Lambda(layers.feature_extractor, output_shape=(n_nodes, n_features),
                       arguments=lambda_args)(both_inputs)
    embedding = Reshape(target_shape=(1, n_nodes*n_features))(embedding)

    # print("--------discriminator model---------")
    # discriminator_hidden1 = Dense(256)(embedding)
    # lrelu(alpha=alpha)(discriminator_hidden1)
    discriminator_hidden1 = Dense(128, activation='tanh')(embedding)
    # discriminator_hidden2 = lrelu(alpha=alpha)(discriminator_hidden1)
    discriminator_hidden3 = Dense(64, activation='tanh')(discriminator_hidden1)
    discriminator_hidden4 = Dense(32, activation='tanh')(discriminator_hidden3)
    discriminator_hidden4 = Dense(16, activation='tanh')(discriminator_hidden4)
    # discriminator_hidden5 = lrelu(alpha=alpha)(discriminator_hidden4)
    discriminator_hidden6 = Dense(1, activation='sigmoid')(discriminator_hidden4)
    discriminator_model = Model(inputs=[geometry_input, morphology_input], outputs=[discriminator_hidden6], name='discriminator')
    discriminator_model.summary()
    return discriminator_model

# def discriminator(n_nodes=256, batch_size=4):
#     geometry_input = Input(shape=(n_nodes, 3))
#     morphology_input = Input(shape=(n_nodes, n_nodes))
#     lambda_args={'n_nodes':n_nodes,'batch_size':batch_size}
#     both_inputs = merge.concatenate([geometry_input,morphology_input],axis=2)
    

def discriminator_on_generator(geometry_model, morphology_model,discriminator_model, input_dim=100):
    # Inputs
    noise_input = Input(shape=(1, input_dim), name='noise_input')
    geometry_output = geometry_model([noise_input])  # n_node,3
    morphology_output = morphology_model([noise_input])   # n_nodes,n_nodes
    discriminator_output = discriminator_model([geometry_output, morphology_output])
    model = Model([noise_input], [discriminator_output])
    return model
