import pandas as pd
import numpy as np
from img_io import *
import os
import copy
import glob as glob
import math

# files=glob.glob(r"E:\contrastres\cropswc\*")
# sortpath=r"E:\contrastres\cropswcsorted"
# for file in files:
#     name=file.split('\\')[-1]
#     print(name)
#     if os.path.exists(os.path.join(sortpath,name)) is False:
#         os.mkdir(os.path.join(sortpath,name))
#     for swcf in glob.glob(os.path.join(file,"*.swc")):
#         swc=Readswc(swcf)
#         if swc.shape[0]>0:
#             swcname=swcf.split('\\')[-1]
#
#             cmd = "E:\Downloads\Vaa3D_v6.007_Windows_64bit\Vaa3D_v6.007_Windows_64bit\Vaa3D-x.exe /x sort_neuron_swc /f sort_swc /i "+swcf+" /o "+os.path.join(sortpath,name)+"/"+swcname
#             print(cmd)
#             os.system(cmd)

for csv in glob.glob(r"E:\testdice\*.csv"):
    imgnamecrop=csv.split('\\')[-1].split('.')[0]
    csv=pd.read_csv(csv)
    data=np.array(csv)
    # print(data)
    croppath=os.path.join("E:\Cropswc",imgnamecrop)
    swcid=0
    blocklist=data[:,0]
    swc=[]

    for i in range(data.shape[0]):
        block=data[i][1]
        block=block.split("\\")[-1].split("_gold")[0]
        line=data[i][2:]
        # index=np.argmax(line)
        indexlist=np.argsort(line)
        index=indexlist[len(indexlist)-3]
        print(index,indexlist)
        filedir=croppath+'\\'+csv.columns[index+2]
        print(filedir)
        if os.path.exists(filedir) is False:
            continue
        files=glob.glob(os.path.join(filedir,"*.swc"))
        mind=1e6
        # print(files)
        if len(files)>0:
            for file in files:
                bx,by,bz=block.split('_')
                x,y,z=file.split('\\')[-1].split('.swc')[0].split('_')
                dis=math.sqrt((int(bx)-int(x))**2+(int(by)-int(y))**2+(int(bz)-int(z))**2)
                print(x,y,z,bx,by,bz)
                if dis<mind:
                    mind=dis
                    blockx,blocky,blockz=x,y,z
            initswc=os.path.join(filedir,blockx+"_"+blocky+"_"+blockz+".swc")
            if os.path.exists(initswc) is True:
                # if count==2:
                #     break
                initswc=Readswc(initswc)
                print(os.path.join(filedir,blockx+"_"+blocky+"_"+blockz+".swc"))
                maxid=0
                for swcline in initswc:
                    if swcline[0]>maxid:
                        maxid=swcline[0]
                    if swcline[6]>maxid:
                        maxid=swcline[6]

                for swcline in initswc:
                    templine=copy.deepcopy(swcline)
                    templine[0]+=swcid
                    if templine[6]!=-1:
                        templine[6]+=swcid
                    swc.append(templine)
                # print(maxid)
                swcid+=maxid

    swc=np.array(swc)
    Writeswc(swc,r"E:\Cropswc\noalexnet"+"\\"+imgnamecrop+"final.swc")


