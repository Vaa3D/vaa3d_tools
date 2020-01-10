import tensorflow as tf
import numpy as np
from branch_code_data import BranchCodeLoader
import vae_
import os

# test_data = "C:/Users/Administrator/Desktop/neuron_factor_tree_test/test/data/L4_BC/data"
# save_dir = "C:/Users/Administrator/Desktop/neuron_factor_tree_test/test/data/L4_BC/data/vae_save"
# test_data = "C:/Users/Administrator/Desktop/TestVirtualPCNeuron/branch_data_fine"
# save_dir = "C:/Users/Administrator/Desktop/TestVirtualPCNeuron/vae_save"
# test_data = "C:/Users/Administrator/Desktop/TestVirtualPC_N/branch_data"
# save_dir = "C:/Users/Administrator/Desktop/TestVirtualPC_N/vae_save"
test_data = "C:/Users/Administrator/Desktop/TestFullNeuron/branch_data"
save_dir = "C:/Users/Administrator/Desktop/TestFullNeuron/vae_save"
# test_data = "C:/Users/Administrator/Desktop/TestVirtualPCNeuron/branch_data_n"
# save_dir = "C:/Users/Administrator/Desktop/TestVirtualPCNeuron/vae_save_n"

data_size = 20*3
batch_size = 100

n_hidden = 100  # num of hidden units in mlp.
dim_z = 20  # dim of latent.

learn_rate = 1e-4
num_epoches = 1000

annealing_weight_step = 1e-4

def train():
    data_loader = BranchCodeLoader(test_data, batch_size)
    # x_test = data_loader.test_data

    # input placeholders
    # In denoising-autoencoder, x_hat == x + noise, otherwise x_hat == x
    # x = tf.placeholder(tf.float32, shape=[None, data_size], name='input')
    x = tf.placeholder(tf.float32, shape=[None, 20, 3, 1], name='input')

    annealing_weight = tf.placeholder(tf.float32, name='annealing_weight')

    # dropout
    keep_prob = tf.placeholder(tf.float32, name='keep_prob')

    # # input for PMLR
    # z_in = tf.placeholder(tf.float32, shape=[None, dim_z], name='latent_variable')

    # network architecture
    y, z, loss, img_loss, latent_loss = vae_.autoencoder(x, data_size, dim_z, n_hidden, keep_prob,
                                                        annealing_weight)

    # optimization
    train_op = tf.train.AdamOptimizer(learn_rate).minimize(loss)

    # train
    total_batch = data_loader.num_batches
    min_tot_loss = 1e99

    with tf.Session() as sess:

        sess.run(tf.global_variables_initializer(), feed_dict={keep_prob: 0.9})
        saver = tf.train.Saver(tf.global_variables())

        ckpt = tf.train.latest_checkpoint(save_dir)

        if ckpt is not None:
            saver.restore(sess, ckpt)
            print("VAE Model restore from[{}]".format(ckpt))

        for epoch in range(num_epoches):

            # Random shuffling
            np.random.shuffle(data_loader.data)

            data_loader.create_batches()
            data_loader.reset_batch()

            # Loop over all batches
            for i in range(total_batch):

                batch_xs_input = data_loader.next_batch_4()

                _, tot_loss, recon_loss, latent_difference, y_value = sess.run(
                    (train_op, loss, img_loss, latent_loss, y),
                    feed_dict={x: batch_xs_input, keep_prob: 0.9,
                               annealing_weight : float(epoch)*annealing_weight_step})
            # print(y_value)
            # print cost every epoch
            print("epoch %d: Loss_tot %03.2f recon_loss %03.2f divergence %03.2f" % (
                epoch, tot_loss, recon_loss, latent_difference))
            x_test = data_loader.get_test()
            test_loss = sess.run(( img_loss),
                feed_dict={x: x_test, keep_prob: 1.0})

            print("TEST loss : Loss_tot %03.2f " % (test_loss))

            # if minimum loss is updated or final epoch, plot results
            if min_tot_loss >= tot_loss:
                min_tot_loss = tot_loss

                checkpoint_path = os.path.join(save_dir, 'model.ckpt')
                # save for the last result
                saver.save(sess, checkpoint_path,
                           global_step=epoch * data_loader.num_batches)
                print("model saved to {}".format(checkpoint_path))


if __name__ == '__main__':
    train()