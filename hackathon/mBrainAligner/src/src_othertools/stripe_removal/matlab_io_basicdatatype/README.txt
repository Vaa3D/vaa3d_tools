Last update: 2009-07-28. by Hanchuan Peng

This directory contains a Matlab toolbox to read and write files with 
some formats defined/supported by V3D software (penglab.janelia.org).

Since some of the functions are mex functions, you may need to run the 
following command to generate the mex function for your machine (assuming
you have mex compiling setup in your Matlab):

>> makeosmex_rawfile_io

The following is a simple explanation of the interface of functions, both
mex functions and plain Matlab .m functions. 

load_v3d_raw_img_file:    load the .raw, .tiff image stacks used in V3D (depending on
						  file extension, the program automatically determine the file type)
load_v3d_marker_file:     load the .marker file 
load_v3d_apo_file:        load the .apo files (point cloud files)
load_v3d_pointcloud_file: load the point cloud files used in V3D (an overloading
						  function provided for convenience)
load_v3d_swc_file:        load the .swc file (for neurons or other graphs)
load_v3d_neuron_file:     load the neuron files in the .swc format (an overloading function
                          provided for convenience)

save_v3d_raw_img_file:    save the .raw, .tiff image stacks used in V3D (depending on
						  file extension, the program automatically determine the file type)
save_v3d_marker_file:     save the .marker file
save_v3d_apo_file:        save the .apo files (point cloud files)
save_v3d_pointcloud_file: save the point cloud files used in V3D (an overloading
						  function provided for convenience)
save_v3d_swc_file:        save the .swc file (for neurons or other graphs)

