import numpy as np
import tensorflow as tf
from tensorflow.contrib import rnn
from tensorflow.contrib import legacy_seq2seq
from tensorflow.python.ops.rnn import dynamic_rnn

class RNN_Model(object):
    def __init__(self, sequence_size, hidden_size, num_layers, factorDB_size, training):
        self.input_x = tf.placeholder(tf.int32, [None, sequence_size], name="input_x")
        # input_x : [batch_size, seq_size]
        self.targets = tf.placeholder(tf.int32, [None, sequence_size], name="targets")
        # targets : [batch_size, seq_size]
        # maybe don't need seq_len.
        # self.input_seq_length = tf.placeholder(dtype=tf.int32, shape=[None],
        #                                        name='input_seq_length')  # record each seqs' size
        self.hidden_size = hidden_size
        self.factorDB_size = factorDB_size
        self.batch_size = tf.placeholder(dtype=tf.int32, shape=[], name='batch_size')
        self.dropout_keep_prob = tf.Variable(0.0, trainable=False, name="dropout_prob")
        self.lr = tf.Variable(1e-3, trainable=False, name="learning_rate")

        # LSTM cell
        cell = rnn.LSTMCell(self.hidden_size)
        # add dropout
        cell = rnn.DropoutWrapper(cell, self.dropout_keep_prob)

        # stack cells.
        self.cells = rnn.MultiRNNCell([cell] * num_layers, state_is_tuple=True)

        self.initial_state = self.cells.zero_state(self.batch_size, dtype=tf.float32)

        with tf.name_scope("embedding"):
            embedding = tf.get_variable("embedding", [self.factorDB_size, self.hidden_size])
            inputs = tf.nn.embedding_lookup(embedding, self.input_x)
            #  inputs->[batch_size, seq_size, hidden_size]
            inputs = tf.split(inputs, sequence_size, 1)
            #  inputs->seq_size*[batch_size, 1, hidden_size] "split into steps."
            inputs = [tf.squeeze(input_, [1]) for input_ in inputs]
            #  inputs->seq_size*[batch_size, hidden_size] "squeeze."

        with tf.name_scope("LSTM"):
            softmax_w = tf.get_variable("softmax_w", shape=[self.hidden_size, self.factorDB_size])
            softmax_b = tf.get_variable("softmax_b", shape=[self.factorDB_size])

            def loop(prev,_):
                """
                loop func generate i+1-th input by i-th output.
                loop func setting:
                :param prev: [batch_size * hidden_size]
                :return: [batch_size * hidden_size]
                """
                prev = tf.matmul(prev, softmax_w) + softmax_b
                prev_symbol = tf.stop_gradient(tf.argmax(prev, 1))
                return tf.nn.embedding_lookup(embedding, prev_symbol)

            # rnn outputs
            outputs, last_state = legacy_seq2seq.rnn_decoder(inputs, self.initial_state, self.cells,
                                                             loop_function=loop if not training else None)
            #  outputs->seq_size*[batch_size, hidden_size]
            outputs = tf.reshape(tf.concat(outputs, 1), [-1, self.hidden_size])
            #  outputs->[batch_size, (hidden_size*seq_size)]
            #         ->[batch_size*seq_size, hidden_size]

            # logits
            self.logits = tf.nn.xw_plus_b(outputs, softmax_w, softmax_b, name="logits")
            self.probs = tf.nn.softmax(self.logits)
            self.predictions = tf.argmax(self.logits, 1, name="predictions")  # label
            #  [batch_size*seq_size,factorDB_size]

        # loss
        with tf.name_scope("cost"):
            self.loss = legacy_seq2seq.sequence_loss_by_example(
                [self.logits],
                [tf.reshape(self.targets, [-1])],  # [batch_size*seq_size]
                [tf.ones([self.batch_size*sequence_size])])

            self.cost = tf.reduce_sum(self.loss)/tf.cast(self.batch_size, tf.float32) /sequence_size

        self.final_state = last_state

        tvars = tf.trainable_variables()
        grads, _ = tf.clip_by_global_norm(tf.gradients(self.loss, tvars), 5.0)  # clip gradients.

        with tf.name_scope("optimizer"):
            optimizer = tf.train.AdamOptimizer(self.lr)
        self.train_op = optimizer.apply_gradients(zip(grads, tvars))

        tf.summary.histogram("loss", self.cost)
        tf.summary.scalar("loss", self.cost)

    # generate one factor each time.
    def generate(self, sess, pre_seq, start_state=None):

        state = start_state
        seq = []  # collect all nf_id.

        #  if don't input state, first run a few header.
        if state is None:
            state = sess.run(self.cells.zero_state(1, tf.float32))
            for nf in pre_seq[:-1]:  # seq_header: a nf list.[nf_id].
                x = np.zeros((1, 1))
                x[0, 0] = nf
                feed = {self.input_x: x, self.initial_state: state}
                [state] = sess.run([self.final_state], feed)

        seq.extend(pre_seq)

        x = np.zeros((1,1))
        x[0,0] = pre_seq[-1]  # get input factor.
        if x[0, 0] >= self.factorDB_size:  # out of boundary.
            return pre_seq, None

        feed = {self.input_x: x, self.initial_state: state}
        [probs, state] = sess.run([self.probs, self.final_state], feed)
        p = probs[0] # get the probs.
        # print(p)
        # weighted pick.
        if np.argmax(p) != 0:
            weighted_pick = lambda p: int(np.searchsorted(np.cumsum(p), np.random.rand(1)*np.sum(p)))
            pre_nf = weighted_pick(p)  # get a predicted nf id.
        else:
            pre_nf = 0

        seq.append(pre_nf)

        return seq, state