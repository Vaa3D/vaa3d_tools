# mBrainAligner: Cross-Modality Coherent Registration of Whole Mouse Brains

![result_example](https://github.com/Vaa3D/vaa3d_tools/blob/master/hackathon/mBrainAligner/result_example.png)

# Contents

- [Overview](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner#Overview)
- [System requirements](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner#System-requirements)
- - [hardware requirements](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner##-Hardware-requirements)
- - [OS requirements](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner##-OS-requirements)
- [Installation mBrainAligner](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner#Install-mBrainAligner)
- - [install precompiled executable files](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner##-Use-provided-compiled-executable-files)
- - [build mBrainAligner from source](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner##-Build-mBrainAligner-from-source)
- [Run mBrainAligner](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner#Run-mBrainAligner)
- - [run mBrainAligner on sample data in batch](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner##-Run-mBrainAligner-on-sample-data-in-batch)
- - [step-by-step tutorial](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner##-Step-by-step-tutorial)
- - [run mBrainAligner on you own data](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner##-Run-mBrainAligner-on-you-own-data)
- [Others](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner#Others)
- - [warp reconstructed neurons or somas to CCF](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner##-Warp-reconstructed-neurons-or-somas-to-CCF)
- - [use 3D UNet to gengerate segmentation features](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner##-Use-3D_UNet_to_gengerate_segmentation_features)
- [License](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner#License)

# Overview

Recent whole brain mapping projects are collecting large-scale 3D images using powerful and informative modalities, such as STPT, fMOST, VISoR, LSFM or MRI. Registration of these multi-dimensional whole-brain images onto a standard atlas is essential for characterizing neuron types and constructing brain wiring diagrams. However, cross-modality image registration is challenging due to intrinsic variations of brain anatomy and artifacts resulted from different sample preparation methods and imaging modalities.

mBrianAligner provides a comprehesive, robust and accurate cross-modality image registration pipeline to support the current large-scale whole brain mapping projects. In addition to aligning 3D mouse brain images of different modalities, mBrainAligner also enables mapping digitally reconstructed compartments (e.g., dendritic, axonal, and soma distributions) to a common target space to faciliate the visualization, comparison and analysis. Using mBrainAligner, we have also generated an fMOST-space mouse brain atlas and showcased the utility of our method for analyzing single cell types. mBrainAligner is not limited to the use of intra- and cross-modality mouse brain registration, its utilities can also be extended to partially imaged data and multi-timepoint registration or other species.

mBrainAligner contains three decoupled modules: (1) image preprocessing and global registration, (2) Coherent Landmark Mapping (CLM) based automatic registration, and (3) optional semi-automatic refinement. To accommodate different registration accuracy or throughput requirements, the above modules can be concatenated or executed separately. In addition, some useful tools including 2.5D corner detector, stripe artifacts removal, and image or metadata warping tools are also provided in this package for the user's convenience.

There are two ways to use mBrainAligner:
1.	follow the below instructions to build and run mBrainAligner on local machines.
2.	visit http://159.75.201.35/ and run mBrainAligner online. (registration results and sample data can also be downloaded there)
Please feel free to contact us by email (17718151568@126.com) any time for any question you have.


# System requirements

## - Hardware requirements

`mBrainAligner` works on desktop computers. The memory requirement depends on the size of the image to be processed. For 25um mouse brain registration, we recommend a computer with the following specifications: 

RAM: 32 GB or larger;

CPU: 2.3GHz dual-10-core Intel Xeon or better.

(Note: Due to limited network and storage, only small downsampled brain images are provided here. For better registration accuracy, we encourage the user to download raw data from the scalable Brain Atlas website https://scalablebrainatlas.incf.org/mouse/ABA_v3#about or contact us by email qulei@ahu.edu.cn. If you want to train the 3D U-Net, Nvidia RTX 3090(24G) or better GPU is needed. We have provided the network segmentation results for fMOST to relief the GPU requirement)


## - OS Requirements

The package has been tested on the following systems:

- Windows 10 64bit
- Linux (e.g. Ubuntu)


# Install mBrainAligner

## - Use provided compiled executable files

If you only want to test or use `mBrainAligner`, the simplest way is to use the provided executable files. In this case, no installation is required, just download the `dist` folder and put it anywhere you like.

>**dist**
> > **mBrainAligner_win64**: contains the global and local registration executable files and `dlls` for Windows.
> > 
> > **mBrainAligner_linux**: contains the global and local registration executable files and `libs` for Linux.
> > 
> > **othertools_win64**: contains the neuron or soma warping tool, 2.5D corner detector executable file and necessary libraries. Note that the stripe removal tool is implement in Matlab, you can find the code in `src/src_othertools/stripe_removal/`.
> > 

## - Build mBrainAligner from source

If you want to build mBrainAligner from source, please follow the instructions here: https://****************************


# Run mBrainAligner

We provide executable files, scripts and sample data (downsampled `fMOST`,`VISoR`,`MRI`,`LSFM` mouse brain images) for running `mBraiAligner` on Windows and Linux platforms. You can find compiled executable files in `dist`, and scripts, sample data and config files in `examples`. In `examples/registered_results` folder, the registration results of brains of four modalities are also provided. Note that due to the size of brain images, only small downsampled brains are provided here. 

An overview of the pipeline is shown below. For a deeper understanding of each module please read the *********Paper link to add...*********

![overview_pipeline](https://github.com/Vaa3D/vaa3d_tools/blob/master/hackathon/mBrainAligner/mBrianAligner_workflow.jpg)

## - Run mBrainAligner on sample data in batch

We provide scripts to globally and locally register all sample data of four modalities in batch. First, you need to create a local copy of executable files, scripts and sample data. Make sure they are in original folder structure. 

Then, follow the below instructions to run script on Windows and Linux respectively. The global and local registration of one brain will finish in about 0.5 hours (If the default parameters are chosen). All results will be save to `results` folder. It will take about two hours to complete the registration of four mouse brain of different modalities.

### Windows

Enter the `examples` folder, double-click `run_script_windows.bat`. 

### Linux

In Linux, you need to first complete a simple environment configuration and unzip step. The path for the lib files `dist/mBrainAligner_linux/lib/` needs to be added to the linker directory: 
    
    Ctrl+Alt+t to open terminal 
    tar -zxvf lib.tar.gz
    sudo gedit /etc/ld.so.conf
    add the "(your file path)/mBrainAligner_data/3rdparty_linux/lib"  to the last line of '/etc/ld.so.conf' and save. 
    sudo ldconfig
    
Then run
   
    sh run_script_linux.sh 


## - Step-by-step tutorial

For a step-by-step user guide, please read: `“link: step-by-step registration tutorial”`


## - Run mBrainAligner on you own data

If you want to practice mBrainAligner on you own data, please read: `“link: register your own data”`

We encourage you to read the `Step-by-step tutorial` and `Scripts` first.

# Others

## - Warp reconstructed neurons or somas to CCF

Once the brain images were registered to CCF, the neurons (.swc format) or somas (.marker format) can also be map to CCF space for visualization, comparison and analysis. Swc_registration tools `dist\othertools_win64\swc_registration\` can be used for this purpose, please read [SWC registration pipeline tutorial](https://github.com/Vaa3D/vaa3d_tools/tree/master/hackathon/mBrainAligner/docs/SWC_reg_pipeline_win.md) for detail. 

## - `(OPTIONAL)` Use 3D UNet to gengerate segmentation features

One merit of `mBrainAligner` is that different features can be effectively integrated and utilized to enhance the robustness and accuracy of registration. Since brain delineation/segmentation is one major application of atlas-based registration, the registration task can also benefit from segmentation by incorporating the semantic information as one of the discriminative feature.

Considering that DNNs have shown tremendous superiority against traditional methods in segmentation applications, we choose to generate the segmentation probability feature using a semantic segmentation network. As a proof of principle, we adopted a slightly modified 3D U-Net to generate the segmentation probability (0~1) of each voxel to six main brain regions (HY, HPF, CTX, CBX, BS, CP) and background. Indeed, 3D U-Net can be readily substituted with other more sophisticated semantic segmentation networks to further improve the registration performance.

The source code of our modified 3D U-Net can be found in `src/src_3DUnet/`. Please read `“link: Use 3D UNet to gengerate segmentation features”` for detailed data preparation, trainning and inference of this network.


# License
mBrainAligner uses a slightly modified revised MIT license unless specifically stated for some rare cases. See detail here: https://github.com/Vaa3D/Vaa3D_Wiki/wiki/License_mBrainAligner
