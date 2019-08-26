import os
import sys
import shutil
import numpy as np
import math
import cv2 as cv
from libtiff import TIFF

def createApo(wholebraintxt,saveapo,multiple):
    path = wholebraintxt
    file = open(path)
    lines = file.readlines();
    for i in range(len(lines)):
        lines[i] = lines[i].strip('\n');
        lines[i] = lines[i].split(',');
        if lines[i][0] == '#':
            continue
        # print((int(lines[i][3])),(int(lines[i][4])),(int(lines[i][5])))
    outfile = open(path + "\\..\\"+saveapo, 'w')
    outfile.write("##n,orderinfo,name,comment,z,x,y, pixmax,intensity,sdev,volsize,mass,,,, color_r,color_g,color_b\n")
    for i in range(len(lines)):
        outfile.write("%d, ,  %d,, %d,%d,%d, 0,0,0,50,0,,,,0,0,255\n" % (
        i + 1, i + 1, (int(lines[i][5]))*multiple, (int(lines[i][3]))*multiple, (int(lines[i][4]))*multiple))
    print("create apo ok")




def crop3Ddraw(teraflyFolder,apoPath,savaFolder,dimX,dimY,dimZ):
    commandStr = "D:/v3d_external/bin/vaa3d_msvc.exe /x D:\\vaa3d_tools\\bin\plugins\\image_geometry\\crop3d_image_series\\cropped3DImageSeries.dll /f cropTerafly " \
                 "/i  {} {}  {} /p {} {} {}".format(teraflyFolder, apoPath, savaFolder,int(dimX),int(dimY),int(dimZ))
    os.system(commandStr)
    print("crop3Ddraw ok")

def classifyGround(groundapo,testapo):
    path=groundapo
    file = open(path)
    lines = file.readlines()
    count = 0
    xyz_like = []
    xyz=[]
    xyz_unique=[]
    for line in lines:
        if line.startswith("#") or line == "\n":
            continue
        line_ = [s.strip() for s in line.split(',')]
        z, x, y = line_[4:7]
        x = int(float(x))
        y = int(float(y))
        z = int(float(z))
        outfile = open(testapo, "r")
        outlines = outfile.readlines()
        for t in outlines:
            if t.startswith("#"):
                continue
            outline_ = [m.strip() for m in t.split(',')]
            zs, xs, ys = outline_[4:7]
            xs = int(float(xs))
            ys = int(float(ys))
            zs = int(float(zs))
            # print(xs, ys, zs)
            if math.sqrt((xs - x) ** 2 + (ys - y) ** 2 + (zs - z) ** 2) <= 20:
                xyz_like.append([xs / 2, ys / 2, zs / 2])
        outfile.close()
    for i in  xyz_like:
        if not i in xyz_unique:
            xyz_unique.append(i)
    outfile = open(testapo, "r")
    outlines = outfile.readlines()
    for t in outlines:
        if t.startswith("#"):
            continue
        outline_ = [m.strip() for m in t.split(',')]
        zs, xs, ys = outline_[4:7]
        xs = int(float(xs))
        ys = int(float(ys))
        zs = int(float(zs))
        # print(xs, ys, zs)
        xyz.append([xs/2, ys/2, zs/2])
    outfile.close()
    for i in xyz_unique :
        if i in xyz :
             xyz.remove(i)
    outfile = open(testapo + "\\..\\true.apo", 'w');
    outfile.write("##n,orderinfo,name,comment,z,x,y, pixmax,intensity,sdev,volsize,mass,,,, color_r,color_g,color_b\n")
    for i in range(len(xyz_unique)):
        outfile.write("{}, ,  {},, {},{},{}, 0,0,0,50,0,,,,0,0,255\n".format(i, i, xyz_unique[i][2],xyz_unique[i][0],xyz_unique[i][1]))
    outfile.close()
    outfile = open(testapo + "\\..\\false.apo", 'w');
    outfile.write("##n,orderinfo,name,comment,z,x,y, pixmax,intensity,sdev,volsize,mass,,,, color_r,color_g,color_b\n")
    for i in range(len(xyz)):
        outfile.write("{}, ,  {},, {},{},{}, 0,0,0,50,0,,,,0,0,255\n".format(i, i, xyz[i][2], xyz[i][0], xyz[i][1]))
    outfile.close()
    print("classify ground ok")

