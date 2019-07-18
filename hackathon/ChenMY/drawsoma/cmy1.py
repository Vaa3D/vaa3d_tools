X=17994
Y=27300
Z=5375

shift_x=0
shift_y=0
shift_z=0

Cx=int((X-shift_x*2)/512);
Cy=int((Y-shift_y*2)/512);
Cz=int((Z-shift_z*2)/512);


def main(path):
    xyz=[]
    file=open(path);
    lines=file.readlines();
    for i in range(len(lines)):
        if lines[i][0]=='#':
            continue
        lines[i]=lines[i].strip('\n').split(',');
        xyz.append([int(lines[i][5]),int(lines[i][6]),int(lines[i][4])])
    print(xyz)
    return xyz;

if __name__=="__main__":
    path="F:\\seuWholebrain\\17545\\3.apo"
    xyz=main(path)
    index=0;
    #center_xyz=xyz.copy()
    center_xyz=[]
    xyz_all=[]
    for i in range(Cz):
        for j in range(Cy):
            for k in range(Cx):
                px=k*512+257+shift_x;
                py=j*512+257+shift_y;
                pz=i*512+257+shift_z;
                for index in range(len(xyz)):
                    if abs(px-xyz[index][0])<=256 and  abs(py-xyz[index][1])<=256 and  abs(pz-xyz[index][2])<=256:
                        center_xyz.append([px,py,pz])



    print(center_xyz)
    outfile=open(path+"\\..\\center_xyz3.apo",'w');
    for i in range(len(center_xyz)):
        outfile.write("{}, ,  {},, {},{},{}, 0,0,0,50,0,,,,0,0,255\n".format(i,i,center_xyz[i][2],center_xyz[i][0],center_xyz[i][1]))






