#!/share/bin/python
# Filename: calc_dist_jobarray.py

from __future__ import division
import os
import glob
i=int(os.getenv('PBS_ARRAYID'))-1  # The first index of a list is zero
input_filenames=glob.glob('/data/mat/data_processing/input_data/*.v3dpbd')
swc_filenames=glob.glob('/data/mat/data_processing/'+str(i)+'/*.swc')
head, tail = os.path.split(input_filenames[i])
standard_swc_filename=glob.glob('/data/mat/data_processing/standard_swc/'+tail+'*stamp*.swc')
os.system('cp '+standard_swc_filename[0]+' /data/mat/data_processing/'+str(i))
for j in range(0,len(swc_filenames)):
    os.system('/data/mat/data_processing/Vaa3D_v3.200/start_vaa3d.sh -x neuron_distance -f neuron_distance -i '+standard_swc_filename[0]+' '+swc_filenames[j]+' -o '+swc_filenames[j]+'_dist')
