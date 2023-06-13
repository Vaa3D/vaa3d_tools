import numpy as np
import SimpleITK as sitk
import os





imgpath=r"E:\soma_img_crop_uint8\18454_img_crop_uint8\18454_4976_6984_5050_crop.tiff"
croped=r"D:\A_beshe\raw"
data = sitk.ReadImage(imgpath, sitk.sitkInt8)
imgraw = sitk.GetArrayFromImage(data)
print(imgraw.shape)
x,y,z=imgraw.shape

for i in range(0,y,64):
    for j in range (0,z,64):
        if i+64<=y and j+64<=z:
            img=imgraw[:,i:i+64,j:j+64]
            img=np.resize(img,(64,64,64))
            new_img = sitk.GetImageFromArray(img)

            sitk.WriteImage(new_img, os.path.join(croped,str(i)+"_"+str(j)+".tiff"))
