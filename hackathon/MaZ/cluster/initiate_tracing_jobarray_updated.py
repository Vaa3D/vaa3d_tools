#!/share/bin/python
# Filename: initiate_tracing_updated.py

from __future__ import division
import os
import glob
i=int(os.getenv('PBS_ARRAYID'))-1  # The first index of a list is zero
input_filenames=glob.glob('/data/mat/data_processing/input_data/*.v3dpbd')
mkdir_command='mkdir /data/mat/data_processing/'+str(i)
os.system(mkdir_command)
copyfolder_command='cp -rp /data/mat/data_processing/Vaa3D_v3.200 /data/mat/data_processing/'+str(i)
os.system(copyfolder_command)
copyinputdata_command='cp '+input_filenames[i]+' /data/mat/data_processing/'+str(i)
os.system(copyinputdata_command)
working_dir='/data/mat/data_processing/'+str(i)+'/Vaa3D_v3.200'
os.chdir(working_dir)
filename_path='/data/mat/data_processing/'+str(i)+'/*.v3dpbd'
data_filenames=glob.glob(filename_path)
conversion_command='./start_vaa3d.sh -x convert_file_format -f convert_format -i '+data_filenames[0]+' -o '+data_filenames[0]+'.v3draw'
os.system(conversion_command)
filename_path='/data/mat/data_processing/'+str(i)+'/*.v3draw'
data_filenames=glob.glob(filename_path)

print('APP1')
tracing_command='timeout 3600s ./start_vaa3d.sh -x vn2 -f app1 -i '+data_filenames[0]+' -p NULL 0 40 1'
os.system(tracing_command)
    
print('APP2')
tracing_command='timeout 3600s ./start_vaa3d.sh -x vn2 -f app2 -i '+data_filenames[0]+' -p NULL 0 10 1 1 0 0 5 0 0 0'
os.system(tracing_command)

print('MOST')
tracing_command='timeout 3600s ./start_vaa3d.sh -x MOST -f MOST_trace -i '+data_filenames[0]+' -p 1 40'
os.system(tracing_command)
    
print('NEUTUBE')
tracing_command='timeout 3600s ./start_vaa3d.sh -x neuTube -f neutube_trace -i '+data_filenames[0]
os.system(tracing_command)
    
print('FARSIGHT Snake')
tracing_command='timeout 3600s ./start_vaa3d.sh -x snake -f snake_trace -i '+data_filenames[0]+' -p 1'
os.system(tracing_command)
    
print('3 from SimpleTracing')
tracing_command='timeout 3600s ./start_vaa3d.sh -x SimpleTracing -f tracing -i '+data_filenames[0]+' -o '+data_filenames[0]+'_simple.swc -p 1'
os.system(tracing_command)

tracing_command='timeout 3600s ./start_vaa3d.sh -x SimpleTracing -f ray_shooting -i '+data_filenames[0]+' -o '+data_filenames[0]+'_Rayshooting.swc'
os.system(tracing_command)
    
tracing_command='timeout 3600s ./start_vaa3d.sh -x SimpleTracing -f dfs -i '+data_filenames[0]+' -o '+data_filenames[0]+'_Rollerball.swc'
os.system(tracing_command)
    
print('TreMap')
tracing_command='timeout 3600s ./start_vaa3d.sh -x TReMap -f trace_mip -i '+data_filenames[0]+' -p 0 1 10 0 1 0 5'
os.system(tracing_command)
    
print('MST')
tracing_command='timeout 3600s ./start_vaa3d.sh -x MST_tracing -f trace_mst -i '+data_filenames[0]+' -p 1 5'
os.system(tracing_command)
    
print('NeuroGPSTree')
tracing_command='timeout 3600s ./start_vaa3d.sh -x NeuroGPSTree -f tracing_func -i '+data_filenames[0]+' -p 0.5 0.5 1 15 10 150'
os.system(tracing_command)
    
print('fastmarching_spanningtree')
tracing_command='timeout 3600s ./start_vaa3d.sh -x fastmarching_spanningtree -f tracing_func -i '+data_filenames[0]
os.system(tracing_command)
  
#print('meanshift')
#tracing_command='./start_vaa3d.sh -x BJUT_meanshift -f meanshift -i '+data_filenames[0]+' -p 1 3.0 10 0.6'
#os.system(tracing_command)
    
print('CWlab_method1_version1')
tracing_command='timeout 3600s ./start_vaa3d.sh -x CWlab_method1_version1 -f tracing_func -i '+data_filenames[0]+' -p 1'
os.system(tracing_command)
    
#print('LCM_boost')
#tracing_command='./start_vaa3d.sh -x LCM_boost -f LCM_boost -i '+data_filenames[0]+' -o '+data_filenames[0]+'_LCMboost.swc' 
#os.system(tracing_command)
    
print('NeuroStalker')
tracing_command='timeout 3600s ./start_vaa3d.sh -x NeuroStalker -f tracing_func -i '+data_filenames[0]+' -p 1 1 1 5 5 30'
os.system(tracing_command)
    
