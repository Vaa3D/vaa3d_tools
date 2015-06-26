# This plugin uses the regression tubularity measure described in [1] in combination with a Minimum Spanning Tree (MST) algorithm [2]
# to reconstruct a neuron in a 3D image.


# Usage of RegMST tracing 
      ./vaa3d -x RegMST -f tracing_func -i <inimg_file> -p <channel> <n_AC> <reg_path_1.cfg> ... <reg_path_n_AC.cfg> <window size>

             inimg_file                                The input image
             channel                                   Data channel for tracing. Start from 1 (default 1)
             n_AC                                      Number of autoncontext iterations to compute tubularity. 1 = one regressor i.e. no auto-context; 0 = apply MST on original image (default 0)
             reg_path_1.cfg ... reg_path_n_AC.cfg      Path to regressors (generated using RegressionTubularityAC plugin). Number of files must be equal to n_AC.
             window size                               Window size for seed detection in MST. (default 10)

            outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it


[1] A. Sironi, E. Türetken, V. Lepetit and P. Fua. Multiscale Centerline Detection, submitted to IEEE Transactions on Pattern Analysis and Machine Intelligence.
[2] H. Peng, Z. Ruan, D. Atasoy and S. Sternson Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model. Bioinformatics.
