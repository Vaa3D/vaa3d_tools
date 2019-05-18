import tensorflow as tf

class CNN_Model(object):

    def __init__(
            self, num_classes, filter_sizes, sequence_size, feature_size, num_filters):

        self.input_x = tf.placeholder(tf.float32, [None, sequence_size, feature_size, 1], name="input_x")
        # input_x : [batch_size, seq_size, feature_size, channels]
        self.input_y = tf.placeholder(tf.float32, [None, num_classes], name="input_y")
        # input_y : [batch_size, num_classes]
        self.dropout_keep_prob = tf.Variable(0.0, trainable=False, name="dropout_prob")
        self.l2_reg_lambda = tf.Variable(0.0, trainable=False, name="l2_reg_lambda")
        self.lr = tf.Variable(1e-3, trainable=False, name="learning_rate")
        # tracking l2 loss for reg
        l2_loss = tf.constant(0.0)

        # Max-pooling
        pooled_outputs = []
        for filter_size in filter_sizes:
            with tf.name_scope("conv-maxpooling-%s" % filter_size):
                # Convolution layer
                filter_shape = [filter_size, feature_size, 1, num_filters]
                W = tf.Variable(tf.truncated_normal(filter_shape, stddev=0.1), name="W")
                # W->[filter_size, feature_size, input_channels, output_channels]
                b = tf.Variable(tf.constant(0.01, shape=[num_filters]), name="b")
                conv = tf.nn.conv2d(
                    self.input_x, W, strides=[1, 1, 1, 1], padding="VALID", name="conv")
                h = tf.nn.relu(tf.nn.bias_add(conv, b), name="ReLu")
                # h->[batch_size, seq_size-filter_size+1, 1, num_filters]
                # Max pooling
                pooled = tf.nn.max_pool(h, ksize=[1, sequence_size-filter_size+1, 1, 1],
                                        strides=[1, 1, 1, 1],
                                        padding="VALID",
                                        name="Max-pooling")
                pooled_outputs.append(pooled)
                # pooled_outputs->[batch_size, 1, 1, num_filters]

        # combine pooled features
        self.pool = tf.concat(pooled_outputs, 3)
        # pool->[batch_size, 1, 1, num_filters*len(filter_sizes)]
        self.pool_flat = tf.reshape(self.pool, [-1, num_filters * len(filter_sizes)])
        # pool_flat->[batch_size, num_filters*len(filter_sizes)]

        # dropout
        with tf.name_scope("dropout"):
            self.dropout = tf.nn.dropout(self.pool_flat, self.dropout_keep_prob)

        # output
        with tf.name_scope("output"):
            W = tf.Variable(tf.truncated_normal(shape=[num_filters * len(filter_sizes), num_classes], stddev=0.1), name="W")
            b = tf.Variable(tf.constant(0.01, shape=[num_classes]), name="b")
            l2_loss += tf.nn.l2_loss(W)
            l2_loss += tf.nn.l2_loss(b)
            self.scores = tf.nn.xw_plus_b(self.dropout, W, b, name="scores")
            self.predictions = tf.argmax(self.scores, 1, name="predictions")

        # Calculate mean cross-entropy loss
        with tf.name_scope("loss"):
            losses = tf.nn.softmax_cross_entropy_with_logits(logits=self.scores, labels=self.input_y)
            self.loss = tf.reduce_mean(losses) + self.l2_reg_lambda * l2_loss  # l2 reg

        # Accuracy
        with tf.name_scope("accuracy"):
            correct_predictions = tf.equal(self.predictions, tf.argmax(self.input_y, 1))
            self.accuracy = tf.reduce_mean(tf.cast(correct_predictions, "float"), name="accuracy")

        tvars = tf.trainable_variables()
        grads, _ = tf.clip_by_global_norm(tf.gradients(self.loss, tvars),
                                          5.0)
        with tf.name_scope('optimizer'):
            optimizer = tf.train.AdamOptimizer(self.lr)
        self.train_op = optimizer.apply_gradients(zip(grads, tvars))

        tf.summary.histogram('loss', self.loss)
        tf.summary.scalar('accuracy', self.accuracy)
        tf.summary.scalar('loss', self.loss)
