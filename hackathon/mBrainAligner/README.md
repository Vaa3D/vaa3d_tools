# mBrainAligner: Cross-Modality Coherent Registration of Whole Mouse Brains

![result_example](https://github.com/Vaa3D/vaa3d_tools/blob/master/hackathon/mBrainAligner/result_example.tif)

# Contents

- [Overview](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner#Overview)
- [System requirements](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner#System-requirements)
- [Installation guide](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner#Installation-guide)
- [Setting up the development environment](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner#Setting-up-the-development-environment)
- [Demo](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner#Demo)

# Overview

Recent whole brain mapping projects are collecting large-scale 3D images using powerful and informative modalities, such as STPT, fMOST, VISoR, LSFM or MRI. Registration of these multi-dimensional whole-brain images onto a standard atlas is essential for characterizing neuron types and constructing brain wiring diagrams. However, cross-modality image registration is challenging due to intrinsic variations of brain anatomy and artifacts resulted from different sample preparation methods and imaging modalities. 

mBrianAligner aims to be a comprehesive, robust and accurate cross-modality image registration pipeline, to support the current large-scale whole brain mapping projects. In addition to aligning 3D mouse brain images of different modalities, mBrainAligner also enables mapping digitally reconstructed compartments (e.g., dendritic, axonal, and soma distributions) to a common target space to faciliate their visualization, comparison and analysis. Using mBrainAligner, we have generated the an fMOST-space mouse brain atlas and showcased the utility of our method for analyzing single cell types. mBrainAligner is not limited to the intra- and cross-modality mouse brain registration, its utilities can also be extended to partially imaged data and multi-timepoint registration or other species. 

mBrainAligner contains three decoupled modules: (1) image preprocessing and global registration, (2) a Coherent Landmark Mapping (CLM) based automatic registration, and (3) an optional semi-automatic refinement . To accommodate different registration accuracy or throughput requirements, different modules can be concatenated or executed separately. In addition, some useful tools including 2.5D corner detector, stripe artifacts removal, and image or metadata warping tools are also provided in this package to promote interdisciplinary researches.

# System requirements

## Hardware requirements

`mBrainAligner` can works on standard PC, the required RAM capacity depends on the size of the image to be processed. For optimal performance, we recommend a computer with the following specs: 

RAM: 32+ GB
CPU: 10+ cores, 2.30+ GHz/core

The runtimes below are generated using a computer with the recommended specs (64 GB RAM, 10 cores@2.30 GHz) .

## Software Requirements

### OS Requirements

The package has been tested on the following systems:

- Windows:  Win10

###  Dependencies

```
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

After Vaa3D is successfully built, put the `mBrainAligner` package in the `PATH\vaa3d_tools\hackathon\` directory. Install opencv3.1.0 and configure the project according to your opencv intallation directory. And configure the project in Release and x64 mode according to your Qt4.8.6 installation directory.

## Matlab Code

`1.preprocessing/StripreMove/stripremove.m` need to be run using MATLAB, make sure MATLAB(>=2016b) is installed .

## C++ Code

Any C++11 compiler should work. If your use Visual Studio 2013, please follow the instructions below to compile the code.

- Use Microsoft Visual Studio 2013 to open **".vcxproj"**.
- Under Build -> Configuration Manager:
  - Choose building in "Release" mode in MS Visual Studio 2013.
  - Choose building in "x64" (for 64bit building) mode  in Property Pages or just select from the building platform located in the toolbar. If “x64” mode does not exist, create one by choosing 'New' and then copying the previous configuration settings.
  - Now hit build button to start building process.

# Demo

## prerequisites

- Python 3.3 or 3.4 or 3.5 (python 3.6 is not supported)

The python script needs to load the matlab engine. If you don't have MATLAB Engine API for Python intalled, you can enter Matlab installation directory `\matlab\extern\engines\python`, and run 

```
python setup.py install
```

 to install the Matlab engine.

## Data

We provide a small downsample fMOST mouse brain `18458.v3draw` for testing. 

### Sub-Folder

>Data:			                 		   the folder where the `18458.v3draw` will be copied to

>matlab_io_basicdatatype:	        matlab toolbox

>mBrainAligner_data
>>3rdparty:                 		  the folder where the final executable and `dlls` will be copied to.
>>
>>data:                           `mBrainAligner` necessary files,including `CCFv3, markers, config.txt, etc`.
>>
>>Seg:									           Deep feature    
>>

>stripremove.m           

>demo.py            

### Run

```
python demo.py 
```
The fMOST to CCF registration will complete in about 0.5 hours, and all the results can be found in the `/Result` folder.

Note: To faciliate internet transmission, a small downsampled brain image (with fewer landmarks) is provided in this demo. For better registration accuracy, we encourage user to download raw data from Allen official website (https://scalablebrainatlas.incf.org/mouse/ABA_v3#about) or contact us by email (qulei@ahu.edu.cn). If you use data of different size, you need to update the "/Seg" file correpondingly. We will upload the segmentation model of different size later.

Please refer to the 'help' of mBrainAligner for more information: 
```
python demo.py -h
```

# License
mBrainAligner used a slightly modified revised MIT license unless specifically stated for some rare cases, see detail here: https://github.com/Vaa3D/Vaa3D_Wiki/wiki/License_mBrainAligner
