import os
import scipy.stats
import numpy
import matplotlib.pylab as pl
import pandas as pd


# program path on this machine
#===================================================================
#blastneuron_DIR = "/home/xiaoxiaol/work/src/blastneuron"
blastneuron_DIR = "/Users/xiaoxiaoliu/work/src/blastneuron"
PRUNE_SHORT_BRANCH = blastneuron_DIR + "/bin/prune_short_branch"
PRE_PROCESSING = blastneuron_DIR + "/bin/pre_processing"
NEURON_RETRIEVE = blastneuron_DIR + "/bin/neuron_retrieve"
BATCH_COMPUTE = blastneuron_DIR + "/bin/batch_compute"  # compute faetures

V3D="/Users/xiaoxiaoliu/work/v3d/v3d_external/bin/vaa3d64.app/Contents/MacOS/vaa3d64"
#V3D="/local1/xiaoxiaol/work/v3d/v3d_external/bin/vaa3d"

data_DIR = '/Volumes/mat/xiaoxiaol/data/lims2/nr_june_25_filter_aligned/apical'

original_data_linker_file =  data_DIR+'/original/mylinker.ano' # will be genereated
preprocessed_data_linker_file = data_DIR+'/preprocessed/mylinker.ano'
feature_file = data_DIR + '/preprocessed/prep_features.nfb'



#===================================================================
def prune(inputswc_fn, outputswc_fn):
    cmd = 'cd '+data_DIR+'/pruned/'
    os.system(cmd)
    cmd = PRUNE_SHORT_BRANCH +  " -i "+inputswc_fn + " -o "+outputswc_fn
    os.system(cmd)
    print cmd
    return

def preprocessing(inputswc_fn, outputswc_fn):
    cmd = 'cd '+data_DIR+'/preprocessed/'
    os.system(cmd)
    cmd = PRE_PROCESSING+  " -i "+inputswc_fn + " -o "+outputswc_fn
    os.system(cmd)
    return

def neuronretrive(inputswc_fn, feature_fn, result_fn, retrieve_number, logfile):
    cmd = NEURON_RETRIEVE + " -d " + feature_fn + " -q " +inputswc_fn + " -n "+ \
          str(retrieve_number) +" -o "+result_fn+" -m 1,3" + " >" + logfile
    print cmd
    os.system(cmd)
    return

def featurecomputing(input_linker_fn, feature_fn):
    cmd = 'cd '+data_DIR+'/preprocessed/'
    os.system(cmd)
    cmd = BATCH_COMPUTE +  " -i "+input_linker_fn + " -o " + feature_fn
    os.system(cmd)
    print cmd
    return

#def genLinkerFile(swcDir, linker_file):
#    cmd = V3D + " -x  linker_file_gen  -f linker -i "+ swcDir +" -o "+ linker_file +" -p 1"
 #   print cmd
 #   os.system(cmd)
 #   return

def removeLinkerFilePath(inputLinkFile, outputLinkFile):
    with open(outputLinkFile, 'w') as out_f:
        with open (inputLinkFile,'r') as in_f:
            for inline in in_f:
                outline = 'SWCFILE=' + inline.split('/')[-1]
                out_f.write(outline)
        in_f.close()
    out_f.close()
    return


def genLinkerFileFromList(listCSVFile, linkFile):
    df = pd.read_csv(listCSVFile, sep=',',header=0)
    fns = df.orca_path
    with open(linkFile, 'w') as f:
      for i in range(len(fns)):
          line = "SWCFILE="+fns[i]+'\n'
          f.write(line)
    f.close()
    return


def pullListFromDB(outputFolder):
    #outputListCSVFile = outputFolder +'/list.csv'
    # copy data to local disk?
    return

#==================================================================================================
def main():

    #TODO:  pullListFromDB() update from lims2 to grab all neuron reconstructions into list.csv

    #genLinkerFileFromList(data_DIR+'/list.csv', original_data_linker_file)

    if  not os.path.exists(data_DIR+'/pruned'):
        os.mkdir(data_DIR+'/pruned')
    if  not os.path.exists(data_DIR+'/preprocessed'):
        os.mkdir(data_DIR+'/preprocessed')

    with open(original_data_linker_file,'r') as f:
        for line in f:
            input_swc_path =  data_DIR+'/original/'+ (line.strip()).split('=')[1] #SWCFILE=*
            swc_fn =  input_swc_path.split('/')[-1]
            print swc_fn
            pruned_swc_fn = data_DIR+'/pruned/'+ swc_fn
            prune(input_swc_path, pruned_swc_fn)
            preprocessed_swc_fn = data_DIR+'/preprocessed/'+ swc_fn
            preprocessing(pruned_swc_fn, preprocessed_swc_fn)

    #removeLinkerFilePath(original_data_linker_file, preprocessed_data_linker_file)

    ##batch computing
    featurecomputing(preprocessed_data_linker_file,feature_file)


if __name__ == "__main__":
      main()
