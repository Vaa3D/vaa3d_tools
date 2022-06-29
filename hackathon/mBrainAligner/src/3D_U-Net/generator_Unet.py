from random import shuffle
import numpy as np
import copy
from preprocess import data_process
from keras.utils.np_utils import to_categorical

# did not using for now

# def random_flip_dimensions(n_dimensions):
#     axis = list()
#     for dim in range(n_dimensions):
#         if np.random.choice([True, False]):
#             axis.append(dim)
#     return axis
#
#
# def flip_image(data0,label, axis):
#     try:
#         for axis_index in axis:
#             data0 = np.flip(data0, axis=axis_index)
#             # print('   flip : ', data.shape)
#     except TypeError:
#         data0 = np.flip(data0, axis=axis)
#         label = np.flip(label, axis=axis)
#     return data0,label
#
#
# def get_multi_class_labels(data, n_labels, labels=None):
#     new_shape = [data.shape[0], n_labels] + list(data.shape[2:])
#     y = np.zeros(new_shape, np.int8)
#     for label_index in range(n_labels):
#         if labels is not None:
#             y[:, label_index][data[:, 0] == labels[label_index]] = 1
#         else:
#             y[:, label_index][data[:, 0] == (label_index + 1)] = 1
#     return y


def get_training_and_validation_generators(train_file,test_file, batch_size, validation_batch_size=None,InputShape = (80,80,80)):
    if not validation_batch_size:
        validation_batch_size = batch_size

    assert len(train_file.root.data) == len(train_file.root.label), "img shape:{} is not equal to mask shape:{}".format(
        len(train_file.root.data) , len(train_file.root.label))
    assert len(train_file.root.data) == len(train_file.root.label), "img shape:{} is not equal to mask shape:{}".format(
        len(test_file.root.data), len(test_file.root.label))

    train_index_list = [i for i in range(len(train_file.root.data))]
    val_index_list = [j for j in range(len(test_file.root.data))]

    training_generator = data_generator(train_file,batch_size=batch_size,InputShape = InputShape,index_list = train_index_list,Train_Flag = 'train')
    validation_generator = data_generator(test_file,batch_size=validation_batch_size,InputShape = InputShape,index_list = val_index_list,Train_Flag = 'val')

    return training_generator, validation_generator


def data_generator(data_file, index_list, batch_size=1, shuffle_index_list=True, Train_Flag = 'train', InputShape=(80,80,80)):
    orig_index_list = index_list
    while True:
        x_list = list()
        y_list = list()
        index_list = copy.copy(orig_index_list)
        # shuffle the image
        if shuffle_index_list:
            shuffle(index_list)
        while len(index_list) > 0:
            index = index_list.pop()
            add_data(x_list, y_list, data_file, index,Train_Flag = Train_Flag,InputShape = InputShape)
            if len(x_list) == batch_size or (len(index_list) == 0 and len(x_list) > 0):
                #print('giving model data index:',index,'\n' )
                yield convert_data(x_list, y_list)
                x_list = list()
                y_list = list()


def add_data(x_list, y_list, data_file, index,Train_Flag = 'train',InputShape = (80,80,80)):
    data, label = data_file.root.data[index], data_file.root.label[index]
    #label[label == 1] = 0
    #label[label == 2] = 0
    # label[label == 7] = 1
    # label[label != 7] = 0

    # 边界裁剪 , 随机裁剪 , Resize , Normlization
    # only using resize and normalization for now
    data, label = data_process(data, label, InputShape, Train_Flag = Train_Flag)

    #   10, 0, 100, 80, 145, 168, 159, 62, 237, 75, 249
    # get array that only belong to one class i.e. input shape->input shape*10
    label = to_categorical(label,10)
    # add data array one more axis for generate 10 predict img for each class
    data = np.asarray(data)[np.newaxis]
    data = np.moveaxis(data, 0, -1)

    x_list.append(data)
    y_list.append(label)


def convert_data(x_list, y_list):
    x = np.asarray(x_list)
    y = np.asarray(y_list)
    return x, y

