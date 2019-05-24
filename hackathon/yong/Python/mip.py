import os
import sys
import cv2 as cv
import numpy as np
from libtiff import TIFF
import matplotlib.pyplot as plt

def mipFile(i,p,o):

    commandStr="D:\\bin\\vaa3d_msvc.exe -x libmipZSlices " \
               "-f mip_zslices " \
               "-i {} " \
               "-p {} " \
               "-o {} ".format(i,p,o)

    os.system(commandStr)


def mipFolder(infolder,p,outfolder):

    if not os.path.exists(outfolder):
        os.mkdir(outfolder)
    for file in os.listdir(infolder):
        print(file)
        if file[-3:]=="tif":
            # print()
            mipFile(infolder+"//"+file,p,outfolder+"//"+p+"_"+file);


def mergeImg(imgList):
    img=cv.imread(imgList[0]);
    for i in len(1,imgList):
        img=cv.imread(imgList[i]);

    return img


def tifFolder(infolder,outfolder):
    filelist=os.listdir(infolder)
    filelist.sort()
    for file in filelist:
        if file[-8:]=="mask.tif" and file[0:5]=="1:1:e":
            # count=1;
            old_p="1:1:e"
            # for p in ["1:1:e","1:e:1","e:1:1"]:
            #     file=file.replace(old_p,p,1)
            #     print(file)
            #     old_p=p;
            #     org=TIFF.open(infolder+"//"+file[:-9]+".tif", mode='r').read_image().astype('uint8');
            #     mask=TIFF.open(infolder+"//"+file, mode='r').read_image().astype('uint8');
            #     img1=np.zeros((org.shape[0],org.shape[1],3)).astype('uint8');
            #     for i in range(org.shape[0]):
            #         for j in range(org.shape[1]):
            #             if mask[i,j]==0:
            #                 img1[i,j,:]=org[i,j]
            #             else:
            #                 img1[i,j,0]=mask[i,j];
            #             # print(mask[i,j])
            #             # img1[i,j,2]=org[i,j];
            #             # img1[i,j,0]=mask[i,j];
            #
            #     plt.subplot(eval("13{}".format(count)))
            #     plt.title(p)
            #     count=count+1;
            # # plt.subplot(131)
            #     plt.imshow(img1);
            # plt.show()

            org = TIFF.open(infolder + "//" + file[:-9] + ".tif", mode='r').read_image().astype('uint8');
            mask=TIFF.open(infolder+"//"+file, mode='r').read_image().astype('uint8');
            img1=np.zeros((org.shape[0],org.shape[1],3)).astype('uint8');
            for i in range(org.shape[0]):
                for j in range(org.shape[1]):
                    if mask[i,j]==0:
                        img1[i,j,:]=org[i,j]
                    else:
                        img1[i,j,0]=mask[i,j];
                    # print(mask[i,j])
                    # img1[i,j,2]=org[i,j];
                    # img1[i,j,0]=mask[i,j];
            plt.subplot(131)
            plt.imshow(org,cmap="gray")
            plt.title("org")

            plt.subplot(132)
            plt.imshow(mask,cmap="gray")
            plt.title("mask")

            plt.subplot(133)
            plt.imshow(img1)
            plt.title("org+mask")

            # plt.show()
            print(file.split("."))
            plt.savefig(outfolder+"//"+file.split('.')[0]+".png")



if __name__=="__main__":

    infolder="C:\\Users\\admin\\Desktop\\zhangyong\\MIP\\samples"
    outfolder="C:\\Users\\admin\\Desktop\\zhangyong\\MIP\\samples_mip"
    rgbfolder="C:\\Users\\admin\\Desktop\\zhangyong\\MIP\\samples_rgb"
    mipFolder(infolder,"1:1:e",outfolder)
    # mipFolder(infolder, "1:e:1", outfolder)
    # mipFolder(infolder, "e:1:1", outfolder)
    # tifTopng("./test.tif","../main.png")

    tifFolder(outfolder,rgbfolder)