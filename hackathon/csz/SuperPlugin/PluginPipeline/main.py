import pandas as pd
import numpy as np
from img_io import *
import glob as glob


Aset= {
    "ImageName",
    "app2new3",
    "Advantra_updated",
    "MOST",
    #"EnsembleNeuronTracerBasic",
    "TreMap",
    "MST_Tracing_Ws_21_th_170_updated",
    "EnsembleNeuronTracerV2n",
    "smartTracing",
    "NeuroGPSTree_updated",
    "EnsembleNeuronTracerV2s",
    "ENT_updated"
    }

def sigmoid(x):
    return 1 / (1 + np.exp(-x))

def multilabel():       #first step to multilabel the alperformance
    print("-----label-----")
    idata=pd.read_csv("labeldata/dice_win.csv")        #need to change
    data=np.array(idata)

    csv=[]

    #need to change according the csv title
    totalal=['ImageName','Advantra_updated','MOST','EnsembleNeuronTracerV2n','EnsembleNeuronTracerV2s','ENT_updated']

    csv.append(totalal)
    # print(data)
    for line in data:
        temp=[]
        if np.max(line[3:])<0.1:
            continue
        maxindex=np.argmax(line[3:])+3
        # print(line[0],maxindex)
        print(line[maxindex])
        value=line[maxindex]-0.1
        if value<0:
            value=0
        temp.append(line[1])
        for i in range(len(line)-3):
            if line[i+3]>=value:
                temp.append(1)
            # if i+2==maxindex:
            #     temp.append(1)
            # elif i+2==maxindex and line[i+2]>0:
            #     temp.append(1)
            else:
                temp.append(0)
        csv.append(temp)

    csv=np.array(csv)
    # print(csv)
    Writecsv(csv,"labeldata/multilabel.csv")

def summarycsv():
    path=r"E:\result\*\raw\*.csv"
    csvfiles=glob.glob(path)
    sumcsv=[]
    columes=['Image_name','Channel','MinIntensity','MaxIntensity','MeanIntensity','MedianIntensity','MADIntensity','StdIntensity','PercentMinimal','PercentMaximal','ThresholdOtsu','SNR_mean','CNR_mean','SNR_otsu','CNR_otsu','FocusScore']
    sumcsv.append(columes)
    for csvfile in csvfiles:
        data=pd.read_csv(csvfile)
        name=data.values[0][0].split('\\')[-1]
        line=[]
        line.append(name)
        for i in range(len(data.values[0])-1):
            # print(data.values[0][i+1])
            line.append(data.values[0][i+1])

        sumcsv.append(line)
    sumcsv=np.array(sumcsv)
    print(sumcsv)
    Writecsv(sumcsv,"image_quality_raw.csv")


def washdata():
    imgperformance = pd.read_csv("labeldata/one_hot_win.csv")
    imgperclass = []
    for line in imgperformance.values:
        # print(line)
        temp=[]
        for i in range(len(line)-1):
            if line[i+1]==1:
                temp.append(line[0])
                temp.append(i)
                break
        imgperclass.append(temp)

    arr=np.array(np.array(imgperclass)[:,1],dtype='int')
    hist, bins = np.histogram(arr, bins=np.arange(arr.min(), arr.max() + 2))
    # 打印结果
    delnum=[]
    print("元素   出现次数")
    for i in range(len(hist)):
        print(bins[i], "        ", hist[i])
        if hist[i]<450:
            delnum.append(bins[i])
    print(delnum)
    data=np.array(imgperformance)
    for i in range(len(delnum)):
        index=delnum[(i+1)*-1]
        data=np.delete(data,index+1,axis=1)
    print(data.shape)

    dataperclass = []
    count=0
    for line in data:
        # print(line)
        temp=[]
        for i in range(len(line)-1):
            if line[i+1]==1:
                temp.append(line[0])
                temp.append(i)
                break
        if len(temp)==0:
            count+=1
            continue
        dataperclass.append(temp)

    arr=np.array(np.array(dataperclass)[:,1],dtype='int')
    hist, bins = np.histogram(arr, bins=np.arange(arr.min(), arr.max() + 2))
    # 打印结果
    print("元素   出现次数")
    for i in range(len(hist)):
        print(bins[i], "        ", hist[i])
    print(imgperformance.columns)
    firstline=""
    for i in range(len(imgperformance.columns)):
        if i in delnum:
            print(i,imgperformance.columns[i])
            continue
        firstline+=imgperformance.columns[i]+", "
    print(firstline)
    newdata=[]
    for line in data:
        for i in range(len(line)-1):
            if line[i+1]==1:
                newdata.append(line)
                break
    newdata=np.array(newdata)
    print(newdata.shape)
    Writecsv(newdata,"labeldata/one_hot_win_washed.csv",firstline=firstline)

