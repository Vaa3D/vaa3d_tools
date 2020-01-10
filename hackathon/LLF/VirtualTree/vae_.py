import tensorflow as tf
import numpy as np

# Gaussian MLP as encoder
def gaussian_MLP_encoder(x, n_hidden, n_output, keep_prob):
    with tf.variable_scope("gaussian_MLP_encoder"):
        # input_size:[?, 20, 3, 1].
        # initializers
        w_init = tf.contrib.layers.variance_scaling_initializer()
        b_init = tf.constant_initializer(0.)

        # Convolution layer 1
        filter_shape = [3, 3, 1, 8]
        CW_0 = tf.get_variable('CW_0', filter_shape, initializer=w_init)
        # W->[filter_size, feature_size, input_channels, output_channels]
        CB_0 = tf.get_variable('CB_0', shape=[8], initializer = b_init)
        conv_0 = tf.nn.conv2d(
            x, CW_0, strides=[1, 1, 3, 1], padding="SAME", name="conv_0")
        # conv : [None, 20, 1, 8]
        conv_0 = tf.nn.relu(tf.nn.bias_add(conv_0, CB_0), name="ReLu")

        # Convolution layer 2
        filter_shape = [3, 1, 1, 8]
        CW_1 = tf.get_variable('CW_1', filter_shape, initializer=w_init)
        # W->[filter_size, feature_size, input_channels, output_channels]
        CB_1 = tf.get_variable('CB_1', shape=[8], initializer=b_init)
        conv_1 = tf.nn.conv2d(
            x, CW_1, strides=[1, 1, 1, 1], padding="SAME", name="conv_1")
        # conv : [None, 20, 3, 8]
        conv_1 = tf.nn.relu(tf.nn.bias_add(conv_1, CB_1), name="ReLu")

        conv = tf.concat([conv_0, conv_1], axis=2)
        # conv : [None, 20, 4, 8]

        # combine pooled features
        flat = tf.reshape(conv, [-1, 20*4*8])

        # 1st hidden layer
        w0 = tf.get_variable('w0', [flat.get_shape()[1], n_hidden], initializer=w_init)
        b0 = tf.get_variable('b0', [n_hidden], initializer=b_init)
        h0 = tf.matmul(flat, w0) + b0
        h0 = tf.nn.elu(h0)
        h0 = tf.nn.dropout(h0, keep_prob)

        # # 2nd hidden layer
        w1 = tf.get_variable('w1', [h0.get_shape()[1], n_hidden], initializer=w_init)
        b1 = tf.get_variable('b1', [n_hidden], initializer=b_init)
        h1 = tf.matmul(h0, w1) + b1
        h1 = tf.nn.tanh(h1)
        h1 = tf.nn.dropout(h1, keep_prob)

        # output layer
        wo = tf.get_variable('wo', [h1.get_shape()[1], n_output * 2], initializer=w_init)
        bo = tf.get_variable('bo', [n_output * 2], initializer=b_init)
        gaussian_params = tf.matmul(h1, wo) + bo

        # The mean parameter is unconstrained
        mean = gaussian_params[:, :n_output]
        # The standard deviation must be positive. Parametrize with a softplus and
        # add a small epsilon for numerical stability
        stddev = 1e-6 + tf.nn.softplus(gaussian_params[:, n_output:])

    return mean, stddev

# Bernoulli MLP as decoder
def bernoulli_MLP_decoder(z, n_hidden, n_output, keep_prob, reuse=False):

    with tf.variable_scope("bernoulli_MLP_decoder", reuse=reuse):
        # initializers
        w_init = tf.contrib.layers.variance_scaling_initializer()
        b_init = tf.constant_initializer(0.)

        # 1st hidden layer
        w0 = tf.get_variable('w0', [z.get_shape()[1], n_hidden], initializer=w_init)
        b0 = tf.get_variable('b0', [n_hidden], initializer=b_init)
        h0 = tf.matmul(z, w0) + b0
        h0 = tf.nn.tanh(h0)
        h0 = tf.nn.dropout(h0, keep_prob)

        # # # 2nd hidden layer
        w1 = tf.get_variable('w1', [h0.get_shape()[1], n_hidden], initializer=w_init)
        b1 = tf.get_variable('b1', [n_hidden], initializer=b_init)
        h1 = tf.matmul(h0, w1) + b1
        h1 = tf.nn.elu(h1)
        h1 = tf.nn.dropout(h1, keep_prob)

        # 3nd hidden layer
        w2 = tf.get_variable('w2', [h1.get_shape()[1], 20*32], initializer=w_init)
        b2 = tf.get_variable('b2', [20*32], initializer=b_init)
        h2 = tf.matmul(h1, w2) + b2
        h2 = tf.nn.elu(h2)
        h2 = tf.nn.dropout(h2, keep_prob)

        h2_ = tf.reshape(h2, [-1, 20, 4, 8])

        h2_0 = h2_[:,:,0,:]
        h2_0 = h2_0[:, :, np.newaxis, :]
        # h2_0 : [None, 20, 1, 8].

        h2_1 = h2_[:,:,1:,:]
        # h2_1 : [None, 20, 3, 8].

        # conv_transpose
        ct_0 = tf.layers.conv2d_transpose(h2_0, 1, kernel_size=[3, 3], strides=[1, 3], padding='SAME',
                                       kernel_initializer=w_init)
        ct_1 = tf.layers.conv2d_transpose(h2_1, 1, kernel_size=[3, 1], strides=[1, 1], padding='SAME',
                                       kernel_initializer=w_init)
        y = tf.reduce_mean([ct_0, ct_1], axis=0)

    return y

# Gateway
def autoencoder(x, dim_img, dim_z, n_hidden, keep_prob, annealing_weight):

    # encoding
    mu, sigma = gaussian_MLP_encoder(x, n_hidden, dim_z, keep_prob)

    # sampling by re-parameterization technique
    z = mu + sigma * tf.random_normal(tf.shape(mu), 0, 1, dtype=tf.float32)

    # decoding
    y = bernoulli_MLP_decoder(z, n_hidden, dim_img, keep_prob)
    y = tf.clip_by_value(y, 1e-8, 1 - 1e-8)

    # loss
    KL_divergence = 0.5 * tf.reduce_sum(tf.square(mu) + tf.square(sigma) - tf.log(1e-8 + tf.square(sigma)) - 1, 1)
    KL_divergence = tf.reduce_mean(KL_divergence)

    img_diff = tf.squared_difference(x, y)
    img_loss = tf.reduce_sum(img_diff)

    loss = (annealing_weight * KL_divergence) + img_loss

    return y, z, loss, img_loss, KL_divergence

def decoder(z, dim_img, n_hidden, reuse):

    y = bernoulli_MLP_decoder(z, n_hidden, dim_img, 1.0, reuse=reuse)

    return y