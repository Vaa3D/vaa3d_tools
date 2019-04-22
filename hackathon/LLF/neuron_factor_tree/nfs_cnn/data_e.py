from data import DataLoader
import os

TESTDIR = "C:/Users/Administrator/Desktop/neuron_factor_tree_test/test_1"
eval_path = TESTDIR + "/data/morpho_eval/L4_BC/C060998B-I1.CNG.nfss"
class_map = {"L4_BC": 1, "L4_PC": 2, "L4_SC": 3, "L5_MC": 4, "L5_PC": 5,
                          "L6_PC": 6, "L23_BC": 7, "L23_MC": 8, "L23_PC": 9}
# data_loader = DataLoader(data_dir= (TESTDIR+"/data"), batch_size=100, sequence_size=10, feature_size=19, mode="train")
data_loader = DataLoader(data_dir= eval_path, batch_size=-1, sequence_size=10, feature_size=19, mode="eval")



path_list = os.listdir(eval_path)
for p in path_list:
    class_path = os.path.join(eval_path, p)
    if os.path.isdir(class_path):  # if is dir, dir's name is neuron class.
        neuron_label = class_map[p]
        print(p)
        f_list = os.listdir(class_path)
        for f in f_list:
            if os.path.isfile(os.path.join(class_path, f)) and f[-4:]=="nfss" :  # f
                print(f)