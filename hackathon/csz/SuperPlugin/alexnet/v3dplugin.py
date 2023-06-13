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

def smartTracing(input):
    cmd=v3dcmd("smartTrace","smartTrace",input,"","1")
    os.system(cmd)

def EnsembleNeuronTracerBasic(input):
    cmd = v3dcmd("EnsembleNeuronTracerBasic", "tracing_func", input, "", "")
    os.system(cmd)

def meanshift(input):
    cmd = v3dcmd("meanshift", "meanshift", input, "", "")
    os.system(cmd)

def EnsembleNeuronTracerV2s(input):
    cmd = v3dcmd("EnsembleNeuronTracerV2s", "tracing_func", input, "", "")
    os.system(cmd)

def getResult(method,input,output="",para=""):
    if method==1:
        smartTracing(input)
    elif method==2:
        EnsembleNeuronTracerBasic(input)
    elif method==3:
        meanshift(input)
    elif method==4:
        EnsembleNeuronTracerV2s(input)