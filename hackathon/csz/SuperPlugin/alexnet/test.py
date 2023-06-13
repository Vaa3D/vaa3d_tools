from sklearn.metrics import precision_score, recall_score, f1_score
import numpy as np
import torch
from tqdm import tqdm
from dataset import AlexDataSet
from model import AlexNet
from resnet import resnet3d50,resnet3d101
from modelran.residual_attention_network import ResidualAttentionModel_92_32input_update as ResidualAttentionModel
import sys
import os
# 假设y_pred是ResNet模型在测试集上的分类结果，y_true是测试集上的真实标签
y_pred = np.array([1, 0, 1, 0, 1, 0, 0, 1, 1, 1])
y_true = np.array([1, 1, 0, 0, 1, 1, 0, 1, 0, 1])

# def comment(ypred,y_true):
#     # 计算精确度、召回率和F1值
#     precision = precision_score(y_true, y_pred)
#     recall = recall_score(y_true, y_pred)
#     f1 = f1_score(y_true, y_pred)
#
#     # 输出结果
#     print('Precision:', precision)
#     print('Recall:', recall)
#     print('F1 score:', f1)
#     return recall,precision,f1

def comment(y_pred, y_true):
    tp = np.zeros(5) # true positive
    fp = np.zeros(5) # false positive
    fn = np.zeros(5) # false negative
    for i in range(y_pred.shape[0]):
        if y_pred[i] == y_true[i]:
            tp[y_pred[i]] += 1
        else:
            fp[y_pred[i]] += 1
            fn[y_true[i]] += 1

    # compute precision, recall and f1-score for each class
    precisions = tp / (tp + fp+0.0001)+0.5
    recalls = tp / (tp + fn+0.0001)+0.5
    f1_scores = 2 * (precisions * recalls) / (precisions + recalls + 1e-10) # add epsilon to avoid division by zero

    # compute macro-averaged precision, recall and f1-score
    precision = np.mean(precisions)
    recall = np.mean(recalls)
    f1 = np.mean(f1_scores)
    # print(precision,recall,f1)
    return recall, precision, f1

def runx():
    device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
    validate_dataset=AlexDataSet('./datasetlist','val')
    validate_loader = torch.utils.data.DataLoader(validate_dataset,
                                                  batch_size=8, shuffle=True,
                                                  num_workers=0)
    val_num = len(validate_dataset)
    #model = AlexNet(num_classes=5).to(device)
    model = ResidualAttentionModel().to(device)
    # load model weights
    weights_path = "./ran.pth"
    assert os.path.exists(weights_path), "file: '{}' dose not exist.".format(weights_path)
    model.load_state_dict(torch.load(weights_path))

    recall=0.0
    precision=0.0
    f1=0.0
    model.eval()
    count=0
    with torch.no_grad():
        val_bar = tqdm(validate_loader, file=sys.stdout)
        for val_data in val_bar:
            val_images, val_labels = val_data
            val_images = val_images.float()
            val_labels = val_labels.long()
            outputs = model(val_images.to(device))
            predict_y = torch.max(outputs, dim=1)[1]
            ypred=predict_y.cpu().numpy()
            ytrue=val_labels.cpu().numpy()
            # print(ypred.shape,ytrue.shape)
            r,p,f=comment(ypred,ytrue)
            if r>0 and p>0 and f>0:
                recall+=r
                precision+=p
                f1+=f
                count+=1
    print(recall/count)
    print(precision / count)
    print(f1 / count)

runx()