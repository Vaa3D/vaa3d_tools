import os
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
    outfile = open(path + "\\..\\"+saveapo, 'w')
    outfile.write("##n,orderinfo,name,comment,z,x,y, pixmax,intensity,sdev,volsize,mass,,,, color_r,color_g,color_b\n")
    for i in range(len(lines)):
        outfile.write("%d, ,  %d,, %d,%d,%d, 0,0,0,50,0,,,,0,0,255\n" % (
        i + 1, i + 1, (int(lines[i][5]))*multiple, (int(lines[i][3]))*multiple, (int(lines[i][4]))*multiple))
    print("create apo ok")
def crop3Ddraw(v3dexe,cropdll,teraflyFolder,apoPath,savaFolder,dimX,dimY,dimZ):
    commandStr = "{} " \
                 "/x {} " \
                 "/f cropTerafly " \
                 "/i  {} {}  {} /p {} {} {}".format(v3dexe,cropdll,teraflyFolder, apoPath, savaFolder,int(dimX),int(dimY),int(dimZ))
    os.system(commandStr)
    print("crop3Ddraw ok")

def  crop2DTif(v3dexe,mippath,datapath,convertTifPath):
    path = datapath
    filename = os.listdir(path)
    for file in filename:
        commandStr = "{} " \
                     "/x {} " \
                     "/f mip_zslices " \
                     "/i {}\\{} " \
                     "/p 1:1:e " \
                     "/o {}\\{}.tif".format(v3dexe,mippath,path,file,convertTifPath,os.path.splitext(file)[0])

        os.system(commandStr)
    print("crop2Dtif ok")

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
    v3dExePath="D:/v3d_external/bin/vaa3d_msvc.exe"# set v3dExePath
    cropped3DImageSeriesPath="D:\\vaa3d_tools\\bin\\plugins\\image_geometry\\crop3d_image_series\\cropped3DImageSeries.dll"#set cropped3DImageSeriesPath
    mipZSlicesPath="D:\\v3d_external\\bin\\plugins\\image_projection\\maximum_intensity_projection_Z_Slices\\mipZSlices.dll"#set mipZSlicesPath
    # step1 generate apo file
    path = "E:\\test"  # set pics saved folder
    if not os.path.exists(path):
        os.mkdir(path)
    wholebrainntxt = "E:\\test\\wholebrain.txt"  # set soma candidates ".txt"path
    saveapo = "wholebrain1.apo"
    multiple = 1
    createApo(wholebrainntxt, saveapo, multiple)
    saveapo2 = "wholebrain2.apo"
    multiple2 = 2
    createApo(wholebrainntxt, saveapo2, multiple2)
    # step2  generate two folders
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
    # step3 genenate apo
    somapath=os.path.dirname(wholebrainntxt) + "\\" + saveapo2
    print(somapath)
    # step 4 genenate v3draw
    teraflyFolder = "H:\\mouse18454_teraconvert\\RES(13149x17500x5520)"  # set teraflyFolder path
    v3drawpath=path + "\\" + path128 + "\\"+ v3draw
    crop3Ddraw( v3dExePath,cropped3DImageSeriesPath,teraflyFolder, somapath, v3drawpath, 128, 128, 64)
    # setp 5 generate tif
    tifpath = path + "\\" + path128 + "\\" + twotif
    crop2DTif(v3dExePath,mipZSlicesPath,v3drawpath,tifpath)
    # step 6 generate png
    pngpath= path + "\\" + path128 + "\\" + twopng
    tif_to_png(tifpath, pngpath)
