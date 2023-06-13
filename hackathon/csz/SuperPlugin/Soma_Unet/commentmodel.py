import glob as glob
import SimpleITK as sitk
import os
import numpy as np
import matplotlib.pyplot as plt
files=glob.glob(r"D:\A_DLcsz\DLtrain\fixed_data\label\*.tiff")


def Precision(logits, targets, class_index):
    logits[logits!=0]=1
    targets[targets !=0] = 1
    tp=logits*targets
    fp=logits
    precision=np.sum(tp)/np.sum(fp)
    return precision

def Recall(logits, targets, class_index):
    logits[logits!=0]=1
    targets[targets !=0] = 1
    tp = logits * targets

    fn=targets
    recall = np.sum(tp) / np.sum(fn)
    return recall

def f1(logits, targets, class_index):
    recall=Recall(logits,targets,class_index)
    precision = Precision(logits, targets, class_index)
    f1=2*precision*recall/(precision+recall)
    return recall,precision,f1


csv=[]
for file in files:
    name=file.split('\\')[-1]
    ilimg=sitk.ReadImage(os.path.join(r"D:\A_DLcsz\DLtrain\fixed_data\incremental",name),sitk.sitkInt8)
    ilimg=sitk.GetArrayFromImage(ilimg)
    ilimg=np.array([[ilimg]])
    limg=sitk.ReadImage(file,sitk.sitkInt8)
    limg=sitk.GetArrayFromImage(limg)
    limg=np.array([[limg]])

    csv.append(f1(ilimg,limg,0))
data=np.array(csv)
print(np.mean(data,axis=0))



labels='Recall','Precision','F1'
data6=data
fig = plt.figure()
plt.title('UnionTraining',fontsize=20)
view = plt.boxplot(data6,labels=labels)
plt.show()