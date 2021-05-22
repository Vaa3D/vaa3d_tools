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

We provide two binary files for mBrainAligner corresponding to global registration and local registration respectively.And provide `main_mbrainaligner.bat` with default parameters. Of course, you can also modify it as needed.

## Data

We provide downsample raw data for four modalities, corresponding registered results：`fMOST`,`VISoR`,`MRI`,`LSFM`,and related files needed in registration.

### Sub-Folder

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
> > > >1.auto_RPM:  results of aligned by rpm algorithm,include registered brain data and its corresponding two sets of landmarks.
> > > >
> > > >2.auto_FFD:  results of aligned by FFD algorithm,including registered brain data and its corresponding deformation field.
> > > >
> > > >loacl_result: results of aligned by mBrainAligner local registration algorithm,include registered brain data and its corresponding landmarks.

## Run
### Windows
   If you want to reproduce our sample data registration results, you can run
   
    run_script_windows.bat

### Linux    
   **Environment configuration**
   
   You need to add the file 'lib' path(mBrainAligner_data/3rdparty_linux/lib/) to the linker directory, the steps are as follows:
    
    tar -zxvf lib.tar.gz
    sudo gedit /etc/ld.so.conf
    add the "(your file path)/mBrainAligner_data/3rdparty_linux/lib"  to the last line of '/etc/ld.so.conf' and save. 
    sudo ldconfig
   After that,you can register our sample data, run
      
    sh run_script_linux.sh 
Single brain registration will complete in about 0.5 hours (If you choose the default parameters), and all modal data registration results will be saved in 'Registration_sample_data\result\'.

If you need to register your own data, you need to modify the registration data path and corresponding parameters in the `run_script_windows.bat` and `run_script_linux.sh`   file. You can also optionally modify the local registration parameters in `...config.txt` to make your data obtain better local registration results. (All parameters have corresponding introduction in the file)

**Note**: To faciliate internet transmission, a small downsampled brain image is provided in this demo (But we provide registration results 'Registration_sample_data\registered_sample_data\' based on the raw brain data). For better registration accuracy, we encourage user to download raw data from Allen official website (https://scalablebrainatlas.incf.org/mouse/ABA_v3#about) or contact us by email (qulei@ahu.edu.cn). If you use the raw data to register the fMOST mouse brain, you need to update the "Registration_sample_data/fMOST_segmentation/". We will upload the segmentation model later.


# License
mBrainAligner used a slightly modified revised MIT license unless specifically stated for some rare cases, see detail here: https://github.com/Vaa3D/Vaa3D_Wiki/wiki/License_mBrainAligner
