import os
import sys
import json

import torch
import torch.nn as nn
from torchvision import transforms, datasets, utils
import matplotlib.pyplot as plt
import numpy as np
import torch.optim as optim
from tqdm import tqdm

from modelran.residual_attention_network import ResidualAttentionModel_92_32input_update as ResidualAttentionModel
from dataset import *
import logger
from collections import OrderedDict


def adjust_learning_rate(optimizer, epoch, lr):
    """Sets the learning rate to the initial LR decayed by 10 every 30 epochs"""
    lr = lr * (0.1 ** (epoch // 150))
    for param_group in optimizer.param_groups:
        param_group['lr'] = lr


def calculate_acuracy_mode_one(model_pred, labels, value=0.5):
    # 注意这里的model_pred是经过sigmoid处理的，sigmoid处理后可以视为预测是这一类的概率
    # 预测结果，大于这个阈值则视为预测正确
    accuracy_th = value
    pred_result = model_pred > accuracy_th
    pred_result = pred_result.float()

    pred_one_num = torch.sum(pred_result)
    if pred_one_num == 0:
        return 0, 0,0

    target_one_num = torch.sum(labels)

    true_predict_num = torch.sum(pred_result * labels)
    # 模型预测的结果中有多少个是正确的
    precision = true_predict_num / pred_one_num
    # 模型预测正确的结果中，占所有真实标签的数量
    recall = true_predict_num / target_one_num
    f1=2*precision*recall/(precision+recall+0.00001)
    return precision,recall,f1


def calculate_acuracy_mode_two(model_pred, labels, ftop=2):
    # 取前top个预测结果作为模型的预测结果
    precision = 0
    recall = 0
    top = ftop
    # 对预测结果进行按概率值进行降序排列，取概率最大的top个结果作为模型的预测结果
    pred_label_locate = torch.argsort(model_pred, descending=True)[:, 0:top]
    for i in range(model_pred.shape[0]):
        temp_label = torch.zeros(1, model_pred.shape[1])
        temp_label[0,pred_label_locate[i]] = 1
        target_one_num = torch.sum(labels[i])
        true_predict_num = torch.sum(temp_label * labels[i])
        # 对每一幅图像进行预测准确率的计算
        precision += true_predict_num / top

        # 对每一幅图像进行预测查全率的计算
        recall += true_predict_num / target_one_num
    precision=precision/model_pred.shape[0]
    recall=recall/model_pred.shape[0]
    f1=2*precision*recall/(precision+recall+0.00001)
    return precision,recall,f1

def main():
    device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
    print("using {} device.".format(device))

    batch_size = 1

    train_dataset=AlexDataSet('./datasetlist','train')
    validate_dataset=AlexDataSet('./datasetlist','val')

    val_num=len(validate_dataset)
    train_loader = torch.utils.data.DataLoader(train_dataset,
                                               batch_size=batch_size, shuffle=True,
                                               num_workers=1)

    validate_loader = torch.utils.data.DataLoader(validate_dataset,
                                                  batch_size=8, shuffle=True,
                                                  num_workers=1)

    net = ResidualAttentionModel().to(device)
    pytorch_total_params = sum(p.numel() for p in net.parameters())
    pytorch_total_required_params = sum(p.numel() for p in net.parameters() if p.requires_grad)
    print(pytorch_total_params, pytorch_total_required_params)
    # weights=[0.216988022,0.867952089,13.26339286,11.47104247,3.723057644,4.935215947,1.651473041]
    # weights=torch.Tensor(weights).to(device)
    # loss_function = nn.CrossEntropyLoss(weight=weights)

    loss_function = nn.CrossEntropyLoss()          #single-label
    # loss_function = nn.BCELoss()                    #multi-label
    # pata = list(net.parameters())
    lr=0.0002
    optimizer = optim.Adam(net.parameters(), lr=lr)

    epochs = 500
    save_path = './ran.pth'
    best_acc = 0.0
    train_steps = len(train_loader)

    log = logger.Logger('log')
    for epoch in range(epochs):
        # train
        adjust_learning_rate(optimizer,epoch,lr)
        for param_group in optimizer.param_groups:
            print(param_group['lr'])
        net.train()
        running_loss = 0.0
        train_bar = tqdm(train_loader, file=sys.stdout)
        for step, data in enumerate(train_bar):
            images, labels = data
            images=images.float()
            labels=labels.long()
            optimizer.zero_grad()
            print(images.shape)
            outputs = net(images.to(device))

            # if epoch%90==0:
            #     predict=torch.softmax(outputs,dim=1)
            #     predict=torch.argmax(predict,dim=1)
            #     print(predict,labels)
            # print(outputs,labels)
            loss = loss_function(outputs, labels.to(device))
            loss.backward()
            optimizer.step()

            # print statistics
            running_loss += loss.item()

            train_bar.desc = "train epoch[{}/{}] loss:{:.3f}".format(epoch + 1,
                                                                     epochs,
                                                                     loss)

        # validate
        net.eval()
        acc = 0.0  # accumulate accurate number / epoch
        pre = 0.0
        rec = 0.0
        with torch.no_grad():
            val_bar = tqdm(validate_loader, file=sys.stdout)
            for val_data in val_bar:
                val_images, val_labels = val_data
                val_images=val_images.float()
                val_labels=val_labels.long()
                outputs = net(val_images.to(device))
                predict_y = torch.max(outputs, dim=1)[1]
                # precision,recall,f1=calculate_acuracy_mode_two(outputs,val_labels)
                # pre+=precision.item()
                # rec+=recall.item()
                # acc+=f1.item()

                acc += torch.eq(predict_y, val_labels.to(device)).sum().item()
        # print(pre/val_num,rec/val_num)
        val_accurate = acc / val_num
        print('[epoch %d] train_loss: %.3f  val_accuracy: %.3f' %
              (epoch + 1, running_loss / train_steps, val_accurate))
        train_log=OrderedDict({'Train Loss': running_loss / train_steps})
        val_log=OrderedDict({'val_accuracy': val_accurate})
        log.update(epoch+1, train_log, val_log)
        if val_accurate > best_acc:
            best_acc = val_accurate
            torch.save(net.state_dict(), save_path)

    print('Finished Training')


if __name__ == '__main__':
    main()
