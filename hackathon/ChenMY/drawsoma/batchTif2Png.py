import cv2 as cv
from libtiff import TIFF
import numpy as np
import os

def readOneTif(path):
    tif = TIFF.open(path, mode='r')
    img = tif.read_image()  # 此时img为一个numpy.array
    # print(img.shape)
    # count=0;
    # imageAll=np.array([0])
    # for image in tif.iter_images():
    #     if count==0:
    #         imageAll=image;
    #     else:
    #         imageAll=np.dstack((imageAll,image))
    #         # print(image.shape)
    #     count=count+1
    #     cv.imshow("main",image)
    #     cv.waitKey(0)
    #     cv.destroyAllWindows()
    #
    # print("z={}".format(count))
    # print(imageAll.shape)
    # cv.imshow("path",img);
    # cv.waitKey(0);
    # cv.destroyAllWindows()
    return img
def main():
    folder ="E:\\mypersonalgit\\somadata\\17302_50_200\\128_128_64\\2DTif\\ture"
    outfolder="E:\\mypersonalgit\\somadata\\17302_50_200\\128_128_64\\png\pngtrue"
    for file in os.listdir(folder):
        if os.path.splitext(file)[1]=='.tif':
            img=readOneTif(folder+"\\"+file)
            cv.imwrite(outfolder+"\\"+file.split('_')[0]+"_"+file.split('_')[1]+"_"+file.split('_')[2].split(".")[0]+".0"+".png",img)

if __name__=='__main__':
    main()