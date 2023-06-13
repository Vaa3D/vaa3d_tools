import pandas as pd
import numpy as np
from img_io import *
import glob as glob
import os


def inBBox(BBox,p):
    if p[0]>=BBox[0] and p[0]<=BBox[3] and p[1]>=BBox[1] and p[1]<=BBox[4] and p[2]>=BBox[2] and p[2]<=BBox[5]:
        return True
    else:
        return False

def crosspoint(p1,p2,plane,axis):
    bias=[p2[0]-p1[0],p2[1]-p1[1],p2[2]-p1[2]]
    # print(p1,p2,plane,axis)
    # print(bias)
    x,y,z=-1,-1,-1
    if axis==0:
        if bias[0]!=0:
            x=plane
            t=(x-p1[0])/bias[0]
            y=p1[1]+t*bias[1]
            z=p1[2]+t*bias[2]
    elif axis==1:
        if bias[1] != 0:
            y=plane
            t=(y-p1[1])/bias[1]
            x=p1[0]+t*bias[0]
            z=p1[2]+t*bias[2]
    elif axis==2:
        if bias[2] != 0:
            z=plane
            t=(z-p1[2])/bias[2]
            x=p1[0]+t*bias[0]
            y=p1[1]+t*bias[1]
    return x,y,z

def CropSwc(swc,BBox):

    crop=[]
    maxi=-2
    for p in swc:
        if inBBox(BBox,p[2:5]) is True:
            crop.append(p)
            if p[0]>maxi:
                maxi=p[0]
    for p in crop:
        point = p[2:5]
        flag=False
        for parp in swc:
            if parp[0]==p[6]:
                parentpoint = parp[2:5]
                flag=True
                break
        if flag is True and (inBBox(BBox, point) is True and inBBox(BBox, parentpoint) is False) or (
                inBBox(BBox, point) is False and inBBox(BBox, parentpoint) is True):
            crossps = []
            for i in range(len(BBox)):
                if i % 3 == 0:
                    crossp = crosspoint(point, parentpoint, BBox[i], 0)
                elif i % 3 == 1:
                    crossp = crosspoint(point, parentpoint, BBox[i], 1)
                elif i % 3 == 2:
                    crossp = crosspoint(point, parentpoint, BBox[i], 2)
                crossps.append(crossp)
            for i in range(len(crossps)):
                if inBBox(BBox, crossps[i]) is True:
                    if inBBox(BBox, point) is True:
                        maxi += 1
                        crop.append([maxi, 3, crossps[i][0], crossps[i][1], crossps[i][2], 1, p[0]])
                    else:
                        maxi += 1
                        crop.append([maxi, 3, crossps[i][0], crossps[i][1], crossps[i][2], 1, swc[int(p[6]) - 1][0]])
                    # if i == 0:
                    #     if inBBox(BBox,point) is True:
                    #         maxi+=1
                    #         crop.append([maxi,3,crossps[i][0],crossps[i][1],crossps[i][2],1,p[0]])
                    #     else:
                    #         maxi+=1
                    #         crop.append([maxi,3,crossps[i][0],crossps[i][1],crossps[i][2],1,swc[p[6]-1][0]])
                    # elif i == 1:
                    #     if inBBox(BBox,point) is True:
                    #         maxi+=1
                    #         crop.append([maxi,3,crossps[i][0],crossps[i][1],crossps[i][2],1,p[0]])
                    #     else:
                    #         maxi+=1
                    #         crop.append([maxi,3,crossps[i][0],crossps[i][1],crossps[i][2],1,swc[p[6]-1][0]])
                    # elif i == 2:
                    #     if inBBox(BBox,point) is True:
                    #         maxi+=1
                    #         crop.append([maxi,3,crossps[i][0],crossps[i][1],crossps[i][2],1,p[0]])
                    #     else:
                    #         maxi+=1
                    #         crop.append([maxi,3,crossps[i][0],crossps[i][1],crossps[i][2],1,swc[p[6]-1][0]])
                    # elif i == 3:
                    #     if inBBox(BBox,point) is True:
                    #         maxi+=1
                    #         crop.append([maxi,3,crossps[i][0],crossps[i][1],crossps[i][2],1,p[0]])
                    #     else:
                    #         maxi+=1
                    #         crop.append([maxi,3,crossps[i][0],crossps[i][1],crossps[i][2],1,swc[p[6]-1][0]])
                    # elif i == 4:
                    #     if inBBox(BBox,point) is True:
                    #         maxi+=1
                    #         crop.append([maxi,3,crossps[i][0],crossps[i][1],crossps[i][2],1,p[0]])
                    #     else:
                    #         maxi+=1
                    #         crop.append([maxi,3,crossps[i][0],crossps[i][1],crossps[i][2],1,swc[p[6]-1][0]])
                    # elif i == 5:
                    #     if inBBox(BBox,point) is True:
                    #         maxi+=1
                    #         crop.append([maxi,3,crossps[i][0],crossps[i][1],crossps[i][2],1,p[0]])
                    #     else:
                    #         maxi+=1
                    #         crop.append([maxi,3,crossps[i][0],crossps[i][1],crossps[i][2],1,swc[p[6]-1][0]])
    crop=np.array(crop)
    return crop

csvpath=r"E:\contrastres\140921c16.tif.v3dpbd.csv"
croppath=r"E:\contrastres\cropswc"
bboxes=np.array(pd.read_csv(csvpath,header=None))[:,:-1]

files=glob.glob(r"E:\contrastres\swc\*")
sortpath=r"E:\contrastres\sorted"
for file in files:
    name=file.split('\\')[-1]
    if name=="140921c16.tif.v3dpbd.swc":
        method="gold"
    else:
        method=name.split(".swc")[0].split(".v3dpbd_")[-1]
    print(method)
    # path=os.path.join(croppath,method)
    # if os.path.exists(path) is False:
    #     os.mkdir(path)
    # cmd="E:\Downloads\Vaa3D_v6.007_Windows_64bit\Vaa3D_v6.007_Windows_64bit\Vaa3D-x.exe /x CropSwc /f cropswc /i "+file+r" E:\contrastres\test.apo "+path+" /p 128 128 128"
    # os.system(cmd)
    print(name)
    cmd = "E:\Downloads\Vaa3D_v6.007_Windows_64bit\Vaa3D_v6.007_Windows_64bit\Vaa3D-x.exe /x sort_neuron_swc /f sort_swc /i "+file+" /o "+sortpath+"/"+name
    os.system(cmd)

