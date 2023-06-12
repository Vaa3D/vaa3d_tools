#!/usr/bin/env python

#================================================================
#   Copyright (C) 2021 Yufeng Liu (Braintell, Southeast University). All rights reserved.
#   
#   Filename     : dataset.py
#   Author       : Yufeng Liu
#   Date         : 2021-04-01
#   Description  : 
#
#================================================================

import numpy as np
import pickle
import torch.utils.data as tudata
import SimpleITK as sitk
import torch
import sys
import os


from process.generic_augmentation import InstanceAugmentation


# To avoid the recursionlimit error, maybe encountered in trim_swc
sys.setrecursionlimit(30000)

class GenericDataset(tudata.Dataset):

    def __init__(self, split_file, phase='train', imgshape=(256,512,512)):
        self.data_list = self.load_data_list(split_file, phase)
        self.imgshape = imgshape
        print(f'Image shape of {phase}: {imgshape}')

        # augmentations
        self.augment = InstanceAugmentation(p=0.2, imgshape=imgshape, phase=phase)
    
    @staticmethod
    def load_data_list(split_file, phase):
        # define helper function for single soma extraction
        def extract_single_soma(dd, list_file):
            new_datas = []
            # remove multi-soma crops 
            # read simple-soma data list
            with open(list_file) as fp: 
                imglist = []
                for line in fp.readlines():
                    line = line.strip()
                    if not line: continue
                    imglist.append(line)
            imglist = set(imglist)
            for sample in dd: 
                imgfile = sample[0]
                prefix = os.path.splitext(os.path.split(imgfile)[-1])[0]
                if prefix in imglist:
                    new_datas.append(sample)
            return new_datas


        with open(split_file, 'rb') as fp:
            data_dict = pickle.load(fp)
        #return data_dict[phase]

        if phase == 'train' or phase == 'val':
            return data_dict[phase]
        elif phase == 'par':
            list_file = './data/par_set_singleSoma.list'
            dd = extract_single_soma(data_dict['par'], list_file)
                
            return dd
        elif phase == 'test':
            dd = data_dict['test']
            # filtering, for debug only!
            #new_dd = []
            #for di in dd:
            #    prefix = os.path.splitext(os.path.split(di[0])[-1])[0]
            #    if not os.path.exists(f'/media/data/lyf/SEU-ALLEN/neuronet_1741/debug_{prefix}_test_img.tiff'):
            #        new_dd.append(di)
            #dd = new_dd
            #print(len(dd))

            ##list_file = './data/img_singleSoma.list'
            #list_file = './data/additional_crops/single_soma.list'
            #dd = extract_single_soma(dd, list_file)
            return dd 
        else:
            raise ValueError
            

    def __getitem__(self, index):
        img, gt, imgfile, swcfile = self.pull_item(index)
        return img, gt, imgfile, swcfile

    def __len__(self):
        return len(self.data_list)

    def pull_item(self, index):
        imgfile, swcfile, spacing = self.data_list[index]
        # parse, image should in [c,z,y,x] format
        img = np.load(imgfile)['data']
        if img.ndim == 3:
            img = img[None]
        if swcfile is not None:
            tree = parse_swc(swcfile)
        else:
            tree = None

        # random augmentation
        img, tree, _ = self.augment(img, tree, spacing)
        if tree is not None:
            # convert swc to image
            # firstly trim_swc via deleting out-of-box points
            tree = trim_out_of_box(tree, img[0].shape, True)
            lab = swc_to_image(tree, imgshape=img[0].shape)
            # imgsave=sitk.GetImageFromArray(np.array(img[0],dtype='uint8'))
            # labsave=sitk.GetImageFromArray(np.array(lab*255,dtype='uint8'))
            # # print(img.shape)
            # sitk.WriteImage(imgsave,'exps/213/'+imgfile.split('\\')[-1]+'.tiff')
            # sitk.WriteImage(labsave, 'exps/213/'+swcfile.split('\\')[-1] + '.tiff')
            return torch.from_numpy(img.astype(np.float32)), torch.from_numpy(lab.astype(np.uint8)), imgfile, swcfile
        else:
            lab = np.random.random(img[0].shape) > 0.5
            return torch.from_numpy(img.astype(np.float32)), torch.from_numpy(lab.astype(np.uint8)), imgfile, imgfile

        
        


if __name__ == '__main__':
    split_file = '../data/task0001_17302/data_splits.pkl'
    idx = 2
    imgshape = (256,512,512)
    dataset = GenericDataset(split_file, 'train', imgshape)
    img, lab = dataset.pull_item(idx)

