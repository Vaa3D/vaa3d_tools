import glob as glob
from pylib.swc_processing import swc_to_image
import os
import numpy as np
from pylib.img_io import Readimg
from copy import deepcopy
import pandas as pd


APrecall = {
    "ImageName": [],
    # "app2new3": [],
    "Advantra_updated": [],
    "MOST": [],
    "EnsembleNeuronTracerBasic": [],
    # "TreMap": [],
    # "MST_Tracing_Ws_21_th_170_updated": [],
    "EnsembleNeuronTracerV2n": [],
    # "smartTracing": [],
    # "NeuroGPSTree_updated": [],
    "EnsembleNeuronTracerV2s": [],
    "ENT_updated": []
}
APprecision = {
    "ImageName": [],
    # "app2new3": [],
    "Advantra_updated": [],
    "MOST": [],
    "EnsembleNeuronTracerBasic": [],
    # "TreMap": [],
    # "MST_Tracing_Ws_21_th_170_updated": [],
    "EnsembleNeuronTracerV2n": [],
    # "smartTracing": [],
    # "NeuroGPSTree_updated": [],
    "EnsembleNeuronTracerV2s": [],
    "ENT_updated": []
}
APf1 = {
    "ImageName": [],
    # "app2new3": [],
    "Advantra_updated": [],
    "MOST": [],
    "EnsembleNeuronTracerBasic": [],
    # "TreMap": [],
    # "MST_Tracing_Ws_21_th_170_updated": [],
    "EnsembleNeuronTracerV2n": [],
    # "smartTracing": [],
    # "NeuroGPSTree_updated": [],
    "EnsembleNeuronTracerV2s": [],
    "ENT_updated": []
}
APdice = {
    "ImageName": [],
    # "app2new3": [],
    "Advantra_updated": [],
    "MOST": [],
    "EnsembleNeuronTracerBasic": [],
    # "TreMap": [],
    # "MST_Tracing_Ws_21_th_170_updated": [],
    "EnsembleNeuronTracerV2n": [],
    # "smartTracing": [],
    # "NeuroGPSTree_updated": [],
    "EnsembleNeuronTracerV2s": [],
    "ENT_updated": []
}

Aset= {
    "ImageName",
    # "app2new3",
    "Advantra_updated",
    "MOST",
    "EnsembleNeuronTracerBasic",
    # "TreMap",
    # "MST_Tracing_Ws_21_th_170_updated",
    "EnsembleNeuronTracerV2n",
    # "smartTracing",
    # "NeuroGPSTree_updated",
    "EnsembleNeuronTracerV2s",
    "ENT_updated"
    }



def appendap(dictvalue):
    print("start appending!")
    tempset=deepcopy(Aset)
    delset=[]
    count=0
    for item in dictvalue:
        count=len(item[1])
        print(item[0])
        for tsitem in tempset:
            if tsitem in item[0]:
                labelname=tsitem
                break
        if labelname in tempset:
            for record in item[1]:
                ImageName=record[0]
                recall=record[1]
                precision=record[2]
                f1=record[3]
                dice=record[4]
                if ImageName not in APrecall['ImageName']:
                    APrecall['ImageName'].append(ImageName)
                APrecall[labelname].append(recall)
                if ImageName not in APprecision['ImageName']:
                    APprecision['ImageName'].append(ImageName)
                APprecision[labelname] .append(precision)
                if ImageName not in APf1['ImageName']:
                    APf1['ImageName'].append(ImageName)
                APf1[labelname] .append(f1)
                if ImageName not in APdice['ImageName']:
                    APdice['ImageName'].append(ImageName)
                APdice[labelname] .append(dice)
            delset.append(labelname)
    for d in delset:
        tempset.remove(d)
    for item in tempset:
        if item=="ImageName":
            continue
        for i in range(count):
            APrecall[item].append(-1)
            APprecision[item].append(-1)
            APf1[item].append(-1)
            APdice[item].append(-1)


