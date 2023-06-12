import SimpleITK as sitk
import numpy as np

def Normalize():
    path=r"D:\A_DLcsz\yy\rawimg\1--C00--T00022.tiff"
    img=sitk.ReadImage(path,sitk.sitkUInt16)
    imgdata=sitk.GetArrayFromImage(img)
    print(imgdata.shape)
    newimg=np.zeros((64,512,512))
    newimg=imgdata[4:68,51:563,51:563]
    newimg=np.array(newimg,dtype='uint16')
    # imgdata=[imgdata]
    # imgdata=np.array(imgdata)
    # print(imgdata.shape)
    img=sitk.GetImageFromArray(newimg)
    sitk.WriteImage(img,r"D:\A_DLcsz\yy\img\1--C00--T00022normal.tiff")

if __name__=="__main__":
    Normalize()