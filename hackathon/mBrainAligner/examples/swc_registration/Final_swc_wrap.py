#!/usr/bin/env python
# coding: utf-8

def flip_y(path_old, path_new, size_y):
    path_old = path_old + '/'
    path_new = path_new + '/'

    old_files = os.listdir(path_old)
    for name in old_files:
        n_skip = 0
        with open(path_old + name, "r") as f:
            for line in f.readlines():
                line = line.strip()
                if line.startswith("#"):
                    n_skip += 1
                else:
                    break
        names = ["##n", "type", "x", "y", "z", "r", "parent"]
        try:
            swc = pd.read_csv(path_old + name, index_col=0, skiprows=n_skip, sep=" ",
                              usecols=[0, 1, 2, 3, 4, 5, 6],
                              names=names
                              )
            swc['y'] = size_y - swc['y']
        except:
            swc = pd.read_csv(path_old + name, index_col=0, skiprows=n_skip, sep=",",
                              usecols=[0, 1, 2, 3, 4, 5, 6],
                              names=names
                              )
            swc['y'] = size_y - swc['y']
        swc.to_csv(path_new + name, sep=" ")


import pandas as pd
import subprocess
import shutil
import numpy as np
import os
import multiprocessing
from multiprocessing import Pool
from functools import partial
import random
import ast
from scipy.spatial.distance import pdist, squareform
import matplotlib as mpl
import matplotlib.pyplot as plt
import multiprocessing
import time
import sys

if len(sys.argv) == 1:
    path = './'
else:
    path = sys.argv[1]

dest = path + '\\dest'
if os.path.exists(dest):
    shutil.rmtree(dest)
    os.mkdir(dest)
else:
    os.mkdir(dest)

