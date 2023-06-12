import pandas as pd
import numpy as np
from pylib.img_io import *
import glob as glob





def onehot():
    idata=pd.read_csv("performance/rate1_win.csv")
    data=np.array(idata)

    jdata=pd.read_csv("performance/dice_win.csv")
    jdata = np.array(jdata)
    # data=data[:,2:]
    # print(data)

    # index=np.argmax(data,axis=1)
    # s=set()
    # # print(idata.columns)
    # for i in index:
    #     # print(idata.columns[2+i])
    #     s.add(idata.columns[2+i])
    # # print(s)
    # encode=[]
    # for col in idata.columns:
    #     if col in s:
    #         encode.append(col)
    # print(encode)
    csv=[]
    #totalal=['ImageName','app1','app2','snake','Advantra','MOST','EnsembleNeuronTracerBasic','Rollerball','meanshift','TreMap','MST_Tracing','EnsembleNeuronTracerV2n','LCMboost','nctuTW_GD','NeuroStalker','spanningtree','Cwlab_ver1','neutu_autotrace','axis_analyzer','nctuTW','Rayshooting','smartTracing','simple','EnsembleNeuronTracerV2s','NeuronChaser','neutube']
    totalal = ["ImageName",
    "app1",
    "app2",
    "Advantra",
    "MOST",
    "EnsembleNeuronTracerBasic",
    "meanshift",
    "TreMap",
    "MST_Tracing",
    "EnsembleNeuronTracerV2n",
    "spanningtree",
    "axis_analyzer",
    "Rayshooting",
    "smartTracing",
    "simple",
    "EnsembleNeuronTracerV2s"]
    # columes=['name','app1', 'app2', 'Advantra', 'MOST', 'Rollerball', 'meanshift', 'TreMap', 'MST_Tracing', 'EnsembleNeuronTracerV2n', 'Cwlab_ver1', 'neutu_autotrace', 'Rayshooting', 'simple']
    # csv.append(['name','app1', 'app2', 'Advantra', 'MOST', 'Rollerball', 'meanshift', 'TreMap', 'MST_Tracing', 'EnsembleNeuronTracerV2n', 'Cwlab_ver1', 'neutu_autotrace', 'Rayshooting', 'simple'])
    # for i in range(index.shape[0]):
    #     line=[]
    #     line.append(idata.values[i][1])
    #     for j in range(len(columes)-1):
    #         if columes[j+1]==idata.columns[2+index[i]]:
    #             line.append(1)
    #         else:
    #             line.append(0)
    #     csv.append(line)

    csv.append(totalal)
    # print(len(csv[0]))
    # for line in data:
    #     temp=[]
    #     maxindex=np.argmax(line[2:])+2
    #     temp.append(line[1])
    #     for i in range(len(line)-2):
    #         if line[i+2]>0.7:
    #             temp.append(1)
    #         elif i+2==maxindex and line[i+2]>0:
    #             temp.append(1)
    #         else:
    #             temp.append(0)
    #     # temp=np.array(temp)
    #     # print(len(temp))
    #     csv.append(temp)
    #     # break
    for i in range(data.shape[0]):
        temp=[]
        maxindex=np.argmax(data[i][2:])+2
        temp.append(data[i][1])
        for i in range(len(data[i])-2):
            if data[i][i+2]>0.5 and data[i][i+2]>0.5:
                temp.append(1)
            elif i+2==maxindex and data[i][i+2]>0:
                temp.append(1)
            else:
                temp.append(0)
        # temp=np.array(temp)
        # print(len(temp))
        csv.append(temp)
    # print(csv)
    csv=np.array(csv)
    print(csv)
    # print(csv.shape)
    Writecsv(csv,"analysis/label_win.csv")

def summarycsv():
    path=r"D:\A_pythonwork\PluginPipeline\gold\imgtiff\*.csv"
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
        print(line)
        sumcsv.append(line)
    sumcsv=np.array(sumcsv)
    # print(sumcsv)
    Writecsv(sumcsv,"image_quality.csv")

def svmcsv():
    imgquality=pd.read_csv("image_quality.csv")
    imgperformance=pd.read_csv("one_hot3.csv")
    # print(imgquality,imgperformance)
    imgperclass=[]
    for line in imgperformance.values:
        # print(line)
        temp=[]
        for i in range(len(line)-1):
            if line[i+1]==1:
                temp.append(line[0])
                temp.append(i+1)
                break
        imgperclass.append(temp)
    # print(imgperclass)
    finaldata=[]
    firstline=['Image_name', 'Channel', 'MinIntensity', 'MaxIntensity', 'MeanIntensity', 'MedianIntensity', 'MADIntensity', 'StdIntensity', 'PercentMinimal', 'PercentMaximal', 'ThresholdOtsu', 'SNR_mean', 'CNR_mean', 'SNR_otsu', 'CNR_otsu', 'FocusScore', 'label']
    finaldata.append(firstline)
    for iqline in imgquality.values:
        for ipline in imgperclass:
            if ipline[0] in iqline[0]:
                line=[]
                line.append(ipline[0])
                # print(iqline)
                for i in range(len(iqline)-2):
                    # print(iqline[i+1])
                    if iqline[i+1]==' inf':
                        # print(iqline[i+1])
                        line.append(1e-12)
                    elif iqline[i+1]==' nan':
                        # print(iqline[i+1])
                        line.append(1e-12)
                    else:
                        line.append(iqline[i+1])
                line.append(ipline[1])
                # print(line)
                finaldata.append(line)
    print(finaldata)
    Writecsv(np.array(finaldata),"dataset2.csv")

def summary2():
    idata=pd.read_csv("analysis/label_win.csv")
    data=idata.values
    data=np.delete(data,-1,axis=1)
    # s=set()
    datasum=np.sum(data[:,1:],axis=0)
    columes=idata.columns[1:-1]
    print(datasum)
    index=np.argsort(datasum)
    # print(index)


    sortal=columes[index]

    s=[]
    n=[]
    for line in data:
        temp = []
        for i in range(len(line)-1):
            if line[i+1]==1:
                temp.append(columes[i])

        if len(temp)==0:
            continue
        no=[]
        for a in temp:
            for i in range(len(sortal)):
                if sortal[i]==a and datasum[i]>=30:
                    no.append(i)
                    break
        #print(temp,no)
        if len(no)<=0:
            continue
        no=np.array(no)
        maxindex=np.argmax(no)
        classal=temp[maxindex]
        #print(classal)
        s.append(classal)
        n.append(line[0])

    # print(s)
    finalal=[]
    for aln in set(s):
        finalal.append(aln)
    print(finalal)
    finalcsv=[]
    firstline=[]
    firstline.append('ImageName')
    for aln in finalal:
        firstline.append(aln)
    finalcsv.append(firstline)
    print(firstline)
    # print(data.shape[0],len(s))
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
    Writecsv(finalcsv,"analysis/one_hot_win.csv")
        # print(name,s[i])
        # f


if __name__=="__main__":
    onehot()
    # summary2()

    # summarycsv()
    # svmcsv()