def  crop2DTif(datapath,convertTifPath):
    path = datapath
    filename = os.listdir(path)
    for file in filename:
        commandStr = "D:/v3d_external/bin/vaa3d_msvc.exe " \
                     "/x D:\\v3d_external\\bin\\plugins\\image_projection\\maximum_intensity_projection_Z_Slices\\mipZSlices.dll " \
                     "/f mip_zslices " \
                     "/i {}\\{} " \
                     "/p 1:1:e " \
                     "/o {}\\{}.tif".format(path,file,convertTifPath,os.path.splitext(file)[0])

        os.system(commandStr)
    print("crop2Dtif ok")


def classify2dtif(tifpath,apopath,savaturepath,savefalsepath):
    filename = os.listdir(tifpath)
    xyz=[]
    pic_xyz=[]
    outfile = open(apopath, "r")
    outlines = outfile.readlines()
    for t in outlines:
        if t.startswith("#"):
            continue
        outline_ = [m.strip() for m in t.split(',')]
        zs, xs, ys = outline_[4:7]
        xs = int(float(xs))
        ys = int(float(ys))
        zs = int(float(zs))
        xyz.append([xs,ys,zs])
    for file in filename:
        print(os.path.splitext(file)[1])
        if os.path.splitext(file)[1]=='.tif':
            name0=os.path.splitext(file)[0]
            print(name0)
            name1=os.path.splitext(file)[1]
            print(name1)
            i=[int(name0.split('_')[0]),int(name0.split('_')[1]),int(name0.split('_')[2])]
            if  i in xyz:
                fullpath=os.path.join(tifpath,file)
                destpath=savaturepath +"\\"+file
                shutil.move(fullpath,destpath)
            else :
                fullpath = os.path.join(tifpath, file)
                destpath = savefalsepath + "\\"+file
                shutil.move(fullpath, destpath)
    print(" classify2dtif ok ")


def readOneTif(path):
    tif = TIFF.open(path, mode='r')
    img = tif.read_image()
    return img

def tif_to_png(tiffolder,pngfolder):
    folder = tiffolder
    outfolder = pngfolder
    for file in os.listdir(folder):
        if os.path.splitext(file)[1]=='.tif':
            img = readOneTif(folder + "\\" + file)
            cv.imwrite(outfolder + "\\" + file.split('.')[0] + ".png", img)



def  apoToInt(apotruepath,apotruepathint):
    path = apotruepath
    file = open(path)
    xyz=[]
    lines = file.readlines();
    for t in lines:
        if t.startswith("#"):
            continue
        outline_ = [m.strip() for m in t.split(',')]
        zs, xs, ys = outline_[4:7]
        xs = int(float(xs))
        ys = int(float(ys))
        zs = int(float(zs))
        xyz.append([xs, ys, zs])
        # print((int(lines[i][3])),(int(lines[i][4])),(int(lines[i][5])))
    outfile = open(apotruepathint, 'w')
    outfile.write("##n,orderinfo,name,comment,z,x,y, pixmax,intensity,sdev,volsize,mass,,,, color_r,color_g,color_b\n")
    for i in range(len(xyz)):
        outfile.write("{}, ,  {},, {},{},{}, 0,0,0,50,0,,,,0,0,255\n".format(i, i, xyz[i][2],xyz[i][0],xyz[i][1]))
    print("apoToInt ok")





