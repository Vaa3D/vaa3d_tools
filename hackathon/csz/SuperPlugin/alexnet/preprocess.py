import glob as glob
from pylib.img_io import *
import random
import os
import copy
import pandas as pd

def write_name_list(fixed_path,name_list, file_name):
    f = open(os.path.join(fixed_path,file_name), 'w')
    for i in range(len(name_list)):
        f.write(str(name_list[i]) + "\n")
    f.close()

def fixed_data(path):
    if os.path.exists(path) is False:
        os.mkdir(path)
    data_name_list=glob.glob(r'E:/result1/*/raw/*.tiff')

    temp=copy.deepcopy(data_name_list)
    d = set()
    # print(len(data_name_list))
    for line in np.array(pd.read_csv('./datasetlist/one_hot_win_washed.csv'))[:, :]:
        imgname = line[0].split('\\')[2]
        position = line[0].split('\\')[-1].split('_gold')[0]
        d.add(imgname + position)
    print(len(d))
    print(len(data_name_list))
    for item in temp:
        imgname=item.split('\\')[1]
        position=item.split('\\')[-1].split('_raw')[0]
        key=imgname+position
        if key not in d:
            data_name_list.remove(item)
    # print(len(data_name_list))
    print(len(data_name_list))
    data_num=len(data_name_list)
    random.shuffle(data_name_list)
    train_rate=0.7
    val_rate=0.2
    testrate=0.1

    train_name_list = data_name_list[0:int(data_num * train_rate)]
    val_name_list = data_name_list[int(data_num * train_rate):int(data_num * (train_rate + val_rate))]
    test_name_list = data_name_list[int(data_num * (train_rate + val_rate)):int(data_num * (train_rate + val_rate+testrate))]
    write_name_list(path,train_name_list, "train_name_list.txt")
    write_name_list(path,val_name_list, "val_name_list.txt")
    write_name_list(path,test_name_list, "test_name_list.txt")

if __name__ == '__main__':
    print(pd.read_csv('./datasetlist/one_hot_win_washed.csv'))
    path="./datasetlist"
    fixed_data(path)