import os
import sys
import shutil
import numpy as np
import math
import time
#
# def mkdir(folder):
#     commandStr="mkdir {}".format(folder);
#     os.system(commandStr);

def _20181223_():
    path="D:\soamdata\\17302\\v3draw";
    for root,dirs,files in os.walk(path):
        # print(files)
        for file in files:
            pass
            subfile=root+"\\..\\"+file.strip('.v3draw');
            # print(subfile)
            if not os.path.exists(subfile):
                print("mkdir "+subfile)
                os.mkdir(subfile)
                print("copy {} to {}".format(root+"\\"+file,subfile+"\\"+file))
                shutil.copy(root+"\\"+file,subfile+"\\"+file)


def _20181224_():
    commandStr="D:/v3d_external/bin/vaa3d_msvc.exe /x D:\\vaa3d_tools\\bin\plugins\wpkenanPlugin\somaDetection\somaDetection.dll " \
               "/f somadetect /i D:\soamdata\somaDetection\\16232.000_15508.000_3058.000.v3draw /p -1 /o D:\soamdata\somaDetection\\output.v3draw "

    os.system(commandStr)

def _20181228_():
    file=open("D:\soamdata\somaDetection\\raw.txt");
    contents=file.read()
    contents=contents.strip(' \n');
    contents=contents.split(' ')
    # print(contents[134217728])
    print(len(contents))
    # print(contents[:10])
    for i in range(len(contents)):
        contents[i]=int(contents[i]);
    # contents.append(0);
    contents=np.array(contents);
    matrix=np.zeros((512,512,512));
    print("Line: ",sys._getframe().f_lineno)
    for i in range(512):
        for j in range(512):
            for k in range(512):
                matrix[i][j][k]=contents[(i*512*512+j*512+k)]

    print("over")

def _20190102_():
    commandStr = "D:/v3d_external/bin/vaa3d_msvc.exe /x D:\\vaa3d_tools\\bin\plugins\wpkenanPlugin\swcDistance\swcDistance.dll " \
                 "/f function " \
                 "/i " \
                 "D:\soamdata\\17302\\test\ID(1)_16232.000_15508.000_3058.000\\16232.000_15508.000_3058.000.v3draw_x8_y448_z320_app2.swc " \
                 "D:\soamdata\\17302\\test\ID(1)_16232.000_15508.000_3058.000\\16232.000_15508.000_3058.000.v3draw_x222_y354_z144_app2.swc " \
                 "/p -1 " \
                 "/o D:\soamdata\somaDetection\\output.v3draw "
    os.system(commandStr)


def _20190103_():
    commandStr = "D:/v3d_external/bin/vaa3d_msvc.exe " \
                 "/x D:\\vaa3d_tools\\bin\\plugins\\wpkenanPlugin\\Vaa3D_Neuron3\\vn3.dll " \
                 "/f app3 " \
                 "/i D:\\soamdata\\zhouzhi\\x_10580_y_19004_z_1769.v3draw " \
                 "/p D:\\soamdata\\zhouzhi\\x_10580_y_19004_z_1769.v3draw_vn3.marker 0 -1"

    os.system(commandStr)

def _20190107_():
    savedStdout = sys.stdout  # 保存标准输出流
    file=open('D:\soamdata\\18454\\out.txt', 'w+')
    sys.stdout = file  # 标准输出重定向至文件
    print('This message is for file!')
    # sys.stdout = savedStdout

def _20190107_2(apoPath,splitFolder,isMul=1):
    file = open(apoPath);
    lines = file.readlines();
    markers = [];
    mulmarker = [];
    for line in lines:
        z, x, y = line.split(',')[4:7]
        # print(x,y,z)
        if line[0] == "#":
            continue;
        id = int(float(line.split(',')[2]))
        z = float(z);
        y = float(y);
        x = float(x);
        markers.append([x, y, z, id])
    for marker in markers:
        count = 0;
        nearIds = [];
        for tmpMarker in markers:
            if max(abs(tmpMarker[0] - marker[0]), abs(tmpMarker[1] - marker[1]), abs(tmpMarker[2] - marker[2])) == 0:
                count = count + 1;
                nearIds.append(tmpMarker[3])
                break;
        for tmpMarker in markers:
            # print(max(abs(tmpMarker[0]-256),abs(tmpMarker[1]-256),abs(tmpMarker[2]-256)))
            if max(abs(tmpMarker[0] - marker[0]), abs(tmpMarker[1] - marker[1]),
                   abs(tmpMarker[2] - marker[2])) < 310 and max(abs(tmpMarker[0] - marker[0]),
                                                                abs(tmpMarker[1] - marker[1]),
                                                                abs(tmpMarker[2] - marker[2])) != 0:
                count = count + 1;
                nearIds.append(tmpMarker[3])
        if count > 0:
            mulmarker.append([marker, count, nearIds]);

    mulmarker = sorted(mulmarker, key=lambda x: x[0][3], reverse=False)
    print("there are {} mulMarkers".format(len(mulmarker)))
    for item in mulmarker:
        print(item)

