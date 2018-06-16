import os

def get_human_trajectories():
    file_list = glob.glob("../data/human/*.swc")
    return file_list

def remove_comments(fpaths, fnames):
    """SWC files start with comments, remove before proceeding"""
    for i in range(len(fpaths)):

        input = open(fpaths[i], "r")
        outdir = "../data/human_clean/"
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

def main():
    # get human trajectories
    fnames = []
    abs_paths = []
    for root, dirs, fnames_ in os.walk("../data/human/"):
        fnames.extend(fnames_)
        for f in fnames_:
            relpath = os.path.join(root, f)
            abs_path = os.path.abspath(relpath)
            abs_paths.append(abs_path)

    remove_comments(abs_paths, fnames)


if __name__ == "__main__":
    main()