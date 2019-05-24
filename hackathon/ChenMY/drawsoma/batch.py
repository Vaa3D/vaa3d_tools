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
    path128="128_128_64"
    path64="64_64_64"
    v3draw="v3draw"
    twotif="2DTif"
    twopng="png"
    if os.path.isdir(path)and not os.path.exists(path+"\\"+path128):
        os.mkdir(os.path.join(path,path128))
    if os.path.isdir(path) and not os.path.exists(path + "\\" + path64):
        os.mkdir(os.path.join(path, path64))
    if os.path.isdir(path)and not os.path.exists(path+"\\"+path128+"\\"+v3draw):
        os.mkdir(os.path.join(path+"\\"+path128+"\\",v3draw))
    if os.path.isdir(path)and not os.path.exists(path+"\\"+path128+"\\"+twotif):
        os.mkdir(os.path.join(path+"\\"+path128+"\\",twotif))
    if os.path.isdir(path)and not os.path.exists(path+"\\"+path64+"\\"+v3draw):
        os.mkdir(os.path.join(path+"\\"+path64+"\\",v3draw))
    if os.path.isdir(path)and not os.path.exists(path+"\\"+path64+"\\"+twotif):
        os.mkdir(os.path.join(path+"\\"+path64+"\\",twotif))
    if os.path.isdir(path)and not os.path.exists(path+"\\"+path64+"\\"+twopng):
        os.mkdir(os.path.join(path+"\\"+path64+"\\",twopng))
    if os.path.isdir(path)and not os.path.exists(path+"\\"+path128+"\\"+twopng):
        os.mkdir(os.path.join(path+"\\"+path128+"\\",twopng))
    # step3 割64和128的v3draw
    teraflyFolder = "J:\\mouse17545_teraconvert\\RES(27300x17994x5375)"#需要设置
    apoPath=wholebrainntxt+"\\..\\"+saveapo
    saveFolder=path+"\\"+path64+"\\"+v3draw
    crop3Ddraw(teraflyFolder, apoPath, saveFolder, 64, 64, 64)#可能改参数
    saveFolder = path + "\\" + path128 + "\\" + v3draw
    crop3Ddraw(teraflyFolder, apoPath, saveFolder, 128, 128, 64)
    # step 4 形成2Dtif文件
    datapath64 = path + "\\" + path64 + "\\" + v3draw
    convertTifPath64 = path + "\\" + path64 + "\\" + twotif
    crop2DTif(datapath64, convertTifPath64)
    datapath128=path+"\\"+path128+"\\"+v3draw
    convertTifPath128=path+"\\"+path128+"\\"+ twotif
    crop2DTif(datapath128, convertTifPath128)
    # step 5 形成真假apo
    groundapo="F:\\SEUAllen_SomaList\\traced\\17545_somalist.ano.apo"#需要设置
    testapo=wholebrainntxt+"\\..\\"+saveapo2
    classifyGround(groundapo, testapo)
    turetif="ture"
    falsetif="flase"
    if os.path.isdir(path)and not os.path.exists(path+"\\"+path64+"\\"+twotif+"\\"+turetif):
        os.mkdir(os.path.join(path+"\\"+path64+"\\"+twotif+"\\",turetif))
    if os.path.isdir(path)and not os.path.exists(path+"\\"+path64+"\\"+twotif+"\\"+falsetif):
        os.mkdir(os.path.join(path+"\\"+path64+"\\"+twotif+"\\",falsetif))
    if os.path.isdir(path)and not os.path.exists(path+"\\"+path128+"\\"+twotif+"\\"+turetif):
        os.mkdir(os.path.join(path+"\\"+path128+"\\"+twotif+"\\",turetif))
    if os.path.isdir(path)and not os.path.exists(path+"\\"+path128+"\\"+twotif+"\\"+falsetif):
        os.mkdir(os.path.join(path+"\\"+path128+"\\"+twotif+"\\",falsetif))
    #step6 将真假tif分开
    tifpath128=path+"\\"+path128+"\\"+twotif
    apotruepath=wholebrainntxt+"\\..\\"+"true.apo"
    savaturepath=tifpath128+"\\"+turetif
    savefalsepath=tifpath128+"\\"+falsetif
    classify2dtif(tifpath128, apotruepath, savaturepath,savefalsepath)
    tifpath64 = path + "\\" + path64 + "\\" + twotif
    savaturepath64= tifpath64 + "\\" + turetif
    savefalsepath64 = tifpath64 + "\\" + falsetif
    classify2dtif(tifpath64, apotruepath, savaturepath64, savefalsepath64)
    pngtrue="pngtrue"
    pngfalse="pngfalse"
    if os.path.isdir(path)and not os.path.exists(path+"\\"+path64+"\\"+twopng+"\\"+pngtrue):
        os.mkdir(os.path.join(path+"\\"+path64+"\\"+twopng+"\\",pngtrue))
    if os.path.isdir(path)and not os.path.exists(path+"\\"+path64+"\\"+twopng+"\\"+pngfalse):
        os.mkdir(os.path.join(path+"\\"+path64+"\\"+twopng+"\\",pngfalse))
    if os.path.isdir(path)and not os.path.exists(path+"\\"+path128+"\\"+twopng+"\\"+pngtrue):
        os.mkdir(os.path.join(path+"\\"+path128+"\\"+twopng+"\\",pngtrue))
    if os.path.isdir(path)and not os.path.exists(path+"\\"+path128+"\\"+twopng+"\\"+pngfalse):
        os.mkdir(os.path.join(path+"\\"+path128+"\\"+twopng+"\\",pngfalse))
    #step7 生成png
    tiffolderture64=path+"\\"+path64+"\\"+twotif+"\\"+turetif
    tiffolderfalse64=path+"\\"+path64+"\\"+twotif+"\\"+falsetif
    pngfolderture64=path+"\\"+path64+"\\"+twopng+"\\"+pngtrue
    pngfolderfalse64= path + "\\" + path64+ "\\" +twopng+"\\"+ pngfalse
    tiffolderture128 = path + "\\" + path128 + "\\" + twotif + "\\" + turetif
    tiffolderfalse128= path + "\\" + path128 + "\\" + twotif + "\\" + falsetif
    pngfolderture128= path + "\\" + path128+ "\\" +twopng+"\\"+ pngtrue
    pngfolderfalse128 = path + "\\" + path128 + "\\"+twopng+"\\" + pngfalse
    tif_to_png(tiffolderture64, pngfolderture64)
    tif_to_png(tiffolderfalse64, pngfolderfalse64)
    tif_to_png(tiffolderture128,pngfolderture128)
    tif_to_png(tiffolderfalse128, pngfolderfalse128)










