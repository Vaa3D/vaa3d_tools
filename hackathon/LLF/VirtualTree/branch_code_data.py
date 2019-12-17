import os
import numpy as np
from sklearn.utils import shuffle
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import MinMaxScaler
import pickle

STEM_SAMPLE_POINT_NUM = 40

'''
Load branch codes from .bcd file.
For branch generator training.
'''

class BranchCodeLoader():
    def __init__(self, data_dir, batch_size):
        self.data_dir = data_dir
        self.batch_size = batch_size

        # .bcd file.
        branch_code_dataset = os.path.join(data_dir, "branch_code.bcd")

        training_data = os.path.join(data_dir, "branch_training.npy")
        testing_data = os.path.join(data_dir, "branch_testing.npy")
        if not (os.path.exists(training_data)) or not (os.path.exists(testing_data)):
            self.preprocess(data_dir, branch_code_dataset)
        else:
            self.load_processed(training_data, testing_data)

        self.create_batches()
        self.reset_batch()

    def preprocess(self, data_dir, branch_dataset):
        print("Preprocessing begins......")

        '''
        data = []  # total branches.
        with open(branch_dataset, "r") as f:
            branch_code = []
            for line in f.readlines():
                if line.startswith("#BRANCH:"):
                    branch_code = []
                elif line.startswith("#BRANCHEND"):
                    data.append(branch_code)

                elif line.startswith("#DATA:"):
                    coord_str = np.array(line[:-1].split(" ")[1:4])
                    coord = list(map(float, coord_str))
                    branch_code.append(coord)
        '''

        ''' Split two stems.'''

        data = []  # total stems.
        with open(branch_dataset, "r") as f:
            stem_code = []
            for line in f.readlines():
                if line.startswith("#STEM"):
                    stem_code = []
                elif line.startswith("#ENDSTEM"):
                    data.append(stem_code)

                elif line.startswith("#DATA:"):
                    coord_str = np.array(line[:-1].split(" ")[1:4])
                    coord = list(map(float, coord_str))
                    stem_code.append(coord)

        print("{} branches loaded.".format(len(data)))
        data = np.array(data, dtype=np.float32)  # BRANCH_NUM*40*3.
        data = data.reshape(-1, 3)  # [BRANCH_NUM*40, 3].

        # normalization
        min_max_scaler = MinMaxScaler()
        min_max_scaler.fit(data)

        self.min_max_scaler = min_max_scaler  # record the scaler.
        scaler_file = os.path.join(data_dir, "scaler.txt")
        with open(scaler_file, "wb") as f:
            pickle.dump(min_max_scaler, f)
        print("Scaler file dumped at [%s]." % scaler_file)

        data = min_max_scaler.transform(data)

        # one row, 120 : x-y-z, x-y-z, ... , x-y-z.
        # data = np.reshape(data, [-1, 120])  # BRANCH_NUM*120.
        data = np.reshape(data, [-1, 20*3])  # BRANCH_NUM*120.
        data = shuffle(data, random_state=42)

        train_data, test_data = train_test_split(data, test_size=0.1, random_state=42)
        print("Get {} training data and {} testing data.".format(len(train_data), len(test_data)))
        self.data = train_data
        self.test_data = test_data

        training_data_path = os.path.join(data_dir, "branch_training")
        testing_data_path = os.path.join(data_dir, "branch_testing")
        np.save(training_data_path, self.data)
        np.save(testing_data_path, self.test_data)

        print("Traing data saved at [%s]."%training_data_path)
        print("Testing data saved at [%s]."%testing_data_path)

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

    def next_batch_4(self):  # in test use for range.
        x = self.x_batches[self.index_id]
        x = x.reshape(self.batch_size, 20 ,3)
        x = x[:,:,:,np.newaxis]
        self.index_id += 1
        return x

    def get_test(self):
        test = self.test_data
        test = test.reshape([-1, 20, 3])
        test = test[:, :,:,np.newaxis]
        return test

# bl = BranchCodeLoader("C:/Users/Administrator/Desktop/neuron_factor_tree_test/test/data/L4_BC/data", 1)
