from cnn_model import CNN_Model
from data import DataLoader
import tensorflow as tf
import os
import time

TESTDIR = "C:/Users/Administrator/Desktop/neuron_factor_tree_test/test_1"

def train():

    log_dir = TESTDIR + "/log"
    save_dir = TESTDIR + "/save"
    data_dir = TESTDIR + "/data"

    # feature_size = 21
    feature_size = 19
    sequence_size = 10
    num_classses = 9

    batch_size = 1000
    num_epoches = 10000
    dropout_keep_prob = 0.5
    l2_reg_lambda = 0.01
    learning_rate = 1e-4
    save_every = 100  # save every 100 batches.

    data_loader = DataLoader(data_dir=data_dir, batch_size=batch_size,
                             sequence_size=sequence_size, feature_size=feature_size, mode="train")
    model = CNN_Model(num_classes=num_classses, filter_sizes=[3, 4, 5], sequence_size=sequence_size,
                  feature_size=feature_size, num_filters=20)

    with tf.Session() as sess:
        summaries = tf.summary.merge_all()
        writer = tf.summary.FileWriter(os.path.join(log_dir, time.strftime("%Y-%m-%d-%H-%M-%S")))
        writer.add_graph(sess.graph)

        sess.run(tf.global_variables_initializer())
        saver = tf.train.Saver(tf.global_variables())

        # ckpt = tf.train.latest_checkpoint(save_dir)
        # if ckpt is not None:
        #     saver.restore(sess, ckpt)
        for e in range(num_epoches):
            sess.run(tf.assign(model.lr,
                               learning_rate))
            sess.run(tf.assign(model.dropout_keep_prob, dropout_keep_prob))
            sess.run(tf.assign(model.l2_reg_lambda, l2_reg_lambda))
            data_loader.reset_batch()
            for b in range(data_loader.num_batches):
                start = time.time()
                x, y = data_loader.next_batch()
                feed = {model.input_x : x, model.input_y : y}

                summ, loss, accuracy, _ = sess.run([summaries, model.loss, model.accuracy, model.train_op], feed)
                writer.add_summary(summ, e * data_loader.num_batches + b)

                end = time.time()
                print("{}/{} (epoch {}), train_loss = {:.3f}, accuracy = {:.3f}, time/batch = {:.3f}"
                          .format(e * data_loader.num_batches + b,
                                  num_epoches * data_loader.num_batches,
                                  e, loss, accuracy, end - start))
                if (e * data_loader.num_batches + b) % save_every == 0 \
                        or (e == num_epoches - 1 and
                            b == data_loader.num_batches - 1):
                    # save for the last result
                    checkpoint_path = os.path.join(save_dir, 'model.ckpt')
                    saver.save(sess, checkpoint_path,
                               global_step=e * data_loader.num_batches + b)
                    print("model saved to {}".format(checkpoint_path))


if __name__ == '__main__':
    train()