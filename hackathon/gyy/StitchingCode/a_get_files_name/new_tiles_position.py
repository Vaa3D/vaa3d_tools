import numpy as np
import matplotlib.pyplot as plt


file_name_list = []
coor=[]
file_path = 'C:/Users/admin/Desktop/Update/Zhaohu/brain1HR_cor.txt'
with open(file_path, 'r') as file_to_read:
    i = 0
    while i < 87:
        lines = file_to_read.readline().strip()  # Read a row of data
        if not lines:
            break
            pass
        file_name_list.append(lines)
        pass
        i += 1

tiles = []
for i in range(len(file_name_list)):
    line=file_name_list[i].split(',')
    line_coor=[round(float(line[0].split('(')[1]),0),round(float(line[1]),0)*(-1),round(float(line[2].split(')')[0]),0)]
    #line_coorfile_name_list[i].split(',')=np.array(line_coor,dtype='')
    coor.append(line_coor)
    tiles.append(file_name_list[i].split('.')[0].split('r')[1])
coor=np.array(coor,dtype='double')

# floor11_y9.tif; ; (29.7689974918776, 215.64832748552726, 1418.0341234614134)
plt.scatter(coor[0:87,0],coor[0:87,1],marker='.')
plt.xlim(-1000, 16000)
plt.ylim(-10000, 1000)
for i in range(87):
    plt.text(coor[i][0],coor[i][1],'floor'+tiles[i],fontsize=8)
plt.show()
print(coor)
