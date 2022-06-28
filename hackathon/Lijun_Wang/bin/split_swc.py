# This Python file uses the following encoding: utf-8
# if __name__ == "__main__":
#     pass
import os


def split_swc(*pArgs):
    datapath = pArgs[0]
    path_list = os.listdir(datapath)  # os.listdir(file)会历遍文件夹内的文件并返回一个列表
    # f = open("E://shuju//F5_json_add.swc", "r")
    f = open(pArgs[1], "r")
    lines = f.readlines()[3:]  # 读取全部内容
    for i in path_list:
        file = open(pArgs[2] + "//" + i + '.swc', 'w')
        a = i.split('-')
        numx = float(a[0])
        numy = float(a[1])
        numz = float(a[2].split('.')[0])
        for line in lines:
            temp = line.split(' ')
            posx = float(temp[2])
            posy = float(temp[3])
            posz = float(temp[4]) - 300
            if (posx >= numx and posx < (numx + 128)):
                if (posy >= numy and posy < (numy + 128)):
                    if (posz >= numz and posz < (numz + 128)):
                        rex = posx - numx
                        rey = posy - numy
                        rez = posz - numz
                        if(rex == 0):
                            rex = 0.5
                        if(rey == 0):
                            rey = 0.5
                        if(rez == 0):
                            rez = 0.5
                        stra = str(format(rex, '.1f'))
                        strb = str(format(rey, '.1f'))
                        strc = str(format(rez, '.1f'))
                        tempA = temp[0] + ' ' + temp[1] + ' ' + stra + ' '
                        tempB = strb + ' ' + strc + ' '
                        tempC = temp[5] + ' ' + temp[6]
                        file.write(tempA + tempB + tempC)
                        print(line)
