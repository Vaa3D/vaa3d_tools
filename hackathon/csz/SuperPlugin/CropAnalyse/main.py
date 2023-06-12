from pylib.swc_handler import *
import math
import numpy as np
import os
import pandas as pd
from pylib.swc_processing import swc_to_image
from pylib.img_io import *
from LocalTracing import *
import glob as glob

def Writeapo(coord,apo_path):
    with open(apo_path,'w') as apo_file:
        apo_file.write("##n,type,x,y,z,radius,parent\n")
        for i in range(coord.shape[0]):
            line=" , , , ,"+str((coord[i][2]+coord[i][5])/2)+" ,"+str((coord[i][0]+coord[i][3])/2)+" ,"+str((coord[i][1]+coord[i][4])/2)+" \n"
            apo_file.write(line)

def dis(x,y):
    assert len(x)==3 and len(y)==3
    return math.sqrt((float(x[0])-y[0])**2+(float(x[1])-y[1])**2+(float(x[2])-y[2])**2)

def GetShape():
    csv=pd.read_csv('shape.csv')
    csv=np.array(csv)
    imgdict=dict()
    for line in csv:
        imgdict[line[0]]=line[1:4]
    return imgdict

def getBoundingBox(center,shape):
    x=int(center[0])
    y=int(center[1])
    z=int(center[2])
    if shape[0]<128 or shape[1]<128:
        return [-1,-1,-1,-1,-1,-1]
    flagx,flagy,flagz=False,False,False
    if x<64:
        xs=0
        xe=x+64
        flagx=True
    elif x>shape[0]-64:
        xs=x-64
        xe=shape[0]
        flagx = True

    if y<64:
        ys=0
        ye=y+64
        flagy = True
    elif y>shape[1]-64:
        ys=y-64
        ye=shape[1]
        flagy = True

    if z<64:
        zs=0
        ze=z+64
        flagz = True
    elif z>shape[2]-64:
        zs=z-64
        ze=shape[2]
        flagz = True

    if flagx==False:
        xs=x-64
        xe=x+64
    if flagy==False:
        ys=y-64
        ye=y+64
    if flagz == False:
        zs=z-64
        ze=z+64

    if shape[2]<128:
        zs=0
        ze=shape[2]
    return [xs,ys,zs,xe,ye,ze]

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

def inRegion(p,region):
    if p[0]<0 or p[1]<0 or p[2]<0:
        return True
    for BBox in region:
        if inBBox(BBox,p):
            return True
    return False

