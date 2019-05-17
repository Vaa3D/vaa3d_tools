import numpy as np
import os
def makeFalsePng(truePngFolder,FalsePngFolder ):
    xyz=[]
    for file in os.listdir(truePngFolder):
        if os.path.splitext(file)[1]=='.png':
            name=os.path.splitext(file)[0]
            x=name.split("_")[0]
            y=name.split("_")[1]
            z=name.split("_")[2]
            #print(x,y,z)
            xyz.append([x,y,z])
    print("step 1 ok")
    for file1 in os.listdir(FalsePngFolder):
        #print(file1)
        if os.path.splitext(file1)[1]=='.png':
            name=os.path.splitext(file1)[0]
            x1=name.split("_")[0]
            y1=name.split("_")[1]
            z1=name.split("_")[2]
            if [x1,y1,z1] in xyz:
                os.remove(os.path.join(FalsePngFolder,file1))
                #print(os.path.join(FalsePngFolder,file1))
    print("remove ok")


if __name__ == "__main__":
    truePngFolder="F:\\17545soma"
    FalsePngFolder ="E:\\mypersonalgit\\somadata\\17545_50_200\\128_128_64\\png\\false"
    makeFalsePng(truePngFolder, FalsePngFolder)



