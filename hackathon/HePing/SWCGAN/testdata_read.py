from datetime import datetime
import sys
import os
from neuron_dataset import *
BASE_DIR = os.path.dirname(os.path.abspath(__file__))  # get this file full path and remove filename
ROOT_DIR = BASE_DIR
if __name__ == '__main__':
    DATA_PATH = os.path.join(ROOT_DIR, 'data/neuron_data1')
    batch_size = 16
    npoints = 200
    TRAIN_DATASET = neuron_dataset(root=DATA_PATH, batch_size=batch_size, npoints=npoints, num_channels=3)
    batch_data = {}
    cur_batch_data_geo = np.zeros([batch_size, npoints, 3])
    cur_batch_data_morp = np.zeros([batch_size, npoints, npoints])
    batch_idx = 0
    while TRAIN_DATASET.has_next_batch():
        batch_data['geometry'], batch_data['morphology'] = TRAIN_DATASET.next_batch(augment=True)
        # bsize = batch_data['geometry'].shape[0]

        cur_batch_data_geo[0:batch_size, :] = batch_data['geometry']
        cur_batch_data_morp[0:batch_size, :, :] = batch_data['morphology']
        print(cur_batch_data_geo.shape, cur_batch_data_morp.shape)
        batch_idx += 1
    print("-----------------------end-----------------------------")
