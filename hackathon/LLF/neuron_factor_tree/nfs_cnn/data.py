import os
import numpy as np
from sklearn.preprocessing import MinMaxScaler
import pickle

class DataLoader():
    def __init__(self, data_dir, batch_size, sequence_size, feature_size, mode):
        self.class_map = {"L4_BC": 0, "L4_PC": 1, "L4_SC": 2, "L5_MC": 3, "L5_PC": 4,
                          "L6_PC": 5, "L23_BC": 6, "L23_MC": 7, "L23_PC": 8}
        self.data_dir = data_dir
        self.batch_size = batch_size
        self.sequence_size = sequence_size
        self.feature_size = feature_size
        self.num_classes = len(self.class_map)

        if mode == "train":  # in training mode, load combined feature file.
            data_train_dir = os.path.join(data_dir, "morpho")
            preprocessed_data_dir_file = os.path.join(data_train_dir, "preprocessed_data.txt")
            if not (os.path.exists(preprocessed_data_dir_file)):
                self.process(data_train_dir, preprocessed_data_dir_file)
            else:
                self.load_processed(preprocessed_data_dir_file)
        elif mode == "eval":  # in evaluation mode, load just one .nfss file.
            base_dir = os.path.abspath(os.path.join(data_dir,"../../.."))
            preprocessed_data_dir_file = os.path.join(base_dir, "morpho/preprocessed_data.txt")
            with open(preprocessed_data_dir_file, "rb") as f:  # assume processed before eval.
                self.min_max_scaler = pickle.load(f)
            data_dir_nfss = data_dir  # in eval mode, input one nfss instead.
            self.process_nfss(data_dir_nfss)
        self.create_batches()
        self.reset_batch()

    # read all the nfss files, normalize then saved as binary mode.
    def process(self, data_base_dir, save_path):

        seq_list_total = []  # total seqs(every neuron).
        label_list_total = []

        path_list = os.listdir(data_base_dir)
        for p in path_list:  # path_list contains each class folder.
            class_path = os.path.join(data_base_dir, p)
            if os.path.isdir(class_path):  # if is dir, dir's name is neuron class.
                neuron_label = self.class_map[p]
                f_list = os.listdir(class_path)  # f_list contains neurons from one class.
                for f in f_list:
                    f = os.path.join(class_path, f)
                    if os.path.isfile(f) and f[-4:] == "nfss":  # meet a nfss file.
                        seq_list = self.load_sequences(f)  # one neuron's seqs.
                        label_list = len(seq_list)*\
                                     [(np.arange(self.num_classes)==neuron_label).astype(np.float32)]# to one hot.
                        seq_list_total.extend(seq_list)
                        label_list_total.extend(label_list)

        #  now seq_list_total contains many seqs, their label stores in label_list.
        x_data_ = np.array(seq_list_total, dtype=np.float32)  # get data array.
        x_data_ = x_data_.reshape(-1,
                                  self.feature_size)  # reshape -> [sequence_num*sequence_size, feature_size]
        #  normalization
        min_max_scaler = MinMaxScaler()
        min_max_scaler.fit(x_data_)
        self.min_max_scaler = min_max_scaler
        x_data_ = min_max_scaler.transform(x_data_)  # normalization : x-min/max-min
        x_data_ = x_data_.reshape(-1, self.sequence_size, self.feature_size)  # reshape back.
        x_data_ = np.expand_dims(x_data_, 3)  # add another dim (sequence_num, sequence_size, feature_size, 1)
        y_data_ = np.array(label_list_total, dtype=np.float32)

        #  in training step, save by pickle
        with open(save_path, "wb") as f:
            pickle.dump(self.min_max_scaler, f)  # save scaler to transfer eval data.
            pickle.dump(zip(x_data_, y_data_), f)  # zip as a tuple.
        self.x_data = x_data_
        self.y_data = y_data_
        self.sequences_num = y_data_.shape[0]

    def load_processed(self, normalized_data_dir_file):
        with open(normalized_data_dir_file, "rb") as f:
            self.min_max_scaler = pickle.load(f)
            x_y_ = pickle.load(f)  # load a zip((sequence), label)
        x_y_ = list(x_y_)
        # self.x_data = np.array([x for x, _ in x_y_])
        self.x_data = np.array([x for x, _ in x_y_])  # don.t use fea_torque.
        self.y_data = np.array([y for _, y in x_y_])
        self.sequences_num = self.y_data.shape[0]


    def process_nfss(self, nfss_file):
        seqs_list = self.load_sequences(nfss_file)
        if(seqs_list<1):
            self.sequences_num = 0
            return
        x_data_ = np.array(seqs_list, dtype=np.float32)  # get data array.
        x_data_ = x_data_.reshape(-1,
                                  self.feature_size)  # reshape -> [sequence_num*sequence_size, feature_size]
        x_data_ = self.min_max_scaler.transform(x_data_)  # normalization : x-min/max-min
        x_data_ = x_data_.reshape(-1, self.sequence_size, self.feature_size)  # reshape back.
        x_data_ = np.expand_dims(x_data_,
                                 3)  # add another dim (sequence_num, sequence_size, feature_size, 1)
        self.x_data = x_data_
        self.sequences_num = x_data_.shape[0]
        self.y_data = np.zeros([self.sequences_num])

    # from a nfss file load sequences.
    def load_sequences(self, nfss_file):
        seq_size_lowerbound = (self.sequence_size + 1) / 2  # defines a sequence's lower bound length.
        with open(nfss_file, "r") as f:
            seq = []  # collect a sequence.
            seqs_list = []  # collect all sequences.
            for line in f.readlines():
                if line.startswith("#LEAF"):  # means a sequence finishes.
                    if len(seq) < self.sequence_size:  # too short, throw away.
                        seq = []
                        continue
                    # elif len(seq) <= self.sequence_size:  # shorter than set sequence_size, add 0
                    #     for i in range(self.sequence_size - len(seq)):
                    #         f_ = [0.0 for _ in range(self.feature_size)]
                    #         seq.append(f_)
                    else:  # too long, drop the top factors.
                        seq = seq[(len(seq) - self.sequence_size):]
                    seqs_list.append(seq)
                    seq = []
                elif line.startswith("#VALUES:"):  # means meet a feature, add it to current seq.
                    def f(str):
                        if str == "-1.#IO" or str == "1.#QO":
                            return 0.0
                        else:
                            return float(str)

                    feature = np.array(line[:-1].split(" ")[1:-2])  # get rid of'\n' and '#VALUES'.
                    feature = list(map(f, feature))  # str to float
                    seq.append(feature)
                else:
                    continue
        return seqs_list

    def create_batches(self):
        if self.batch_size == -1:  # don't assign a batch size, use total sequences as a batch.
            self.batch_size = self.sequences_num
        self.num_batches = int(self.sequences_num / self.batch_size)

        if self.num_batches == 0:
            assert False, "We don't have enough data, set a smaller batch_size."
        # use the first self.batch_size*self.num_batches data.
        x_data = self.x_data[:self.batch_size*self.num_batches]
        y_data = self.y_data[:self.batch_size*self.num_batches]

        self.x_batches = np.split(x_data, self.num_batches)  # get num_batches batches.
        self.y_batches = np.split(y_data, self.num_batches)

    def next_batch(self):
        x, y = self.x_batches[self.index[self.index_id]], \
               self.y_batches[self.index[self.index_id]]  # randomly select batch.
        self.index_id += 1
        return x, y

    def reset_batch(self):
        index = np.arange(self.num_batches)
        np.random.shuffle(index)
        self.index = index  # get a random index.
        self.index_id = 0
