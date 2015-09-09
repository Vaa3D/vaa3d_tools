This plugin will generate a binary 3D image of traced neuron and a swc file under the Vaa3d root directory.

In order to run this plugin, Matlab or MCR(Matlab Compiler Runtime) is needed.

You can use command line ./vaa3d -x aVaaTrace3D -f func1 -p "50 5 2" -i input_image to use the plugin
And need to set LD_LIBRARY_PATH for Matlab Runtime libs. A sample
 start_vaa3d.sh is given where "/home/.../matlab" is the directory for the
folder matlab_plugin(which contains the lib and header file generated from our matlab code)
You can download MCR here http://www.mathworks.com/products/compiler/mcr/
And following the instructions here http://www.mathworks.com/help/compiler/install-the-matlab-runtime.html