print('nctuTW')
tracing_command='timeout 3600s ./start_vaa3d.sh -x nctuTW -f tracing_func -i '+data_filenames[0]+' -p NULL'
os.system(tracing_command)
    
print('tips_GD')
tracing_command='timeout 3600s ./start_vaa3d.sh -x tips_GD -f tracing_func -i '+data_filenames[0]
os.system(tracing_command)
    
print('SimpleAxisAnalyzer')
tracing_command='timeout 3600s ./start_vaa3d.sh -x SimpleAxisAnalyzer -f medial_axis_analysis -i '+data_filenames[0]
os.system(tracing_command)
    
print('NeuronChaser')
tracing_command='timeout 3600s ./start_vaa3d.sh -x NeuronChaser -f nc_func -i '+data_filenames[0]+' -p 1 10 0.7 20 60 10 5 1 0'
os.system(tracing_command)
    
print('smartTracing')
tracing_command='timeout 3600s ./start_vaa3d.sh -x smartTrace -f smartTrace -i '+data_filenames[0]
os.system(tracing_command)
    
print('neutu_autotrace')
tracing_command='timeout 3600s ./start_vaa3d.sh -x neutu_autotrace -f tracing -i '+data_filenames[0]
os.system(tracing_command)
    
print('Advantra')
tracing_command='timeout 3600s ./start_vaa3d.sh -x Advantra -f advantra_func -i '+data_filenames[0]+' -p 10 0.5 0.7 20 60 10 5 1'
os.system(tracing_command)
    
print('RegMST')
tracing_command='timeout 3600s ./start_vaa3d.sh -x RegMST -f tracing_func -i '+data_filenames[0]+' -p ./filter_banks/oof_fb_3d_scale_1_2_3_5_size_13_sep_cpd_rank_49.txt ./filter_banks/oof_fb_3d_scale_1_2_3_5_size_13_weigths_cpd_rank_49.txt ./filter_banks/proto_filter_AC_lap_633_822_sep_cpd_rank_49.txt ./filter_banks/proto_filter_AC_lap_633_822_weigths_cpd_rank_49.txt 1 2 ./trained_models/model_S/Regressor_ac_0.cfg ./trained_models/model_S/Regressor_ac_1.cfg 21 170'
os.system(tracing_command)
    
print('EnsembleNeuronTracer')
tracing_command='timeout 3600s ./start_vaa3d.sh -x EnsembleNeuronTracerBasic -f tracing_func -i '+data_filenames[0]
os.system(tracing_command)
    
print('EnsembleNeuronTracerV2n')
tracing_command='timeout 3600s ./start_vaa3d.sh -x EnsembleNeuronTracerV2n -f tracing_func -i '+data_filenames[0]
os.system(tracing_command)

print('EnsembleNeuronTracerV2s')
tracing_command='timeout 3600s ./start_vaa3d.sh -x EnsembleNeuronTracerV2s -f tracing_func -i '+data_filenames[0]
os.system(tracing_command)

#print('3DTraceSWC')
#tracing_command='./start_vaa3d.sh -x aVaaTrace3D -f func1 -i '+data_filenames[0]+' -p 50 5 2.5'
#os.system(tracing_command)

print('Rivulet')
tracing_command='timeout 3600s ./start_vaa3d.sh -x Rivulet -f tracing_func -i '+data_filenames[0]+' -p 1 2 1 10 0 2 0.95 3 0.5 1'
os.system(tracing_command)

print('ENT')
tracing_command='timeout 3600s ./start_vaa3d.sh -x ENT -f tracing_func -i '+data_filenames[0]
os.system(tracing_command)

print('APP2_new1')
tracing_command='timeout 3600s ./start_vaa3d.sh -x vn2 -f app2 -i '+data_filenames[0]+' -o '+data_filenames[0]+'_app2new1.swc  -p NULL 0 AUTO 1 1 1 1 5 0 0 0'
os.system(tracing_command)

print('APP2_new2')
tracing_command='timeout 3600s ./start_vaa3d.sh -x vn2 -f app2 -i '+data_filenames[0]+' -o '+data_filenames[0]+'_app2new2.swc  -p NULL 0 AUTO 1 1 0 1 5 0 0 0'
os.system(tracing_command)

print('APP2_new3')
tracing_command='timeout 3600s ./start_vaa3d.sh -x vn2 -f app2 -i '+data_filenames[0]+' -o '+data_filenames[0]+'_app2new3.swc  -p NULL 0 10 1 1 1 1 5 0 0 0'
os.system(tracing_command)

print('LCM_boost_2')
tracing_command='timeout 3600s ./start_vaa3d.sh -x LCM_boost -f LCM_boost_2 -i '+data_filenames[0]+' -o '+data_filenames[0]+'_LCMboost_2.swc'
os.system(tracing_command)

#print('LCM_boost_3')
#tracing_command='./start_vaa3d.sh -x LCM_boost -f LCM_boost_3 -i '+data_filenames[0]+' -o '+data_filenames[0]+'_LCMboost_3.swc'
#os.system(tracing_command)
