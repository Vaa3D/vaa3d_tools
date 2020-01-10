import os
import tensorflow as tf
from lstm_model import RNN_Model
import pickle
import queue
import random
from branch_sequence_loader import SeqLoader
from vae_ import decoder
import numpy as np

os.environ['TF_CPP_MIN_LOG_LEVEL'] = '1'

# class_map = {"p_h_m": 0, "p_n_m": 1}
# TESTDIR = "C:/Users/Administrator/Desktop/neuron_factor_tree_test/test_generate_fm"
# TESTDIR = "C:/Users/Administrator/Desktop/neuron_factor_tree_test/test/data/L4_BC"
TESTDIR = "C:/Users/Administrator/Desktop/TestVirtualPCNeuron"
# TESTDIR = "C:/Users/Administrator/Desktop/TestMotoNeuron"
log_dir = TESTDIR + "/log"
model_save_path = TESTDIR + "/rnn_save"
data_path = TESTDIR + "/seq_data"
# generation_path = TESTDIR + "/generation_files"
generation_path = TESTDIR + "/generation/neurons"

# vae_model_save_dir = TESTDIR + "/vae_save_forRNN"
vae_model_save_dir = TESTDIR + "/vae_save"


STEM_SAMPLE_POINT_NUM = 40

latent_size = 100

seq_size = 20
model_name = "rnn"

n_hidden = 100  # num of hidden units in mlp.
dim_z = 20  # dim of latent.
data_size = STEM_SAMPLE_POINT_NUM * 3

sequence_size = 1
seq_size_lowerbound = 0
feature_size = 100

num_layers = 2
hidden_size = 10
# generating_length = 10
NUM = 100

class Factor(object):

    def __init__(self, index, id, parent):
        self.index = index  # a global index.
        self.id = id  # a id which this factor indicate.
        self.parent = parent

def read_soma_branch(path):
    stems = []
    with open(path, "r") as f:
        for line in f.readlines():
            if line.startswith("#STEMTYPE"):
                stem_type = line.split(" ")[2][:-1]  # get rid of "\n".
                stem_type = int(stem_type)
                stems.append(stem_type)
    return stems

def generate_seq(seq_type):

    print("Generating type:"+str(seq_type)+" seq begins.")

    tf.reset_default_graph()

    with tf.Session() as sess:

        model = RNN_Model(sequence_size=1, hidden_size=hidden_size, num_layers=num_layers,
                          factorDB_size=factorDB_size, training=False)

        tf.global_variables_initializer().run()
        saver = tf.train.Saver(tf.global_variables())
        ckpt = tf.train.get_checkpoint_state(model_save_path+"/"+str(seq_type))
        if ckpt :
            print("Restore type[{}] model.".format(seq_type))
            saver.restore(sess, ckpt.model_checkpoint_path)

        sess.run(tf.assign(model.dropout_keep_prob, 1))

        nf_factors = []  # collect all factors.
        nf_seqs = []  # collect embedding nf ids.
        q = queue.Queue()  # a queue for (id, state, last_index).
        index = 0  # initialize index.

        # random set a generating header.
        header, _ = model.generate(sess, pre_seq=[1], start_state=None)
        seq = [header[-1]]
        factor = Factor(index=index, id=seq[-1], parent=-1)
        nf_factors.append(factor)
        q.put((seq, None, index))
        index += 1

        # generate with layer
        while not q.empty():
            seq, state, parent_index = q.get()
            seq_1, state_1 = model.generate(sess, pre_seq=seq, start_state=state)
            seq_2, state_2 = model.generate(sess, pre_seq=seq, start_state=state)
            fa_1 = Factor(index=index, id=seq_1[-1], parent=parent_index)
            index+=1
            nf_factors.append(fa_1)
            fa_2 = Factor(index=index, id=seq_2[-1], parent=parent_index)
            index+=1
            nf_factors.append(fa_2)
            print("get seq, size:{}, {}.".format(len(seq_1), seq_1))
            print("get seq, size:{}, {}.".format(len(seq_2), seq_2))

            ran = random.uniform(0, 1)
            ter = seq_ter_prob[len(seq_1)]
            # print(ran)
            # print(ter)
            if seq_1[-1] == 0 or (ran <= ter):
            # if seq_1[-1] == 0 or len(seq_1)==20:
                seq_1.append(0)
                nf_seqs.append(seq_1)
            else:
                q.put((seq_1, state_1, fa_1.index))

            ran = random.uniform(0, 1)
            ter = seq_ter_prob[len(seq_2)]
            # ter = 0.5
            # print(ran)
            # print(ter)
            if seq_2[-1] == 0 or (ran <= ter):
            # if seq_2[-1] == 0 or len(seq_1)==20:
                seq_2.append(0)
                nf_seqs.append(seq_2)
            else:
                q.put((seq_2, state_2, fa_2.index))


    return nf_seqs, nf_factors