def GetCropCenterbySwc(swc,startpoint,shape):
    pidx=-1
    mind=1e6
    for p in swc:   #find root node
        if p[6]==-1:
            d=dis(p[2:5],startpoint)
            if d<mind:
                pidx=int(p[0])
                mind=d
    if pidx==-1:
        return
    flag=np.ones(len(swc))
    queue=[]

    queue.append([swc[pidx-1][2:5],-1]) #root node insert queue(x,y,z)
    # print(swc[pidx-1][2:5])
    cropcenter=[]
    count=6
    Region=[]
    # print(shape)
    while len(queue)>0 :#np.sum(flag)>0 and
        count-=1
        # for item in queue:
        #     print(item[0])
        # print(queue[0][0])
        BBox=getBoundingBox(queue[0][0],shape) #get xs ys zs xe ye ze by x y z
        oridir = queue[0][1]
        queue.pop(0)
        pcenter=[(BBox[0]+BBox[3])/2,(BBox[1]+BBox[4])/2,(BBox[2]+BBox[5])/2]
        # print(pcenter)
        if inRegion(pcenter,Region) is True:
            continue
        Region.append(BBox)


        if BBox[0]>=BBox[3] or BBox[1]>=BBox[4] or BBox[2]>=BBox[5]:
            continue

        cropcenter.append(BBox)
        tipleft=[]
        tipright=[]
        tipup=[]
        tipdown=[]
        tipout=[]
        tipin=[]
        for p in swc:
            # if p[2]>=BBox[0] and p[2]<=BBox[3] and p[3]>=BBox[1] and p[3]<=BBox[4] and p[4]>=BBox[2] and p[4]<=BBox[5] and flag[int(p[0])-1]==1:
            #     flag[int(p[0])-1]=0
            #     if p[2]<BBox[0]+0.05*128:
            #         tipleft.append(p)                   #left-1
            #         continue
            #     elif p[2]>BBox[3]-0.05*128:
            #         tipright.append(p)                  #right-2
            #         continue
            #     elif p[3]<BBox[1]+0.05*128:
            #         tipdown.append(p)                   #down-3
            #         continue
            #     elif p[3]>BBox[4]-0.05*128:
            #         tipup.append(p)                     #up-4
            #         continue
            #     elif p[4]<BBox[2]+0.05*128:
            #         tipout.append(p)                    #front-5
            #         continue
            #     elif p[4]>BBox[5]-0.05*128:
            #         tipin.append(p)                     #back-6
            #         continue
            #id 0 ,pid 6 ,x y z 2 3 4
            if p[6]==-1:
                continue
            point=p[2:5]
            parentpoint=swc[p[6]-1][2:5]
            if (inBBox(BBox,point) is True and inBBox(BBox,parentpoint) is False) or (inBBox(BBox,point) is False and inBBox(BBox,parentpoint) is True):
                crossps=[]
                # print(point,parentpoint,BBox)

                for i in range(len(BBox)):
                    if i%3==0:
                        crossp=crosspoint(point,parentpoint,BBox[i],0)
                    elif i%3==1:
                        crossp = crosspoint(point, parentpoint, BBox[i], 1)
                    elif i%3==2:
                        crossp=crosspoint(point,parentpoint,BBox[i],2)
                    crossps.append(crossp)
                for i in range(len(crossps)):
                    if inBBox(BBox,crossps[i]) is True:
                        # print(crossps[i])
                        if i==0:
                            #continue
                            tipleft.append(crossps[i])
                        elif i==1:
                            #continue
                            tipup.append(crossps[i])
                        elif i==2:
                            #continue
                            tipout.append(crossps[i])
                        elif i==3:
                            #continue
                            tipright.append(crossps[i])
                        elif i==4:
                            #continue
                            tipdown.append(crossps[i])
                        elif i==5:
                            #continue
                            tipin.append(crossps[i])

        if len(tipleft)>0:
            # if oridir==1:
            #     continue
            maxy=-1e6
            miny=1e6
            maxz=-1e6
            minz=1e6
            for tp in tipleft:
                if float(tp[1])<=miny:
                    miny=int(tp[1])
                if float(tp[1])>=maxy:
                    maxy=int(tp[1])
                if float(tp[2])<=minz:
                    minz=int(tp[2])
                if float(tp[2])>=maxz:
                    maxz=int(tp[2])
            centert=[BBox[0]-100,(maxy+miny)/2,(maxz+minz)/2]
            if inRegion(centert,Region) is False:
                queue.append([centert,2])
        if len(tipright)>0:
            # if oridir==2:
            #     continue
            maxy=-1e6
            miny=1e6
            maxz=-1e6
            minz=1e6
            for tp in tipright:
                if float(tp[1])<=miny:
                    miny=int(tp[1])
                if float(tp[1])>=maxy:
                    maxy=int(tp[1])
                if float(tp[2])<=minz:
                    minz=int(tp[2])
                if float(tp[2])>=maxz:
                    maxz=int(tp[2])
            centert=[BBox[3]+100,(maxy+miny)/2,(maxz+minz)/2]
            if inRegion(centert,Region) is False:
                queue.append([centert,1])
        if len(tipup)>0:
            print("lenup", len(tipup))
            # if oridir==3:
            #     continue
            maxx=-1e6
            minx=1e6
            maxz=-1e6
            minz=1e6
            for tp in tipup:
                if float(tp[0])<=minx:
                    minx=int(tp[0])
                if float(tp[0])>=maxx:
                    maxx=int(tp[0])
                if float(tp[2])<=minz:
                    minz=int(tp[2])
                if float(tp[2])>=maxz:
                    maxz=int(tp[2])
            centert=[(maxx+minx)/2,BBox[1]-100,(maxz+minz)/2]
            if inRegion(centert,Region) is False:
                queue.append([centert,4])
            print("up",centert)
        if len(tipdown)>0:
            # if oridir==4:
            #     continue
            maxx=-1e6
            minx=1e6
            maxz=-1e6
            minz=1e6
            for tp in tipdown:
                if float(tp[0])<=minx:
                    minx=int(tp[0])
                if float(tp[0])>=maxx:
                    maxx=int(tp[0])
                if float(tp[2])<=minz:
                    minz=int(tp[2])
                if float(tp[2])>=maxz:
                    maxz=int(tp[2])
            centert=[(maxx+minx)/2,BBox[4]+100,(maxz+minz)/2]
            if inRegion(centert,Region) is False:
                queue.append([centert,3])
        if len(tipout)>0:
            # if oridir==5:
            #     continue
            maxy=-1e6
            miny=1e6
            maxx=-1e6
            minx=1e6
            for tp in tipout:
                if float(tp[1])<=miny:
                    miny=int(tp[1])
                if float(tp[1])>=maxy:
                    maxy=int(tp[1])
                if float(tp[0])<=minx:
                    minx=int(tp[0])
                if float(tp[0])>=maxx:
                    maxx=int(tp[0])
            centert=[(maxx+minx)/2,(maxy+miny)/2,BBox[2]-100]

            if inRegion(centert,Region) is False:
                queue.append([centert,6])
        if len(tipin)>0:
            # if oridir==6:
            #     continue
            maxy=-1e6
            miny=1e6
            maxx=-1e6
            minx=1e6
            for tp in tipin:
                if float(tp[1])<=miny:
                    miny=int(tp[1])
                if float(tp[1])>=maxy:
                    maxy=int(tp[1])
                if float(tp[0])<=minx:
                    minx=int(tp[0])
                if float(tp[0])>=maxx:
                    maxx=int(tp[0])
            centert=[(maxx+minx)/2,(maxy+miny)/2,BBox[5]+100]
            # print(BBox)
            # print("centert:",centert)
            if inRegion(centert,Region) is False:
                queue.append([centert,5])

    return cropcenter


