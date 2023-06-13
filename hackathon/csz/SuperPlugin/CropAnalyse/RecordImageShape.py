import glob as glob
from pylib.img_io import Readimg,Writecsv
import numpy as np

def writeshape():
    files=glob.glob(r'D:\A_pythonwork\PluginPipeline\gold\imgtiff\*.tiff')
    csv=[]
    for file in files:
        name=file.split('\\')[-1]
        shape=Readimg(file).shape
        line=[name,shape[0],shape[1],shape[2]]
        csv.append(line)
    csv=np.array(csv)
    Writecsv(csv,'shape.csv')


def dadas():
    files=glob.glob(r'E:\review\*')
    count=0
    imgc=0
    for file in files:
        fs=glob.glob(file+'\*')
        # print(fs)
        if file+'\label_CSZ.csv' in fs:
            count+=1
            imgc+=len(fs)
    print(count,imgc)

if __name__ == '__main__':
    #writeshape()
    dadas()