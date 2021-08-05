#   Copyright (C) 2020 Shuxia Guo (Braintell, Southeast University). All rights reserved.
#   
#   Filename     : im_quality_check.py
#   Author       : Shuxia Guo
#   Date         : 2020-12-03
#   Description  : This package calculates different metrics to benchmark image quality 
#                  from the perspectives of signal-background contrast, dynamic range of 
#                  background/signal, for both within- and between-images. 


import os
import fnmatch
import numpy as np
from skimage import io
import pandas as pd
from scipy.stats import entropy
from sklearn.decomposition import NMF

def img_rescale(im):
    im = (im-np.min(im))/(np.max(im)-np.min(im))
    
    return im

def find_matching_files(directory, file_pattern):
    # directory could be a relative path, so transform it into an absolute path
    directory = os.path.abspath(directory)

    directory_to_matched_files = []
    for root, _, files in os.walk(directory):
        for file in files:
            if fnmatch.fnmatch(file.lower(), file_pattern):
                directory_to_matched_files.append(os.path.join(root, file))

    return directory_to_matched_files

def entropy1(labels, base=None):
    labels = np.uint8(img_rescale(labels)*255)
    labels = labels[labels<np.percentile(labels, 99)]
    value, counts = np.unique(labels, return_counts=True)
    return entropy(counts, base=base)

def uniformity(img):
    img = np.uint8(img_rescale(img)*255)
    upper = np.uint8(np.max((2, np.percentile(img, 99))))
    hist, _ = np.histogram(np.uint8(img), bins=range(1, upper+1))
    hist = hist/np.sum(hist)
    return np.sum(hist**2)


def cal_metric(im_file, im_dim, z_step=10):
    
    fea_keys = ('per', 'per_bg', 'per_fg', 'entropy_bg', 'uniformity_bg', \
                'median_bg', 'std_bg', 'median_fg', 'std_fg', 'files')


    n_sep = int(im_dim[0]/z_step)
    pers = np.arange(0, 101, 5)
    
    f1 = open(im_file,'rb')   # only opens the file for reading
    im3d = np.fromfile(f1, dtype=np.uint8)
    n_cut = len(im3d) - np.cumprod(im_dim, axis=0)[2]
    im3d = im3d[n_cut:].reshape(im_dim[0], im_dim[1], im_dim[2])
        
    im3d = np.uint8((im3d-np.min(im3d))/(np.max(im3d)-np.min(im3d))*255)
    im3d1 = im3d.reshape(im_dim[0], im_dim[1]*im_dim[2])
    mat_nmf = []
    for j in range(n_sep):
        mat_nmf.append(np.median(im3d1[j*z_step:(j+1)*z_step, :], axis=0))
    mat_nmf = np.stack(mat_nmf, axis=0)    
            
    nmf = NMF(n_components=3)
    nmf.fit(mat_nmf)
    ix_comp = np.argsort(np.max(nmf.components_, axis=1))
        
    r_nmf = nmf.transform(im3d1)
    im_bg = np.outer(r_nmf[:, ix_comp[0]], nmf.components_[ix_comp[0],:])
    im_bg = im_bg.reshape(im_dim[0], im_dim[1], im_dim[2])
    im_fg = im3d-im_bg
    im_fg = np.uint8(img_rescale(im_fg)*255)
        
    feas = {fea_keys[0]: np.percentile(im3d, pers),   \
            fea_keys[1]: np.percentile(im_bg, pers),  \
            fea_keys[2]: np.percentile(im_fg, pers),  \
            fea_keys[3]: entropy1(im_bg),   \
            fea_keys[4]: uniformity(im_bg), \
            fea_keys[5]: np.median(im_bg[im_bg<np.percentile(im_bg, 99)]),\
            fea_keys[6]: np.std(im_bg[im_bg<np.percentile(im_bg, 99)]),\
            fea_keys[7]: np.median(im_fg[im_fg>np.percentile(im_fg, 90)]),\
            fea_keys[8]: np.std(im_fg[im_fg>np.percentile(im_fg, 90)]),\
            fea_keys[9]: f
            }
        
    im_bg = np.max(im_bg, axis=0)
    im_bg = np.uint8(img_rescale(im_bg)*255)
    io.imsave(im_file+ '_NMF_bg.tif', im_bg, check_contrast=False)
    im_fg = np.max(im_fg, axis=0)
    im_fg = np.uint8(img_rescale(im_fg)*255)
    io.imsave(im_file+ '_NMF_fg.tif', im_fg, check_contrast=False)
    return feas


#### example usage      
im_dim = (256, 512, 512)
cdir = os.path.dirname(os.getcwd())
fea_keys = ('per', 'per_bg', 'per_fg', 'entropy_bg', 'uniformity_bg', \
            'median_bg', 'std_bg', 'median_fg', 'std_fg', 'files')
z_step = 10
pers = np.arange(0, 101, 5)
    
features = pd.DataFrame(columns=fea_keys)
features_pre = pd.DataFrame(columns=fea_keys)

root = "./test_images/"
files = find_matching_files(root, '*.v3draw')
for f in files:        
    f2 = f.replace('.v3draw', '_pre.raw')
    
    ## metrics of raw image blocks        
    feas = cal_metric(f, im_dim)
    features = features.append(feas, ignore_index=True)
    
    ## metrics of enhanced image blocks
    feas = cal_metric(f2, im_dim)
    features_pre = features_pre.append(feas, ignore_index=True)

##### save results to files    
features.to_pickle(os.path.join(root, "features.npy"))   
features_pre.to_pickle(os.path.join(root, "features_pre.npy"))   