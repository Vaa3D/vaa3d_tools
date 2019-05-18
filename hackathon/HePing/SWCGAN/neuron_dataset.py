
from sklearn.preprocessing import OneHotEncoder
import os
import numpy as np
from Neuron import Neuron
import provider
from keras import backend as K


class neuron_dataset():
    """
    -----2019/5/14-------heping----
     read data from files,and processing data
    """
    def __init__(self, root, batch_size=32, npoints=500, cache_size=10000, num_channels=3):
        self.data_root = root
        self.npoints = npoints
        self.cache_size = cache_size
        self.batch_size = batch_size
        self.num_channel = num_channels
        neuron_id = {}
        neuron_id['train'] = [line.rstrip() for line in open(os.path.join(self.data_root, 'train'))]
        # all data path
        self.datapath_train = os.path.join(self.data_root, 'jacobs')
        self.datapath = [os.path.join(self.datapath_train, neuron_id['train'][i]) for i in range(len(neuron_id['train']))]
        print('datapath', len(self.datapath))
        self.idx = np.arange(0, len(self.datapath))
        self.batch_idx = 0
        self.num_batch = len(self.datapath)//self.batch_size
        print("num_batch:", self.num_batch)
        # shuffle
        np.random.shuffle(self.idx)
        # from index to (geometry, morphology) tuple
        self.cache = {}

    # geometry normalization
    def pc_normalize(self, pc):
        l = pc.shape[0]
        centroid = np.mean(pc, axis=0)
        pc = pc - centroid
        m = np.max(np.sqrt(np.sum(pc ** 2, axis=1)))
        pc = pc / m
        return pc

    # get a swc file, return swc: morphology, geometry
    def _get_item(self, index):
        if index in self.cache:
            geometry, morphology, diameter = self.cache[index]
        else:
            fn = self.datapath[index]
            print(fn)
            neuron = Neuron(file_format='swc', input_file=fn)

            point_set = neuron.nodes_list  # Node: xyz, r, parent, type, children

            # point_set = np.loadtxt(fn, delimiter=',').astype(np.float32)
            # get geometry and morphology
            print('-----')
            # move soma to original coordination
            geometry = np.transpose(neuron.location)
            # geometry = np.array([node.getxyz() for node in point_set])  # real location
            diameter = np.array([node.r for node in point_set])
            morphology = neuron.parent_index
            #print("geo_diam_morph shape", geometry.shape, diameter.shape, morphology.shape)
            #print(diameter)
            #print(geometry)
            np.reshape(diameter, [-1])
            np.reshape(morphology, [-1])

            # print('geometry', geometry)
            # print('radius', diameter)
            # print('morphology', morphology)
            # diameter = [node.get_radius() for node in point_set]
            # morphology = point_set[6, :]
            morphology = morphology[0:self.npoints-1]  # cut
            geometry = geometry[0:self.npoints-1, :]  # cut
            print("geo_diam_morph shape", geometry.shape, diameter.shape, morphology.shape)
            # normalization
            geometry[:, 0:3] = self.pc_normalize(geometry[:, 0:3])
            if len(self.cache) < self.cache_size:
                self.cache[index] = (geometry, morphology, diameter)  # tuple: geometry, morphology, radius, diameter not use now
        return geometry, morphology

    # whether have next batch
    def has_next_batch(self):
        return self.batch_idx < self.num_batch

    # GET A BATCH DATA, last batch size maybe < self.batch_size
    def next_batch(self, augment=False):
        start_idx = self.batch_idx * self.batch_size
        # end_idx = min((self.batch_idx+1)*self.batch_size, len(self.datapath))
        # bsize = end_idx - start_idx
        batch_data = {}
        batch_data['geometry'] = np.zeros([self.batch_size, self.npoints, self.num_channel])
        batch_data['morphology'] = np.zeros([self.batch_size, self.npoints, self.npoints])
        geometry_batch = np.zeros([self.batch_size, self.npoints-1, self.num_channel])
        morphology_batch = np.zeros([self.batch_size, self.npoints-1, self.npoints])
        morphology_one = np.zeros([self.batch_size, self.npoints-1])

        for i in range(self.batch_size):
            geometry, morphology = self._get_item(self.idx[i+start_idx])
            geometry_batch[i] = geometry
            morphology_one[i] = morphology
            # batch_data['morphology'][i] = tf.Session().run(self.encoder_parent(morphology))
        self.batch_idx += 1
        morphology_batch = self.encoder_parent(morphology_one)
        if augment:
            batch_data['geometry'] = self._augment_batch_data(batch_data['geometry'])
        batch_data['geometry'] = np.concatenate([np.zeros(shape=(self.batch_size, 1, 3)), geometry_batch], axis=1)
        batch_data['morphology'] = np.concatenate([np.zeros(shape=(self.batch_size, 1, self.npoints)),
                                    morphology_batch], axis=1)
        print('batch geometry,morphology shape',batch_data['geometry'].shape,batch_data['morphology'].shape)
        # print(batch_data['geometry'])
        # print(batch_data['morphology'])
        return batch_data['geometry'], batch_data['morphology']

    # one-hot encoder morphology
    def encoder_parent(self, X_parent):
        enc = OneHotEncoder(n_values=self.npoints)
        parent = np.reshape(enc.fit_transform(X_parent).toarray(),  # X_parent value exceed range enc n_value
                               [self.batch_size, self.npoints-1, self.npoints])
        print('enc_parent shape', parent.shape)
        return parent

    # augment batch data: rotate, shift, jitter
    # displacement invariance
    def _augment_batch_data(self, batch_data):  # augment point xyz
        rotated_data = provider.rotate_point_cloud(batch_data)
        rotated_data = provider.rotate_perturbation_point_cloud(rotated_data)

        jitter_data = provider.random_scale_point_cloud(rotated_data[:, :, 0:3])
        jitter_data = provider.shift_point_cloud(jitter_data)
        jitter_data = provider.jitter_point_cloud(jitter_data)
        rotated_data[:, :, 0:3] = jitter_data
        return rotated_data
