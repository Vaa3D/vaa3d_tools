# mBrainAligner: Cross-Modality Coherent Registration of Whole Mouse Brains

![result_example](https://github.com/Vaa3D/vaa3d_tools/blob/master/hackathon/mBrainAligner/result_example.png)

# Contents

- [Overview](#Overview)
- [System requirements](#System-requirements)
   - [hardware requirements](#--Hardware-requirements)
   - [OS requirements](#--os-requirements)
- [Installation mBrainAligner](#Install-mBrainAligner)
   - [install precompiled executable files](#--Install-precompiled-executable-files)
   - [build mBrainAligner from source](#--Build-mBrainAligner-from-source)
- [Run mBrainAligner](#Run-mBrainAligner)
   - [run mBrainAligner on sample data in batch](#--run-mbrainaligner-on-sample-data-in-batch)
   - [step-by-step tutorial](#--Step-by-step-tutorial)
   - [run mBrainAligner on you own data](#--run-mbrainaligner-on-your-own-data)
- [Others](#Others)
   - [warp reconstructed neurons or somas to CCF](#--Warp-reconstructed-neurons-or-somas-to-CCF)
   - [use 3D UNet to generate segmentation features](#--use-3d-unet-to-generate-segmentation-features)
- [License](#license)

# Overview

Recent whole brain mapping projects are collecting large-scale 3D images using powerful and informative modalities, such as STPT, fMOST, VISoR, LSFM or MRI. Registration of these multi-dimensional whole-brain images onto a standard atlas is essential for characterizing neuron types and constructing brain wiring diagrams. However, cross-modality image registration is challenging due to intrinsic variations of brain anatomy and artifacts resulted from different sample preparation methods and imaging modalities.

`mBrianAligner` provides a cross-modality image registration pipeline to support whole brain mapping projects. In addition to aligning 3D mouse brain images of different modalities,`mBrainAligner` also enables mapping digitally reconstructed compartments (e.g., dendritic, axonal, and soma distributions) to a common target space to faciliate the visualization, comparison and analysis. Using `mBrainAligner`, we have also generated an fMOST-space mouse brain atlas and showcased the utility of our method for analyzing single cell types. `mBrainAligner` is not limited to the use of intra- and cross-modality mouse brain registration, it may also be extended to partially imaged data and multi-timepoint registration or other species.

`mBrainAligner` contains three modules: (1) image preprocessing and global registration, (2) Coherent Landmark Mapping (CLM) based automatic registration, and (3) optional semi-automatic refinement. To accommodate different registration accuracy or throughput requirements, the above modules can be concatenated or executed separately. In addition, some useful tools including 2.5D corner detector, stripe artifacts removal, and image or metadata warping tools are also provided in this package for the user's convenience.

There are two ways to use `mBrainAligner`:
1. follow the below instructions to build and run `mBrainAligner` on local machines.
2. visit one of our Tencent cloud web portals, run `mBrainAligner online` (registration results and sample data can also be downloaded there).
Questions can be sent to 17718151568@126.com . 
China South (Guangzhou): http://159.75.201.35/ . China South (AHU): http://mbrainaligner.ahu.edu.cn
Note: This cloud server is provided for tentative testing purpose only while we are moving to a more permanent solution. Different ISPs might have different access restrictions to some of these websites, and different web browsers might also show warning messages when naked IP addresses (i.e. without a domain name) are visited. You may choose to change the ISPs (such as switching among Verion, AT&T, Xfinity, etc), change your 5G/4G setting (sometimes with the same ISP still has different restriction for 5G and 4G), or ignore the warning messages. Feel to reach out to the above email address if you also want to deploy the web service code on your own local server for faster access.


# System requirements

## - Hardware requirements

`mBrainAligner` works on desktop computers. The memory requirement depends on the size of the image to be processed. For 25um mouse brain registration, we recommend a computer with the following specifications:

RAM: 32 GB or larger;

CPU: 2.3GHz dual-10-core Intel Xeon or better.

(Note: Due to limited network and storage, only small down-sampled brain images (~50um) are provided here. For better registration accuracy, we encourage the user to download raw data from the scalable Brain Atlas website https://scalablebrainatlas.incf.org/mouse/ABA_v3#about or contact us by email qulei@ahu.edu.cn. If you want to train the 3D U-Net, Nvidia RTX 3090(24G) or better GPU is needed. We have provided the network segmentation results for fMOST to relieve the GPU requirement)


## - OS Requirements

The package has been tested on the following systems:

- Windows 10 64bit
- Linux (e.g. Ubuntu)


# Install mBrainAligner

## - Install precompiled executable files

If you only want to test or use `mBrainAligner`, the simplest way is to use the provided executable files. In this case, no installation is required, just download the `binary` folder and put it anywhere you like.

>**binary**
> > **win64_bin**: contains the global and local registration, 2.5D corner detector and neuron or soma warping related executable files and `dlls` for Windows.
> >
> > **linux_bin**: contains the global and local registration executable files and `libs` for Linux.
> >
> > The stripe removal tool is implement in Matlab, you can find the code in `src/src_othertools/stripe_removal/`. 
> > The precompiled semi-automatic registration program for windows can be found in `src/semi_automatic_registration/semi_automatic_registration.7z`.

## - Build mBrainAligner from source

If you want to build `mBrainAligner` from source, please follow the instructions here: [How to build mBrainAligner](https://github.com/Vaa3D/vaa3d_tools/blob/master/hackathon/mBrainAligner/doc/How%20to%20build%20mBrainAligner.md)

Note that mBrainAligner has been only tested on Windows 10 and Linux.

# Run mBrainAligner

We provide executable files, scripts and sample data (downsampled `fMOST`,`VISoR`,`MRI`,`LSFM` mouse brain images (~50um)) for running `mBraiAligner` on Windows and Linux platforms. You can find compiled executable files in `binary`, and scripts, sample data and config files in `examples`. In `examples/registered_results` folder, the registration results of brains of four modalities are also provided. Note that due to the size of brain images, only small down-sampled brains are provided here (even part of the data can only be compressed into the ".zip" format, so you need to make sure that the data has been decompressed before running any script). For better registration accuracy, 25um image resolution is recommanded and we encourage the user to download raw data from the scalable Brain Atlas website https://scalablebrainatlas.incf.org/mouse/ABA_v3#about or contact us by email qulei@ahu.edu.cn.

An overview of the pipeline is shown below. For a deeper understanding of each module please read the paper: <https://www.researchsquare.com/article/rs-321118/v1>.

![overview_pipeline](https://github.com/Vaa3D/vaa3d_tools/blob/master/hackathon/mBrainAligner/mBrianAligner_workflow.jpg)

## - Run mBrainAligner on sample data in batch

We provide scripts to globally and locally register all sample data of four modalities in batch. First, you need to create a local copy of executable files, scripts and sample data. Make sure they are in original folder structure.

Then, follow the below instructions to run script on Windows and Linux respectively. The global and local registration of one brain will finish in about 0.5 hours (If the default parameters are chosen). All results will be save to `results` folder. It will take about two hours to complete the registration of four mouse brain of different modalities.

### Windows

Enter the `examples` folder, double-click `run_script_windows.bat`.

### Linux

In Linux, you need to first complete a simple environment configuration and unzip step. The path for the lib files `binary/linux_bin/lib/` needs to be added to the linker directory.

Ctrl+Alt+t to open terminal

    cd "(your file path)/binary/linux_bin/"
    tar -zxvf lib.tar.gz
    sudo gedit /etc/ld.so.conf
    add the "(your file path)/binary/linux_bin/lib/"  to the last line of '/etc/ld.so.conf' and save.
    sudo ldconfig
    sudo chmod 777 -R  global_registration
    sudo chmod 777 -R  local_registration
    cd "(your file path)/example/"

Then run

    sh run_script_linux.sh


## - Step-by-step tutorial

For a step-by-step user guide, please read: [Image registration pipeline tutorial](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner/doc/Image_reg_pipeline_win.md)


## - Run mBrainAligner on your own data

If you want to practice `mBrainAligner` on your own data, please read: [Register your own data](https://github.com/Vaa3D/vaa3d_tools/blob/master/hackathon/mBrainAligner/doc/register%20your%20own%20data.md)

We encourage you to read the `Step-by-step tutorial` and `run_script_windows.bat` first.  


# Others

## - Warp reconstructed neurons or somas to CCF

Once the brain images were registered to CCF, the neurons (.swc format) or somas (.marker format) can also be map to CCF space for visualization, comparison and analysis. Swc_registration tools `src/src_othertools/swc_registration/` can be used for this purpose, please read [SWC registration pipeline tutorial](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner/doc/SWC_reg_pipeline_win.md) for detail.

## - Use 3D UNet to generate segmentation features

With `mBrainAligner`, one can integrate different features to enhance the robustness and accuracy of registration. Since brain delineation/segmentation is one major application of atlas-based registration, the registration task can also benefit from segmentation by incorporating the semantic information as a feature.

DNNs have shown tremendous superiority against traditional methods in segmentation applications. We choose to generate the segmentation probability feature using a semantic segmentation network. We slightly modify a 3D U-Net to generate the segmentation probability (0~1) of each voxel to six main brain regions (HY, HPF, CTX, CBX, BS, CP) and background. Also note, 3D U-Net may be replaced by other segmentation methods to further improve the registration performance, mBrainAligner can be improved for specific applications using such an approach.

The source code of our modified 3D U-Net can be found in `src/src_3DUnet/`. Please read [Use 3D UNet to generate segmentation features](https://github.com/Vaa3D/vaa3d_tools/blob/master/hackathon/mBrainAligner/doc/3D_U-Net.md) for details of data preparation, training and inference.


# License
`mBrainAligner` uses a slightly modified revised MIT license unless specifically stated for some rare cases. See detail here: https://github.com/Vaa3D/Vaa3D_Wiki/wiki/License_mBrainAligner
