import numpy as np
import shutil
import os
def copyImgAndRm(srcFolder,comparFolder,desFoler):
    xyzcompar = []
    for file in os.listdir(comparFolder):
        if os.path.splitext(file)[1] == '.png':
            name = os.path.splitext(file)[0]
            x = name.split("_")[0]
            y = name.split("_")[1]
            z = name.split("_")[2]
            # print(x,y,z)
            xyzcompar.append([x, y, z])
    xyzsrc=[]
    for file1 in os.listdir(srcFolder):
        if os.path.splitext(file1)[1] == '.png':
            name1 = os.path.splitext(file1)[0]
            x1 = name1.split("_")[0]
            y1 = name1.split("_")[1]
            z1 = name1.split("_")[2]
            # print(x,y,z)
            xyzsrc.append([x1, y1, z1])
    xyz_unique=[]
    for i in  xyzsrc:
        if not i in xyzcompar:
            xyz_unique.append(i)
    #print(xyz_unique)
    for filet in os.listdir(srcFolder):
        if os.path.splitext(filet)[1] == '.png':
            namet = os.path.splitext(filet)[0]
            xt= namet.split("_")[0]
            yt = namet.split("_")[1]
            zt = namet.split("_")[2]
            #print(xt,yt,zt)
            if [xt,yt,zt] in xyz_unique:
                #print(xt,yt,zt)
                fullpath = os.path.join(srcFolder, filet)
                destepath = desFoler + "\\" + filet
                shutil.copy(fullpath, destepath)
if __name__ == "__main__":
    srcFolder="F:\\17545soma"
    comparFolder="F:\\reTrain\\17545\\true"
    desFoler="F:\\demopredict\\17545\\can"
    copyImgAndRm(srcFolder, comparFolder, desFoler)