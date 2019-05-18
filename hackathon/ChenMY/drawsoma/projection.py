import os
import sys
import shutil
import numpy as np
import math
def _0422_():
    path="F:\\data"
    filename=os.listdir(path)
    for file in filename:
        print (os.path.splitext(file)[0])
        commandStr = "D:/v3d_external/bin/vaa3d_msvc.exe " \
                     "/x D:\\v3d_external\\bin\\plugins\\image_projection\\maximum_intensity_projection_Z_Slices\\mipZSlices.dll " \
                     "/f mip_zslices " \
                     "/i F:\\data\\{} " \
                     "/p 1:1:e "\
                     "/o F:\\convert\\{}.tif".format(file,os.path.splitext(file)[0])

        os.system(commandStr)
    print("ok")
if __name__=="__main__":
    _0422_()