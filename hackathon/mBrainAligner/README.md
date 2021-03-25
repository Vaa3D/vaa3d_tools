# Cross-Modality Coherent Registration of Whole Mouse Brains

# Contents

- [Overview](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner#Overview)
- [System requirements](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner#System-requirements)
- [Installation guide](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner#Installation-guide)
- [Setting up the development environment](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner#Setting-up-the-development-environment)
- [Demo](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner#Demo)

# Overview

Recent whole brain mapping projects are collecting large-scale 3D images using powerful and informative modalities. Registration of these multi-dimensional whole-brain images onto a standard atlas is essential for characterizing neuron types and constructing brain wiring diagrams. However, cross-modality image registration is challenging due to intrinsic variations of brain anatomy and artifacts resulted from different sample preparation methods and imaging modalities. We introduced a cross-modality registration method, called mBrainAligner,  we designed three modules in mBrainAligner: (1) image preprocessing and robust point matching based automatic global registration , (2) a CLM based automatic local registration, and (3) an optional semi-automatic refinement . 

# System requirements

## Hardware requirements

The  `mBrainAligner`  package requires only a standard computer with enough RAM to support the operations defined by a user. For optimal performance, we recommend a computer with the following specs:

RAM: 32+ GB
CPU: 10+ cores, 2.30+ GHz/core

The runtimes below are generated using a computer with the recommended specs (64 GB RAM, 10 cores@2.30 GHz) .

## Software Requirements

### OS Requirements

This package is supported for *Windows* . The package has been tested on the following systems:

- Windows:  Win10

###  Dependencies

```
Microsoft Visual Studio 2013
MATLAB 2016b 
Vaa3D
Opencv3.1.0
```

# Installation Guide

## Install Vaa3D 

Please follow this link to build Vaa3D on Windows with qmake using VS2013 and Qt4.8.6:

```
https://github.com/Vaa3D/Vaa3D_Wiki/wiki/Build-Vaa3D-on-Windows-with-qmake-using-VS2013-and-Qt4.8.6
```

# Setting up the development environment

After Vaa3D is successfully installed, enter the directory where the [mBrainAligner](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner) package is located. In the `1.preprocessing ` folder, the code is based on Matlab.  In the `2.global_registration`,`3.local_registration` and `Tools` folders, the code is based on C++ and  also provides a Visual Studio 2013 project file **".vcxproj"** for compilation.

## Matlab Code

`1.preprocessing/StripreMove/stripremove.m` need to run using MATLAB 2016b

## C++ Code

Any C++11 compiler should work. If your compiler has the C++11 features that are available in Visual Studio 2013 then it will work.

- Use Microsoft Visual Studio 2013 to open **".vcxproj"**.
- Under Build -> Configuration Manager:
  - Choose building in "Release" mode in MS Visual Studio 2013.
  - Choose building in "x64" (for 64bit building) mode  in Property Pages or just select from the building platform located in the toolbar. If “x64” mode does not exist, create one by choosing 'New' and then copying the previous configuration settings.
  - Now hit build button to start building process.

# Demo

## prerequisites

- Python 3.3, 3.4 and 3.5 not support python 3.6

This demo needs to load the matlab engine, engine API for python. if you don't have the API, you can found `\matlab\extern\engines\python` .Before extern is the matlab installation directory, and the subsequent is the fixed directory. Enter 

```
python setup.py install
```

 to install the Matlab engine.

## Data

We provide a fMOST mouse brain `18458.v3draw` to be registered  and supporting documents can be obtained from:

### Sub-Folder

>Data:			                 		   the folder where the `18458.v3draw` will be copied to

>matlab_io_basicdatatype:	matlab toolbox,you can be obtained from `Vaa3d _external`

>mBrainAligner_data
>>3rdparty:                 		  the folder where the final executable and `dlls` will copied to.
>>
>>data:                           `mBrainAligner` necessary files,including `CCFv3, markers, config.txt, etc`.
>>
>>Seg:									           Deep feature    
>>
>>Manual_marker:                  affine marker,including`tar.marker,sub.marker`.

>stripremove.m           

>demo.py            

### Run

```
python demo.py 
```

Note: The above is the result under the default parameters.  If you want to change the parameter settings, you can refer to the help document.

```
python demo.py -h
```

The registration result can be obtained after 6 hours, and all the results will be stored in the `/Result` folder.


# License
mBrainAligner used a slightly modified revised MIT license unless specifically stated for some rare cases, see detail here: https://github.com/Vaa3D/Vaa3D_Wiki/wiki/License_mBrainAligner