if __name__ == "__main__":

    # rnn_graph = tf.Graph()
    vae_graph = tf.Graph()
    # sess1 = tf.Session(graph=rnn_graph)
    sess2 = tf.Session(graph=vae_graph)

    with sess2.as_default():
        with vae_graph.as_default():
            z = tf.placeholder(tf.float32, shape=[None, dim_z], name='latent')
            y_op = decoder(z, data_size, n_hidden, False)

            tf.global_variables_initializer().run()
            saver = tf.train.Saver(tf.global_variables())
            ckpt = tf.train.get_checkpoint_state(vae_model_save_dir)
            if ckpt:
                print("VAE model restored.")
                saver.restore(sess2, ckpt.model_checkpoint_path)

    soma_branch_name = ""
    soma_branch_file = ""
    for _, _, files in os.walk(generation_path):
        for file in files:
            if os.path.splitext(file)[-1] == ".sob":
                soma_branch_name = os.path.splitext(file)[0]
                soma_branch_file = os.path.join(generation_path, file)
                print("Find soma branch file at[" + soma_branch_file + "].")

    stems = read_soma_branch(soma_branch_file)
    print("Read " + str(len(stems)) + " stems.")

    # load scaler for reverse transform.
    scaler_file = os.path.join(data_path, "scaler.txt")
    with open(scaler_file, "rb") as f:
        min_max_scaler = pickle.load(f)

    for n in range(1, NUM + 1):
        # swc_generation_file = os.path.join(generation_path, "generation-" + str(i).zfill(4) + ".gen")
        swc_generation_file = os.path.join(generation_path, soma_branch_name + "-" + str(n).zfill(4) + ".gen")
        sb_f = open(soma_branch_file)
        with open(swc_generation_file, "w") as f:
            # write header
            for line in sb_f.readlines():
                f.write(line)
        sb_f.close()
        # generate nfss.
        for i in range(1, len(stems)+1):
            # load a type of seqs' data.
            db_file = os.path.join(data_path, "factorDB_"+str(stems[i-1])+".txt")
            # Loading data.
            with open(db_file, "rb") as f:
                factors = pickle.load(f)
            factorDB_size = len(factors)
            factorDB = dict(zip(factors, range(len(factors))))
            factorDB_reverse = {v: k for k, v in factorDB.items()}

            data_loader = SeqLoader(test_path=TESTDIR, seq_size=seq_size, branch_size=STEM_SAMPLE_POINT_NUM,
                                    batch_size=1, seq_type=stems[i-1])
            seq_ter_prob = data_loader.get_seq_ter_prob()

            tf.reset_default_graph()

            seqs, facs = generate_seq(stems[i-1])
            # print(seqs)

            # tf.reset_default_graph()


            # with sess2.as_default():
            #     with vae_graph.as_default():
            #
            #         z = tf.placeholder(tf.float32, shape=[None, dim_z], name='latent')
            #         y_op = decoder(z, data_size, n_hidden, False)
            #
            #         tf.global_variables_initializer().run()
            #         saver = tf.train.Saver(tf.global_variables())
            #         ckpt = tf.train.get_checkpoint_state(vae_model_save_dir)
            #         if ckpt:
            #             print("VAE model restored.")
            #             saver.restore(sess2, ckpt.model_checkpoint_path)

            nf_infos = []
            for f in facs:
                coords_branch = []
                mu_sigma = factorDB_reverse[f.id][1:-1].split(", ")
                # print(mu_sigma)
                # print(mu_sigma[:dim_z])
                # print(mu_sigma[dim_z:])
                z_value = np.array(mu_sigma[:dim_z], dtype="float32") + \
                    np.array(mu_sigma[dim_z:2*dim_z], dtype="float32")*random.uniform(0, 1)
                coords_ = sess2.run(y_op, feed_dict={z: [z_value]})[0]
                coords = min_max_scaler.inverse_transform(np.reshape(coords_, [20, 3]))
                coords_branch.append(coords)
                z_value = np.array(mu_sigma[2*dim_z:3*dim_z], dtype="float32") + \
                    np.array(mu_sigma[3*dim_z:], dtype="float32")*random.uniform(0, 1)
                coords_ = sess2.run(y_op, feed_dict={z: [z_value]})[0]
                coords = min_max_scaler.inverse_transform(np.reshape(coords_, [20, 3]))
                coords_branch.append(coords)
                nf_infos.append((f.index, coords_branch, f.parent))

            # print(nf_infos)

            # save swc origins
            with open(swc_generation_file, "a") as f:
                #write header
                f.write("#GENSTART file format:\n#branch index, branch parent index, "
                        "branch coords.\n")
                for nf_info in nf_infos:  # for a branch's coords.
                    # print(nf_info)
                    f.write(str(nf_info[0]))
                    f.write(" "+ str(nf_info[2]))
                    [f.write(" "+ str(coord)) for coords in nf_info[1][0] for coord in coords ]
                    [f.write(" "+ str(coord)) for coords in nf_info[1][1] for coord in coords ]
                    f.write("\n")
                f.write("#GENEND.\n")