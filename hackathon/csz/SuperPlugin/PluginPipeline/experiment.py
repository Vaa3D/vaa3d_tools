import os
import glob as glob

AList=["Advantra","app2new3","EnsembleNeuronTracerV2n","MOST","NeuroGPSTree","TreMap","MST_Tracing","smartTracing","ENT"]

def neuron_distance(input1,input2,output):
    cmd = "E:\Downloads\Vaa3D_v6.007_Windows_64bit\Vaa3D_v6.007_Windows_64bit\Vaa3D-x.exe /x neuron_dist /f neuron_distance /i " + input1+" "+input2+" /p 5 " + " /o " + output
    print(cmd)
    os.system(cmd)

# outputpath=r"E:\contrastres\contrast"
# spresult=r"E:\contrastres\final.swc"
#
# swcfiles=glob.glob(r"E:\contrastres\sorted\*.swc")
#
# for swcfile in swcfiles:
#     name=swcfile.split('\\')[-1].split('.swc')[0]
#     if name=="140921c16.tif.v3dpbd.swc":
#         continue
#     gold=r"E:\contrastres\swc\140921c16.tif.v3dpbd.swc"
#     neuron_distance(gold,swcfile,outputpath+"\\gold_"+name+".csv")


def contrast():
    dirfiles = glob.glob(r"E:\csz\gold166\20161101_reconstruction_for_gold166_rhea_5571\*\*")

    sortpath=r"E:\Cropswc\noalexnetcontrast"
    # print(dirfiles)
    for dirf in dirfiles:
        dirname = dirf.split('\\')[-1]

        if dirname.startswith("14") and os.path.isdir(dirf):
            # print(dirname)
            files = glob.glob(os.path.join(dirf, "*.swc"))
            # print(files)
            flag=False
            for file in files:
                name = file.split('\\')[-2]

                spfile="E:\\Cropswc\\noalexnet"+'\\'+name+"final.swc"
                if os.path.exists(os.path.join(sortpath, name)) is False:
                    os.mkdir(os.path.join(sortpath, name))
                # if os.path.exists(os.path.join(sortpath, name) + "\\" + name + "_sp_gold.csv"):
                #     flag = True
                # if flag==False:
                neuron_distance(spfile,"E:\Cropswc\gold"+'\\'+name+".tif.v3dpbd.swc",os.path.join(sortpath, name)+"\\"+name+"_sp_gold.csv")
                break
                # #     flag=True
                # if name.startswith("14") is False:
                #     continue
                # method = file.split("\\")[-1].split(".swc")[0].split(".v3dpbd_")[-1]
                # # print(method)
                # if method in AList:
                #     temppath = os.path.join(sortpath, name)
                #     if os.path.exists(temppath) is False:
                #         os.mkdir(temppath)
                #     path = os.path.join(temppath, name+"_"+method+"_gold.csv")
                #     if os.path.exists(path):
                #         continue
                #     print(spfile, file,path)
                #
                #     neuron_distance(file,"E:\Cropswc\gold"+'\\'+name+".tif.v3dpbd.swc", path)

contrast()