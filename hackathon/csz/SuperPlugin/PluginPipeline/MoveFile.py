import glob as glob
import shutil
import os


def FindFileWithSuffix(Src,Suffix):
    FilePaths=[]
    if os.path.isdir(Src):
        if len(os.listdir(Src))>0:
            for file in os.listdir(Src):
                path=Src+"\\"+file
                if os.path.isdir(path):
                    ChildFilePath=FindFileWithSuffix(path,Suffix)
                    for cfp in ChildFilePath:
                        FilePaths.append(cfp)
                elif path.endswith(Suffix):
                    FilePaths.append(path)
    return FilePaths

def MoveFileToDist(Src,Dist):
    for f in Src:
        img=f
        swc=f+".swc"
        shutil.copy(img, Dist + "\\img\\" + img.split('\\')[-1])
        print(os.path.dirname(f))
        if os.path.exists(swc):
            shutil.copy(swc, Dist + "\\goldswc\\" + swc.split('\\')[-1])
        else:
            swcf=glob.glob(os.path.dirname(f)+'\\*.swc')[0]
            shutil.copy(swcf, Dist + "\\goldswc\\" + swc.split('\\')[-1])


if __name__ == '__main__':
    src=r"E:\gold166"
    dist=r"D:\A_pythonwork\PluginPipeline\gold"
    suffix="v3dpbd"
    V3DPBDFILES=FindFileWithSuffix(src,suffix)
    MoveFileToDist(V3DPBDFILES,dist)