def checkcsv(csv):
    itemlen=csv["ImageName"]
    for item in csv:
        if len(csv[item])!=len(itemlen):
            print(item,len(csv[item]))
            return -1
    return 1

def NeuronAnalysis(img,label):
    assert img.shape==label.shape
    consensus=np.multiply(img,label)
    inter=np.sum(consensus)
    union=np.sum(img)+np.sum(label)
    dice = (2. * inter ) / (union + 0.0000001)
    recall=inter/(np.sum(img)+ 0.0000001)
    precision = inter / (np.sum(label)+ 0.0000001)
    f1=(2*recall*precision)/(recall+precision+ 0.0000001)
    return recall,precision,f1,dice

dataset=[r"E:\result1\140918c3.tif.v3dpbd",
r"E:\result1\140918c7.tif.v3dpbd",
r"E:\result1\140918c8.tif.v3dpbd",
r"E:\result1\140918c9.tif.v3dpbd",
r"E:\result1\140921c1.tif.v3dpbd",
r"E:\result1\140921c12.tif.v3dpbd",
r"E:\result1\140921c14.tif.v3dpbd",
#r"E:\result1\140921c16.tif.v3dpbd",
r"E:\result1\140921c22.tif.v3dpbd",
r"E:\result1\140921c3.tif.v3dpbd",
r"E:\result1\140921c4.tif.v3dpbd",
r"E:\result1\140921c5.tif.v3dpbd",
r"E:\result1\140921c6.tif.v3dpbd",
r"E:\result1\140921c9.tif.v3dpbd",
r"E:\result1\14_3dpf_Live_1-28-2010_6-16-41_PM_med_Red.tif_uint8.v3dpbd"]

def main():
    for testname in dataset:
        path=r'E:\result1'
        testimgname=testname.split('\\')[-1]
        imgnamelist=glob.glob(path+"\*")
        # count=0
        for imgnamefile in imgnamelist:
            # if count==1:
            #     break
            # count+=1
            print(imgnamefile)
            if imgnamefile!=testname:
                continue
            if imgnamefile.split('\\')[-1]=="CropCenter":
                continue
            files=glob.glob(os.path.join(imgnamefile, "*"))
            if len(files)<4:
                continue
            dictvalue=[]
            for file in files:
                # print(file)
                algorithm=file.split('\\')[-1]
                # print(algorithm)
                flag=False
                for alitem in Aset:
                    if alitem in algorithm:
                        flag=True
                if flag==False:
                    continue
                print(file)
                ap=[]
                for imgblock in glob.glob(os.path.join(imgnamefile,"gold/*.tiff")):
                    block=imgblock.split('\\')[-1].split('gold')[0]
                    tracingmaskpath=os.path.join(file,block+algorithm+".tiff")
                    goldmask=Readimg(imgblock)
                    tracingmask=Readimg(tracingmaskpath)
                    recall,precision,f1,dice=NeuronAnalysis(goldmask,tracingmask)
                    ap.append([imgblock,recall,precision,f1,dice])
                dictvalue.append([algorithm,ap])
            appendap(dictvalue)
        # recallData = pd.DataFrame(APrecall)
        # recallcsv = recallData.to_csv("labeldata/recall_win.csv")
        # precisionData = pd.DataFrame(APprecision)
        # precisioncsv = precisionData.to_csv("labeldata/precision_win.csv")
        # f1Data = pd.DataFrame(APf1)
        # f1csv = f1Data.to_csv("labeldata/f1_win.csv")
        diceData = pd.DataFrame(APdice)
        #dicecsv = diceData.to_csv("labeldata/dice_win.csv")
        dicecsv = diceData.to_csv(r"E:\testdice/"+testimgname+".csv")
        # print(recallcsv)
        # print(precisioncsv)
        # print(f1csv)
        # print(dicecsv)




if __name__ == '__main__':
    main()
