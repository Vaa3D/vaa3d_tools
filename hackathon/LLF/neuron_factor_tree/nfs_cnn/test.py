import numpy as np
import pickle
from sklearn import preprocessing
t = [("1", 2),("10",3)]
print("10" in [key for key,_ in t])
print("212131.nfss"[-4:]=="nfss")
a = np.array([1,2,3,4,5])
l = [1,2,3]
z= 3*[l]
print(z)
b = [4]
b.extend(l)
print(b)
b.append(l)
print(b)

a = np.expand_dims(a,1)

label_list = []
label_list.append((np.arange(9)==7).astype(np.float32))
label_list.append((np.arange(9)==8).astype(np.float32))
l = np.array(label_list)


with open("C:/Users/Administrator/Desktop/neuron_factor_tree_test/test_1/data/morpho/preprocessed_input.txt", "rb") as f:
    x_y_ = pickle.load(f)  # load a tuple((sequence), label)
x_y_ = list(zip([1,2,3],[9,9,9]))

x_data = [x for x, y in x_y_]
y_data = [y for _, y in x_y_]
# sequences_num = y_data.size

x_data = np.array([[[1,2,3,4],[2,2,3,4],[1,2,3,4],[1,2,3,4],[5,2,3,4]],
[[1,2,3,4],[1,2,3,4],[1,2,3,4],[1,2,3,4],[1,2,3,4]],
[[1,2,3,4],[1,2,3,4],[1,2,3,4],[1,2,3,4],[1,2,3,4]],
[[1,2,3,4],[1,2,3,4],[1,2,3,4],[1,2,3,4],[1,2,3,4]],
[[1,2,3,4],[1,2,3,4],[1,2,3,4],[1,2,3,4],[1,2,3,4]],
[[1,2,3,4],[1,2,3,4],[1,2,3,4],[1,2,3,4],[1,2,3,4]],
[[1,2,3,4],[1,2,3,4],[1,2,3,4],[1,2,3,4],[1,2,3,4]],
[[1,2,3,4],[1,2,3,4],[1,2,3,4],[1,2,3,4],[1,2,3,4]],
[[1,2,3,4],[1,2,3,4],[1,2,3,4],[1,2,3,4],[1,2,3,4]]])

print(preprocessing.minmax_scale(x_data.reshape(-1, 4)))

x_data = np.expand_dims(x_data, 3)
print(x_data.shape)
# with open("C:/Users/Administrator/Desktop/neuron_factor_tree_test/morpho_copy/L6_PC/2007-07-30-A.CNG.swc", 'r') as f:
#     y_data = (1, 2)
s="22\n"
print(float(s[:-1]))
x_y_ = zip(x_data, y_data)
print([y for _, y in x_y_])
x_data = x_data.reshape(-1, 4)
#x_data = preprocessing.scale(x_data)
x_data = x_data.reshape(-1, 5, 4)
x_y = zip(x_data, y_data)
print([x for x, _ in x_y])
x_batches = np.split(x_data, 3, 0)
print(np.shape(x_batches[0]))

# with open("C:/Users/Administrator/Desktop/neuron_factor_tree_test/data/collected_features.txt", "r") as f:
#     label = -1
#     seq = []  # collect a sequence.
#     seqs_list = []  # collect all sequences.
#     label_list = []
#     for line in f.readlines():
#         if line.startswith("#CLASS"):  # means a class block starts.
#             label += 1
#         elif line == "\n":  # means a sequence finishes.
#             if len(seq) < 5:  # too short, throw away.
#                 seq = []
#                 continue
#             elif len(seq) <= 10:  # shorter than set sequence_size, add 0
#                 for i in range(10 - len(seq)):
#                     f_ = [0.0 for _ in range(21)]
#                     seq.append(f_)
#             else:  # too long, drop the end factors. ??? MAYBE drop the fronts better.
#                 for i in range(len(seq) - 10):
#                     seq.pop()
#             seqs_list.append(seq)
#             label_list.append(label)
#             seq = []
#         else:  # means meet a feature, add it to current seq.
#             feature = np.array(line.split(" "))
#             feature = list(map(float, feature))  # str to float
#             seq.append(feature)
#
#     print(seqs_list)
#     a = np.array(seqs_list, dtype=np.float)
#     b = np.array(label_list)
#     print(np.shape(b))
#     print(np.shape(a))


