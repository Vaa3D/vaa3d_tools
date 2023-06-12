from sklearn.preprocessing import MultiLabelBinarizer
import numpy as np
from sklearn.model_selection import train_test_split # 训练集测试集分割模块


def get_label():
    label_set = set()
    with open('数据集.txt', 'r') as fr:
        for id, line in enumerate(fr.readlines()):
            text, labels = line.split('\t')
            for label in labels.split(','):
                label_set.add(label.strip())
    print(label_set)
    return label_set

def label_one_hot(label_set):
    # 创建多nominal
    label_list = list(label_set)
    print(label_set)
    label_set = ",".join(label_set)
    print(label_set)
    multi_nominal = []
    text_list = []
    # np.array([["A", "Black"],
    #                       ["B", "White"],
    #                       ["C", "Green"],
    #                       ["D", "Red"]])
    with open('数据集.txt', 'r') as fr:
        for id, line in enumerate(fr.readlines()):
            text, labels = line.split('\t')
            labels = labels.strip()
            multi_nominal.append(labels.split(','))
            text_list.append(text)
    # multi_nominal = np.array(multi_nominal)
    # print(multi_nominal)
    multi_one_hot = MultiLabelBinarizer(classes=label_list)
    multi_ = multi_one_hot.fit_transform(multi_nominal)
    print('multi_one_hot.classes_:', multi_one_hot.classes_)

    return np.array(text_list), multi_, multi_one_hot.classes_


def data_split(X,y,all_labels):
    train_X, test_X, train_y, test_y = train_test_split(X, y, random_state=0)
    all_labels_str = ",".join(all_labels)
    with open('train_onehot_new.csv', 'w') as fw:
        fw.write(f'content,{all_labels_str}')
        fw.write('\n')
        for t, m in zip(train_X, train_y):
            label_str = [str(m_) for m_ in m]
            label_str = ','.join(label_str)
            print(f'{t},{label_str}')
            fw.write(f'{t},{label_str}')
            fw.write('\n')
    with open('test_onehot_new.csv', 'w') as fw:
        fw.write(f'content,{all_labels_str}')
        fw.write('\n')
        for t, m in zip(test_X, test_y):
            label_str = [str(m_) for m_ in m]
            label_str = ','.join(label_str)
            print(f'{t},{label_str}')
            fw.write(f'{t},{label_str}')
            fw.write('\n')
    return train_X, test_X, train_y, test_y


if __name__ == '__main__':
    label_set = get_label()
    with open('label.txt', 'w') as fw:
        for l in list(label_set):
            fw.write(l)
            fw.write('\n')
    X, Y, all_labels = label_one_hot(label_set)
    print(X.shape)
    print(Y.shape)
    train_X, test_X, train_y, test_y = data_split(X, Y, all_labels)
    print(train_X.shape, test_X.shape, train_y.shape, test_y.shape)


