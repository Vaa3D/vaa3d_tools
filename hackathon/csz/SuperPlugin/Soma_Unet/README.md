## SOMA3DUNet implemented with pytorch

## Introduction
soma_unet用作东南大学脑智院的soma segmentation项目。

### Directory
train.py,test.py,predict.py用作基础的训练。
config.py用作超参数设置。
FT.py为对网络的微调，ILtrain.py为增量学习，两者起对照作用。
utilsa中可自定义损失函数
process中有预处理和后处理代码
models中为各种模型。
dataset中定义了数据读取格式。
output为训练好的模型。