#!/usr/bin/env python
# coding: utf-8

# In[25]:

from __future__ import print_function
import numpy as np
import pandas as pd
from random import shuffle
#from skimage.filters import threshold_adaptive
from math import ceil, floor
#import cv2 as cv

import os
import shutil

import matplotlib
import matplotlib.pyplot as plt

# plot nrrd
import SimpleITK as sitk

#get_ipython().run_line_magic('matplotlib', 'inline')


# In[2]:


import sys
import keras
from keras.datasets import cifar10
from keras.preprocessing.image import ImageDataGenerator
from keras.models import Sequential
from keras.layers import Dense, Dropout, Activation, Flatten
from keras.layers import Conv3D, MaxPooling3D
import os

from keras import backend as K
K.tensorflow_backend._get_available_gpus()

from keras.utils import plot_model


# In[3]:


axis_names = ["Z", "Y", "X"]
axis_name_to_int = dict(zip(axis_names, [0,1,2]))
def mip_plot(x, axis='Z', ax=None, cmap='viridis', do_thresholding=False, block_size = 45, offset = 20):
    
    # Dimension selection
    if x.ndim >= 4:
        x = x[:,:,:,0]
    
    # MIP and thresholding
    axis_int = axis_name_to_int[axis]
    mip_array = np.max(x, axis=axis_int)
    #if do_thresholding:
        #binary_adaptive = threshold_adaptive(mip_array, method='gaussian', block_size=block_size, offset=offset)
        #mip_array = mip_array * binary_adaptive

    # Plot
    if ax is None:
        fig, ax = plt.subplots(1,1,figsize=(5,5))
    ax.imshow(mip_array, cmap=cmap)
    axis_names_plot = [j for i,j in enumerate(axis_names) if i!=axis_int]
    ax.set_xlabel(axis_names_plot[1])
    ax.set_ylabel(axis_names_plot[0])
    return ax


# In[4]:


def show_samples(nrrd_path, nrrd_files, disp_n=50):
    # Show some examples
    sample_files = [j for i,j in enumerate(nrrd_files) if i<disp_n]
    ct = len(sample_files)

    # 2. Prepare canvas
    n_per_row = 10
    n_per_col = ceil(ct / n_per_row)

    singlewidth = 4
    singleheight = 4

    figsize = (singlewidth * n_per_row, singleheight * n_per_col)

    fig, axes = plt.subplots(n_per_col, n_per_row, figsize=figsize)
    axes = axes.reshape(-1,)

    do_thresholding = False

    # 3. Plot
    for i, sample_file in enumerate(sample_files):
        ax = axes[i]
        img = sitk.ReadImage(nrrd_path + sample_file)
        img_array = np.array(sitk.GetArrayFromImage(img), dtype='int32')
        _ = mip_plot(img_array, ax=ax, do_thresholding=False)
        ax.axis("off")
        ax.text(10, 10, sample_file.replace(".nrrd", ""), fontdict={'size':14, 'color':'white'})

    plt.subplots_adjust(wspace=0.01, hspace=0.01)
    return

def get_sample_list(nrrd_path):

    # 1. Read image files
    ct = 0
    nrrd_files = []
    for file in sorted(os.listdir(nrrd_path)):
        if file.endswith("nrrd"):
            ct += 1
            nrrd_files.append(file)
    shuffle(nrrd_files)
    print("# of tips: %d" % ct)
    return (nrrd_files,ct)


# In[35]:


positive_path = "/home/braincenter4/Desktop/pos_released/"
#negative_path_over = "/home/braincenter4/Desktop/over1_after_thre/"
#negative_path_under = "/home/braincenter4/Desktop/under1_after_thre/"

positive_nrrd,num_input = get_sample_list(sys.argv[1])
#print("===========11111=============="+sys.argv[1])
#print("============================",num_input)
# print("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n")
print(positive_nrrd)
#negative_nrrd_over= get_sample_list(negative_path_over)
#negative_nrrd_under = get_sample_list(negative_path_under)

