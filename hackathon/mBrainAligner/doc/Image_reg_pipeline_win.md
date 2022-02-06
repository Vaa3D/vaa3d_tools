
# mBrainAligner registration pipeline—windows version 

 <center>
  <img src= https://github.com/Vaa3D/vaa3d_tools/blob/master/hackathon/mBrainAligner/doc/step_by_step_tutorial/image000.jpg>

  </center>

Registration of whole-brain images of different modalities onto a standard atlas is essential for characterizing neuron types and constructing brain wiring diagrams. Here we introduce a step-by-step tutorial of using mBrainAligner to register the fMOST image to Allen Common Coordinate Framework atlas (CCFv3). 

Since the brain area in CCFv3 average template is close to image boundaries in the anterior-posterior direction, to eliminate the computational ambiguity, we padded the CCFv3 average template (25µm) by 20 pixels in both ends of anterior-posterior axes and use the padded image as the target. **Note that** only 50um CCFv3 and sample images are provided here to faciliate the network transfer and storage. The downsampled and padded CCFv3 average template can be found in `examples/target/CCF_25_u8_xpad.v3draw` (see Appendix for detailed meaning of files in ` examples/target/` folder). 

We assume all example and executable files are in the original folder structure as shown in the github. 


**Step 1: Image downsampling**

The raw whole mouse brain image stack of fMOST modality generally has the volume size of 40k x 30k x 10k voxels. First, we need to downsample the raw image anisotropically to roughly match the size of the target image (we recommend using CCFv3 25um to achieve a good balance between registration accuracy and computation time). 


**Step 2: Stripe artifact removal** 

The periodic stripe noise present in the raw fMOST images is mainly caused by fluorescent bleaching during the knife cutting and imaging process. It will deteriorate the image registration performance. This kind of periodic stripe noise can be effectively removed using our `stripe_removal` tool. This tool was implement in Matlab, and the source code can be found in `src\src_othertools\stripe_removal\`. 

Open Matlab and navigate to the ‘/src/src_othertools/stripe_removal/’ directory, and modify script:
```
foldername_input='../../../examples/subject/';
foldername_output='../../../examples/subject_stripe_removal_result/';
angle=175
cutoff=10
radius=10
```
Run `stripremove.m`. This script will find all `.raw` or `v3draw` files in the specified directory `../../../examples/subject/` and perform stripe removal and output the results images to directory `../../../subject_stripe_removal_result/`. 

We can determine the "angle (orientation), cutoff (cutoff frequency), radius (bandwidth)" parameters of notch filter by examining the frequency spectrum of one 2D coronal slice of subject image. 

**Note that** stripe_removal step may not be necessay if there is no tissue sectioning involved in the imaging process. 


**Step 3: Global affine registration and intensity normalization**

- open terminal in windows, and run the following commands：
  ```
  cd <your mBrainAligner_win64 directory>
  
  <your mBrainAligner_win64 directory>/global_registration.exe -f ../../examples/target/CCF_25_u8_xpad.v3draw -c ../../examples/target/CCF_mask.v3draw -m ../../examples/subject_stripe_removal_result/fMOST_18458_raw_stripremove.v3draw  -p r+f -o ../../examples/result/fMOST/ -d 1

  ```

The globally aligned image will be save in the `results` directory. 
If we use parameter `-p r+f+n`, the intensity normalization will be performed following the global registration. 

The detailed desciption of all avialble parameters can be found in `examples/run_script_windows.bat`.

  <center>
  <img src= https://github.com/Vaa3D/vaa3d_tools/blob/master/hackathon/mBrainAligner/doc/step_by_step_tutorial/image002.png width=70%>

  </center>


**(OPTIONAL) Perform global registration manually**

For partially imaged or damaged images, if you cannot obtain satisfactory results with automatic global registration, you try to implement it by providing matching points manually. 
1. Launch the Vaa3D software (http://penglab.janelia.org/proj/v3d);

2. Drag the target image (e.g. `examples/target/CCF_25_u8_xpad.v3draw`) and subject image (e.g. `examples/subject/fMOST_18458_raw.v3draw` into Vaa3D window. Once images are loaded, you will see the following window. 
  <center>
  <img src= https://github.com/Vaa3D/vaa3d_tools/blob/master/hackathon/mBrainAligner/doc/step_by_step_tutorial/image003.png width=70%>

  </center>

3.  Click `See in 3D` buton to display the image in 3D mode. Right-click the mouse and select the "2-right-clicks to define a maker" button to generate matching marker-pairs in two images. The number of marker is preferably more than 10, and then save the two sets of points as marker files (example: target_global.marker,sub_global.marker).
  <center>
  <img src= https://github.com/Vaa3D/vaa3d_tools/blob/master/hackathon/mBrainAligner/doc/step_by_step_tutorial/image004.png width=70% >

  </center>

4. open terminal in windows, and run the following commands：
```
cd <your mBrainAligner_win64 directory>
<your mBrainAligner_win64 directory>/global_registration.exe -f ../../examples/target/CCF_25_u8_xpad.v3draw -m ../../examples/subject/fMOST_18458_raw.v3draw  -p a -o ../../examples/result/fMOST/ -d 1 -t target_global.marker -s sub_global.marker

