import numpy as np
import matplotlib.pyplot as plt

filePath="C:\\Users\\admin\\Desktop\\ImageBgDetect066.txt";
file=open(filePath);
content=file.readlines();

for i in range(len(content)):
    content[i]=float(content[i].strip('\n'));

# print(content)
print(type(content))
print(type(content[0]))

def Normal(x, mu, sigma):  # 一元正态分布概率密度函数
    return np.exp(-(x - mu) ** 2 / (2 * sigma ** 2)) / (np.sqrt(2 * np.pi) * sigma);

'''
下面给出K=2，即由两个高斯分布组成的混合模型，分别是男女生身高分布。

已经给出了各自分布的比重、参数。用来检验算法生成的参数估计是否准确。
'''
content=np.array(content)/100;
print(content)

N=len(content);
k = 2;  # 高斯分布数量

data = content;
data.shape=N,1
# print(data);

# 随机初始化模型参数
Mu = np.random.random((1, 2));  # 平均值向量 # Mu[0][0]#Mu[0][1]
SigmaSquare = np.random.random((1, 2));  # 模型迭代用Sigma平方 # SigmaSquare[0][0]#SigmaSquare[0][1]

# 随机初始化各模型比重系数（大于等于0，且和为1）
a=np.random.random();
b = 1 - a;
Alpha = np.array([[a, b]]); # Alpha[0][0]#Alpha[0][1]

i = 0  # 迭代次数

flag=1000;
while flag:
    flag=flag-1;
    PreAlpha = Alpha.copy();
    i += 1;

    # Expectation
    gauss1 = Normal(data, Mu[0][0], np.sqrt(SigmaSquare[0][0]));  # 模型一
    gauss2 = Normal(data, Mu[0][1], np.sqrt(SigmaSquare[0][1]));


    Gamma1 = Alpha[0][0] * gauss1;
    Gamma2 = Alpha[0][1] * gauss2;

    M = Gamma1 + Gamma2;
    M=M+1e-47;
    # Gamma=np.concatenate((Gamma1/m,Gamma2/m),axis=1) 元素(j,k)为第j个样本来自第k个模型的概率，聚类时用来判别样本分类
    # Maximization
    # 更新Alpha
    # print(Gamma1)
    # print("\n")
    # print(M)
    # print('\n')
    # print(np.sum(Gamma1/M)/N)

    Alpha[0][0] = np.sum(Gamma1/M)/N;
    Alpha[0][1] = np.sum(Gamma2/M)/N;


    # 更新mu
    Mu[0][0] = np.dot((Gamma1 / M).T, data) / np.sum(Gamma1 / M);
    Mu[0][1] = np.dot((Gamma2 / M).T, data) / np.sum(Gamma2 / M);

    # 更新sigma
    SigmaSquare[0][0] = np.dot((Gamma1 / M).T, (data - Mu[0][0]) ** 2) / np.sum(Gamma1 / M)
    SigmaSquare[0][1] = np.dot((Gamma2 / M).T, (data - Mu[0][1]) ** 2) / np.sum(Gamma2 / M)

    if i % 1 == 0:
        print("第%d次迭代" % (i));
        print("Mu:", Mu);
        print("Sigma:", np.sqrt(SigmaSquare));
        print("Alpha:", Alpha);
        print(PreAlpha,Alpha,(PreAlpha - Alpha))

    # if ((PreAlpha - Alpha) ** 2).sum() < 1e-20:
        # break;