import glob as glob
import numpy as np
import SimpleITK as sitk

imgs=glob.glob("D:\A_beshe\pred\*.tiff")
namelists=[]
maxy,maxz=0,0
for name in imgs:
    y,z=name.split("\\")[-1].split("seg_")[-1].split(".tiff")[0].split("_")
    y,z=int(y),int(z)
    if y>maxy:
        maxy=y
    if z>maxz:
        maxz=z
print((64,maxy,maxz))   #(60, 976, 848)
newimg=np.zeros((64,maxy+64,maxz+64))
for img in imgs:
    data = sitk.ReadImage(img, sitk.sitkInt8)
    imgraw = sitk.GetArrayFromImage(data)

    y, z = img.split("\\")[-1].split("seg_")[-1].split(".tiff")[0].split("_")
    y, z = int(y), int(z)

    newimg[:,y:y+64,z:z+64]=imgraw


new_img=np.array(newimg,dtype='uint8')
new_img = sitk.GetImageFromArray(new_img)
sitk.WriteImage(new_img, r"D:\A_beshe\pred\4976.tiff")