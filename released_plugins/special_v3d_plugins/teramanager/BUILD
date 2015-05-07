*********** BUILD INSTRUCTIONS for all platforms ************

This plugin  uses  the  third party library  OpenCV 2.4.4. To 
successfully build this  plugin, you can  either use the pre-
compiled static libreries provided  under the ./lib folder or
install the latest version of OpenCV by your own.

-------------------------------------------------------------
Building the plugin using precompiled static libraries
-------------------------------------------------------------
1) open the "teramanager.pro" Qt project file with a text ed-
   itor
2) in the "generic set up" section, add the following line:
   CONFIG += use_static_libs
3) use qmake to generate your platform-specific makefile from
   the "teramanager.pro" Qt  project file  (you can  also use
   QtCreator to open the .pro file), then build the plugin

If build fails, it means that the  precompiled static librar-
ies are not suited for your platform. Then, you need to build
them as well (see next section).

-------------------------------------------------------------
Building the plugin after installing OpenCV by your own
-------------------------------------------------------------
1) download the latest version of OpenCV from:
   http://opencv.willowgarage.com
2) install OpenCV following the installation guide at:
   http://opencv.willowgarage.com/wiki/InstallGuide
   Depending on your  platform, you may need to  build OpenCV
   with CMake by your own.  If that is the case, you can fol-
   low the quick installation guide at the end of this file.
3) locate the "include" and "lib" directories where OpenCV
   has been installed
4) open the "teramanager.pro" Qt project file with a text ed-
   itor and search for "#OpenCV headers and library folders"
5) substitute "/usr/local/include" and "/usr/local/lib" with
   the absolute paths of step 3
6) use qmake to generate your platform-specific makefile from
   the "teramanager.pro" Qt  project file  (you can  also use
   QtCreator to open the .pro file), then build the plugin


-------------------------------------------------------------
Building OpenCV with CMake
-------------------------------------------------------------
1) download and install CMake with gui from:
   http://www.cmake.org/
2) launch CMake gui
3) in "Where is the source code" select the folder containing
   the OpenCV source folder
4) in "Where to build the binaries" select a different folder
   where to  build binary files  (it should be different from
   the source folder)
5) press "Configure" and select  your platform-specific make-
   file you want to generate  (Unix makefile,  Visual Studio,
   etc.). If asked, be  careful to  select  also the same ar-
   chitecture (32 or 64 bit) used to build Qt and Vaa3D.
6) check the "CMAKE_INSTALL_PREFIX" variable: it contains the
   path where OpenCV will be installed
7) press "Generate"  and your  platform-specific makefiles or
   project files will be generated in the folder indicated at
   step 4
8) use the generated makefile / project file to build and in-
   stall OpenCV with your toolchain  ("make" + "make install"
   on Linux/MacOS X).

*************************************************************
