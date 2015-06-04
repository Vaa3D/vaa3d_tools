Currently, this plugin will generate a binary 3D image of traced neuron under the Vaa3d root directory.
In order to run this plugin, Matlab or MCR(Matlab Compiler Runtime) is needed.
And need to set LD_LIBRARY_PATH for Matlab Runtime libs. A sample
start_vaa3d.sh is given where "/home/.../matlab" is the directory for the
folder matlab_plugin(which contains the lib and header file generated from our matlab code)
