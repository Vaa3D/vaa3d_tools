import tensorflow as tf
import numpy as np
from branch_code_data import BranchCodeLoader
from sklearn.preprocessing import MinMaxScaler
import vae
import os
import pickle

'''
From a branch code generate its latent vector.
'''

test_data = "C:/Users/Administrator/Desktop/neuron_factor_tree_test/test/data/L4_BC/data"
save_dir = "C:/Users/Administrator/Desktop/neuron_factor_tree_test/test/data/L4_BC/data/vae_save"
gen_data = "C:/Users/Administrator/Desktop/neuron_factor_tree_test/test/data/L4_BC/data/gen"


data_size = 40*3
batch_size = 50

n_hidden = 200  # num of hidden units in mlp.
dim_z = 100  # dim of latent.


def get_neuron_latent():
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

    # generate seqs.
    for i in range(1, len(stems) + 1):
        # load a type of seqs' data.
        db_file = os.path.join(data_path, "factorDB_" + str(stems[i - 1]) + ".txt")
        # Loading data.
        with open(db_file, "rb") as f:
            factors = pickle.load(f)
        factorDB_size = len(factors)
        factorDB = dict(zip(factors, range(len(factors))))
        factorDB_reverse = {v: k for k, v in factorDB.items()}

        data_loader = SeqLoader(test_path=TESTDIR, seq_size=seq_size, branch_size=STEM_SAMPLE_POINT_NUM,
                                batch_size=1, seq_type=stems[i - 1])
        seq_ter_prob = data_loader.get_seq_ter_prob()

        tf.reset_default_graph()

        seqs, facs = generate_seq(stems[i - 1])

        nf_infos = []
        for f in facs:
            branch_latent = []
            mu_sigma = factorDB_reverse[f.id][1:-1].split(", ")
            # print(mu_sigma)
            # print(mu_sigma[:dim_z])
            # print(mu_sigma[dim_z:])
            z_value = np.array(mu_sigma[:dim_z], dtype="float32") + \
                      np.array(mu_sigma[dim_z:2 * dim_z], dtype="float32") * random.uniform(0, 1)

            branch_latent.append(z_value)

            z_value = np.array(mu_sigma[2 * dim_z:3 * dim_z], dtype="float32") + \
                      np.array(mu_sigma[3 * dim_z:], dtype="float32") * random.uniform(0, 1)

            branch_latent.append(z_value)

            nf_infos.append((f.index, branch_latent, f.parent))


if __name__ == '__main__':

    data_loader = BranchCodeLoader(test_data, batch_size)
    # x_test = data_loader.test_data
    x, _ = data_loader.next_batch()
    latent_generate(save_dir, x)