def svmcsv():
    imgquality=pd.read_csv("image_quality_raw.csv")
    imgperformance=pd.read_csv("one_hot_win_washed.csv")
    # print(imgquality,imgperformance)
    imgperclass=[]
    count=0
    for line in imgperformance.values:
        # print(line)
        temp=[]
        for i in range(len(line)-1):
            if line[i+1]==1:
                temp.append(line[0])
                temp.append(i)
                break
        if len(temp)==0:
            continue
        imgperclass.append(temp)
    print(count)
    # print(imgperclass)
    arr=np.array(np.array(imgperclass)[:,1],dtype='int')
    hist, bins = np.histogram(arr, bins=np.arange(arr.min(), arr.max() + 2))

    # 打印结果
    print("元素   出现次数")
    for i in range(len(hist)):
        print(bins[i], "        ", hist[i])

    finaldata=[]
    firstline=['Image_name', 'Channel', 'MinIntensity', 'MaxIntensity', 'MeanIntensity', 'MedianIntensity', 'MADIntensity', 'StdIntensity', 'PercentMinimal', 'PercentMaximal', 'ThresholdOtsu', 'SNR_mean', 'CNR_mean', 'SNR_otsu', 'CNR_otsu', 'FocusScore', 'label']
    finaldata.append(firstline)
    for iqline in imgquality.values:
        for ipline in imgperclass:
            name=ipline[0].split('\\')[-1].split('_gold')[0]
            # print(ipline[1])
            if name in iqline[0]:
                line=[]
                line.append(ipline[0])
                # print(iqline)
                for i in range(len(iqline)-2):
                    # print(iqline[i+1])
                    if iqline[i+1]==' inf':
                        # print(iqline[i+1])
                        line.append(1e12)
                    elif iqline[i+1]==' nan':
                        # print(iqline[i+1])
                        line.append(1e12)
                    else:
                        line.append(iqline[i+1])
                line.append(ipline[1])
                # print(line)
                finaldata.append(line)
    print(finaldata)
    Writecsv(np.array(finaldata),"dataset_win_raw.csv")

def multilabelsummary():             #second step to get the final label encode to onehot
    idata=pd.read_csv("labeldata/multilabel.csv")      #need to change
    data=idata.values
    data=np.delete(data,-1,axis=1)
    # # print(data)
    # s=set()
    datasum=np.sum(data[:,1:],axis=0)       #get number of each algorithm

    # totalnum=np.sum(datasum)                #get the 1-possibility of each algorithm
    # p=datasum/totalnum
    # for i in range(datasum.shape[0]):
    #     p[i]=totalnum/(datasum.shape[0]*datasum[i])
    # p=p/np.sum(p)
    # print(p)



    columes=idata.columns[1:-1]             #get algorithm name
    # print(p)
    # print(columes)
    # print(datasum)
    index=np.argsort(datasum)               #get the sort of the algorithm according to the multilabel
    print(index)
    print(columes)

    sortal=columes[index]

    s=[]
    n=[]
    for line in data:
        temp = []
        tempp=[]
        for i in range(len(line)-1):        #get all the algorithm labeled
            if line[i+1]==1:
                temp.append(columes[i])
                # tempp.append(p[i])

        if len(temp)==0:
            continue

        no=[]                               #Mark the algorithm with the least number of occurrences
        for a in temp:
            for i in range(len(sortal)):
                if sortal[i]==a:
                    no.append(i)
                    break
        no=np.array(no)
        maxindex=np.argmax(no)

        # tempp=np.array(tempp)
        # totalp=np.sum(tempp)
        # tempp=tempp/totalp
        # selected=np.random.choice(temp,p=tempp)
        #print(selected)



        classal=temp[maxindex]

        # classal=selected
        s.append(classal)
        n.append(line[0])
    finalal=[]
    for aln in set(s):
        finalal.append(aln)
    # print(finalal)
    finalcsv=[]
    firstline=[]
    firstline.append('ImageName')
    for aln in finalal:
        firstline.append(aln)
    finalcsv.append(firstline)
    # print(firstline)
    for i in range(len(n)):
        finaline=[]
        name=n[i]
        finaline.append(name)
        for j in finalal:
            if s[i]==j:
                finaline.append(1)
            else:
                finaline.append(0)
        finalcsv.append(finaline)
    finalcsv=np.array(finalcsv)
    Writecsv(finalcsv,"labeldata/one_hot_win.csv")



if __name__=="__main__":
    multilabel()
    # multilabelsummary()
    # washdata()
