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
    geometry_hidden3 = Deconv2D(32, kernel_size=(3, 3), strides=(2, 2), name='g_deconv_1', padding='same', data_format='channels_last', kernel_initializer='normal')(geometry_hidden2_reshape)
    geometry_hidden3 = Activation(activation='tanh')(BatchNormalization(name='g_bn_1')(geometry_hidden3))

    geometry_hidden4 = Deconv2D(3, kernel_size=(3, 3), strides=(2, 2), name='g_deconv_2', padding='same', data_format='channels_last', kernel_initializer='normal')(geometry_hidden3)
    geometry_hidden4 = Activation(activation='tanh')(BatchNormalization(name='g_bn_2')(geometry_hidden4))

    geometry_hidden_reshape = Reshape(target_shape=(n_nodes, 3))(geometry_hidden4)
    geometry_output = geometry_hidden_reshape
    geometry_model = Model(inputs=[noise_input],
                           outputs=[geometry_output],
                           name='geometry')

    # -----------
    # morphology
    # ----------

    morphology_hidden1 = Dense(units=512, activation='relu', name='m_den_1', input_dim=100)(noise_input)
    morphology_hidden1 = Dense(8*8*64, activation='relu', name='m_den_2')(morphology_hidden1)
    morphology_hidden1 = Reshape(target_shape=(8, 8, 64), name='m_reshape')(morphology_hidden1)

    morphology_hidden2 = Deconv2D(64, kernel_size=(3, 3), padding='same', strides=(2, 2), name='m_deconv_1', data_format='channels_last', kernel_initializer='uniform')(morphology_hidden1)
    morphology_hidden2 = Activation(activation='relu')(BatchNormalization(name='m_bn_1')(morphology_hidden2))

    morphology_hidden3 = Deconv2D(32, kernel_size=(3, 3), strides=(2, 2), padding='same', name='m_deconv_2', data_format='channels_last', kernel_initializer='uniform')(morphology_hidden2)
    morphology_hidden3 = Activation(activation='relu')(BatchNormalization(name='m_bn_2')(morphology_hidden3))

    morphology_hidden4 = Deconv2D(16, kernel_size=(3, 3), strides=(2, 2), padding='same', name='m_deconv_3', data_format='channels_last', kernel_initializer='uniform')(morphology_hidden3)
    morphology_hidden4 = Activation(activation='relu')(BatchNormalization(name='m_bn_3')(morphology_hidden4))

    morphology_hidden5 = Deconv2D(8, kernel_size=(3, 3), strides=(2, 2), padding='same', name='m_deconv_4', data_format='channels_last', kernel_initializer='uniform')(morphology_hidden4)
    morphology_hidden5 = Activation(activation='relu')(BatchNormalization(name='m_bn_4')(morphology_hidden5))

    morphology_hidden6 = Deconv2D(1, kernel_size=(3, 3), strides=(2, 2), padding='same', name='m_deconv_5', data_format='channels_last', kernel_initializer='uniform')(morphology_hidden5)
    morphology_hidden6 = Activation(activation='sigmoid')(BatchNormalization(name='m_bn_5')(morphology_hidden6))

    morphology_reshape = Reshape(target_shape=(n_nodes, -1))(morphology_hidden6)

    lambda_args = {'n_nodes': n_nodes, 'batch_size': batch_size}

    morphology_output = Lambda(layers.masked_softmax,
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
    morphology_input = Input(shape=(n_nodes, n_nodes))
    lambda_args = {'n_nodes': n_nodes, 'batch_size': batch_size}
    n_features = 5*n_nodes+3  # feature embedding size
    both_inputs = merge.concatenate([geometry_input, morphology_input], axis=2)

    embedding = Lambda(layers.feature_extractor, output_shape=(n_nodes, n_features),
                       arguments=lambda_args)(both_inputs)
    embedding = Reshape(target_shape=(16, -1, n_features))(embedding)
    discriminator_hidden1 = Conv2D(64, kernel_size=(3, 3), padding='same', strides=(1, 1), name='d_conv_1', data_format='channels_last')(embedding)
    # discriminator_hidden1 = Dense(1024, activation='tanh', name='d_dense_1')(embedding)
    # discriminator_hidden1 = Dense(1024, activation='tanh', name='d_dense_2')(discriminator_hidden1)

    discriminator_hidden2 = Conv2D(64, kernel_size=(3, 3), padding='same', strides=(2, 2), name='d_conv_2', data_format='channels_last')(discriminator_hidden1)
    discriminator_hidden2 = Activation(activation='tanh')(BatchNormalization(name='d_bn_1')(discriminator_hidden2))

    discriminator_hidden3 = Conv2D(128, kernel_size=(3, 3), padding='same', strides=(2, 2), name='d_conv_3', data_format='channels_last')(discriminator_hidden2)
    discriminator_hidden3 = Activation(activation='tanh')(BatchNormalization(name='d_bn_2')(discriminator_hidden3))

    # discriminator_hidden4 = Conv2D(128, kernel_size=(3, 3), padding='same', strides=(2, 2), name='d_conv_2', data_format='channels_last')(discriminator_hidden3)
    # discriminator_hidden4 = (Activation(activation='tanh')(BatchNormalization(name='d_bn_2')(discriminator_hidden4)))

    # discriminator_hidden5 = Conv2D(16, kernel_size=(3, 3), padding='same', strides=(2, 2), name='d_conv_3', data_format='channels_last')(discriminator_hidden4)
    # discriminator_hidden5 = BatchNormalization(name='d_bn_3')(Activation(activation='tanh')(discriminator_hidden5))

    discriminator_hidden5 = Reshape(target_shape=(1, -1))(discriminator_hidden3)
    discriminator_hidden6 = Dense(32, activation='sigmoid', name='d_dense_3')(discriminator_hidden5)
    discriminator_hidden6 = Dense(1, activation='sigmoid', name='d_dense_4')(discriminator_hidden6)
    discriminator_model = Model(inputs=[geometry_input, morphology_input], outputs=[discriminator_hidden6], name='discriminator')
    discriminator_model.summary()
    return discriminator_model


def discriminator_on_generator(geometry_model, morphology_model,discriminator_model, input_dim=100):
    # Inputs
    noise_input = Input(shape=(1, input_dim), name='noise_input')
    geometry_output = geometry_model([noise_input])  # n_node,3
    morphology_output = morphology_model([noise_input])   # n_nodes,n_nodes
    discriminator_output = discriminator_model([geometry_output, morphology_output])
    model = Model([noise_input], [discriminator_output])
    return model

