import glob as glob
import SimpleITK as sitk
import os
from img_io import *
import copy

files=glob.glob(r"E:\result1\140921c9.tif.v3dpbd\raw\*.tiff")

finalfile=r"E:\Cropswc\140921c9\1.swc"


imgdict=dict()
for file in files:
    img=sitk.ReadImage(file)
    img=sitk.GetArrayFromImage(img)         #shape (z,y,x)
    print(file.split('\\')[-1].split("_raw")[0],img.shape)

    imgdict[file.split('\\')[-1].split("_raw")[0]]=[img.shape[2],img.shape[1],img.shape[0]]




swcfiles=glob.glob(r"E:\Cropswc\140921c9\app2new3\*.swc")
swcid=0
swc=[]
for swcfile in swcfiles:
    initswc = swcfile
    block=swcfile.split('\\')[-1].split(".swc")[0].split('_')[:3]
    key=swcfile.split('\\')[-1].split('_Advantra')[0].split('.swc')[0]

    keyx,keyy,keyz=key.split("_")

    for i in range(3):
        for j in range(3):
            for k in range(3):
                fkey=str(int(keyx)+i-1)+"_"+str(int(keyy)+j-1)+"_"+str(int(keyz)+k-1)
                if fkey in imgdict:
                    xbias=int(block[0])-imgdict[fkey][0] / 2
                    ybias = int(block[1]) - imgdict[fkey][1] / 2
                    zbias = int(block[2]) - imgdict[fkey][2] / 2
                    print(xbias,ybias,zbias)
                    break

    if os.path.exists(initswc) is True:
        initswc = Readswc(initswc)
        if len(initswc)==0:
            continue
        initswc[:,2]+=xbias
        initswc[:,3]+=ybias
        initswc[:, 4] += zbias
        maxid = 0
        for swcline in initswc:
            if swcline[0] > maxid:
                maxid = swcline[0]
            if swcline[6] > maxid:
                maxid = swcline[6]

        for swcline in initswc:
            templine = copy.deepcopy(swcline)

            templine[0] += swcid
            if templine[6] != -1:
                templine[6] += swcid
            swc.append(templine)
        print(maxid)
        swcid += maxid

swc=np.array(swc)
Writeswc(swc,finalfile)