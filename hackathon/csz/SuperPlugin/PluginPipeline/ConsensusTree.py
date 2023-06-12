import os
import glob as glob
from img_io import *
from swc_processing import *
v3d=r"E:\Downloads\Vaa3D_v6.007_Windows_64bit\Vaa3D_v6.007_Windows_64bit\Vaa3D-x.exe"

def GenerateConsensusTree(filepath):
    firstfiles=glob.glob(filepath+"\\*")
    for firstfile in firstfiles:
        if os.path.isdir(firstfile):
            secondfiles=glob.glob(firstfile+"\\*")
            for secondfile in secondfiles:
                if os.path.isdir(secondfile):
                    name=secondfile.split('\\')[-1]
                    if name=='PaxHeader':
                        continue
                    inputfile=secondfile+"\\*.swc"
                    files=glob.glob(inputfile)
                    length=len(files)
                    for file in files:
                        try:
                            swc=Readswc(file)
                        except:
                            continue
                        if swc.shape[0]==0:
                            length-=1
                            continue
                        resswcname=file.split('\\')[-1]+"_resampled.swc"
                        step=5
                        while swc.shape[0]>8000:
                            if step>100:
                                length-=1
                                break
                            dir=os.path.dirname(file)
                            savepath=dir+'\\'+resswcname
                            resample=v3d+" /x resample_swc /f resample_swc /i "+file+" /o "+savepath+" /p "+str(step)
                            # print(resample)
                            os.system(resample)
                            try:
                                swc=Readswc(savepath)
                            except:
                                length-=1
                                break
                            step+=5
                    if length==0:
                        continue
                    cmd=v3d+" /x consensus_swc /f consensus_swc /i "+inputfile+" /o D:\A_result\consensus_swc1\\"+name+".eswc"+" /p "+str(int(length/4))+" 10 1"
                    # print(cmd)
                    os.system(cmd)
                    #print(inputfile,int(length/3))

def GenerateConsensusImage(filepath,namelist):
    firstfiles=glob.glob(filepath+"\\*")
    for firstfile in firstfiles:
        if os.path.isdir(firstfile):
            secondfiles=glob.glob(firstfile+"\\*")
            for secondfile in secondfiles:
                if os.path.isdir(secondfile):
                    name=secondfile.split('\\')[-1]
                    if name=='PaxHeader':
                        continue
                    inputfile=secondfile+"\\*.swc"
                    files=glob.glob(inputfile)
                    if len(files)>0:
                        imgname=files[0].split('\\')[-1].split(".v3dpbd")[0]
                    x,y,z=0,0,0
                    for imgnm in namelist:
                        if imgname in imgnm:
                            x,y,z=Readimg(imgnm).shape
                    print(x,y,z,imgname)
                    if x!=0 and y!=0 and z!=0:
                        swcmask=np.zeros((z,y,x),dtype='uint8')
                        for file in files:
                            # try:
                            #     swc=Readswc(file)
                            # except:
                            #     continue
                            # swcmaskt=np.zeros((x,y,z))
                            # for line in swc:
                            #     mx,my,mz=int(line[2]),int(line[3]),int(line[4])
                            #     if swcmaskt[mx][my][mz]==0:
                            #         swcmaskt[mx][my][mz]+=1
                            #     r=int(line[5])
                            #     print(line[0])
                            #     for i in range(r*2):
                            #         for j in range(r*2):
                            #             for k in range(r*2):
                            #                 if mx+(i-r)<0 or mx+(i-r)>x:
                            #                     continue
                            #                 if my+(j-r)<0 or my+(j-r)>y:
                            #                     continue
                            #                 if mz+(k-r)<0 or mz+(k-r)>z:
                            #                     continue
                            #                 if (i-r)**2+(j-r)**2+(k-r)**2<r**2:
                            #                     if swcmask[mx+(i-r)][my+(j-r)][mz+(k-r)]==0:
                            #                         swcmask[mx+(i-r)][my+(j-r)][mz+(k-r)]+=1
                            try:
                                tree=parse_swc(file)
                                if len(tree)>20000:
                                    continue
                                swcmaskt=swc_to_image(tree,imgshape=(z,y,x))
                                # Writeimg(swcmaskt, r"D:\A_result\swcmask" + "\\" + imgname + ".tiff")
                                print(swcmaskt.shape,file)
                                swcmask+=swcmaskt
                            except:
                                continue
                        if np.max(swcmask)>0:
                            Writeimg(swcmask,r"D:\A_result\swcmask2016"+"\\"+imgname+".tiff")


def rotategold():
    path=r"D:\A_pythonwork\PluginPipeline\gold\swcimg"
    masks=glob.glob(path+'\\*.tiff')
    for mask in masks:
        name=mask.split('\\')[-1]
        maskdata=Readimg(mask)
        maskdata = np.rot90(maskdata, 1, axes=(0, 2))
        # img = np.rot90(img, 1, axes=(0, 1))
        maskdata = np.flip(maskdata, axis=0)
        maskdata = np.flip(maskdata, axis=1)
        Writeimg(maskdata,r"D:\A_result\swcmaskgold"+'\\'+name)



if __name__ == '__main__':
    # # path=r"E:\csz\gold166\reconstructions_for_gold166_titan_edison_identical_2407"
    path = r"E:\csz\gold166\20161101_reconstruction_for_gold166_rhea_5571"
    goldimgpath=r'D:\A_pythonwork\PluginPipeline\gold\imgtiff'
    imgnamelist=glob.glob(goldimgpath+'\\*.tiff')
    # GenerateConsensusTree(path)
    GenerateConsensusImage(path,imgnamelist)


    # rotategold()