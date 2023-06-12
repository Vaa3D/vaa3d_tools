import glob as glob
import os

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

files=glob.glob(r"E:\result1\*")
for file in files:
    name=file.split("\\")[-1]
    if name=="CropCenter":
        continue
    imgpath=file+"\\raw\\*"
    imgs=glob.glob(imgpath)
    for img in imgs:
        cmd=v3dcmd("image_quality","image_quality",img,"output.csv","")
        os.system(cmd)