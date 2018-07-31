import os
from collections import defaultdict
import numpy as np
import pandas as pd
from skimage import io as tiff2array
import subprocess


def get_fnames_and_abspath_from_dir(reldir, as_dict=False):
    """given relative directory, return all filenames and their full absolute paths"""
    fnames = []
    abs_paths = []
    for root, dirs, fnames_ in os.walk(reldir):
        if not as_dict:
            fnames.extend(fnames_)
            for f in fnames_:
                relpath = os.path.join(root, f)
                abs_path = os.path.abspath(relpath)
                abs_paths.append(abs_path)
            return fnames, abs_paths
        else:
            abs_path_dict = {}
            for f in fnames_:
                relpath = os.path.join(root, f)
                abs_path = os.path.abspath(relpath)
                abs_path_dict[f] = abs_path
            return abs_path_dict
    

def remove_comments_from_swc(fpaths, fnames, output_dir="../data/02_human_clean/"):
    """SWC files start with comments, remove before proceeding"""
    for i in range(len(fpaths)):

        input = open(fpaths[i], "r")
        output_dir = os.path.abspath(output_dir)
        if not os.path.exists(output_dir):
            os.mkdir(output_dir)
        outfile = os.path.join(output_dir, fnames[i])
        output = open(outfile, "w+")

        for line in input:
            if not line.lstrip().startswith("#"):
                output.write(line)

        input.close()
        output.close()
        # assert os.  # TODO

def swc_to_linked_list(fpath: list):
    
    # https://stackoverflow.com/a/17756005/4212158
    linked_list = defaultdict(list)
    
    input = open(fpath, "r")
    for line in input:
        # note: if the parent node is -1, then the child_node_id is the true identity of the root node
        child_node_id, type_, x_coord, y_coord, z_coord, radius, parent_node = line.split()
        new_node = (child_node_id, x_coord, y_coord, z_coord)
        if (len(new_node) != 4 and isinstance(new_node, tuple)):
            raise Exception("faulty node: {}".format(new_node))
        linked_list[parent_node].append(new_node)
    
    input.close()
    return linked_list

def swc_to_nparray(swc_abspath):
    # should make an np array with node_id, x,y,z coords
    # note: by default, node_id is coerced from int to float
    # TODO: use all cols
    arr = np.genfromtxt(swc_abspath, usecols=(0,2,3,4), delimiter=" ")
    return arr

def swc_to_dframe(swc_abspath):
    # should make a pandas datafame with node_id, x,y,z coords
    # note: by default, node_id is coerced from int to float
    #arr = np.genfromtxt(swc_abspath, usecols=(0,2,3,4), delimiter=" ")
    # comment skips all commented lines
    try:
        df = pd.read_table(swc_abspath, sep=' ',
                           comment='#',
                           header=None,
                           names=["node_id", "type", "x","y","z", "radius", "parent_node_id"], 
                           dtype={"node_id": np.int32,
                                  "type": np.int32, 
                                  "x": np.float64, 
                                  "y": np.float64, 
                                  "z":np.float64, 
                                  "radius": np.float64, 
                                  "parent_node_id": np.int32})

        return df
    except:
        print("reading swc failed. If getting dtype errors, check if swc has trailing spaces. see https://stackoverflow.com/questions/51214020/pandas-cannot-safely-convert-passed-user-dtype-of-int32-for-float64")
        raise

def dframe_to_swc(fname, df, output_dir="../data/05_sampled_cubes/"):
    """
    SWC convention:
    node_id type x_coordinate y_coordinate z_coordinate radius parent_node
    """
    output_dir = os.path.abspath(output_dir)
    if not os.path.exists(output_dir):
        os.mkdir(output_dir)

    
    out_fpath = os.path.join(output_dir, fname+".swc")
    
    # make sure col order is preserved
    df.to_csv(out_fpath, sep=' ', header=False, encoding='utf-8', \
             columns=["node_id", "type", "x", "y", "z", "radius", "parent_node_id"],
             index=False)   # do not save the pandas index

    assert (os.path.isfile(out_fpath))
    return out_fpath


