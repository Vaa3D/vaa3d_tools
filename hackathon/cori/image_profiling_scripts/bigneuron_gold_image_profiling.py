__author__ = 'coriannaj'

###

import os
import subprocess, threading
import pandas as pd

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

    arguments = " -x profiling -f profile_swc -i "+input_img+" "+input_swc+" -o "+output_file+" -p "+str(dilation_ratio)+" "+str(flip)+"  "+str(invert)+" "+str(cutoff_ratio)+logfile

    RUN_Vaa3d_Job(arguments)
    return


#works specificially for the format set up in the gold folder
def runGold(data_path, write_path, id_file, overall_csv):

    #create df table with headers
    cols = ['image_id', 'CNR', 'SNR', 'dynamic_range', 'mean_fg', 'mean_bg', 'mean_tubularity']
    overall_profile = pd.DataFrame(columns=cols)

    print data_DIR

    folder_num = 0
    run_folders = 0

    # go through all image directories in gold163
    for dirName in os.listdir(data_DIR):

        subfolder_path = os.path.join(data_DIR, dirName)
        subfolder_write_path = os.path.join(write_path, os.path.basename(dirName))

        swc = None
        img = None

        print "Directory: %s aka %s" %(dirName, subfolder_path)
        if os.path.isdir(subfolder_path):
            profile_path = os.path.join(subfolder_write_path, img_csv)
            folder_num += 1
            print "Is directory %d" %folder_num

            for f in os.listdir(subfolder_path):

                #checking all files in directory for img and trace
                if f.endswith('strict.swc.out.swc'):
                    swc = os.path.join(subfolder_path, f)
                    print "found swc"
                if f.endswith(('.v3dpbd','.v3draw')):
                    img = os.path.join(subfolder_path, f)

            #formatting log file construction - add .log, join with path, then add command line argument ' > '
            logfile = img_csv + '.log'
            logfile = os.path.join(subfolder_write_path, logfile)
            logfile = " > " + logfile

            #create profile if all files necessary were found
            if swc != None and img != None:
                profiling(img, swc, profile_path, 3, 0, 0, 0.01, logfile)
                print "Img: %s" % img
                print "Swc: %s" % swc
                print "Profile path: %s" %profile_path
                print "Log File: %s" %logfile
                run_folders += 1

                #read in CSV output file

                try:
                    profile_df = pd.read_csv(profile_path)
                except IOError:
                    print "Folder %d did not create a profile.csv" %folder_num
                else:
                    #if file exists get data
                    # take from row 2 because only want dendrite data, type=3
                    stats = [int(os.path.basename(dirName)), profile_df.at[2,'cnr'], profile_df.at[2,'snr'], profile_df.at[2,'dynamic_range'], profile_df.at[2,'fg_mean'], profile_df.at[2,'bg_mean'], profile_df.at[2,'tubularity_mean']]
                    print stats
                    overall_profile.loc[len(overall_profile)]=stats


    print "exporting csv"
    print "visited %d folders" %folder_num
    print "ran on %d folders" %run_folders
    print overall_profile
    overall_profile.sort_values(by='image_id', inplace=True)
    overall_profile.to_csv(os.path.join(write_path, overall_csv), index=False)

    return

runGold('/data/mat/xiaoxiaol/data/big_neuron/silver/0401_gold163_all_soma_sort/', 'profile_strict.csv', 'radius_estimation_profiling-strict.csv')

#to add additional folders to data compilation
def runAddGold(data_dir, out_file, images):
    out_path = os.path.join(data_DIR, out_file)

    data_df = pd.read_csv(out_path)

    for img_d in images:
        data_path = os.path.join(data_dir, img_d)
        files = os.listdir(data_path)
        for f in files:
            profile_path = os.path.join(data_path, 'profile.csv')

            if f.endswith('.swc'):
                swc = os.path.join(data_path, f)
            if f.endswith(('.v3dpbd', '.v3draw')):
                img = os.path.join(data_path, f)

        if swc != None and img != None:
            profiling(img, swc, profile_path)

            try:
                profile_df = pd.read_csv(profile_path)
            except IOError:
                print "Folder %s did not create a profile.csv" %img_d
            else:
                # if file exists get data
                # take from row 2 because only want dendrite data, type=3
                stats = [158, img_d, profile_df.at[2, 'cnr'], profile_df.at[2, 'dynamic_range'],
                         profile_df.at[2, 'fg_mean'], profile_df.at[2, 'bg_mean'], profile_df.at[2, 'tubularity_mean']]
                print stats
                data_df.loc[len(data_df)] = stats

    data_df.to_csv(out_file)

#server_data = '/data/mat/xiaoxiaol/data/big_neuron/silver/0401_gold163_all_soma_sort/'

#runAddGold(server_data, "/local1/home/coriannaj/Desktop/0401_gold163_all_soma_sort/img_profiling.csv", ["292", "293"])