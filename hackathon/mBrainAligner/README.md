# mBrainAligner: Cross-Modality Coherent Registration of Whole Mouse Brains

![result_example](https://github.com/Vaa3D/vaa3d_tools/blob/master/hackathon/mBrainAligner/result_example.png)

# Contents

- [Overview](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner#Overview)
- [System requirements](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner#System-requirements)
- [Installation guide](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner#Installation-guide)
- [Setting up the development environment](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner#Setting-up-the-development-environment)
- [Demo](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner#Demo)

# Overview

Recent whole brain mapping projects are collecting large-scale 3D images using powerful and informative modalities, such as STPT, fMOST, VISoR, LSFM or MRI. Registration of these multi-dimensional whole-brain images onto a standard atlas is essential for characterizing neuron types and constructing brain wiring diagrams. However, cross-modality image registration is challenging due to intrinsic variations of brain anatomy and artifacts resulted from different sample preparation methods and imaging modalities. 

mBrianAligner provides a comprehesive, robust and accurate cross-modality image registration pipeline to support the current large-scale whole brain mapping projects. In addition to aligning 3D mouse brain images of different modalities, mBrainAligner also enables mapping digitally reconstructed compartments (e.g., dendritic, axonal, and soma distributions) to a common target space to faciliate the visualization, comparison and analysis. Using mBrainAligner, we have also generated an fMOST-space mouse brain atlas and showcased the utility of our method for analyzing single cell types. mBrainAligner is not limited to the use of intra- and cross-modality mouse brain registration, its utilities can also be extended to partially imaged data and multi-timepoint registration or other species. 

mBrainAligner contains three decoupled modules: (1) image preprocessing and global registration, (2) Coherent Landmark Mapping (CLM) based automatic registration, and (3) optional semi-automatic refinement. To accommodate different registration accuracy or throughput requirements, the above modules can be concatenated or executed separately. In addition, some useful tools including 2.5D corner detector, stripe artifacts removal, and image or metadata warping tools are also provided in this package for the user's convenience.

In general, there are two ways to use mBrainAligner:
1. follow the below instructions and run mBrainAligner on local machines.
2. visit http://159.75.201.35/ and run mBrainAligner online.

# System requirements

## Hardware requirements

`mBrainAligner` works on desktop computers. The memory requirement depends on the size of the image to be processed. For optimal performance, we recommend a computer with the following specifications: 

RAM: 32 GB or larger;

CPU: 2.3GHz dual-10-core Intel Xeon or better.


## Software Requirements

### OS Requirements

The package has been tested on the following systems:

- Windows
- Linux (e.g. Ubuntu)

###  Dependencies

```
Vaa3D
OpenCV 3.1.0
```

# Installation Guide

## Setting up environments

Put the `mBrainAligner` package in the `PATH\vaa3d_tools\hackathon\` directory. 

Configure the project in Release and x64 mode.

## Pre-requisites
### Vaa3D 

Please follow this link to build Vaa3D on Windows with qmake using VS2013 and Qt4.8.6:

```
https://github.com/Vaa3D/Vaa3D_Wiki/wiki/Build-Vaa3D-on-Windows-with-qmake-using-VS2013-and-Qt4.8.6
```
### OpenCV
Install OpenCV 3.1.0.

Configure the project according to the OpenCV intallation directory.


## MATLAB Code

`1.preprocessing/StripreMove/stripremove.m` needs to be executed using MATLAB(>=2016b).


## C++ Code
### for Windows

Before compiling, modify the **".pro"** file according to the path of the installed dependencies (Vaa3D,Qt etc). 

Launch the "Visual Studio x64 Win64 Command Prompt" from Start->Microsoft Visual Studio 2013->Visual Studio Tools->Visual Studio x64 Win64 Command Prompt
example(global_registration):

    cd (YOUR PATH)/vaa3d_tools/hackathon/mBrainAligner/2.global_registration/
    qmake global_registration.pro
    vcvars64.bat
    nmake release
You can find the executable file from the location "(YOUR PATH)/vaa3d_tools/hackathon/mBrainAligner/2.global_registration/global_registration.exe". 

### for Linux
Before compiling, you also need to modify the **".pro"** file according to the path of the installed dependencies (Vaa3D,QT etc). 
example(global_registration):

     cd (YOUR PATH)/vaa3d_tools/hackathon/mBrainAligner/2.global_registration/
     qmake global_registration.pro
     Make
You can find the executable file from the location "(YOUR PATH)/vaa3d_tools/hackathon/mBrainAligner/2.global_registration/global_registration". 

# Demo
We have provided downsampled raw data for four modalities (`fMOST`,`VISoR`,`MRI`,`LSFM`), corresponding registered results, and related files needed during registration.

### Structure of the Demo folder

>**mBrainAligner_data**
>
> > **3rdparty_linux**: executable file and `lib` file.
> > 
> > **3rdparty_windows**: executable file and `dlls`files.
> > 
> > **data**: `mBrainAligner` necessary files,including `CCFv3 (template), density_landmarks(local registration initial landmarks) etc`.
> > 
>**Registration_sample_data**
> >
> > **fMOST_segmentation**: fmost brain segmentation_result.
> > 
> > **raw_sample_data**: raw sample data. 
> > 
> > **Parameter**: optimal parameters for registering sample data.
> > 
> > **result**:  If you use the default result save path in the script file, you will get the registration result here,and view them in [Vaa3d](https://github.com/Vaa3D).
> > 
> > **registered_sample_data**: registration results of fMOST,LSFM,MRI and VISoR modal sample data (we provided).
> > 
> > > LSFM
> > > 
> > > MRI
> > > 
> > > VISoR
> > > 
> > > fMOST
> > > 
> > > > global_result
> > > >
> > > > >1.auto_RPM:  results of aligned by rpm algorithm,include registered brain data and its corresponding two sets of landmarks.
> > > > >
> > > > >2.auto_FFD:  results of aligned by FFD algorithm,including registered brain data and its corresponding deformation field.
> > > >
> > > >loacl_result: results of aligned by mBrainAligner local registration algorithm,include registered brain data and its corresponding landmarks.

## Run
### Windows
   Directly run
   
    run_script_windows.bat

### Linux    
   **Environment configuration**
   
   The path for the lib files (mBrainAligner_data/3rdparty_linux/lib/)  needs to be added to the linker directory, the steps are as follows:
    
    tar -zxvf lib.tar.gz
    sudo gedit /etc/ld.so.conf
    add the "(your file path)/mBrainAligner_data/3rdparty_linux/lib"  to the last line of '/etc/ld.so.conf' and save. 
    sudo ldconfig
   After that, run
      
    sh run_script_linux.sh 
    
    
Single brain registration will finish in about 0.5 hours (If the default parameters are chosen). For all modals, sample data registration results will be saved in 'Registration_sample_data\result\'.

**Register your own data**: You need to modify the registration data path and corresponding parameters in `run_script_windows.bat` and `run_script_linux.sh`. You can also optionally modify the local registration parameters in `...config.txt` for better local registration results (Descrptions of the parameters are given in the corresponding files).


**Note**: Due to limited network and storage, small downsampled brain images are provided here (but the registration results 'Registration_sample_data\registered_sample_data\' are based on the raw brain data).
For better registration accuracy, we encourage the user to download raw data from the scalable Brain Atlas website (https://scalablebrainatlas.incf.org/mouse/ABA_v3#about) or contact us by email (qulei@ahu.edu.cn). If you use the raw data to register the fMOST mouse brain, you need to update the "Registration_sample_data/fMOST_segmentation/" at well. The segmentation model will be uploaded soon.


# License
mBrainAligner uses a slightly modified revised MIT license unless specifically stated for some rare cases. See detail here: https://github.com/Vaa3D/Vaa3D_Wiki/wiki/License_mBrainAligner