def _20190107_3(teraflyFolder, apoPath, v3drawFolder):
    commandStr = "D:/v3d_external/bin/vaa3d_msvc.exe /x D:\\vaa3d_tools\\bin\plugins\\image_geometry\\crop3d_image_series\\cropped3DImageSeries.dll /f cropTerafly " \
                 "/i  {} {}  {} /p 600 600 600".format(teraflyFolder, apoPath, v3drawFolder)
    os.system(commandStr)

def _20190107_4():

    # manual
    teraflyFolder = "E:\mouse18454_teraconvert\RES(26298x35000x11041)"
    apoPath = "D:\soamdata\\18454\\test.apo"
    # apoPath = "D:\soamdata\\18454\\soma_list.ano.apo"
    v3drawFolder = "D:\soamdata\\18454\\v3draw"
    srcManualSwcFolder = v3drawFolder + "\\..\\manualRawSwc"

    # auto
    splitFolderApp2 = v3drawFolder + "\\..\\splitToApp2"
    splitFolderApp3_1 = v3drawFolder + "\\..\\splitToApp3.1"
    tarManualSwcFolder = v3drawFolder + "\\..\\manualCutSwc"
    tarManualSwcFolder_prun = v3drawFolder + "\\..\\manualPrunedSwc"
    distanceFolder = v3drawFolder + "\\..\\distance";
    # _20190107_2(apoPath,splitFolderApp2,0)
    _20190107_3(teraflyFolder,apoPath,v3drawFolder)

def _20190108_():
    path="F:\\wholebrain.txt"
    file=open(path)
    lines=file.readlines();
    #tmp=[]

    for i in range(len(lines)):
        lines[i]=lines[i].strip('\n');
        lines[i]=lines[i].split(' ');
        #tmp.append(int(lines[i][3]))
    #print(lines)
    #print(len(lines))
    #print(tmp)
    #tmp.sort(reverse=True)
    #print(tmp)

    outfile=open(path+"\\..\\wholebrain.marker",'w')
    outfile.write("##x,y,z,radius,shape,name,comment, color_r,color_g,color_b\n")
    for i in range(len(lines)):
        outfile.write("{},{},{},{},{},{},{},{},{},{}\n".format(lines[i][0],lines[i][1],lines[i][2],3,0,0,0,125,0,0))

def _20190401_():
    path="F:\\southeast17302\\17545\\wholebrain.txt"
    file=open(path)
    lines=file.readlines();

    for i in range(len(lines)):
        lines[i] = lines[i].strip('\n');
        lines[i] = lines[i].split(',');
        if lines[i][0] == '#':
            continue

        #print((int(lines[i][3])),(int(lines[i][4])),(int(lines[i][5])))


    outfile=open(path+"\\..\\wholebrain2.apo",'w')
    outfile.write("##n,orderinfo,name,comment,z,x,y, pixmax,intensity,sdev,volsize,mass,,,, color_r,color_g,color_b\n")
    for i in range(len(lines)):
        outfile.write("%d, ,  %d,, %d,%d,%d, 0,0,0,50,0,,,,0,0,255\n"%(i+1,i+1,(int(lines[i][5]))*2,(int(lines[i][3]))*2,(int(lines[i][4]))*2))

    print("ok")

def _20190410_():
    path="F:\\SEUAllen_SomaList\\traced\\17545_somalist.ano.apo"
    file=open(path)
    lines=file.readlines()
    xyzgt=[]
    count=0
    for line in lines:
        if line.startswith("#") or line=="\n":
            continue
        line_ = [s.strip() for s in line.split(',')]
        z,x,y = line_[4:7]
        x=int(float(x))
        y = int(float(y))
        z = int(float(z))
        xyzgt.append([x,y,z])
        outfile=open("F:\\seuWholebrain\\17545\\wholebrain2.apo","r")
        outlines=outfile.readlines()
        for t in outlines:
            if t.startswith("#"):
                continue
            outline_=[m.strip() for m in t.split(',')]
            zs, xs, ys = outline_[4:7]
            xs = int(float(xs))
            ys = int(float(ys))
            zs = int(float(zs))
           # print(xs, ys, zs)
            flag=0
            if math.sqrt((xs-x)**2+(ys-y)**2+(zs-z)**2)<=25:
                flag=1
            if flag==1:
                #print(x,y,z)
                xyzgt.remove([x,y,z])
                count=count+1
                break
        outfile.close()
    for i in xyzgt:
        print(i[0],i[1],i[2])
    savefile=open("F:\\seuWholebrain\\17545\\%ds.apo"%len(xyzgt),"w")
    savefile.write("##n,orderinfo,name,comment,z,x,y, pixmax,intensity,sdev,volsize,mass,,,, color_r,color_g,color_b\n")
    for i in range(len(xyzgt)):
        savefile.write("%d, ,  %d,, %d,%d,%d, 0,0,0,50,0,,,,0,0,255\n"%(i+1,i+1,xyzgt[i][2],xyzgt[i][0],xyzgt[i][1]))
    print("end:%d"%(count))