def resample_swc(input_fname, input_fpath, vaad3d_bin_path, step_length=1.0, 
                 output_dir="../data/04_human_branches_filtered_upsampled/"):
    """
    sometimes, inter-node distances can be v large, which is bad for subsampling.
    this is a wrapper to call Vaa3D's resample_swc script
    
    see https://github.com/Vaa3D/Vaa3D_Wiki/wiki/resample_swc.wiki
    """
    vaa3d_bin_dir = os.path.abspath(os.path.join(vaad3d_bin_path, os.pardir))
    libresample_swc_dir = os.path.join(vaa3d_bin_dir, "plugins/neuron_utilities/resample_swc/libresample_swc.so")
    
    assert os.path.isfile(libresample_swc_dir), """libresample_swc not found.
      please go to /path/to/vaa3d_tools/released_plugins/v3d_plugins/resample_swc and run qmake and then make
      note: if you get an error that says ‘resample’ was not declared in this scope,
      make sure resampling.h is completely uncommented before running make
      """
    
    output_dir = os.path.abspath(output_dir)
    if not os.path.exists(output_dir):
        os.mkdir(output_dir)
        
    outfile_fpath = os.path.join(output_dir, input_fname)
    
    # don't overwrite
    if not os.path.isfile(outfile_fpath):

        # https://stackoverflow.com/a/4376421/4212158
        v3d_plugin_name = "resample_swc"

        cli_dict = {"v3d_bin": vaad3d_bin_path,
                   "plugin": v3d_plugin_name,
                   "in": input_fpath,
                   "out": outfile_fpath,
                   "step": step_length}

        #print("running \n")
        #print("{v3d_bin} -x {plugin} -f {plugin} -i {in} -o {out}".format(**cli_dict))
        subprocess.run("{v3d_bin} -x {plugin} -f {plugin} -i {in} -o {out} -p {step}".format(**cli_dict), shell=True)
        # os.system("{v3d_bin} -x {plugin} -f {plugin} -i {in} -o {out} -p {step}".format(**cli_dict))

        assert (os.path.isfile(outfile_fpath))
        return outfile_fpath

def swc_to_TIFF(input_fname, input_fpath, vaad3d_bin_path="$HOME/Desktop/v3d_external/bin/vaa3d", overwrite=False,
                 output_dir="../data/07_cube_TIFFs",
                bounds = [0, 14, 0, 14, 0, 14]): # TODO: provide explicit bounds
    """note: swc2mask crops the img

    example usage in cli:
    ./vaa3d -x swc2mask -f swc2maskbb -i input.swc -o output.tif -p xmin xmax ymin ymax zmin zmax
    """
    # print("writing Tiff")
    bounds = [str(bound) for bound in bounds]
    output_dir = os.path.abspath(output_dir)
    if not os.path.exists(output_dir):
        os.mkdir(output_dir)
    
    outfile_fpath = os.path.join(output_dir, input_fname + ".tiff")
    
    # don't overwrite
    if not os.path.isfile(outfile_fpath) or overwrite:

        # https://stackoverflow.com/a/4376421/4212158
        v3d_plugin_name = "swc2mask"

        cli_dict = {"v3d_bin": vaad3d_bin_path,
                   "plugin": v3d_plugin_name,
                   "function": "swc2maskbb",
                   "in": input_fpath,
                   "out": outfile_fpath,
                   "bounds": " ".join(bounds)}
                   #"step": step_length}

        # the &>/dev/null surpressed output
        # print("running SWC to TIFF\n")
        # print("{v3d_bin} -x {plugin} -f {plugin} -i {in} -o {out} -p {dims} >/dev/null 2>&1".format(**cli_dict))
        subprocess.run("{v3d_bin} -x {plugin} -f {function} -i {in} -o {out} -p {bounds} >/dev/null 2>&1".format(**cli_dict), shell=True)
        # os.system("{v3d_bin} -x {plugin} -f {function} -i {in} -o {out} -p {bounds} >/dev/null 2>&1".format(**cli_dict))
        try:
            # TODO make similar assertion errors for other funcs
            assert (os.path.isfile(outfile_fpath))
        except AssertionError:
            print("TIFF saving failed!")
            print("running {v3d_bin} -x {plugin} -f {function} -i {in} -o {out} -p {bounds}".format(**cli_dict))
            # do not surpress output
            os.system("{v3d_bin} -x {plugin} -f {function} -i {in} -o {out} -p {bounds}".format(**cli_dict))
            raise
        return outfile_fpath
    
