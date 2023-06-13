import glob as glob
from pylib.swc_processing import swc_to_image
import os
import numpy as np
from pylib.img_io import Readimg
from copy import deepcopy
import pandas as pd


APrate1 = {
    "ImageName": [],
    "app1": [],
    "app2": [],
    #"snake": [],
    "Advantra": [],
    "MOST": [],
    "EnsembleNeuronTracerBasic": [],
    # "Rollerball": [],
    "meanshift": [],
    "TreMap": [],
    "MST_Tracing": [],
    "EnsembleNeuronTracerV2n": [],
    #"LCMboost": [],
    #"nctuTW_GD": [],
    #"NeuroStalker": [],
    "spanningtree": [],
    #"Cwlab_ver1": [],
    #"neutu_autotrace": [],
    "axis_analyzer": [],
    #"nctuTW": [],
    "Rayshooting": [],
    "smartTracing": [],
    "simple": [],
    "EnsembleNeuronTracerV2s": [],
    # "NeuronChaser": [],
    #"neutube": []
}
APrate2 = {
    "ImageName": [],
    "app1": [],
    "app2": [],
    #"snake": [],
    "Advantra": [],
    "MOST": [],
    "EnsembleNeuronTracerBasic": [],
    # "Rollerball": [],
    "meanshift": [],
    "TreMap": [],
    "MST_Tracing": [],
    "EnsembleNeuronTracerV2n": [],
    # "LCMboost": [],
    # "nctuTW_GD": [],
    # "NeuroStalker": [],
    "spanningtree": [],
    # "Cwlab_ver1": [],
    # "neutu_autotrace": [],
    "axis_analyzer": [],
    # "nctuTW": [],
    "Rayshooting": [],
    "smartTracing": [],
    "simple": [],
    "EnsembleNeuronTracerV2s": [],
    # "NeuronChaser": [],
    # "neutube": []
}
APdice = {
    "ImageName": [],
    "app1": [],
    "app2": [],
    #"snake": [],
    "Advantra": [],
    "MOST": [],
    "EnsembleNeuronTracerBasic": [],
    # "Rollerball": [],
    "meanshift": [],
    "TreMap": [],
    "MST_Tracing": [],
    "EnsembleNeuronTracerV2n": [],
    # "LCMboost": [],
    # "nctuTW_GD": [],
    # "NeuroStalker": [],
    "spanningtree": [],
    # "Cwlab_ver1": [],
    # "neutu_autotrace": [],
    "axis_analyzer": [],
    # "nctuTW": [],
    "Rayshooting": [],
    "smartTracing": [],
    "simple": [],
    "EnsembleNeuronTracerV2s": [],
    # "NeuronChaser": [],
    # "neutube": []
}
Aset= {"ImageName",
    "app1",
    "app2",
    # "snake",
    "Advantra",
    "MOST",
    "EnsembleNeuronTracerBasic",
    # "Rollerball",
    "meanshift",
    "TreMap",
    "MST_Tracing",
    "EnsembleNeuronTracerV2n",
    # "LCMboost",
    # "nctuTW_GD",
    # "NeuroStalker",
    "spanningtree",
    # "Cwlab_ver1",
    # "neutu_autotrace",
    "axis_analyzer",
    # "nctuTW",
    "Rayshooting",
    "smartTracing",
    "simple",
    "EnsembleNeuronTracerV2s",
    # "NeuronChaser",
    # "neutube"
    }



def appendap(dictvalue):
    print("start appending!")
    tempset=deepcopy(Aset)
    delset=[]
    count=0
    for item in dictvalue:
        count=len(item[1])
        if item[0] in tempset:
            for record in item[1]:
                ImageName=record[0]
                rate1=record[1]
                rate2=record[2]
                dice=record[3]
                if ImageName not in APrate1['ImageName']:
                    APrate1['ImageName'].append(ImageName)
                APrate1[item[0]].append(rate1)
                if ImageName not in APrate2['ImageName']:
                    APrate2['ImageName'].append(ImageName)
                APrate2[item[0]] .append(rate2)
                if ImageName not in APdice['ImageName']:
                    APdice['ImageName'].append(ImageName)
                APdice[item[0]] .append(dice)
            delset.append(item[0])
    for d in delset:
        tempset.remove(d)
    # print("count:",count)
    for item in tempset:
        if item=="ImageName":
            continue
        for i in range(count):
            APrate1[item].append(-1)
            APrate2[item].append(-1)
            APdice[item].append(-1)
    # for item in APrate1:
    #     print(item,len(APrate1[item]))
    # for item in APrate2:
    #     print(item,len(APrate2[item]))
    # for item in APdice:
    #     print(item,len(APdice[item]))
    # print(APrate1["ImageName"])




def NeuronAnalysis(img,label):
    assert img.shape==label.shape
    consensus=np.multiply(img,label)
    inter=np.sum(consensus)
    union=np.sum(img)+np.sum(label)
    dice = (2. * inter + 0.0000001) / (union + 0.0000001)
    recall=inter/(np.sum(img)+ 0.0000001)
    precision = inter / (np.sum(label)+ 0.0000001)
    f1=(2*recall*precision)/(recall+precision+ 0.0000001)
    return recall,precision,f1,dice

def main():
    path=r'E:\result'
    imgnamelist=glob.glob(path+"\*")
    for imgnamefile in imgnamelist:
        files=glob.glob(os.path.join(imgnamefile, "*"))
        if len(files)<4:
            continue
        dictvalue=[]
        for file in files:
            print(file)
            algorithm=file.split('\\')[-1]
            if algorithm not in Aset:
                continue
            if algorithm=="gold" or algorithm=="raw":
                continue
            ap=[]
            for imgblock in glob.glob(os.path.join(imgnamefile,"gold/*")):
                block=imgblock.split('\\')[-1].split('gold')[0]
                tracingmaskpath=os.path.join(file,block+algorithm+".tiff")
                goldmask=Readimg(imgblock)
                tracingmask=Readimg(tracingmaskpath)
                recall,precision,f1,dice=NeuronAnalysis(goldmask,tracingmask)
                ap.append([imgblock,recall,precision,f1,dice])
            dictvalue.append([algorithm,ap])
        appendap(dictvalue)
        print(APrate1)
        print(APrate2)
        print(APdice)
    rate1Data = pd.DataFrame(APrate1)
    rate1csv = rate1Data.to_csv("performance/rate1_win.csv")
    rate2Data = pd.DataFrame(APrate2)
    rate2csv = rate2Data.to_csv("performance/rate2_win.csv")
    diceData = pd.DataFrame(APdice)
    dicecsv = diceData.to_csv("performance/dice_win.csv")
    print(rate1csv)
    print(rate2csv)
    print(dicecsv)




if __name__ == '__main__':
    main()
