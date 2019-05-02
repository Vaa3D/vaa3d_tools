import os
import sys
import re

person=[]
Vaa3d=""
def outputExcel(resultFolder,distanceFolder,excelFolder):
    files=os.listdir(distanceFolder);
    print(files)
    files.sort()
    colorSet=set();
    excelMap={}

    for file in files:
        color=re.findall("\(color(.+?)\)", file)[0]
        if  color not in excelMap:
            excelMap[color]=[];
        excelMap[color].append(file)
    output = open(excelFolder + "\\disBetSwc.csv", 'w');
    for key in excelMap:
        output.write("color{},".format(key))
        # output = open(excelFolder + "\\{}.csv".format(key), 'w');
        # paths=excelMap[key];
        # paths.sort();
        matrix=[[0]*len(person) for i in range(len(person))]
        print(matrix)
        for i in range(len(person)):
            for j in range(len(person)):
                if i==j:
                    continue
                dis=re.findall("entire-structure-average \(from neuron 1 to 2\) =(.+?)\n",open(distanceFolder+"\\"+person[i]+"_(color{})".format(key)+"__"+person[j]+"_(color{}).txt".format(key)).read())[0]
                print(dis)
                matrix[i][j]=eval(dis);

        print(person)
        # output.write(' ,')
        for i in person:
            output.write("{},".format(i))
        output.write('\n')
        for i in range(len(person)):
            output.write('{},'.format(person[i]))
            for j in range(len(person)):
                output.write("{},".format(matrix[i][j]))
            output.write('\n')
        output.write("\n")
    output.close()
    #*********************************length******************************************
    output = open(excelFolder + "\\length.csv", 'w');
    output.write(" total length of each tract (voxel)\n")
    keys=list(excelMap.keys());
    keys.sort();
    output.write(" ,")
    for key in keys:
        output.write("color{},".format(key))
    output.write("\n")
    for i in range(len(person)):
        output.write("{},".format(person[i]))
        for key in keys:
            try:
                swctotalLength=totalLength(resultFolder+"\\"+person[i]+"_(color{}).swc".format(key),excelFolder+"\\log")
                output.write("{},".format(swctotalLength))
            except:
                pass;
        output.write("\n")
    output.close()



def sortSwc(inpath,outpath):
    # print("******{}".format(Vaa3d))
    commandStr = "{} " \
                 "/x sort_neuron_swc.dll " \
                 "/f sort_swc " \
                 "/i  {} " \
                 "/p 0,1 " \
                 "/o {}".format(Vaa3d,inpath, outpath)
    os.system(commandStr)

def totalLength(inpath,logpath):
    commandStr = "{} " \
                 "/x global_neuron_feature.dll " \
                 "/f compute_feature " \
                 "/i  {} "\
                "> {}".format(Vaa3d,inpath,logpath)
    a=os.system(commandStr)
    # print("**********************")
    log=open(logpath).read();
    swctotalLength=re.findall("Total Length:(.+?)\n", log)[0];
    os.remove(logpath)
    print(eval(swctotalLength))
    return eval(swctotalLength)

def disBetSwc(swc1,swc2,resultPath):
    commandStr = "{} " \
                 "/x neuron_dist.dll " \
                 "/f neuron_distance " \
                 "/i  {} {} " \
                 "/p 2 /o {}".format(Vaa3d,swc1, swc2,resultPath)
    os.system(commandStr)

def splitSwc(swcfile,resultFolder):
    file=open(swcfile)
    lines=file.readlines();
    file.close()

    swcColor={}
    for i in range(len(lines)):
        if lines[i][0] == '#':
            continue
        color = eval(lines[i].strip('\n').split(' ')[1])
        if color not in swcColor:
            swcColor[color] = []
        tmp=''
        count=0;
        for j in  lines[i].strip('\n').split(' '):
            if count<7:
                tmp=tmp+' '+j
            count = count + 1
        # print(tmp)
        tmp=tmp+'\n'
        swcColor[color].append(lines[i])

    for key in swcColor:
        output=open(resultFolder+"\\"+swcfile.split("\\")[-1].split('.')[0]+'_(color{})'.format(key)+".swc",'w');
        output.write("##n,type,x,y,z,radius,parent\n")
        # print((re.findall("\(color(.+?)\)", resultFolder+"\\"+swcfile.split("\\")[-1].split('.')[0]+'_(color{})'.format(key)+'.'+swcfile.split("\\")[-1].split('.')[1])))
        for i in range(len(swcColor[key])):
            output.write(swcColor[key][i]);
        output.close();
        sortSwc(resultFolder+"\\"+swcfile.split("\\")[-1].split('.')[0]+'_(color{})'.format(key)+".swc",resultFolder+"\\"+swcfile.split("\\")[-1].split('.')[0]+'_(color{})'.format(key)+".swc")



def readSwcFolder_CompareDifType(inputFolder,resultFolder,distanceFolder):
    #分散swc
    for file in os.listdir(inputFolder):
        if file[-3:]=='swc':
            splitSwc(inputFolder+"\\"+file,resultFolder);



    #disBetSwc
    for file1 in os.listdir(resultFolder):
        for file2 in os.listdir(resultFolder):
            if file1==file2:
                continue
            if file1[-3:]==file2[-3:]=='swc' and re.findall("\(color(.+?)\)",file1)==re.findall("\(color(.+?)\)",file2):
                # print(file1+' '+file2+' '+resultFolder+"\\distance\\"+file1.split('.')[0]+"_"+file2.split('.')[0]+'.txt')
                # resultFolder + "\\distance\\" + file1.split('.')[0] + "_" + file2.split('.')[0] +
                # print(distanceFolder + "\\")
                # print(resultFolder + "\\distance\\" )

                disBetSwc(resultFolder+"\\"+file1,resultFolder+"\\"+file2,distanceFolder+ "\\" + file1.split('.')[0] + "__" + file2.split('.')[0] +'.txt')


def main(argv):
    global Vaa3d
    Vaa3d="D:\\v3d_external\\bin\\vaa3d_msvc.exe"
    inputFolder = "C:\\Users\Anzhi\Documents\WXWork\\1688850161522981\Cache\File\\2019-05\\testdata\\group1_c_VR"
    if len(argv)>2:
        Vaa3d=argv[1];
        inputFolder=argv[2];

    print(Vaa3d)
    print(inputFolder)


    for file in os.listdir(inputFolder):
        if file[-3:]=='swc':
            if file.split('.')[0] not in person:
                person.append(file.split('.')[0])
    person.sort()

    resultFolder=inputFolder+"\\"+"result"
    distanceFolder=resultFolder+"\\distance"
    excelFolder = resultFolder + "\\excel"
    if not os.path.exists(resultFolder):
        os.mkdir(resultFolder)
    if not os.path.exists(distanceFolder):
        os.mkdir(distanceFolder)
    if not os.path.exists(excelFolder):
        os.mkdir(excelFolder)

    readSwcFolder_CompareDifType(inputFolder,resultFolder,distanceFolder)
    outputExcel(resultFolder,distanceFolder,excelFolder)


if __name__=="__main__":
    argv=sys.argv;
    print(argv)
    main(argv)