import SimpleITK as sitk
import numpy as np
import glob as glob
import os

path=r"D:\A_beshe\pred\*.tiff"
imgs=glob.glob(path)
print(imgs)
raws=glob.glob("E:/soma_img_crop_uint8/*/*.tiff")
# print(raws)
for img in imgs:
    imgrawpath=img.split("seg_")[-1]
    rawpath=""
    for raw in raws:
        if imgrawpath in raw:
            rawpath=raw
    imgraw=sitk.ReadImage(rawpath,sitk.sitkInt8)
    imgraw=sitk.GetArrayFromImage(imgraw)
    imgdata=sitk.ReadImage(img,sitk.sitkInt8)
    imgdata=sitk.GetArrayFromImage(imgdata)
    x,y,z=imgdata.shape
    print(x,y,z)
    review=np.zeros((128*2,128*3))
    review[0:128, 0:128] = imgraw[63, :, :]
    review[0:128, 128:256] = imgraw[:, 63, :]
    review[0:128, 256:384] = imgraw[:, :, 63]
    review[128:256,0:128]=imgdata[63,:,:]
    review[128:256,128:256]=imgdata[:,63,:]
    review[128:256, 256:384] = imgdata[:, :, 63]
    review=np.array(review,dtype='uint8')
    imgn=sitk.GetImageFromArray(review)
    sitk.WriteImage(imgn,os.path.join("D:\A_beshe",img.split("\\")[-1]))