def CropSwc(swc,cropcenter):
    for center in cropcenter:
        tempswc=[]
        x=(center[0]+center[3])/2
        y = (center[1] + center[4]) / 2
        z = (center[2] + center[5]) / 2
        for p in swc:
            if float(p[2])>=center[0] and float(p[2])<=center[3] and float(p[3])>=center[1] and float(p[3])<=center[4] and float(p[4])>=center[2] and float(p[4])<=center[5]:
                tempp=list(p)
                # print(tempp,center)
                tempp[2]-=center[0]
                tempp[3]-=center[1]
                tempp[4]-=center[2]
                tempswc.append(p)
        write_swc(tempswc,os.path.join(r'D:\A_pythonwork\CropAnalyse\6',str(int(x))+"_"+str(int(y))+"_"+str(int(z))+".swc"))

def CropMask(swcmask,cropcenter,suffix,savepath):
    if os.path.exists(savepath) is False:
        os.mkdir(savepath)
    for center in cropcenter:
        x=(center[0]+center[3])/2
        y = (center[1] + center[4]) / 2
        z = (center[2] + center[5]) / 2
        makstemp=swcmask[center[0]:center[3],center[1]:center[4],center[2]:center[5]]
        Writeimg(makstemp,os.path.join(savepath,str(int(x))+"_"+str(int(y))+"_"+str(int(z))+"_"+suffix+".tiff"))

def CropRaw(imgraw,cropcenter,savepath):
    if os.path.exists(savepath) is False:
        os.mkdir(savepath)
    imgnamelist=[]
    # for center in cropcenter:
    #     x=(center[0]+center[3])/2
    #     y = (center[1] + center[4]) / 2
    #     z = (center[2] + center[5]) / 2
    #     makstemp=imgraw[center[0]:center[3],center[1]:center[4],center[2]:center[5]]
    #     name=os.path.join(savepath,str(int(x))+"_"+str(int(y))+"_"+str(int(z))+"_raw.tiff")
    #     name=os.path.abspath(name)
    #     Writeimg(makstemp,name)
    #     imgnamelist.append(name)
    swc=[]
    for i in range(len(cropcenter)):
        x=(cropcenter[i][0]+cropcenter[i][3])/2
        y = (cropcenter[i][1] + cropcenter[i][4]) / 2
        z = (cropcenter[i][2] + cropcenter[i][5]) / 2
        swc.append([i+1,3,x,y,z,10,-1])
    swc=np.array(swc)
    Writeswc(swc,savepath+"/1.swc")
    print(savepath+"/1.swc")
    return imgnamelist

