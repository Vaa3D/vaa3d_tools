import glob as glob
from img_io import *
import os
import math
import numpy as np
from sklearn.decomposition import PCA

v3d=r'E:\Downloads\Vaa3D_v6.007_Windows_64bit\Vaa3D_v6.007_Windows_64bit\Vaa3D-x.exe'

def dis(a,b):
    return math.sqrt((a[0]-b[0])**2+(a[1]-b[1])**2+(a[2]-b[2])**2)


if __name__ == '__main__':

    print("666")