# -*- coding: utf-8 -*-
"""
Created on Mon Jul 27 17:58:24 2015

@author: xiaoxiaoliu
"""

# script for statci to pull out apical dendrites from swc files  for separate analysis


import pandas as pd


def read_swc(infile):
    ###n,type,x,y,z,radius,parent
     swc_df = pd.read_csv(infile, sep=" ",skiprows = 3, header =None, names =['n','type','x','y','z','radius','parent'] )
     return swc_df



def write_swc(df_out,outfile):
     ###n,type,x,y,z,radius,parent
     df_out.to_csv(outfile, index=False, header= False, sep=" ")


def extract_swc_by_id(type_id, swc_file, output_swc_file = None):
      if (output_swc_file == None):
          output_swc_file = swc_file[0:-4] +'_'+ str(type_id) +'.swc'
      swc_df = read_swc(swc_file)
      extracted = swc_df[ swc_df['type'] == type_id ]  # keep the soma
      soma = swc_df[ swc_df['type'] == 1 ]

      frames = [soma, extracted]
      result = pd.concat(frames)

      write_swc(result,output_swc_file)


data_DIR = '/Volumes/mat/xiaoxiaol/data/lims2/nr_june_25_filter_aligned/original'
df = pd.read_csv(data_DIR +'/list.csv')

for i in range(df.shape[0]):
     swc_file = df['swc_file'][i]
     output_swc_file = data_DIR +'/../apical_original/'+swc_file[0:-4]+'.swc'
     extract_swc_by_id(4, data_DIR+'/'+swc_file, output_swc_file)  #apical dendrite ==4
