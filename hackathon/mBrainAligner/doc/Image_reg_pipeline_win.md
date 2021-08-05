
# mBrainAligner registration pipeline—windows version 

![image000](https://github.com/Vaa3D/vaa3d_tools/blob/master/hackathon/mBrainAligner/doc/step_by_step_tutorial/image000.jpg)

Registration of whole-brain images of different modalities onto a standard atlas is essential for characterizing neuron types and constructing brain wiring diagrams. Here we introduce a step-by-step tutorial of using mBrainAligner to register the fMOST image to Allen Common Coordinate Framework atlas (CCFv3). 

Since the brain area in CCFv3 average template is close to image boundaries in the anterior-posterior direction, to eliminate the computational ambiguity, we padded the CCFv3 average template (25µm) by 20 pixels in both ends of anterior-posterior axes and use the padded image as the target. The padded CCFv3 average template can be found in `examples/target/CCF_25_u8_xpad.v3draw` (see Appendix for detailed meaning of files in ` examples/target/` folder). 

We assume all example and executable files are in the original folder structure as shown in the github. 

**Step 1: Image downsampling**

The raw whole mouse brain image stack of fMOST modality generally has the volume size of 40k x 30k x 10k voxels. First, we need to downsample the raw image anisotropically (XYZ multipliers = 64x64x16) to roughly match the size of the target image (CCFv3 25um). 

**Step 2: Stripe artifact removal** 

The periodic stripe noise present in the raw fMOST images is mainly caused by fluorescent bleaching during the knife cutting and imaging process. It will deteriorate the image registration performance. We designed a log-space frequency notch filter to realize the high-quality stripe artifacts removal. This module was implement in Matlab, and the source code can be found in `src\src_othertools\stripe_removal\`. 

Open Matlab and navigate to the ‘/src/src_othertools/stripe_removal/’ directory, and run the following command:
```
stripremove('../../../examples/subject/ ', '../../../examples/subject_stripe_removal/', 175, 10, 10)
```

This command will find all `.raw` or `v3draw` files in the specified directory `../../../examples/subject/` and perform stripe removal and output the results images to directory `../../../examples/subject_stripe_removal/`. 

We can determine the "angle (orientation), cutoff (cutoff frequency), radius (bandwidth)" parameters of notch filter by examining the frequency spectrum of one 2D coronal slice of subject image. 



**Step 3: Global affine registration and intensity normalization**


- open terminal in windows, and run the following commands：
  ```
  cd <your mBrainAligner_win64 directory>
  
  global_registration.exe -f ../example/target/CCF_25_u8_xpad.v3draw -c ../example/target/CCF_mask.v3draw -m ../example/subject_stripe_removal/fMOST_18458_stripremove.v3draw  -p r+f -o ../example/result/fMOST/ -d 1

  ```


The globally aligned image will be save in the `results` directory. 
If we use parameter `-p r+f+n`, the intensity normalization will be performed following the global registration. 

  <center>
  <img src= https://github.com/Vaa3D/vaa3d_tools/blob/master/hackathon/mBrainAligner/doc/step_by_step_tutorial/image002.png width=50%>

  </center>


**(OPTIONAL) Perform global registration manually**

For partially imaged or damaged images, if you cannot obtain satisfactory results with automatic global registration, you try to implement it by providing matching points manually. 
1. Launch the Vaa3D software (http://penglab.janelia.org/proj/v3d);

2. Drag the target image (e.g. `examples/target/CCF_25_u8_xpad.v3draw`) and subject image (e.g. `examples/subject/fMOST_18458_raw.v3draw` into Vaa3D window. Once images are loaded, you will see the following window. 
  <center>
  <img src= https://github.com/Vaa3D/vaa3d_tools/blob/master/hackathon/mBrainAligner/doc/step_by_step_tutorial/image003.png width=50% >

  </center>

3.  Click `See in 3D` buton to display the image in 3D mode. Right-click the mouse and select the "2-right-clicks to define a maker" button to generate matching marker-pairs in two images. The number of marker is preferably more than 10, and then save the two sets of points as marker files.
  <center>
  <img src= https://github.com/Vaa3D/vaa3d_tools/blob/master/hackathon/mBrainAligner/doc/step_by_step_tutorial/image004.png width=50% >

  </center>

4.	Click menu <Plug-In/image_registation/alignment_affine_TPS_warp/warp image based on given matched point pairs >, a dialog will pop up. 
  <center>
  <img src= https://github.com/Vaa3D/vaa3d_tools/blob/master/hackathon/mBrainAligner/doc/step_by_step_tutorial/image005.png width=25% ><img src= https://github.com/Vaa3D/vaa3d_tools/blob/master/hackathon/mBrainAligner/doc/step_by_step_tutorial/image006.png width=25% >

  </center>

5.	In this dialog, choose marker file (that is, the marker  saved in the fourth step), and the image to be warpped (“examples/subject/fMOST_18458_raw.v3draw”), and the output file name. Then click `OK`.

**Step 4: Local registration**

open terminal in windows, and run the following commands：
```
local_registration.exe -p ../example/config/fMOST_config.txt -s ../example/result/fMOST/global.v3draw -m ../example/subject/fMOST_segmentation/ -l ../example/target/target_landmarks/low_landmarks.marker  -g ../example/target/ -o ../example/result/fMOST/
```
The local registration parameters are defined in `fMOST_config.txt`. Noted that if you don't have segmentation images, the “Select_modal” in the “fMOST_config.txt” needs to be set to 1. 
The globally aligned image will be save in the `results` directory.
   <center>
  <img src= https://github.com/Vaa3D/vaa3d_tools/blob/master/hackathon/mBrainAligner/doc/step_by_step_tutorial/image007.png width=50% >

  Fig3 fMost brain segmentation image
  </center>


**Appendix (meaning of files in `examples/target/` folder)**
1.	CCF_mask.v3draw

    The foreground mask of CCFv3 average template. It is generated using Otsu thersholding.

2.	CCF_contour.v3draw

    The out-coutour mask of CCFv3 average template. It is generated from CCF_mask.v3draw using morphological filters. 

3.	CCF_roi.v3draw

    Mask of interest brain regions. Run “src/src_othertools/process/annotation_recolor.ipynb” to get the “CCF_roi.v3draw” image which only contains six mouse brain region (HY, HPF, CTX, CBX, BS and CP). You can still set the brain region of interest according to your needs. 
    The specific operations are: 
    - Select the number of the region you are interested in from “Mouse.csv”, for example, CTX is 688. 
    -  Modify the “areas_ids” of "In [18]" in annotation_recolor.ipynb, and put the number of the brain region you need in “areas_ids”.

4.	“fMOST_space_prior_sub.marker” and “fMOST_space_prior_tar.marker”

    This file is optional in registration; it can only be used when registering fMOST mouse brain. It contains modality-specific shape priors that can improve the performance of local registration. The method to obtain these two markers files is by registering the fMOST average templete image to the CCFv3 average template image, and then obtaining an accurate registration result by semi-automatic registration, and generating this two markers files at the same time. Registration the template brain to the CCFv3 can be obtained according to the next section “Step-by-step instructions to run mBrainAligner algorithm”.

5.	target_landmarks

    The marker files in “target_landmarks” are predefined target landmarks used for local registration which are generated using 2.5D corner detector. Three different densities of landmark (low, middle, high) are provided. Generally, better registration performance can be obtained if more target landmarks are used, however, with the cost of computation time. 