if __name__ == "__main__":
    #step1 生成apo
    path="E:\\mypersonalgit\\somadata\\17545_50_200"#需要设置
    if not os.path.exists(path):
        os.mkdir(path)
    wholebrainntxt="F:\\seuWholebrain\\17545\\wholebrain.txt"#需要设置
    saveapo="wholebrain1.apo"
    multiple=1
    #createApo(wholebrainntxt, saveapo, multiple)
    saveapo2="wholebrain2.apo"
    multiple2=2
    #createApo(wholebrainntxt, saveapo2, multiple2)
    #step2  生成两个文件目录
    path128 = "128_128_64"
    v3draw = "v3draw"
    twotif = "2DTif"
    twopng = "png"
    if os.path.isdir(path) and not os.path.exists(path + "\\" + path128):
        os.mkdir(os.path.join(path, path128))
    if os.path.isdir(path) and not os.path.exists(path + "\\" + path128 + "\\" + v3draw):
        os.mkdir(os.path.join(path + "\\" + path128 + "\\", v3draw))
    if os.path.isdir(path) and not os.path.exists(path + "\\" + path128 + "\\" + twotif):
        os.mkdir(os.path.join(path + "\\" + path128 + "\\", twotif))
    if os.path.isdir(path) and not os.path.exists(path + "\\" + path128 + "\\" + twopng):
        os.mkdir(os.path.join(path + "\\" + path128 + "\\", twopng))
    #step 3 生成真假apo
    groundapo = "F:\\SEUAllen_SomaList\\traced\\17545_somalist.ano.apo"  # 需要设置
    testapo = wholebrainntxt + "\\..\\" + saveapo2
    #classifyGround(groundapo, testapo)
    trueName="true"
    falseName="false"
    trueapo=wholebrainntxt + "\\..\\" + trueName+".apo"
    falseapo=wholebrainntxt + "\\..\\" +falseName+".apo"
    trueapoInt=wholebrainntxt + "\\..\\" +"trueint.apo"
    falseapoInt = wholebrainntxt + "\\..\\" + "falseint.apo"
    #apoToInt(trueapo, trueapoInt)
    #apoToInt(falseapo,falseapoInt)
    #step 4 生成真假v3draw
    if os.path.isdir(path) and not os.path.exists(path + "\\" + path128 + "\\" + v3draw+"\\"+trueName):
        os.mkdir(os.path.join(path + "\\" + path128 + "\\" + v3draw+"\\", trueName))
    if os.path.isdir(path) and not os.path.exists(path + "\\" + path128 + "\\" + v3draw+"\\"+falseName):
        os.mkdir(os.path.join(path + "\\" + path128 + "\\" + v3draw+"\\", falseName))
    trueVdraw=path + "\\" + path128 + "\\" + v3draw+"\\"+trueName
    falseVdraw=path + "\\" + path128 + "\\" + v3draw+"\\"+falseName
    teraflyFolder = "J:\\mouse17545_teraconvert\\RES(27300x17994x5375)"  # 需要设置
    #crop3Ddraw(teraflyFolder, trueapoInt, trueVdraw, 128, 128, 64)#可能改参数
    #crop3Ddraw(teraflyFolder, falseapoInt, falseVdraw, 128, 128, 64)  # 可能改参数
    #setp 5 生成真假tif
    if os.path.isdir(path)and not os.path.exists(path+"\\"+path128+"\\"+twotif+"\\"+trueName):
        os.mkdir(os.path.join(path+"\\"+path128+"\\"+twotif+"\\",trueName))
    if os.path.isdir(path)and not os.path.exists(path+"\\"+path128+"\\"+twotif+"\\"+falseName):
        os.mkdir(os.path.join(path+"\\"+path128+"\\"+twotif+"\\",falseName))
    trueTif=path+"\\"+path128+"\\"+twotif+"\\"+trueName
    falseTif=path+"\\"+path128+"\\"+twotif+"\\"+falseName
    #crop2DTif(trueVdraw, trueTif)
    crop2DTif(falseVdraw, falseTif)
    #step 6 生成真假png
    if os.path.isdir(path)and not os.path.exists(path+"\\"+path128+"\\"+twopng+"\\"+trueName):
        os.mkdir(os.path.join(path+"\\"+path128+"\\"+twopng+"\\",trueName))
    if os.path.isdir(path)and not os.path.exists(path+"\\"+path128+"\\"+twopng+"\\"+falseName):
        os.mkdir(os.path.join(path+"\\"+path128+"\\"+twopng+"\\",falseName))
    truePng=path+"\\"+path128+"\\"+twopng+"\\"+trueName
    falsePng=path+"\\"+path128+"\\"+twopng+"\\"+falseName
    #tif_to_png(trueTif, truePng)
    tif_to_png(falseTif, falsePng)












