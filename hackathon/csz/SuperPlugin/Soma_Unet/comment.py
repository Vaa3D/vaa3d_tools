import numpy as np
import torch



def Precision(logits, targets, class_index):
    logits[logits>=0.5]=1
    logits[logits < 0.5] = 0
    targets[targets >= 0.5] = 1
    targets[targets < 0.5] = 0
    tp=logits[:,class_index,:,:,:]*targets
    fp=logits[:,class_index,:,:,:]
    precision=torch.sum(tp)/torch.sum(fp)
    return precision

def Recall(logits, targets, class_index):
    logits[logits>=0.5]=1
    logits[logits < 0.5] = 0
    targets[targets >= 0.5] = 1
    targets[targets < 0.5] = 0
    tp = logits[:, class_index, :, :, :] * targets
    fn=targets
    recall = torch.sum(tp) / torch.sum(fn)
    return recall

def f1(logits, targets, class_index):
    recall=Recall(logits,targets,class_index)
    precision = Precision(logits, targets, class_index)
    f1=2*precision*recall/(precision+recall)
    return recall,precision,f1