def single_brain_log(brain_regis, path, input_brainID):
    exist_ccfbrain = os.path.exists(os.path.join(path, 'average_template_25_u8_xpad.v3draw'))
    if not exist_ccfbrain:
        print('Make sure the CCF standard brain template exists.')
    path_ccfbrain = os.path.join(path, 'average_template_25_u8_xpad.v3draw')

    ##
    exist_exe_affine = os.path.exists(os.path.join(os.path.join(path, 'second_affine'), 'main_warp_from_affine.exe'))
    if not exist_exe_affine:
        print('Make sure the .exe to perform affinement exists')
    path_exe_affine = os.path.join(os.path.join(path, 'second_affine'), 'main_warp_from_affine.exe')

    exist_manaulM = os.path.exists(os.path.join(os.path.join(path, 'second_affine'), 'Manual_marker'))
    if not exist_manaulM:
        print('Make sure the folder containing all manually-labelled marker files exists.')
    path_manaulM = os.path.join(os.path.join(path, 'second_affine'), 'Manual_marker')

    exist_ccfM = os.path.exists(os.path.join(os.path.join(path, 'second_affine'), 'CCF_marker'))
    if not exist_ccfM:
        print('Make sure the folder containing marker inside CCF exists.')
    path_ccfM = os.path.join(os.path.join(path, 'second_affine'), 'CCF_marker')

    exist_stripmove = os.path.exists(os.path.join(os.path.join(path, 'second_affine'), 'stripMove'))
    if not exist_stripmove:
        print('Make sure the folder containing CCF brain of removing strips exists.')
    path_stripm = os.path.join(os.path.join(path, 'second_affine'), 'stripMove')

    ##
    exist_exe_warp = os.path.exists(os.path.join(os.path.join(path, 'third_warp_swc'), 'main_warp_from_df.exe'))
    if not exist_exe_warp:
        print('Make sure the .exe to perform warp exists')
    path_exe_warp = os.path.join(os.path.join(path, 'third_warp_swc'), 'main_warp_from_df.exe')

    exist_afBrain = os.path.exists(os.path.join(os.path.join(path, 'third_warp_swc'), 'affined_brain'))
    if not exist_afBrain:
        print('Make sure the folder containing affined brains exists.')
    path_norm_brain = os.path.join(os.path.join(path, 'third_warp_swc'), 'affined_norm_brain')

    exist_auto_brain_m = os.path.exists(os.path.join(os.path.join(path, 'third_warp_swc'), 'brain_auto_marker'))
    if not exist_auto_brain_m:
        print('Make sure the folder containing auto generated marker for all brains exists.')
    path_autoBrain_m = os.path.join(os.path.join(path, 'third_warp_swc'), 'brain_auto_marker')

    exist_auto_ccf_m = os.path.exists(os.path.join(os.path.join(path, 'third_warp_swc'), 'CCF_auto_marker'))
    if not exist_auto_ccf_m:
        print('Make sure the folder containing auto generated marker for CCF brains exists.')
    path_autoCCF_m = os.path.join(os.path.join(path, 'third_warp_swc'), 'CCF_auto_marker')

    print(" Process brain %s" % (input_brainID))
    folder_ori = os.path.join(os.path.join(path, input_brainID), 'ori')
    exist_ori = os.path.exists(folder_ori)
    if not exist_ori:
        os.makedirs(folder_ori)
    for iter_check in os.listdir(os.path.join(path, input_brainID)):
        if iter_check.endswith(("swc", "SWC")):
            shutil.move(os.path.join(os.path.join(path, input_brainID), iter_check), folder_ori)
            print('Move ' + str(iter_check) + ' to folder ori')
    input_brainID = input_brainID
    folder_affine = os.path.join(os.path.join(path, input_brainID), 'affine')
    exist_affine = os.path.exists(folder_affine)
    if not exist_affine:
        print('Creat new folder' + str(folder_affine))
        os.makedirs(folder_affine)
    folder_resample = os.path.join(os.path.join(path, input_brainID), 'resample')
    exist_resample = os.path.exists(folder_resample)
    if not exist_resample:
        print('Creat new folder ' + str(folder_resample))
        os.makedirs(folder_resample)

    folder_stps = os.path.join(os.path.join(path, input_brainID), 'stps')
    exist_stps = os.path.exists(folder_stps)
    if not exist_stps:
        print('Creat new folder ' + str(folder_stps))
        os.makedirs(folder_stps)

    print('\n-----DOWNSAMPLE-------')
    # specific downsample size for the input_brainID
    x_ds_size = brain_regis.loc[int(input_brainID), 'x_downsample']
    y_ds_size = brain_regis.loc[int(input_brainID), 'y_downsample']
    z_ds_size = brain_regis.loc[int(input_brainID), 'z_downsample']

    for iter_swc in os.listdir(folder_ori):
        if not iter_swc.endswith(("swc", "SWC")):
            continue
        (neuron_info, extension) = os.path.splitext(iter_swc)
        print('Neuron information: ' + str(neuron_info))
        path_old = os.path.join(folder_ori, iter_swc)

        n_skip = 0
        with open(path_old, "r") as f:
            for line in f.readlines():
                line = line.strip()
                if line.startswith("#"):
                    n_skip += 1
                else:
                    break
        names = ["##n", "type", "x", "y", "z", "r", "parent"]
        try:
            swc = pd.read_csv(path_old, index_col=0, skiprows=n_skip, sep=" ",
                              usecols=[0, 1, 2, 3, 4, 5, 6],
                              names=names
                              )
        except:
            swc = pd.read_csv(path_old, index_col=0, skiprows=n_skip, sep=",",
                              usecols=[0, 1, 2, 3, 4, 5, 6],
                              names=names
                              )
        # swc.loc[swc.type == nan, 'type'] = 0
        swc.loc[:, 'x'] = swc.loc[:, 'x'] / round(x_ds_size, 1)
        swc.loc[:, 'y'] = swc.loc[:, 'y'] / round(y_ds_size, 1)
        swc.loc[:, 'z'] = swc.loc[:, 'z'] / round(z_ds_size, 1)

        swc = swc.round({'x': 4, 'y': 4, 'z': 4})


        # assert (max(swc.loc[:, 'x'])) < brain_regis.loc[int(input_brainID), 'x_after'], " x has exceeds the boundary"
        # assert (max(swc.loc[:, 'y'])) < brain_regis.loc[int(input_brainID), 'y_after'], " y has exceeds the boundary"
        # assert (max(swc.loc[:, 'z'])) < brain_regis.loc[int(input_brainID), 'z_after'], " z has exceeds the boundary"
        path_new = os.path.join(folder_resample, neuron_info) + '.' + 'x' + str(int(round(x_ds_size, 0))) + 'y' + str(
            int(round(y_ds_size, 0))) + 'z' + str(int(round(z_ds_size, 0)))
        # if a != "":
        #     path_new = path_new + "_" + a
        # if b != "":
        #     path_new = path_new + "_" + b
        # if c != "":
        #     path_new = path_new + "_" + c
        path_new = path_new + str(extension)
        print(path_new)
        swc.to_csv(path_new, sep=" ")
        print('Saving current file to ' + str(path_new))

    flip_brain_list = ['17788', '18047', '18453', '18868', '18452']
    print('input_brainID ', input_brainID)
    if input_brainID in flip_brain_list:
        print('input_brainID in flip_brain_list')
        folder_flip = os.path.join(os.path.join(path, input_brainID), 'flip')
        exist_flip = os.path.exists(folder_flip)
        if not exist_flip:
            print('Creat new folder' + str(folder_flip))
            os.makedirs(folder_flip)
        flip_y(folder_resample, folder_flip, brain_regis.loc[int(input_brainID), 'y_after'])
        folder_resample = folder_flip
        print('changed the resample folder to flip folder')

    print('\n-----Affinement-------')
    print('Obtaining three files for brain ' + str(input_brainID) + ' for affinement:')
    for iter_s in os.listdir(path_stripm):
        if iter_s.startswith(input_brainID):
            path_rm_strip = os.path.join(path_stripm, iter_s)
            print('\tObtained .v3draw of strips-removed version for brain ' + str(input_brainID))
    # Record manually-labeled file for specific brain
    # Record size
    for iter_m in os.listdir(path_manaulM):
        if iter_m.startswith(input_brainID):
            path_manual_m = os.path.join(path_manaulM, iter_m)
            print('\tObtained manually labelled marker file for brain ' + str(input_brainID))
            (brain_info, extension) = os.path.splitext(iter_m)
            affine_size = (brain_info.split('_')[-1])

    # Record standard-labeled file for specific brain
    for iter_c in os.listdir(path_ccfM):
        if iter_c.startswith(input_brainID):
            path_ccf_m = os.path.join(path_ccfM, iter_c)
            print('\tObtained ccf marker file for brain ' + str(input_brainID))

    if os.path.exists(path_exe_affine) != 1:
        print('Make sure the .exe to perform affinement exists')
    if os.path.exists(path_ccfbrain) != 1:
        print('Make sure the CCF standard brain template exists.')
    if os.path.exists(path_rm_strip) != 1:
        print('Make sure the folder containing CCF brain of removing strips exists.')
    if os.path.exists(path_ccf_m) != 1:
        print('Make sure the folder containing marker inside CCF exists.')
    if os.path.exists(path_manual_m) != 1:
        print('Make sure the folder containing all manually-labelled marker files exists.')
    if os.path.exists(path_manual_m) != 1:
        print('Make sure the folder containing all manually-labelled marker files exists.')
    assert os.path.exists(path_exe_affine) == 1 and os.path.exists(path_ccfbrain) == 1 and os.path.exists(
        path_rm_strip) == 1 and os.path.exists(path_ccf_m) == 1 and os.path.exists(
        path_manual_m) == 1 and os.path.exists(path_manual_m) == 1, "Check the corresponding document"
    path_op_affine = os.path.join(folder_affine, '%%~ni_affine.swc')
    with open(os.path.join(os.path.join(path, input_brainID), 'affine_swc.bat'), 'w') as OPATH:
        OPATH.writelines(['\n',
                          'set input_path=' + str(folder_resample),
                          '\n',
                          'for /r %input_path% %%i in (*.swc) do (',
                          '\n',
                          str(path_exe_affine) + ' -t ' + str(path_ccfbrain) + ' -s ' + str(path_rm_strip) +
                          ' -T ' + str(path_ccf_m) + ' -S ' + str(path_manual_m) + '  -w %%i -o ' + str(path_op_affine),
                          '\n',
                          '\n',
                          ')',
                          '\n'])
    filepath_Aff = os.path.join(os.path.join(path, input_brainID), 'affine_swc.bat')
    print('Run code at ' + str(filepath_Aff))
    os.system(filepath_Aff)
    assert len(os.listdir(folder_affine)) == len(
        os.listdir(folder_resample)), "Note that affined swc is not equal to resampled swc!"
    print('Have finished affinement')

    print('-----WRAP-----')
    subtime = time.time()
    print('Obtaining four files for brain ' + str(input_brainID) + ' for warp:')
    for iter_d in os.listdir(path_norm_brain):
        if iter_d.startswith(input_brainID):
            path_af_norm_brainID = os.path.join(path_norm_brain, iter_d)
            print('\tObtained affined version for brain ' + str(input_brainID))
    for iter_cm in os.listdir(path_autoCCF_m):
        if iter_cm.startswith(input_brainID):
            path_ccfAT_m = os.path.join(path_autoCCF_m, iter_cm)
            print('\tObtained auto integrated marker for CCF brain ')
    for iter_bm in os.listdir(path_autoBrain_m):
        if iter_bm.startswith(input_brainID):
            path_brainAT_m = os.path.join(path_autoBrain_m, iter_bm)
            print('\tObtained auto integrated marker for brain ' + str(input_brainID))

    path_op_warp = os.path.join(folder_stps, '%%~ni_stps.swc')
    # path_op_v3draw = os.path.join(os.path.join(path, input_brainID), 'stps')
    # directory = 'C:/'
    with open(os.path.join(os.path.join(path, input_brainID), 'warp_swc.bat'), 'w') as OPATH_2:
        OPATH_2.writelines(['\n',
                            'set input_path=' + str(folder_affine),
                            '\n',
                            'for /r %input_path% %%i in (*.swc) do (',
                            '\n',
                            str(path_exe_warp) + ' -t ' + str(path_ccfbrain) + ' -s ' + str(path_af_norm_brainID) +
                            ' -T ' + str(path_ccfAT_m) + ' -S ' + str(path_brainAT_m) +
                            ' -w %%i  -o ' + str(path_op_warp) +
                            " -f 1",  # 1: froward; 0: reverse
                            '\n',
                            ')',
                            '\n'])
    filepath_Warp = os.path.join(os.path.join(path, input_brainID), 'warp_swc.bat')
    print('Run code at ' + str(filepath_Warp))
    os.system(filepath_Warp)
    print(" Process brain %s Finished." % (input_brainID))
    sub_elapsed = time.time() - subtime
    print(" Time used for warpping brain " + str(input_brainID) + ' is %.2f ' % sub_elapsed)
    print('Have finished Warp')