def judgeSameground():
    path="C:\\Users\\Ashley Chen\\Desktop\\apo\\groudtruth.apo"
    file=open(path)
    lines=file.readlines()
    count=0
    for line in lines:
        if line.startswith("#") or line=="\n":
            continue
        line_ = [s.strip() for s in line.split(',')]
        z,x,y = line_[4:7]
        x=int(float(x))
        y = int(float(y))
        z = int(float(z))
        outfile=open("F:\\southeast17302\\18454\\one\\wholebrain2.apo","r")
        outlines=outfile.readlines()
        for t in outlines:
            if t.startswith("#"):
                continue
            outline_=[m.strip() for m in t.split(',')]
            zs, xs, ys = outline_[4:7]
            xs = int(float(xs))
            ys = int(float(ys))
            zs = int(float(zs))
           # print(xs, ys, zs)
            flag=0
            if math.sqrt((xs-x)**2+(ys-y)**2+(zs-z)**2)<=25:
                print(x,y,z)
                path1 = "C:\\Users\\Ashley Chen\\Desktop\\apo\\groudtruth.apo"
                file1 = open(path1)
                lines1 = file1.readlines()
                for linet in lines1:
                    if linet.startswith("#") or linet == "\n":
                        continue
                    line_t = [st.strip() for st in linet.split(',')]
                    zt, xt, yt = line_t[4:7]
                    xt = int(float(xt))
                    yt = int(float(yt))
                    zt = int(float(zt))
                    if xt==x and yt==y and zt==z:
                        continue
                    if math.sqrt((xs - xt) ** 2 + (ys - yt) ** 2 + (zs - zt) ** 2) <= 25:
                        print("duplicate:%d,%d,%d"%(xt,yt,zt))


            # if flag==1:
            #     print(x,y,z)
            #     count=count+1
            #     break
        outfile.close()

def _20190420_():
    path = "I:\\mouseID_321237-17302\\17302.apo"
    file = open(path)
    lines = file.readlines()
    count = 0
    for i in range(len(lines)):
        lines[i] = lines[i].strip('\n');
        lines[i] = lines[i].split(' ');
        x = int(float(lines[i][0]))
        y = int(float(lines[i][1]))
        z = int(float(lines[i][2]))
        outfile = open("F:\\compare\\second.txt", "r")
        outlines = outfile.readlines()
        for t in range(len(outlines)):
            outlines[t] = outlines[t].strip('\n');
            outlines[t] = outlines[t].split(' ');
            xs = int(float(outlines[t][0]))
            ys = int(float(outlines[t][1]))
            zs= int(float(outlines[t][2]))

           # print(xs, ys, zs)
            flag = 0
            if math.sqrt((xs - x) ** 2 + (ys - y) ** 2 + (zs - z) ** 2) <=200:
                flag = 1
            if flag == 1:
                print(x, y, z)
                count = count + 1
                break
        outfile.close()
    print("end:%d" % (count))

def _groundone_():
    path="C:\\Users\\Ashley Chen\\Desktop\\apo\\groudtruth.apo"
    file=open(path)
    outlines=file.readlines();
    for t in outlines:
        if t.startswith("#"):
            continue
        outline_ = [m.strip() for m in t.split(',')]
        zs, xs, ys = outline_[4:7]
        xs = int(float(xs))
        ys = int(float(ys))
        zs = int(float(zs))
        print(xs, ys, zs)


def txtToapo():
    path="F:\\southeast17302\\18454\\one\\10ge.txt"
    file=open(path)
    lines=file.readlines();

    for i in range(len(lines)):
        lines[i] = lines[i].strip('\n');
        lines[i] = lines[i].split(' ');
        if lines[i][0] == '#':
            continue

        #print((int(lines[i][3])),(int(lines[i][4])),(int(lines[i][5])))


    outfile=open(path+"\\..\\10.apo",'w')
    outfile.write("##n,orderinfo,name,comment,z,x,y, pixmax,intensity,sdev,volsize,mass,,,, color_r,color_g,color_b\n")
    for i in range(len(lines)):
        outfile.write("%d, ,  %d,, %d,%d,%d, 0,0,0,50,0,,,,0,0,255\n"%(i+1,i+1,(int(lines[i][2]))/2,(int(lines[i][0]))/2,(int(lines[i][1]))/2))

    print("ok")

if __name__=="__main__":
    # _20181224_()
    # _20181228_()


    # _20190103_();


   #
    #_20190401_()
    #_20190108_()
    _20190410_()
   #_20190420_()
    #_groundone_()
    #txtToapo()
    #judgeSameground()



