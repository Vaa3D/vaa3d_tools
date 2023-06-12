import SimpleITK as sitk
import numpy as np

np.set_printoptions(suppress=True)

def Readimg(img_path):
    img = sitk.ReadImage(img_path)
    img = sitk.GetArrayFromImage(img)
    img=np.rot90(img,1,axes=(0,2))
    #img = np.rot90(img, 1, axes=(0, 1))
    img=np.flip(img,axis=0)
    img = np.flip(img, axis=1)
    #img = np.flip(img, axis=2)
    return img

def Readswc(swc_path):
    swc=[]
    with open(swc_path) as swc_file:
        lines=swc_file.readlines()
        for line in lines:
            if line[0]=="#":
                continue
            line=line.split(" ")
            if line[-1]=='\n':
                line=line[:-1]
            swc.append(line)

    swc=np.array(swc,dtype='float')
    return swc

def ReadMarker(marker_path):
    marker=[]
    with open(marker_path) as marker_file:
        lines=marker_file.readlines()
        for line in lines:
            if line[0]=="#":
                continue
            line=line.split(", ")
            line=line[:3]
            # print(line)
            marker.append(line)

    marker=np.array(marker,dtype='float')
    return marker

def Writeimg(img,img_path):
    #img = np.flip(img, axis=2)
    img = np.flip(img, axis=1)
    img = np.flip(img, axis=0)
    img=np.rot90(img,-1,axes=(0,2))

    img = sitk.GetImageFromArray(img)
    sitk.WriteImage(img, img_path)

def Writeswc(swc,swc_path):
    with open(swc_path,'w') as swc_file:
        swc_file.write("##n,type,x,y,z,radius,parent\n")
        for i in range(swc.shape[0]):
            line=str(int(swc[i][0]))+" "+str(int(swc[i][1]))+" "+str(swc[i][2])+" "+str(swc[i][3])+" "+str(swc[i][4])+" "+str(swc[i][5])+" "+str(int(swc[i][6]))+"\n"
            swc_file.write(line)


def Writemarker(marker_point,marker_path):
    with open(marker_path,'w') as marker_file:
        marker_file.write("##x,y,z,radius,shape,name,comment, color_r,color_g,color_b\n")
        line = str(float(marker_point[0]))+", "+str(float(marker_point[1]))+", "+str(float(marker_point[2]))+", 0.000, 1, , , 255,0,0"
        marker_file.write(line)

def Writeapo(coord,apo_path):
    with open(apo_path,'w') as swc_file:
        swc_file.write("##n,type,x,y,z,radius,parent\n")
        for i in range(coord.shape[0]):
            line=str(int(coord[i][0]))+" ,"+str(int(coord[i][1]))+" ,"+str(coord[i][2])+" ,"+str(coord[i][0])+" ,"+str(coord[i][2])+" ,"+str(coord[i][0])+" ,"+str(int(coord[i][1]))+" \n"
            swc_file.write(line)

def Readapo(apo_path):
    apo=[]
    with open(apo_path) as apo_file:
        lines=apo_file.readlines()
        for line in lines:
            if line[0]=="#":
                continue
            line=line.split(",")
            apo.append(line)
    apo=np.array(apo,dtype='float')
    return apo

def Writecsv(numpy,csv_path,firstline=""):
    with open(csv_path,'w',encoding='utf-8') as csv_file:
        csv_file.write(firstline+"\n")
        for i in range(numpy.shape[0]):
            line=""
            for j in range(numpy.shape[1]):
                line+=str(numpy[i][j])+", "
            line+="\n"
            csv_file.write(line)
