import os
# color red :tp ;green:fp ;blue:fn
def createApo(list,saveapo,multiple,id,color0,color1,color2):
    outfile = open(saveapo, 'w')
    outfile.write("##n,orderinfo,name,comment,z,x,y, pixmax,intensity,sdev,volsize,mass,,,, color_r,color_g,color_b\n")
    lines=len(list)
    for i in range(lines):
        outfile.write("%d, ,  %d,, %d,%d,%d, 0,0,0,50,0,,,,%d,%d,%d\n" % (
        i + 1+id, i + 1+id, (list[i][2])*multiple, (list[i][0])*multiple, (list[i][1])*multiple,color0,color1,color2))
    print("create apo ok")
def gtApo(gtpath):
    file = open(gtpath)
    lines = file.readlines()
    xyz=[]
    for line in lines:
        if line.startswith("#") or line == "\n":
            continue
        line_ = [s.strip() for s in line.split(',')]
        z, x, y = line_[4:7]
        x = int(float(x))
        y = int(float(y))
        z = int(float(z))
        xyz.append([x,y,z])
    return xyz

def picToapo(picfolder):
    xyz=[]
    filename=os.listdir(picfolder)
    for file in filename:
        if os.path.splitext(file)[1]==".png":
            name0=os.path.splitext(file)[0]
            x=int(name0.split('_')[0])
            y = int(name0.split('_')[1])
            z = int(name0.split('_')[2])
            xyz.append([x,y,z])

    return xyz

if __name__=="__main__":
    gtpath="C:\\Users\\Ashley Chen\\Desktop\\apo\\groudtruth.apo"#需要设置
    xyz=gtApo(gtpath)
    savePath="E:\\mypersonalgit\\somadata\\18454_50_200\\64_64_64\\apo\\gt.apo"#需要设置
    createApo(xyz, savePath,1,40000,255,255,0) #需要设置