```
The image to be warpped (“examples/subject/fMOST_18458_raw.v3draw”), and the result image will be saved ("examples/result/fMOST/fMOST_18458_raw_affine.v3draw"). However, it is not necessary in our pipeline since the local registraion module can do it much better.

**Step 4: Local registration**

open terminal in windows, and run the following commands：
```
<your mBrainAligner_win64 directory>/local_registration.exe -p ../../examples/config/fMOST_config.txt -s ../../examples/result/fMOST/global.v3draw -m ../../examples/subject/fMOST_segmentation/ -l ../../examples/target/target_landmarks/low_landmarks.marker  -g ../../examples/target/ -o ../../examples/result/fMOST/
```
The local registration parameters are defined in `fMOST_config.txt`. Noted that if you don't have segmentation images, the `Select_modal` in the `fMOST_config.txt` needs to be set to 1. The detailed desciption of all avialble parameters can be found in `examples/run_script_windows.bat`.
The globally aligned image will be save in the `results` directory.
   <center>
  <img src= https://github.com/Vaa3D/vaa3d_tools/blob/master/hackathon/mBrainAligner/doc/step_by_step_tutorial/image008.png width=70% >

  </center>


**Appendix (meaning of files in `examples/target/` folder)**
1.	CCF_mask.v3draw

    The foreground mask of CCFv3 average template. It is generated using Otsu thersholding.

2.	CCF_contour.v3draw

    The out-coutour mask of CCFv3 average template. It is generated from CCF_mask.v3draw using morphological filters. 

3.	CCF_roi.v3draw

    Mask of interest brain regions. Run `src/src_othertools/process/annotation_recolor.ipynb` to generate the `CCF_roi.v3draw` image which contains the mask of  six mouse brain region (HY, HPF, CTX, CBX, BS and CP). You can still set the brain region of interest according to your needs. 
    The specific operations are: 
    - Select the number of the region you are interested in from `Mouse.csv`, for example, CTX is 688. 
    - Modify the `areas_ids` of "In [18]" in `annotation_recolor.ipynb`, and input the number of the brain region you need in `areas_ids`.

4.	fMOST_space_prior_sub.marker and fMOST_space_prior_tar.marker

    These files are optional in registration, it can only be used when registering fMOST mouse brain. It contains modality-specific shape priors that can improve the performance of local registration. One can obtain these markers files by first registering the fMOST average templete to the CCFv3 average template using automatic local reigistration module, and then finetuning the registration result using semi-automatic registration. The semi-automatic registration module will generates these markers files as output.

5.	target_landmarks

    The marker files in “target_landmarks” are predefined target landmarks used for local registration which are generated using 2.5D corner detector. Three different densities of landmark (low, middle, high) are provided. Generally, better registration performance can be obtained if more target landmarks are used, however, with the cost of computation time. 