if __name__ == '__main__':
    start = time.time()
    exist_table = os.path.exists(os.path.join(path, 'brain_registration.xlsx'))
    if not exist_table:
        print('Make sure the table containing brain downsample information exists.')
    brain_regis = pd.read_excel(os.path.join(path, 'brain_registration.xlsx'), index_col=[0], skiprows=[0],
                                usecols=[0, 1, 2, 3, 4, 5, 6],
                                names=['ID', 'y_initial', 'x_initial', 'z_initial', 'y_after', 'x_after', 'z_after'])
    brain_regis.loc[:, 'x_downsample'] = brain_regis.loc[:, 'x_initial'] / brain_regis.loc[:, 'x_after']
    brain_regis.loc[:, 'y_downsample'] = brain_regis.loc[:, 'y_initial'] / brain_regis.loc[:, 'y_after']
    brain_regis.loc[:, 'z_downsample'] = brain_regis.loc[:, 'z_initial'] / brain_regis.loc[:, 'z_after']
    # brain_regis.drop_duplicates(keep='first', inplace=True)
    brain_regis.fillna(value='NA', inplace=True)
    brain_regis = brain_regis.loc[~brain_regis.index.duplicated(keep='first')]
    brain_regis.loc[:, 'x_initial'] = brain_regis.loc[:, 'x_initial'].astype(float)
    brain_regis.loc[:, 'y_initial'] = brain_regis.loc[:, 'y_initial'].astype(float)
    brain_regis.loc[:, 'z_initial'] = brain_regis.loc[:, 'z_initial'].astype(float)
    brain_folder_list = []
    print("brain list ", list(map(str, brain_regis.index.tolist())))
    for subpath in os.listdir(path):
        if not (subpath) in list(map(str, brain_regis.index.tolist())):
            print("Skip file " + str(subpath.split('/')[-1]))
            continue
        brain_folder_list.append(subpath)
        print('Find new brain folder ' + str(subpath))
    print(brain_folder_list)

    cores = int(multiprocessing.cpu_count() * 1)  # multiprocessing.cpu_count()
    pool = multiprocessing.Pool(processes=cores)

    for iter_bb in brain_folder_list:
        pool.apply_async(single_brain_log, (brain_regis, path, iter_bb))
        # single_brain_log(brain_regis,path,iter_bb)
    pool.close()
    pool.join()
    elapsed = (time.time() - start)
    print("Used %.2f to run all the brian" % elapsed)

    import os
    import pandas as pd
    import shutil


    def flip_z(path_old, path_new):
        old_files = os.listdir(path_old)
        for name in old_files:
            file = open(path_old + '/' + name, 'r')
            f = open(path_new + '/' + name, 'w+')
            lines = file.readlines()

            for line in lines:
                S = line.strip().split(' ')

                if S[0][0:1] == '#':
                    f.write(line)
                    continue

                z = 456 - float(S[4]) - 1
                z = round(z, 3)

                f.write('{} {} {} {} {} {} {}\n'.format(S[0], S[1], S[2], S[3], z, S[5], S[6]))


    brain_regis = pd.read_excel(os.path.join(path, 'brain_registration.xlsx'), index_col=[0], skiprows=[0],
                                names=['ID', 'y_initial', 'x_initial', 'z_initial', 'y_after', 'x_after', 'z_after',
                                       'ration', 'flip', 'flip_axis'])
    brain_list = []
    for file_name in os.listdir(path):
        if not (file_name) in list(map(str, brain_regis.index.tolist())):
            continue
        brain_list.append(file_name)

    flip_brain_list = ['17788', '18047', '18453', '18868', '18452']
    for brain in brain_list:
        path_brain_folder = path + '/' + brain
        path_stps = path_brain_folder + '/stps'
        if not os.path.exists(path_stps):
            continue
        if os.listdir(path_stps) == 0:
            continue
        else:
            new = dest + '/' + brain
            if not os.path.exists(new):
                os.mkdir(new)
            if brain in flip_brain_list:
                flip_z(path_stps, new)
            else:
                stps_swc_list = os.listdir(path_stps)
                for swc_stps in stps_swc_list:
                    path_stps_swc = path_stps + '/' + swc_stps
                    shutil.copy(path_stps_swc, new)

    for brain in brain_list:
        path_ori_folder = path + '/' + brain + '/' + 'stps'
        ori_swc_list = os.listdir(path_ori_folder)
        final_swc_list = os.listdir(dest + '/' + brain)
        if (len(ori_swc_list) != len(final_swc_list)) or (len(final_swc_list) == 0):
            shutil.rmtree(dest + '/' + brain)
            print(brain)

# import shutil
# import os
# dest=r''
# brain_list=os.listdir(dest)
# for brain in brain_list:
#     old=dest+'/'+brain+'/'+os.listdir(dest+'/'+brain)[0]
#     new = dest
#     shutil.copy(old,new)
