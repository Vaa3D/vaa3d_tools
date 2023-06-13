import os
import glob as glob
import time
import numpy as np
# from pylib.img_io import *

v3d=r"E:\Downloads\Vaa3D_v6.007_Windows_64bit\Vaa3D_v6.007_Windows_64bit\Vaa3D-x.exe"
def v3dcmd(plugin,func,input,output,para):
    cmd=v3d+" /x "+plugin+" /f "+func+" /i "
    if isinstance(input,list) is False:
        input=[input]
    if isinstance(output,list) is False:
        output=[output]
    if isinstance(para,list) is False:
        para=[para]

    for i in input:
        cmd+=i+" "
    if len(output)>0:
        cmd+="/o "
        for o in output:
            cmd+=o+" "
    if len(para)>0:
        cmd+="/p "
        for p in para:
            cmd+=p+" "
    return cmd


def advantra(inputf):
    plugin="Advantra"
    func="advantra_func"
    input=inputf
    output=[]
    para=['10','0.5','0.75','12.5','60','5','5','1']
    cmd=v3dcmd(plugin,func,input,output,para)
    os.system(cmd)
    return inputf+"_Advantra.swc"

def cwlab(inputf):
    plugin="CWlab_method1_version1"
    func="tracing_func"
    input=inputf
    output=[]
    para=[]
    cmd = v3dcmd(plugin, func, input, output, para)
    os.system(cmd)
    return inputf + "Cwlab_ver1.swc"

def ENTV2n(inputf):
    plugin="EnsembleNeuronTracerV2n"
    func="tracing_func"
    input=inputf
    output=[]
    para=[]
    cmd = v3dcmd(plugin, func, input, output, para)
    os.system(cmd)
    return inputf + "EnsembleNeuronTracerV2n.swc"

def ENTV2s(inputf):
    plugin="EnsembleNeuronTracerV2s"
    func="tracing_func"
    input=inputf
    output=[]
    para=[]
    cmd = v3dcmd(plugin, func, input, output, para)
    os.system(cmd)
    return inputf + "EnsembleNeuronTracerV2s.swc"

def ENT(inputf):
    plugin="ENT"
    func="tracing_func"
    input=inputf
    output=[]
    para=[]
    cmd = v3dcmd(plugin, func, input, output, para)
    os.system(cmd)
    return inputf + "ENT.swc"

def app2(inputf,outputf=""):
    plugin="vn2"
    func="app2"
    input=inputf
    output=[]
    if len(outputf)>0:
        output.append(outputf)
    para=["NULL","0","1"]
    cmd = v3dcmd(plugin, func, input, output, para)
    os.system(cmd)
    return inputf + "_app2.swc"

if __name__ == '__main__':

    # csv=[]
    # files=glob.glob(r"E:\cntdomain\*")
    # count=0
    # for file in files:
    #     # if count==1:
    #     #     break
    #     # count+=1
    #     imgname=file.split('\\')[-1]
    #     imgs=glob.glob(os.path.join(file,"*.tiff"))
    #     starttime = time.time()
    #     for img in imgs:
    #         advantra(img)
    #     endtime = time.time()
    #     csv.append([imgname,endtime-starttime])
    # csv=np.array(csv)
    # Writecsv(csv,r"E:\cntdomain\1.csv")

    csv=[]
    files=glob.glob(r"E:\cntdomain\all\*.tiff")
    for file in files:
        imgname = file.split('\\')[-1]
        starttime = time.time()
        advantra(file)
        endtime = time.time()
        csv.append([imgname, endtime - starttime])
    csv=np.array(csv)
    # Writecsv(csv,r"E:\cntdomain\2.csv")
