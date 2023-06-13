import pandas as pd
import numpy as np
from sklearn.svm import SVC
# from sklearn.datasets import make_classification
from sklearn.model_selection import train_test_split,GridSearchCV,StratifiedKFold,StratifiedShuffleSplit
from sklearn.preprocessing import StandardScaler
from joblib import dump,load
import time
from sklearn.decomposition import PCA
from sklearn.metrics import accuracy_score, precision_score, recall_score
import pickle
import os

def ImportDataset():
    data=pd.read_csv("dataset_win_raw.csv")
    # data=data.fillna(0)
    print(data)
    data=np.array(data.values)
    x_set=data[:,2:-2]
    y_set = data[:, -2]
    x_set=np.array(x_set,dtype='float')
    y_set=np.array(y_set,dtype='int')
    return x_set,y_set

def mysvm(X,Y):

    print(Y)
    # 使用numpy.histogram函数统计各元素出现次数
    hist, bins = np.histogram(Y, bins=np.arange(Y.min(), Y.max() + 2))

    # 打印结果
    print("元素   出现次数")
    for i in range(len(hist)):
        print(bins[i], "        ", hist[i])

    sc = StandardScaler()
    X = sc.fit_transform(X)

    # 使用 PCA 进行降维
    if os.path.exists('pca_model.pkl') is False:
        print("new PCA")
        pca = PCA(n_components=5)
        X = pca.fit_transform(X)
        with open('pca_model.pkl', 'wb') as f:
            pickle.dump(pca, f)
    else:
        print("load PCA")
        with open('pca_model.pkl', 'rb') as f:
            pca = pickle.load(f)
            X=pca.transform(X)
    print(pca.explained_variance_ratio_)

    # 将样本集分为训练集和测试集
    X_train, X_test, y_train, y_test = train_test_split(X, Y, test_size=0.1, random_state=42)

    # 创建SVM模型
    clf = SVC(kernel='linear', decision_function_shape='ovo',verbose=1)


    # param_grid = {'C': [0.1, 1, 10], 'kernel': ['linear', 'rbf'], 'gamma': ['scale', 'auto']}
    param_grid = {'C': [0.1, 1, 10], 'kernel': ['linear', 'rbf'], 'degree': [2, 3, 4],
                  'gamma': ['scale', 'auto']}

    #使用GridSearchCV进行超参数调优
    clf = GridSearchCV(clf, param_grid=param_grid, cv=StratifiedKFold(n_splits=5))
    start_time = time.time()
    print(start_time)
    # 代码执行
    # 训练SVM模型
    clf.fit(X_train, y_train)

    end_time = time.time()
    time_used = end_time - start_time
    print("Time used: ", time_used, " seconds")

    dump(clf,"svmmodel_raw_ovo_poly.joblib")
    # 在测试集上评估模型性能
    score = clf.score(X_test, y_test)
    print("Accuracy: ", score)

    y_pred = clf.predict(X_test)

    acc = accuracy_score(y_test, y_pred)
    # 计算精确率
    precision = precision_score(y_test, y_pred, average='macro')

    # 计算召回率
    recall = recall_score(y_test, y_pred, average='macro')
    # 打印评估指标
    print('Accuracy: %.3f' % acc)
    print('Precision: %.3f' % precision)
    print('Recall: %.3f' % recall)


if __name__ == '__main__':
    X,Y=ImportDataset()
    print(X.shape)
    mysvm(X,Y)
