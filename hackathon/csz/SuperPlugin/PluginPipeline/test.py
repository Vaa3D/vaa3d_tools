import glob as glob
import os
import numpy as np
import SimpleITK as sitk
from img_io import *
import pandas as pd
from swc_processing import *
import pandas as pd

AList=["Advantra","app2new3","EnsembleNeuronTracerBasic","EnsembleNeuronTracerV2s","EnsembleNeuronTracerV2n","ENT","MOST"]

# files=glob.glob(r"E:\result1\140921c16.tif.v3dpbd"+"\*")

def similarity(img,label):
    assert img.shape==label.shape
    img[img!=0]=1
    label[label!=0]=1
    consensus=np.multiply(img,label)
    inter=np.sum(consensus)
    union=np.sum(img)+np.sum(label)
    dice = (2. * inter + 0.0000001) / (union + 0.0000001)
    recall=inter/(np.sum(img)+ 0.0000001)
    precision = inter / (np.sum(label)+ 0.0000001)
    f1=(2*recall*precision)/(recall+precision+ 0.0000001)
    return recall,precision,f1,dice


def getBestResult(name):
    print(1)
    # goldfiles=glob.glob(r"E:\result1\140921c6.tif.v3dpbd\gold\*.tiff")
    #
    # flag=False
    # firstline="img,"
    # for al in AList:
    #     path=os.path.join(r"E:\result1\140921c6.tif.v3dpbd",al)
    #     csv=[]
    #     for goldfile in goldfiles:
    #         name=goldfile.split('_gold')[0].split('\\')[-1]
    #         file=os.path.join(path,name+"_"+al+".tiff")
    #         gold=sitk.ReadImage(goldfile,sitk.sitkInt8)
    #         golddata=sitk.GetArrayFromImage(gold)
    #         img = sitk.ReadImage(file, sitk.sitkInt8)
    #         imgdata = sitk.GetArrayFromImage(img)
    #         s=similarity(imgdata,golddata)
    #         csv.append([name,s[0],s[1],s[2],s[3]])
    #     csv=np.array(csv)
    #     firstline+=al+","
    #     if flag==False:
    #         total=csv[:,(0,3)]
    #         flag=True
    #     #print(total.shape,csv[:,3].shape)
    #     else:
    #         total=np.append(total,np.transpose([csv[:,3]]),axis=1)
    # print(total)
    #
    #
    # Writecsv(total,r"E:/result1/total.csv",firstline)
    # maxarr=total[:,1:]
    # index=np.argmax(maxarr,axis=1)
    # print(index)



    # flag=np.zeros(maxarr.shape)
    # for i in range(len(index)):
    #     print(i,index[i])
    #     flag[i][index[i]]=1
    # Writecsv(flag,r"E:/result1/flag.csv")



    # path=r"E:\result1\res"
    #
    # csvs=glob.glob(os.path.join(path,'*'))
    #
    # newdata=np.array(pd.read_csv(""))
    # for csv in csvs:
    #     name=csv.split('\\')[-1].split('.')[0]
    #     data=np.array(pd.read_csv(csv))[:,2]
    #
    # csvpath=r"E:\contrastres\140921c16.tif.v3dpbd.csv"

def cropswc():
    croppath=r"E:\Cropswc"
    # bboxes=np.array(pd.read_csv(csvpath,header=None))[:,:-1]

    dirfiles=glob.glob(r"E:\Sortedswc\*")

    # sortpath=r"E:\contrastres\sorted"

    for dirf in dirfiles:
        dirname=dirf.split('\\')[-1]

        if dirname.startswith("14") and os.path.isdir(dirf):
            # print(dirname)
            files=glob.glob(os.path.join(dirf,"*.swc"))
            # print(files)
            for file in files:
                print(file)
                name=file.split('\\')[-2]
                # print(name)
                if name.startswith("14") is False:
                    continue
                method=file.split("\\")[-1].split(".swc")[0].split(".v3dpbd_")[-1].split("_")[1]
                print(method)
                if method in AList:
                    temppath=os.path.join(croppath,name)
                    if os.path.exists(temppath) is False:
                        os.mkdir(temppath)
                    path=os.path.join(temppath,method)
                    if os.path.exists(path) is False:
                        os.mkdir(path)
                    print(path)

                    cmd="E:\Downloads\Vaa3D_v6.007_Windows_64bit\Vaa3D_v6.007_Windows_64bit\Vaa3D-x.exe /x CropSwc /f cropswc /i "+file+r" E:\apopath/"+name+".apo"+" "+path+" /p 128 128 128"
                    print(cmd)
                    os.system(cmd)


    # swc=Readswc(r"E:\contrastres\sorted\final.swc")
    # swc[:,5]=5
    # Writeswc(swc,r"E:\contrastres\sorted\final1.swc")
    #
    # files=glob.glob("E:\contrastres\swcmask\*.tiff")
    #
    # for file in files:
    #     name=file.split('\\')[-1]
    #     print(name)
    #     if name=="140921c16.tif.v3dpbd.swc.tiff":
    #         continue
    #     goldmask=Readimg(r"E:\contrastres\swcmask\140921c16.tif.v3dpbd.swc.tiff")
    #     tracemask=Readimg(file)
    #     print(name,similarity(tracemask,goldmask))


        # outpath="E:\contrastres\swcmask"
        # cmd="E:\Downloads\Vaa3D_v6.007_Windows_64bit\Vaa3D_v6.007_Windows_64bit\Vaa3D-x.exe /x swc2mask /f swc2mask /i "+file+" /o "+outpath+"\\"+name+".tiff"+" /p 1184 1300 30"
        # os.system(cmd)
def getAPO():
    files=glob.glob(r"E:\testdata\CropCenter\*.csv")
    for file in files:
        name=file.split('\\')[-1].split('.')[0]
        if name.startswith("14"):
            data=np.array(pd.read_csv(file))
            apo=[]
            for line in data:
                apo.append([(line[0]+line[3])/2,(line[1]+line[4])/2,(line[2]+line[5])/2])
            apo=np.array(apo)
            Writeapo(apo,r"E:\apopath"+"\\"+name+'.apo')

def sortedswc():
    dirfiles = glob.glob(r"E:\csz\gold166\20161101_reconstruction_for_gold166_rhea_5571\*\*")

    sortpath=r"E:\Sortedswc"

    for dirf in dirfiles:
        dirname = dirf.split('\\')[-1]

        if dirname.startswith("14") and os.path.isdir(dirf):
            # print(dirname)
            files = glob.glob(os.path.join(dirf, "*.swc"))
            # print(files)
            for file in files:
                # print(file)
                name = file.split('\\')[-2]
                print(name)
                if name.startswith("14") is False:
                    continue
                method = file.split("\\")[-1].split(".swc")[0].split(".v3dpbd_")[-1]
                # print(method)
                if method in AList:
                    temppath = os.path.join(sortpath, name)
                    if os.path.exists(temppath) is False:
                        os.mkdir(temppath)
                    path = os.path.join(temppath, name+"_"+method+".swc")
                    # if os.path.exists(path) is False:
                    #     os.mkdir(path)
                    if os.path.exists(path):
                        continue
                    if os.stat(file).st_size>1024*1024:
                        continue
                    cmd = "E:\Downloads\Vaa3D_v6.007_Windows_64bit\Vaa3D_v6.007_Windows_64bit\Vaa3D-x.exe /x sort_neuron_swc /f sort_swc /i " + file + " /o " + path
                    os.system(cmd)
if __name__ == '__main__':
    # getAPO()
    cropswc()
    # sortedswc()