def WriteArray(array,savefile):
    savefile=os.path.abspath(savefile)
    dir=os.path.dirname(savefile)
    if os.path.exists(dir) is False:
        os.mkdir(dir)
    with open(savefile,'w') as f:
        for line in array:
            writeline=""
            for i in line:
                writeline+=str(i)+","
            writeline+='\n'
            f.write(writeline)


def main():
    savepath=r'E:\testdata'
    imgshapedict = GetShape()
    goldswclist=glob.glob(r"E:\gold166\*\*\*.v3dpbd.swc")
    tracingswclist=glob.glob(r"E:\csz\gold166\20161101_reconstruction_for_gold166_rhea_5571\*\*\*.swc")
    for goldswc in goldswclist:
        try:
            name=goldswc.split('\\')[-1][:-4]
            print(name)
            if name!='140918c9.tif.v3dpbd':
                continue


            namepath=os.path.join(savepath,name)
            if os.path.exists(namepath) is False:
                os.mkdir(namepath)
            # else:
            #     continue
            imgrawpath='D:/A_pythonwork/PluginPipeline/gold/imgtiff/'+name+".tiff"
            swc=parse_swc(goldswc)
            #maskgold=swc_to_image(swc,imgshape=(imgshapedict[name+".tiff"][0],imgshapedict[name+".tiff"][1],imgshapedict[name+".tiff"][2]))

            cropcenter=GetCropCenterbySwc(swc,imgshapedict[name+".tiff"]/2,imgshapedict[name+".tiff"])
            WriteArray(cropcenter,os.path.join(savepath,"CropCenter/"+name+".csv"))
            #CropMask(maskgold,cropcenter,"gold",os.path.join(namepath,"gold"))
            CropRaw(Readimg(imgrawpath),cropcenter,os.path.join(namepath,"raw"))


            # for t in tracingswclist:
            #     if name.split('.')[0] in t:
            #         print(t)
            #         try:
            #             filename=t.split('.v3dpbd_')[-1][:-4]
            #             # print(filename)
            #             swctracing=parse_swc(t)
            #             swctracingmask=swc_to_image(swctracing,imgshape=(imgshapedict[name+".tiff"][0],imgshapedict[name+".tiff"][1],imgshapedict[name+".tiff"][2]))
            #             CropMask(swctracingmask,cropcenter,filename,os.path.join(namepath,filename))
            #         except:
            #             continue
        except:
            continue


if __name__ == '__main__':
    # swc=parse_swc(r'E:\gold166\e_checked6_chick_uw\DONE_09-2902-04R-01C-60x_merge_c1\09-2902-04R-01C-60x_merge_c1.v3dpbd.swc')
    # swc2=parse_swc(r'E:\csz\gold166\reconstructions_for_gold166_titan_edison_identical_2407\checked6_chick_uw\DONE_10-2900-control-cell-05.oif-C0\10-2900-control-cell-05.oif-C0.v3dpbd_EnsembleNeuronTracerV2n.swc')
    # imgshapedict=GetShape()
    # name="09-2902-04R-01C-60x_merge_c1.v3dpbd.tiff"
    # img=Readimg(r"D:\A_pythonwork\PluginPipeline\gold\imgtiff\09-2902-04R-01C-60x_merge_c1.v3dpbd.tiff")
    # # label = swc_to_image(swc,imgshape=(imgshapedict[name][0],imgshapedict[name][1],imgshapedict[name][2]))
    # # label2=swc_to_image(swc2,imgshape=(imgshapedict[name][0],imgshapedict[name][1],imgshapedict[name][2]))
    # print(imgshapedict[name]/2)
    # cropcenter=GetCropCenterbySwc(swc,imgshapedict[name]/2,imgshapedict[name])
    # WriteArray(cropcenter,"1.csv")
    # CropMask(label,cropcenter,"gold")
    # rawimg=CropRaw(img, cropcenter,savepath="6")
    # advantraname=[]
    # for name in rawimg:
    #     res=advantra(name)
    #     advantraname.append(res)
    # Writeimg(label,"09-2902-04R-01C-60x_merge_c1.v3dpbd.tiff")
    main()
