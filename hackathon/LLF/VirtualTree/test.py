from branch_code_data import BranchCodeLoader
import os
import pickle
import numpy as np

# a = np.array([[[1, 2, 3],[4, 5, 6]],
#           [[7, 8, 9], [10, 11, 12]],
#               [[13, 14, 15], [16, 17, 18]]])
# print(a.shape)
# print(a.reshape([len(a), -1]))

test_data = "C:/Users/Administrator/Desktop/neuron_factor_tree_test/test/data/L4_BC/data"

# scaler_file = os.path.join(test_data, "scaler.txt")
#
# with open(scaler_file, "rb") as f:
#     min_max_scaler = pickle.load(f)
# print("Scaler file loaded from [%s]." % scaler_file)
#
# data_loader = BranchCodeLoader(test_data, 1)
# x_test = data_loader.test_data
#
# x_arr = np.array(x_test)
# print(x_arr * min_max_scaler.data_range_ + min_max_scaler.data_min_)