def TIFF_to_npy(input_fname,  input_fpath, output_dir="../data/08_cube_npy", overwrite=False):
    output_dir = os.path.abspath(output_dir)
    if not os.path.exists(output_dir):
        os.mkdir(output_dir)
    
    outfile_fpath = os.path.join(output_dir, input_fname + ".npy")
    
    # don't overwrite
    if not os.path.isfile(outfile_fpath) or overwrite:
        desired_len = 15  # FIXME should not be explicit shape
        img_array = tiff2array.imread(input_fpath)
        # make all arrays the same shape
        # format: ((top, bottom), (left, right))
        shp = img_array.shape
        #print(shp, flush=True)
        if shp != (desired_len, desired_len, desired_len):
            # print("tiff is wrong shape: ", shp, "; padding!")
            try:
                img_array = np.pad(img_array, ((0, desired_len-shp[0]), (0, desired_len-shp[1]), (0, desired_len-shp[2])), 'constant')
            except ValueError:
                raise
                #print(shp, flush=True)  # don't wait for all threads to finish before printing
                
        np.save(outfile_fpath, img_array)
        assert (os.path.isfile(outfile_fpath))
        return outfile_fpath


def linked_list_2_swc(branch: list, branch_name: str, output_dir="../data/03_human_branches_splitted/", overwrite = False):
    """
    SWC convention:
    node_id type x_coordinate y_coordinate z_coordinate radius parent_node
    """
    assert(isinstance(branch, (list, np.ndarray)))
    output_dir = os.path.abspath(output_dir)
    if not os.path.exists(output_dir):
        os.mkdir(output_dir)
    outfile = os.path.join(output_dir, branch_name+".swc")
    # don't overwrite
    if not os.path.isfile(outfile) or overwrite:
        #print("saving SWC {}".format(branch_name))
        output = open(outfile, "w+")

        default_radius = "1.0"
        default_type = "3"

        parent_node_id = "-1"
        for i, node in enumerate(branch):
            if len(node) == 4:
                child_node_id, x, y, z = node
            elif len(node) == 3:
                x, y, z = node
                child_node_id = str(i)
            # this is the SWC file convention
            # \n is important to separate into new lines
            swc_items = [child_node_id, default_type, x, y, z, default_radius, parent_node_id, "\n"]
            swc_items = [str(x) for x in swc_items]
            #swc_items = [str(item) for item in str_items]
            try:
                swc_line = " ".join(swc_items)
            except TypeError:
                print(swc_items)
                raise
            except:
                print(swc_items)
                for item in swc_items:
                    print(type(item))
                    raise
            output.write(swc_line)
            parent_node_id = child_node_id

        output.close()
        assert (os.path.isfile(outfile))
        return outfile

def locations_to_swc(locations_list: list, branch_name: str, output_dir: str, overwrite = False):
    assert len(locations_list) > 0
    output_dir = os.path.abspath(output_dir)
    outfile = os.path.join(output_dir, branch_name + ".swc")
    # don't overwrite
    if not os.path.isfile(outfile) or overwrite:
        # print("saving SWC {}".format(branch_name))
        output = open(outfile, "w+")

        default_radius = "1.0"
        default_type = "3"

        parent_node_id = "-1"
        for i, node in enumerate(locations_list):
            x, y, z = node
            child_node_id = str(i)
            # this is the SWC file convention
            # \n is important to separate into new lines
            swc_items = [child_node_id, default_type, x, y, z, default_radius, parent_node_id, "\n"]
            swc_items = [str(x) for x in swc_items]
            # swc_items = [str(item) for item in str_items]
            try:
                swc_line = " ".join(swc_items)
            except TypeError:
                print(swc_items)
                raise
            except:
                print(swc_items)
                for item in swc_items:
                    print(type(item))
                    raise
            output.write(swc_line)
            parent_node_id = child_node_id

        output.close()

        assert(os.path.isfile(outfile))
        # print(outfile, " exists!")
        return outfile
