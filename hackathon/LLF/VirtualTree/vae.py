import tensorflow as tf
import math

# Gaussian MLP as encoder
def gaussian_MLP_encoder(x, n_hidden, n_output, keep_prob):
    with tf.variable_scope("gaussian_MLP_encoder"):
        # input_size:[?, 20, 3, 1].
        # initializers
        w_init = tf.contrib.layers.variance_scaling_initializer()
        b_init = tf.constant_initializer(0.)
        # Convolution layer
        filter_shape = [3, 1, 1, 8]
        CW_0 = tf.get_variable('CW_0', filter_shape, initializer=w_init)
        # W->[filter_size, feature_size, input_channels, output_channels]
        CB_0 = tf.get_variable('CB_0', shape=[8], initializer = b_init)
        conv = tf.nn.conv2d(
            x, CW_0, strides=[1, 1, 1, 1], padding="SAME", name="conv_0")
        # conv : [None, 20, 3, 8]
        h = tf.nn.relu(tf.nn.bias_add(conv, CB_0), name="ReLu")

        filter_shape = [3, 3, 8, 32]
        CW_1 = tf.get_variable('CW_1', filter_shape, initializer=w_init)
        CB_1 = tf.get_variable('CB_1', shape=[32], initializer = b_init)
        conv = tf.nn.conv2d(
            h, CW_1, strides=[1, 1, 3, 1], padding="SAME", name="conv_1")
        # conv : [None, 20, 3, 16]
        # print(conv.shape)
        h = tf.nn.relu(tf.nn.bias_add(conv, CB_1), name="ReLu")

        # filter_shape = [3, 3, 16, 32]
        # CW_2 = tf.get_variable('CW_2', filter_shape, initializer=w_init)
        # CB_2 = tf.get_variable('CB_2', shape=[32], initializer=b_init)
        # conv = tf.nn.conv2d(
        #     h, CW_2, strides=[1, 1, 3, 1], padding="SAME", name="conv_2")
        # # print(conv.shape)
        # # conv : [None, 20, 1, 32]
        #
        # h = tf.nn.relu(tf.nn.bias_add(conv, CB_2), name="ReLu")

        h_squeeze = tf.squeeze(h, [2])
        # h_squeeze->[batch_size, seq_size-filter_size+1, num_filters]
        # h_squeeze : [None, 18, 32]

        # combine pooled features
        flat = tf.reshape(h_squeeze, [-1, 20*32])

        # 1st hidden layer
        w0 = tf.get_variable('w0', [flat.get_shape()[1], n_hidden], initializer=w_init)
        b0 = tf.get_variable('b0', [n_hidden], initializer=b_init)
        h0 = tf.matmul(flat, w0) + b0
        h0 = tf.nn.elu(h0)
        h0 = tf.nn.dropout(h0, keep_prob)

        # # # 2nd hidden layer
        # w1 = tf.get_variable('w1', [h0.get_shape()[1], n_hidden], initializer=w_init)
        # b1 = tf.get_variable('b1', [n_hidden], initializer=b_init)
        # h1 = tf.matmul(h0, w1) + b1
        # h1 = tf.nn.tanh(h1)
        # h1 = tf.nn.dropout(h1, keep_prob)

        # output layer
        wo = tf.get_variable('wo', [h0.get_shape()[1], n_output * 2], initializer=w_init)
        bo = tf.get_variable('bo', [n_output * 2], initializer=b_init)
        gaussian_params = tf.matmul(h0, wo) + bo

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
        # w1 = tf.get_variable('w1', [h0.get_shape()[1], n_hidden], initializer=w_init)
        # b1 = tf.get_variable('b1', [n_hidden], initializer=b_init)
        # h1 = tf.matmul(h0, w1) + b1
        # h1 = tf.nn.elu(h1)
        # h1 = tf.nn.dropout(h1, keep_prob)

        # 3nd hidden layer
        w2 = tf.get_variable('w2', [h0.get_shape()[1], 20*32], initializer=w_init)
        b2 = tf.get_variable('b2', [20*32], initializer=b_init)
        h2 = tf.matmul(h0, w2) + b2
        h2 = tf.nn.elu(h2)
        h2 = tf.nn.dropout(h2, keep_prob)

        h2_ = tf.reshape(h2, [-1, 20, 1, 32])

        # conv_transpose
        conv_t1 = tf.layers.conv2d_transpose(h2_, 8, kernel_size=[3, 3], strides=[1, 3], padding='SAME',
                                       kernel_initializer=w_init)
        # print(conv_t1.shape)
        # [?,20,3,16]
        y = tf.layers.conv2d_transpose(conv_t1, 1, kernel_size=[3, 1], strides=[1, 1], padding='SAME',
                                       kernel_initializer=w_init)
        # print(conv_t2.shape)
        # [?,20,3,8]
        # y = tf.layers.conv2d_transpose(conv_t2, 1, kernel_size=[3, 1], strides=[1, 1], padding='SAME',
        #                                      kernel_initializer=w_init)
        # print(y.shape)
        # [?,20,3,1]


        # # output layer-mean
        # wo = tf.get_variable('wo', [h1.get_shape()[1], n_output], initializer=w_init)
        # bo = tf.get_variable('bo', [n_output], initializer=b_init)
        # y = tf.sigmoid(tf.matmul(h1, wo) + bo)

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

    img_diff = tf.reduce_sum(tf.squared_difference(x, y), 1)
    img_diff = tf.reduce_mean(img_diff)

    loss = (annealing_weight * KL_divergence) + img_diff

    '''
    branch_edge_x = tf.reduce_sum(x[:, 0:60:3], 1)
    branch_edge_x_target = tf.reduce_sum(y[:, 0:60:3], 1)
    branch_edge_y = tf.reduce_sum(x[:, 1:60:3], 1)
    branch_edge_y_target = tf.reduce_sum(y[:, 1:60:3], 1)
    branch_edge_z = tf.reduce_sum(x[:, 2:60:3], 1)
    branch_edge_z_target = tf.reduce_sum(y[:, 2:60:3], 1)

    branch_edge_diff = tf.squared_difference(branch_edge_x, branch_edge_x_target) +\
                                    tf.squared_difference(branch_edge_y, branch_edge_y_target) +\
                                    tf.squared_difference(branch_edge_z, branch_edge_z_target)

    img_diff = tf.reduce_sum(tf.abs(x - y), 1)
    img_loss = img_diff

    latent_loss = 0.5 * tf.reduce_sum(tf.square(mu) + tf.square(sigma) - tf.log(1e-8 + tf.square(sigma)) - 1, 1)
    loss = tf.reduce_mean(img_loss) + tf.reduce_mean(latent_loss)

    img_loss = tf.reduce_mean(img_loss)
    latent_loss = tf.reduce_mean(latent_loss)
    '''

    return y, z, loss, img_diff, KL_divergence

def decoder(z, dim_img, n_hidden, reuse):

    y = bernoulli_MLP_decoder(z, n_hidden, dim_img, 1.0, reuse=reuse)

    return y