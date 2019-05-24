import cv2 as cv
from libtiff import TIFF
import numpy as np
import os

def classifyGround(groundapo,testapo):
    path=groundapo
    xyz=[]
    file = open(path)
    lines = file.readlines()
    for line in lines:
        if line.startswith("#") or line == "\n":
            continue
        line_ = [s.strip() for s in line.split(',')]
        z, x, y = line_[4:7]
        x = int(float(x))
        y = int(float(y))
        z = int(float(z))
        xyz.append([x,y,z])
    outfile = open(testapo, 'w');
    outfile.write("##n,orderinfo,name,comment,z,x,y, pixmax,intensity,sdev,volsize,mass,,,, color_r,color_g,color_b\n")
    for i in range(len(xyz)):
        outfile.write("{}, ,  {},, {},{},{}, 0,0,0,50,0,,,,0,0,255\n".format(i, i, xyz[i][2]/64, xyz[i][0]/64, xyz[i][1]/64))
    outfile.close()
    print("generate apo ok")
def crop3Dtif(teraflyFolder,apoPath,savaFolder,dimX,dimY,dimZ):
    commandStr = "D:/v3d_external/bin/vaa3d_msvc.exe /x D:\\vaa3d_tools\\bin\plugins\\image_geometry\\crop3dTif\\cropped3DTif.dll /f cropTerafly " \
                 "/i  {} {}  {} /p {} {} {}".format(teraflyFolder, apoPath, savaFolder,int(dimX),int(dimY),int(dimZ))
    os.system(commandStr)
    print("crop3Dtif ok")

if __name__ == "__main__":
    groundapo="I:\\mouseID_321237-17302\\17302.apo"
    testapo="F:\\tif.apo"
    classifyGround(groundapo, testapo)
    savaFolder="F:\\tiflow"
    teraflyFolder="I:\\mouseID_321237-17302\\RES(853x537x153)"
    crop3Dtif(teraflyFolder, testapo, savaFolder, 32, 32, 32)