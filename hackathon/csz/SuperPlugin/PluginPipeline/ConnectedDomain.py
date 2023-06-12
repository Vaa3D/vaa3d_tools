import glob as glob
import SimpleITK as sitk
import numpy as np
import os

def DetachConnectedDomain(imgpath,resultpath):
    name=imgpath.split('\\')[-1]
    image=sitk.ReadImage(imgpath,sitk.sitkInt8)
    #image=sitk.GetArrayFromImage(imgraw)

    # 查找所有连通域并分配标签
    label_image = sitk.ConnectedComponent(image)
    label_image = sitk.RelabelComponent(label_image)

    # 保存每个连通域
    statistics_filter = sitk.LabelShapeStatisticsImageFilter()
    statistics_filter.Execute(label_image)
    number_of_labels = statistics_filter.GetNumberOfLabels()+1
    # if number_of_labels>5:
    #     return
    for label in range(1, number_of_labels):
        # 创建标签二值图像
        label_map = sitk.Threshold(label_image, label, label, 0)
        label_data=sitk.GetArrayFromImage(label_map)
        label_data[label_data!=0]=1
        if np.sum(label_data)<125:
            continue
        label_map=sitk.BinaryThreshold(label_map, lowerThreshold=label, upperThreshold=label, insideValue=255,
                             outsideValue=0)
        label_map=sitk.Cast(label_map,sitk.sitkUInt8)
        # Save the component to a file
        newname=name.split('.')[0]+"_"+str(label)+".tiff"
        sitk.WriteImage(label_map, os.path.join(resultpath,newname))



if __name__ == '__main__':
    resultpath=r"E:\cntdomain"
    files=glob.glob(r"E:\neuroseg\*.tiff")
    for file in files:
        DetachConnectedDomain(file,resultpath)

    neuronblocks=glob.glob(r"E:\neuroseg\*")
    for neuron in neuronblocks:
        name=neuron.split('\\')[-1]
        finalpath=os.path.join(resultpath,name)
        if os.path.exists(finalpath) is False:
            os.mkdir(finalpath)
        imgs=glob.glob(os.path.join(neuron,"*.tiff"))
        for img in imgs:
            DetachConnectedDomain(img,finalpath)


