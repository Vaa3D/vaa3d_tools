import os
import sys
import numpy as np
from PIL import Image
from sklearn.utils import shuffle
from sklearn.model_selection import train_test_split

class DataLoader():
    def __init__(self, data_dir, stem_size, batch_size):
        self.data_dir = data_dir
        self.stem_size = stem_size
        self.batch_size = batch_size

        training_data = os.path.join(data_dir, "training.npy")
        testing_data = os.path.join(data_dir, "testing.npy")
        if not (os.path.exists(training_data)) or not (os.path.exists(testing_data)):
            self.preprocess(data_dir)
        else:
            self.load_processed(training_data, testing_data)

        self.create_batches()
        self.reset_batch()

    def preprocess(self, data_dir):
        print("Preprocessing begins......")

        branch_file_list = []
        branch_list = os.listdir(data_dir)  # nfss_list contains nfss from one class.
        for branch in branch_list:
            branch_path = os.path.join(data_dir, branch)  # get a full_path of nfss.
            if os.path.isfile(branch_path) and branch_path[-3:] == "bmp":  # meet a nfss file.
                img = Image.open(branch_path)
                img = img.convert("RGBA")
                f1, f2, f3, f4 = img.split()  # four channels
                array = np.array(f1)
                print(array)
                # print(array.shape)
                # print(type(array[0][0][0]))
                branch_file_list.append(branch_list)
        print("Find %d branch files." %(len(branch_file_list)))


    def load_bmp(self):
        return

    def load_processed(self, training_data, testing_data):
        self.data = np.load(training_data)
        self.test_data = np.load(testing_data)
        print("Load {} branches from [{}].".format(len(self.data), training_data))

    def create_batches(self):
        self.num_batches = int(len(self.data) / self.batch_size)
        if self.num_batches == 0:
            assert False, "Don't have enough data, finish."
        # use the first self.batch_size*self.num_batches data.
        data = self.data[:self.batch_size * self.num_batches]

        x_data = data
        y_data = np.copy(x_data)

        self.x_batches = np.split(x_data, self.num_batches)
        self.y_batches = np.split(y_data, self.num_batches)

    def reset_batch(self):
        self.index_id = 0

    def next_batch(self):  # in test use for range.
        x, y = self.x_batches[self.index_id], \
               self.y_batches[self.index_id]
        self.index_id += 1
        return x, y

dataLoader = DataLoader("C:/Users/Administrator/Desktop/neuron_factor_tree_test/test/data/L4_BC/data",
                        256, 50)