#show_samples(sys.argv[1], positive_nrrd)
#show_samples(negative_path_over, negative_nrrd_over)
#show_samples(negative_path_under, negative_nrrd_under)


# In[36]:


X = []
predi = num_input

metadata = pd.DataFrame({'path':([sys.argv[1]]*predi),
                         'nrrd':(positive_nrrd[:predi])
                        })
# Shuffle
lab = np.arange(len(metadata))
np.random.shuffle(lab)
metadata = metadata.iloc[lab, :]
#print(len(metadata))
# Load samples
for i in range(len(metadata)):
    print("------------------:"+metadata.path.iloc[i] + metadata.nrrd.iloc[i])
    img = sitk.ReadImage(metadata.path.iloc[i] + metadata.nrrd.iloc[i])
    img_array = np.array(sitk.GetArrayFromImage(img), dtype='int32')
    if img_array.shape[0]!=50:
        img_array=img_array[:50,:,:]
    if img_array.shape[1]!=50:
        img_array=img_array[:,0:50,:]
    if img_array.shape[2]!=50:
        img_array=img_array[:,:,0:50]
    img_array_reshape = img_array.reshape(img_array.shape[0], img_array.shape[1], img_array.shape[2], 1)
    X.append(img_array_reshape)
    if ((i+1) % 100)==0:
        print(len(X))
#print(img_array_reshape)
# Deal with dimensions
#print(X)
X = np.array(X)

X = X.astype('float32')
X /= 255 
# In[37]:


model1=keras.models.load_model('/home/braincenter5/Desktop/ML/80x40000_50x50x50_nore_CNN_aa_axon.model')
prediction=model1.predict(X)
#ohl=keras.utils.to_categorical([1,1],2)


# In[38]:


print(prediction)
print(len(prediction))
result=np.eye(2,dtype=int)[np.argmax(prediction,axis=1)]
print(result)
#metadata.dtypes
output_frame = pd.DataFrame(data={'filename':positive_nrrd, 'prediction':prediction[:, 0], 'result':result[:, 0]}).set_index('filename').sort_values('prediction', ascending=False)
output_frame['result'].replace({0:'Yes', 1:'No'}, inplace=True)
#print(output_frame)
output_frame.to_csv(os.path.join(sys.argv[1], 'prediction&result_axon.csv'), sep=' ')

# In[39]:


import shutil
out = []
metadata=metadata.astype(str)  #change dataframe(all elements) to str
cp_save_dir="/home/braincenter4/Desktop/17302_prediction1/"
files=os.listdir(sys.argv[1])
for i in range(len(prediction)):
    #print(i)
    if result[i][0]==1:
        print('Predicting false number:',i)
        out.append(i)
        print(metadata.path.iloc[i] + metadata.nrrd.iloc[i])
        #shutil.copy(metadata.path.iloc[i] + metadata.nrrd.iloc[i],sys.argv[2])
        if len(metadata.nrrd.iloc[i].split('_'))>10:
            str1=metadata.nrrd.iloc[i].split('_')[0:15]
        else:
            str1=metadata.nrrd.iloc[i].split('_')[0:6]
        str2="_"
        str3=(str2.join( str1 )) 
        #shutil.copy(metadata.path.iloc[i] + metadata.nrrd.iloc[i],sys.argv[2])
        shutil.move(metadata.path.iloc[i] + str3 + "_croped.tif",sys.argv[2])
        shutil.move(metadata.path.iloc[i] + str3 + "_croped.nrrd",sys.argv[2])
        shutil.move(metadata.path.iloc[i] + str3 + "_croped.eswc",sys.argv[2])

        #out.append(metadata.path.iloc[i] + metadata.nrrd.iloc[i])
        #for file in files:
        #    if metadata.nrrd.iloc[i] in file:
        #        print('Found it!',file)        


# In[40]:
print(len(out))
exit()




