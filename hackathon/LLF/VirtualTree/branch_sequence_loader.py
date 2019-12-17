import numpy as np
import os
from sklearn.preprocessing import MinMaxScaler
import pickle
import tensorflow as tf
import vae_
from sklearn.utils import shuffle
import collections

STEM_SAMPLE_POINT_NUM = 40

data_size = 40*3
batch_size = 50

n_hidden = 100  # num of hidden units in mlp.
dim_z = 20  # dim of latent.

# data_path = "C:/Users/Administrator/Desktop/neuron_factor_tree_test/test/data/L4_BC/data"
#
# sess_save_dir = "C:/Users/Administrator/Desktop/neuron_factor_tree_test/test/data/L4_BC/data/save"
# save_dir = "C:/Users/Administrator/Desktop/neuron_factor_tree_test/test/data/L4_BC/data/save"

'''
Load both branch codes and branch id sequences.
'''

class SeqLoader():
    def __init__(self, test_path, seq_size, branch_size, batch_size, seq_type):
        self.data_path = os.path.join(test_path, "seq_data")
        self.save_path = os.path.join(test_path, "vae_save")
        self.seq_size = seq_size
        self.branch_size = branch_size
        self.batch_size = batch_size
        self.type = seq_type

        # load rnn data.
        training_data = os.path.join(self.data_path, "factorDB"+"_"+str(self.type)+".txt")
        if not (os.path.exists(training_data)):
            self.build_seq_data()
        else:
            self.load_processed(self.data_path)
        # self.build_seq_data()

        self.create_batches()
        self.reset_batch()


    # Collect all the branch codes(normalized).
    def branches_loader(self, branch_codes_file):
        branches = []
        branch = []
        with open(branch_codes_file) as f:
            for line in f.readlines():
                if line.startswith("#BRANCH:"):
                    branch = []
                elif line.startswith("#DATA:"):
                    code_str = np.array(line[:-1].split(" ")[1:4])
                    code = list(map(float, code_str))
                    branch.append(code)
                elif line.startswith("#BRANCHEND"):
                    branches.append(branch)

        # print(len(branches))
        return branches

    # Load sequence as branch ids, only load a type each time.
    def sequences_loader(self, branch_database_path):
        seqs = []
        seq = []
        current_type = 0
        with open(branch_database_path) as f:
            for line in f.readlines():
                if line.startswith("#SEQ"):
                    if current_type==self.type and len(seq) != 0:
                        seqs.append(seq)
                    seq = []
                    current_type = int(np.array(line[:-1].split(" ")[1])) # update current seq type

                elif line.startswith("#BID"):
                    b_id_str = np.array(line[:-1].split(" ")[1])
                    b_id = float(b_id_str)
                    seq.append(b_id)

        if current_type==self.type and len(seq)!=0:
            seqs.append(seq)

        # print(len(seqs))
        return seqs

    def process_branches(self, branches):

        print("Processing branches.")
        data = np.array(branches, dtype=np.float32)  # BRANCH_NUM*40*3.
        data = data.reshape(-1, 3)  # [BRANCH_NUM*40, 3].

        # load scaler_file created in branch_generator traning step.
        scaler_file = os.path.join(self.data_path, "scaler.txt")
        with open(scaler_file, "rb") as f:
            min_max_scaler = pickle.load(f)
        print("Scaler file loaded from [%s]." % scaler_file)
        self.min_max_scaler = min_max_scaler  # record the scaler.

        # Normalixation
        data = min_max_scaler.transform(data)

        # one row, 120 : x-y-z, x-y-z, ... , x-y-z.
        data = np.reshape(data, [-1, STEM_SAMPLE_POINT_NUM, 3, 1])  # BRANCH_NUM

        return data

    def build_seq_data(self):

        print("Building sequence datas.")

        branch_seq_path = os.path.join(self.data_path, "branch_dataset.brd")
        branch_code_path = os.path.join(self.data_path, "branch_code.bcd")
        # Load branches.
        branches = self.branches_loader(branch_code_path)
        print("{} branches loaded.".format(len(branches)))
        self.branches = self.process_branches(branches)
        print("Branches data : [{}]".format(self.branches.shape))  # 4743*120.

        # Load branch id sequences.
        self.sequences = self.sequences_loader(branch_seq_path)
        print("{} sequences loaded.".format(len(self.sequences)))
        if int(len(self.sequences) / self.batch_size) == 0:
            self.tensor = []
            return
        # print(self.sequences)

        # Sequence normalization.
        seq_ids = []
        for seq in self.sequences:
            seq.reverse()
            seq_n = []
            for id in [int(f) for f in seq]:
                if id!=-1: # empty branches.
                    seq_n.append(id)
            if(len(seq_n)==0):
                continue
            elif len(seq_n) < self.seq_size:
                for i in range(self.seq_size - len(seq_n)):
                    seq_n.append(-1)
            else:
                seq_n = seq_n[(len(seq_n) - self.seq_size) + 1:]
                seq_n.append(-1) # make sure last one is -1.

            seq_ids.append(seq_n)

        # print("shape")
        # print(np.array(seq_ids).shape)

        # x = tf.placeholder(tf.float32, shape=[None, data_size], name='input_img')
        x = tf.placeholder(tf.float32, shape=[None, 20, 3, 1], name='input')
        latent = vae_.gaussian_MLP_encoder(x, n_hidden, dim_z, 1.0)

        with tf.Session() as sess:

            tf.global_variables_initializer().run()
            saver = tf.train.Saver(tf.global_variables())
            ckpt = tf.train.get_checkpoint_state(self.save_path)
            if ckpt:
                print("VAE model restored for branch latent transform.")
                saver.restore(sess, ckpt.model_checkpoint_path)
            else:
                return

            latent_data = []
            for seq in seq_ids:
                seq_latents = []
                seq_latents.append(np.array([1.0 for _ in range(4*dim_z)])) # all 1.0 for soma.
                for br in seq:
                    if br != -1:
                        # _br_mean, _br_stdv = sess.run(latent, feed_dict={x_hat: [self.branches[br]]})
                        _st_mean, _st_stdv = sess.run(latent, feed_dict={x: [self.branches[br][:20]]})# stem 1
                        st_latent = np.append(_st_mean[0], _st_stdv[0])
                        _st_mean, _st_stdv = sess.run(latent, feed_dict={x: [self.branches[br][20:]]})
                        st_latent_ = np.append(_st_mean[0], _st_stdv[0])
                        br_latent = np.append(st_latent, st_latent_)
                        # print(len(br_latent))
                        # print(br_latent)
                        seq_latents.append(br_latent)
                for i in range(self.seq_size - len(seq_latents)):
                    seq_latents.append(np.array([0.0 for _ in range(4*dim_z)])) # all 0 for leaf.

                latent_data.append(seq_latents)
            # print(len(latent_data))
            # print(latent_data[0][0])
            x_data_ = np.array(latent_data, dtype=np.float32)
            print(x_data_.shape) # (data_size, seq_size, dim_z)

        print("Training data normalized.")

        x_data = shuffle(x_data_, random_state=42)

        # make up a neuronFactor-database. Each nf in DB have a id.
        factors_ = x_data.reshape(-1, 4*dim_z)  # reshape to count factors.
        factors_s = [str(f) for f in factors_.tolist()]
        counter = collections.Counter(factors_s)
        count_pairs = sorted(counter.items(), key=lambda x: -x[1])
        self.factors, _ = zip(*count_pairs)
        print(len(self.factors))
        # for f in self.factors:
        #     print(f)
        self.factorDB_size = len(self.factors)
        self.factorDB = dict(zip(self.factors, range(len(self.factors))))
        db_file = os.path.join(self.data_path, "factorDB"+"_"+str(self.type)+".txt")
        print("Got a NeuronFactor Database of size %d." % self.factorDB_size)
        tensor = np.array(list(map(self.factorDB.get, factors_s)))
        tensor_file = os.path.join(self.data_path, "tensor"+"_"+str(self.type)+"")
        self.tensor = tensor.reshape(-1, self.seq_size)  # seq_num * seq_size

        # save data.
        with open(db_file, "wb") as f:
            pickle.dump(self.factors, f)
            print("NeuronFactorDB saved at [%s]." % db_file)
        np.save(tensor_file, self.tensor)
        print("Factor tensors saved at [%s]." % tensor_file)

        print("Data preprocessed.")

    def load_processed(self, data_base_dir):
        db_file = os.path.join(data_base_dir, "factorDB_"+str(self.type)+".txt")
        tensor_file = os.path.join(data_base_dir, "tensor_"+str(self.type)+".npy")

        # Loading data.
        with open(db_file, "rb") as f:
            self.factors = pickle.load(f)
        self.factorDB_size = len(self.factors)
        self.factorDB = dict(zip(self.factors, range(len(self.factors))))
        self.tensor = np.load(tensor_file)

        print("[%d] training data loaded from [%s]." % (len(self.tensor), tensor_file))

    def create_batches(self):
        self.num_batches = int(len(self.tensor) / self.batch_size)
        if self.num_batches== 0:
            print("Don't have enough data, finish.")
            return
        # use the first self.batch_size*self.num_batches data.
        tensor = self.tensor[:self.batch_size*self.num_batches]
        x_data = tensor
        y_data = np.copy(tensor)
        for y_ in y_data:
            y_[:-1] = y_[1:]
            y_[-1] = 0

        self.x_batches = np.split(x_data, self.num_batches)
        self.y_batches = np.split(y_data, self.num_batches)

    def reset_batch(self):
        self.index_id = 0

    def next_batch(self):  # in test use for range.
        x, y = self.x_batches[self.index_id], \
               self.y_batches[self.index_id]
        self.index_id += 1
        return x, y

    def get_seq_ter_prob(self):
        branch_seq_path = os.path.join(self.data_path, "branch_dataset.brd")
        seqs = self.sequences_loader(branch_seq_path)
        seq_len_list = [len(seq) for seq in seqs]
        seq_len_counter = collections.Counter(seq_len_list)

        t_prob = {}  # A dict for terminate prob.
        for l, _ in seq_len_counter.items():
            c = sum([c_ for l_, c_ in seq_len_counter.items() if l_ == l])
            # rest longer one
            r = 0  # rest longer one
            for l_, c_ in seq_len_counter.items():
                r_ = 0
                if l_ > l:  # for longer one
                    for i in range(l_ - l):
                        c_ = c_ / 2  # get rid of repetitive sub tree
                    r_ += c_
                r += r_

            t_prob[l] = float(c) / float(c + r)

        print("Got termination prob.")

        return t_prob

# loader = SeqLoader(data_path, 20, STEM_SAMPLE_POINT_NUM, 50)
# x, y = loader.next_batch()
# print(x)
# print(y)




