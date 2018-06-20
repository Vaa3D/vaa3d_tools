import os
from collections import defaultdict
import numpy as np
import pandas as pd


def get_fnames_and_abspath_from_dir(reldir):
    """given relative directory, return all filenames and their full absolute paths"""
    fnames = []
    abs_paths = []
    for root, dirs, fnames_ in os.walk(reldir):
        fnames.extend(fnames_)
        for f in fnames_:
            relpath = os.path.join(root, f)
            abs_path = os.path.abspath(relpath)
            abs_paths.append(abs_path)
            
    return fnames, abs_paths
    

def remove_comments_from_swc(fpaths, fnames, outdir="../data/02_human_clean/"):
    """SWC files start with comments, remove before proceeding"""
    for i in range(len(fpaths)):

        input = open(fpaths[i], "r")
        outdir = os.path.abspath(outdir)
        if not os.path.exists(outdir):
            os.mkdir(outdir)
        outfile = os.path.join(outdir, fnames[i])
        output = open(outfile, "w+")

        for line in input:
            if not line.lstrip().startswith("#"):
                output.write(line)

        input.close()
        output.close()

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
    arr = np.genfromtxt(swc_abspath, usecols=(0,2,3,4), delimiter=" ")
    return arr

def swc_to_dframe(swc_abspath):
    # should make a pandas datafame with node_id, x,y,z coords
    # note: by default, node_id is coerced from int to float
    #arr = np.genfromtxt(swc_abspath, usecols=(0,2,3,4), delimiter=" ")
    df = pd.read_table(swc_abspath, sep=' ', names=["node_id", "x","y","z"], index_col=0, usecols=(0,2,3,4), dtype={"node_id": int, "x": float, "y": float, "z":float})
    return df

def resample_swc(input_path, output_dir=):
    """
    sometimes, inter-node distances can be v large, which is bad for subsampling.
    this is a wrapper to call Vaa3D's resample_swc script
    """
    
    raise Exception("not implemented")
    

def save_branch_as_swc(branch: list, branch_name: str, outdir="../data/03_human_branches_splitted/"):
    """
    node_id type x_coordinate y_coordinate z_coordinate radius parent_node
    """
    assert(isinstance(branch, list))
    outdir = os.path.abspath(outdir)
    if not os.path.exists(outdir):
        os.mkdir(outdir)
    outfile = os.path.join(outdir, branch_name+".swc")
    #print("saving SWC {}".format(branch_name))
    output = open(outfile, "w+")
    
    default_radius = "1.0"
    default_type = "3"
    
    parent_node_id = "-1"
    for i, node in enumerate(branch):
        try:
            child_node_id, x, y, z = node
        except ValueError:
            print("error in save-branch", len(node), node)
            raise
        # this is the SWC file convention
        # \n is important to separate into new lines
        swc_items = [child_node_id, default_type, x, y, z, default_radius, parent_node_id, "\n"]
        #swc_items = [str(item) for item in str_items]
        try:
            swc_line = " ".join(swc_items)
        except:
            print(swc_items)
            for item in swc_items:
                print(type(item))
                raise
        output.write(swc_line)
        parent_node_id = child_node_id

    output.close()
