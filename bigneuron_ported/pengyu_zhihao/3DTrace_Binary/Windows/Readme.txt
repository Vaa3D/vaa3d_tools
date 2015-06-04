VSproject	contains plugin source code( Visual Studio 2013 )
		may need to modify settings of Properties->C/C++->General->Additional Include Directories, Properties->Linker->General->Additional Library Directories
Matlab_lib	contains the dll, lib, header and source file generated from our matlab code. Need to be included in VS Project
Plugin_lib	contains the compiled dll of the plugin

In order to run this plugin, Matlab or MCR(Matlab Compile Runtime) need to be installed. In addition, the dll in the folder matlab_lib need to be copied into the Vaa3d root directory