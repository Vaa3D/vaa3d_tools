import tensorflow as tf
import numpy as np
from branch_code_data import BranchCodeLoader
import vae_
import os
import pickle

'''
From a BranchCode(STEMSIZE * 3) generator a branch code.
'''
# test_data = "C:/Users/Administrator/Desktop/TestVirtualPCNeuron/branch_data"
# test_data = "C:/Users/Administrator/Desktop/TestVirtualPC_N/branch_data"
# save_dir = "C:/Users/Administrator/Desktop/TestVirtualPC_N/vae_save"
# gen_data = "C:/Users/Administrator/Desktop/TestVirtualPC_N/generation"

# test_data = "C:/Users/Administrator/Desktop/TestVirtualPCNeuron/branch_data_fine"
# save_dir = "C:/Users/Administrator/Desktop/TestVirtualPCNeuron/vae_save"
# gen_data = "C:/Users/Administrator/Desktop/TestVirtualPCNeuron/generation"

# test_data = "C:/Users/Administrator/Desktop/TestFullNeuron/branch_data"
# save_dir = "C:/Users/Administrator/Desktop/TestFullNeuron/vae_save"
# gen_data = "C:/Users/Administrator/Desktop/TestFullNeuron/generation"

test_data = "C:/Users/Administrator/Desktop/TestVirtualPCNeuron/branch_data_n"
save_dir = "C:/Users/Administrator/Desktop/TestVirtualPCNeuron/vae_save_n"
gen_data = "C:/Users/Administrator/Desktop/TestVirtualPCNeuron/generation_n"

# data_size = 40*3
data_size = 20*3
batch_size = 100

n_hidden = 100  # num of hidden units in mlp.
dim_z = 20  # dim of latent.

def generate(x_test, min_max_scaler):

    x = tf.placeholder(tf.float32, shape=[None, 20, 3, 1], name='input')

    # x = tf.placeholder(tf.float32, shape=[None, data_size], name='input')

    keep_prob = tf.placeholder(tf.float32, name='keep_prob')

    y, z, loss, neg_marginal_likelihood, KL_divergence = vae_.autoencoder(x, data_size, dim_z, n_hidden,
                                                                         keep_prob, annealing_weight=1.0)

    with tf.Session() as sess:

        tf.global_variables_initializer().run()
        saver = tf.train.Saver(tf.global_variables())
        ckpt = tf.train.get_checkpoint_state(save_dir)
        if ckpt:
            print("restore VAE.")
            saver.restore(sess, ckpt.model_checkpoint_path)


        y_test = sess.run(y, feed_dict={x: x_test, keep_prob: 1})

        for i in range(50):
            compare_file = gen_data + "/com_" + str(i+1) + ".br"

            # print(len(x_test[20]))
            x_arr = np.array(x_test[i])
            # x_arr = x_arr * min_max_scaler.data_range_ + min_max_scaler.data_min_
            x_arr = np.reshape(x_arr, [-1, 3])
            # print(x_arr)
            x_arr = min_max_scaler.inverse_transform(x_arr)
            # x_arr = np.reshape(x_arr, [40, 3])
            print(x_arr)
            np.savetxt(compare_file, x_arr)

            generate_file = gen_data + "/ger_" + str(i+1) + ".br"


            y_arr = np.array(y_test[i])
            # y_arr = y_arr * min_max_scaler.data_range_ + min_max_scaler.data_min_
            y_arr = np.reshape(y_arr, [-1, 3])
            y_arr = min_max_scaler.inverse_transform(y_arr)
            # y_arr = np.reshape(y_arr, [40, 3])
            print(y_arr)
            np.savetxt(generate_file, y_arr)




if __name__ == '__main__':

    data_loader = BranchCodeLoader(test_data, batch_size)
    # x_test = data_loader.test_data
    x_test = data_loader.get_test()

    scaler_file = os.path.join(test_data, "scaler.txt")

    with open(scaler_file, "rb") as f:
        min_max_scaler = pickle.load(f)
    print("Scaler file loaded from [%s]." % scaler_file)

    generate(x_test, min_max_scaler)