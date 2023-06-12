# 简介
1、本项目是在tensorflow版本1.15.0的基础上对BERT模型进行的微调，使其适用于Vaa3D-x的插件推荐系统。

2、本项目为多标签分类BERT模型。

3、本项目可以在data文件夹下增加数据集进行扩充。

使用方法

 1、准备数据  
 使用dataprocess.py即可进行数据处理。数据格式为：classifier_multi_label/data/test_onehot.csv  
 2、参数设置  
 参考脚本 hyperparameters.py，直接修改里面的数值即可。  
 3、训练  
 python train.py  
 4、预测  
python predict.py  

预测的结果为插件的名称。

