__author__ = 'coriannaj'

###

import os
import subprocess, threading
import pandas as pd
import math

V3D =  "/data/mat/xiaoxiaol/work/bin/bin_vaa3d_for_clusters/start_vaa3d.sh"

class Command(object):
    def __init__(self, cmd):
        self.cmd = cmd
        self.process = None

    def run(self, timeout):
        def target():
            #print 'Thread started'
            self.process = subprocess.Popen(self.cmd, shell=True)
            self.process.communicate()
            #print 'Thread finished'

        thread = threading.Thread(target=target)
        thread.start()

        thread.join(timeout)
        if thread.is_alive():
            print 'Terminating process'
            print self.cmd
            self.process.terminate()
            thread.join()
        #print self.process.returncode

def RUN_Vaa3d_Job(arguments):

    # run in local python env
    cmd = V3D + arguments
    print cmd
    command = Command(cmd)
    command.run(timeout=60*10)
    return

def profiling(input_img, input_swc, output_file, dilation_ratio = 3, flip = 0, invert = 0, cutoff_ratio=0.05, logfile=""):

    arguments = " -x profiling -f profile_swc -i "+input_img+" "+input_swc+" -o "+output_file+" -p "+ str(dilation_ratio)+" "+str(flip)+"  "+str(invert)+logfile

    RUN_Vaa3d_Job(arguments)
    return

def convert_img(input_img, output_img):
    arguments = "/data/mat/zhi/idpImageReadWrite " + input_img + " " + output_img + " UCHAR 2"
    print arguments
    command = Command(arguments)
    command.run(timeout=60*10)
    return

def distanceFromRoot(root_x, root_y, root_z, x, y, z):
    #use the following line for 3D distance
    #return math.sqrt(pow((root_x - x), 2) + pow((root_y - y), 2) + pow((root_z - z), 2))

    #use the following line for 2D distance - excluding the z-plane
    return math.sqrt(pow((root_x - x), 2) + pow((root_y - y), 2))

def buildSpliceSWC(df, orig_df, max_dist):
    assert(orig_df.at[0, 'pid'] == -1) #asserting it is root
    df.loc[len(df)] = orig_df.loc[0]
    root_x = orig_df.at[0, 'x']
    root_y = orig_df.at[0, 'y']
    root_z = orig_df.at[0, 'z']

    nodes = len(orig_df.index)

    for i in xrange(1, nodes):
        dist = distanceFromRoot(root_x, root_y, root_z, orig_df.at[i, 'x'], orig_df.at[i, 'y'], orig_df.at[i, 'z'])
        if dist < max_dist:
            df.loc[len(df)] = orig_df.loc[i]

def spliceSWC(swc, write_path, splice_dist):

    swc_identifier = swc.split('/')[-1]
    print swc_identifier

    cols = ['id', 'type', 'x', 'y', 'z', 'radius', 'pid']
    fullswc_df = pd.read_csv(swc, delim_whitespace=True, comment='#', skiprows=1, names=cols)

    spliceswc_df = pd.DataFrame(columns=cols)

    buildSpliceSWC(spliceswc_df, fullswc_df, splice_dist)

    swc_loc = os.path.join(write_path, swc_identifier.split('.swc')[0] + '_splice.swc')

    spliceswc_df['id'] = spliceswc_df['id'].astype(dtype=int)
    spliceswc_df['type'] = spliceswc_df['type'].astype(dtype=int)
    spliceswc_df['pid'] = spliceswc_df['pid'].astype(dtype=int)

    spliceswc_df.to_csv(swc_loc, sep=' ', header=False, index=False)

    print 'swc stored : %s' %swc_loc
    #id = spliceswc_df['id'].astype(dtype=int)
    #type = spliceswc_df['type'].astype(dtype=int)
    #x = spliceswc_df['x']
    #y = spliceswc_df['y']
    #z = spliceswc_df['z']
    #r = spliceswc_df['radius']
    #pid = spliceswc_df['pid'].astype(dtype=int)

    #spliceswc_df = pd.concat([id, type, x, y, z, r, pid], axis=1)

    print spliceswc_df
    return swc_loc

#works specificially for the format set up in the gold folder
def runLIMS(data_path, write_path, overall_csv, splice_dist):

    #create df table with headers
    cols = ['image_id', 'CNR', 'SNR', 'dynamic_range', 'mean_fg', 'std_fg', 'mean_bg', 'std_bg', 'mean_tubularity', 'std_tubularity']
    overall_profile = pd.DataFrame(columns=cols)

    print data_path

    dataimg_path = '/projects/mousecelltypes/vol1/'

    for f in os.listdir(data_path):

        #subfolder_path = os.path.join(data_DIR, dirName)
        #subfolder_write_path = os.path.join(write_path, os.path.basename(dirName))

        identifier = f.split('.swc')[0]

        swc = os.path.join(data_path, f)
        img = os.path.join(write_path, identifier) + '.tif'

        print "swc is %s" %swc

        runID = f.split('-')[-1]
        runID = runID.split('.')[0]

        sub_path = os.path.join('specimen_' + runID, 'min_xy_' + runID + '.jp2')
        if (runID == 515435668):
            print("PATH TO IMAGE: %s") %sub_path
        profile_path = os.path.join(write_path, identifier + '_splice.csv')

        args = dataimg_path + '*/' + sub_path

        path_list = subprocess.check_output('ls ' + args, shell=True)
        path_list = path_list.split('\n')[0]

        convert_img(path_list, img)
        swc = spliceSWC(swc, write_path, splice_dist)

        if swc != None and img != None:
            profiling(img, swc, profile_path, 3, 1, 1)
            print "Img: %s" % img
            print "Swc: %s" % swc
            print "Profile path: %s" % profile_path
            #print "Log File: %s" % logfile

            try:
                profile_df = pd.read_csv(profile_path)
            except IOError:
                print "Image %s did not create a profile.csv" %swc
            else:
                #if file exists get data
                # take from row 2 because only want dendrite data, type=3
                stats = [int(runID), profile_df.at[2,'cnr'], profile_df.at[2,'snr'], profile_df.at[2,'dynamic_range'], profile_df.at[2,'fg_mean'], profile_df.at[2,'fg_std'], profile_df.at[2,'bg_mean'], profile_df.at[2,'bg_std'], profile_df.at[2,'tubularity_mean'], profile_df.at[2,'tubularity_std']]
                print stats
                overall_profile.loc[len(overall_profile)]=stats

    print "exporting csv"
    print overall_profile
    overall_profile.sort_values(by='image_id', inplace=True)
    overall_profile.to_csv(os.path.join(write_path, overall_csv), index=False)

    return

runLIMS('/local1/home/coriannaj/Desktop/NiviData/Version1','/data/mat/xiaoxiaol/data/NiviData/Version1', 'overall_splice.csv', 500)