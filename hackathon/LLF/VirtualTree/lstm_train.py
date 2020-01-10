from lstm_model import RNN_Model
from branch_sequence_loader_n import SeqLoader
import tensorflow as tf
import os
import time

# TESTDIR = "C:/Users/Administrator/Desktop/neuron_factor_tree_test/test/data/L4_BC"
TESTDIR = "C:/Users/Administrator/Desktop/TestVirtualPCNeuron"
# TESTDIR = "C:/Users/Administrator/Desktop/TestMotoNeuron"
# TESTDIR = "C:/Users/Administrator/Desktop/TestFullNeuron"

log_dir = TESTDIR + "/log"
save_dir = TESTDIR + "/rnn_save"

STEM_SAMPLE_POINT_NUM = 20

dim_z = 10

br_data_size = 4 * dim_z + 3

seq_size = 20

num_layers = 2
hidden_size = 10
batch_size = 100
num_epoches = 20000
dropout_keep_prob = 0.5
learning_rate = 1e-3
save_every = 100  # save every 100 batches.

def train(seq_type):

    tf.reset_default_graph()

    data_loader = SeqLoader(test_path=TESTDIR, seq_size=seq_size, branch_size=br_data_size,
                            batch_size=batch_size, seq_type=seq_type)
    if data_loader.num_batches==0:
        return
    model = RNN_Model(factorDB_size=data_loader.factorDB_size,sequence_size=seq_size,
                            hidden_size=hidden_size, num_layers=num_layers,training=True)

    with tf.Session() as sess:
        summaries = tf.summary.merge_all()
        writer = tf.summary.FileWriter(os.path.join(log_dir, time.strftime("%Y-%m-%d-%H-%M-%S")))
        writer.add_graph(sess.graph)

        sess.run(tf.global_variables_initializer())
        saver = tf.train.Saver(tf.global_variables())

        ckpt = tf.train.latest_checkpoint(save_dir+"/"+str(seq_type))
        if ckpt is not None:
            saver.restore(sess, ckpt)
            print("RNN Model restore from[{}]".format(ckpt))

        sess.run(tf.assign(model.lr, learning_rate))
        sess.run(tf.assign(model.dropout_keep_prob, dropout_keep_prob))

        for e in range(num_epoches):

            data_loader.reset_batch()
            for b in range(data_loader.num_batches):
                start = time.time()
                x, y = data_loader.next_batch()
                feed = {model.input_x : x, model.targets : y,model.batch_size : batch_size}

                summ, cost, _ = sess.run([summaries, model.cost, model.train_op], feed)
                writer.add_summary(summ, e * data_loader.num_batches + b)

                end = time.time()
                print("{}/{} (epoch {}), train_loss = {:.3f}, time/batch = {:.3f}"
                          .format(e * data_loader.num_batches + b,
                                  num_epoches * data_loader.num_batches,
                                  e, cost, end - start))
                if (e * data_loader.num_batches + b) % save_every == 0 \
                        or (e == num_epoches - 1 and
                            b == data_loader.num_batches - 1):
                    # save for the last result
                    checkpoint_path = os.path.join(save_dir+"/"+str(seq_type), 'model.ckpt')
                    saver.save(sess, checkpoint_path,
                               global_step=e * data_loader.num_batches + b)
                    print("model saved to {}".format(checkpoint_path))

if __name__=="__main__":
    train(1)
    train(2)
    train(3)
    train(4)
