# Bouton Detection
> author: Shengdian Jiang; date: 2020-2021
---
- [Bouton Detection](#bouton-detection)
  - [> author: Shengdian Jiang; date: 2020-2021](#author-shengdian-jiang-date-2020-2021)
  - [Introduction](#introduction)
  - [Documentation](#documentation)
  - [update log](#update-log)
## Introduction
This plugin can detect axonal boutons based on axonal arbors. 
1. from image block
2. from whole-brain terafly datasets
## Documentation
+ Bouton file document
  + bouton swc file
    + radius
    + level (intensity)
    + type (99)
    + bouton features will be saved at SWC feature value list:
     * 1. bouton_flag {0: not a bouton; 1: bouton; 2: tip-bouton; 3: terminaux-bouton}
       * *en passant bouton: 1 and 2*
     * 2-5. bouton_radius,bouton_branch_radius_threshold,bouton_branch_radius_mean,bouton_branch_radius_std
     * 6-9. bouton_intensity, bouton_branch_intensity_threshold,bouton_branch_intensity_mean,bouton_branch_intensity_std
     * 10. bouton density
  + bouton apo file
    + volsize(radius)
    + intensity
    + color {r:0,g:20,b:255}
+ function list:(only for 8bit-3D image data)
  + For terafly datasets and image block
    + `BoutonDetection_terafly` or `BoutonDetection_image`
      + Workflow
        + get axon
        + refinement
        + linear Interpolation
        + swc profile
        + bouton filter
        + map bouton to neuron tree
        + near bouton pruning
        + map bouton to APO
        + save
      + parameters
        + bkg_thre_bias
        + refine_radius
        + Shift_Pixels
        + radius_delta
        + intensity_delta
        + axon_trunk_radius
        + allnode
        + half_crop_size
        + block_upsample_factor
    + `SWC_profile_terafly` or `SWC_profile`
      + Note: get Intensity,radius profile
        + preprocessing
          + use `Refinement` function to refine and interpolation
        + intensity
        + radius
          + crop image block from terafly datasets: block size= 128
          + background threshold: default= Mean(Image_intensity)+Std(Image_intensity)+bias(15)
          + compute at XY-2D-MIP (MIP_block_size=128x128x8)
          + upsample MIP block (x*4,y*4)
          + bkg_nodes ratio less than 1/1000
      + parameters
        + bkg_thre_bias: default=15
        + half_crop_size: default=128
        + block_upsample_factor: default=4
    + `RefinementAll_terafly` or `RefinementAll_image`
      + Note: this function combines `Refinement_terafly` and `NodeRefinement_terafly`
         > after refinement, line interpolation will be used
      + parameters
        + refine radius window: default=8
        + node refine radius: default=2
        + line interpolation pixels: default=3
        + half crop block size: default=128
      + `Refinement_terafly` or `Refinement_image`
        + Note: use mean-shift for refining reconstruction into center line 
           > background threshold is also used for filtering out background noise.
           bkg_thre=mean(image_intensity)
        + parameters:
          + refine radius window: default=8
      + `NodeRefinement_terafly` or `NodeRefinement_image`
        + Note: in a small area, shift SWC node into maximum intensity point
           >if more than one node, get mean of all the candidated nodes
        + parameters:
          + node refine radius: default=2
    + `Bouton_filter`
      + Note
        + swc to list of branches 
          + > (only process that the number of branch-nodes is more than 7 nodes)
          + > reading feature strategy: from child-tip to branch-head
        + peak detection of intensity and radius feature
        + get the nodes are both intensity_peak and radius_peak
        + filter
          + Intensity(bouton)> mean_branch_intensity+std_branch_intensity
          + Radius(bouton)> radius_delta* mean_branch_radius
        + remove near boutons (distance of two boutons is less than 4)
        + bouton swc internode prunning
        + tip redundancy nodes prunning
        + bouton type labeling {terminaux or en passant bouton}
        + bouton type rendering
        + save to apo and swc file
      + parameters
        + radius_delta(1.5)
        + intensity_delta (1)
        + axon_trunk_radius (2.5)
  + Image related
    + `TeraImage_SWC_Crop`
      + Note: crop image block and swc-nodes inside this block
      + Input
        + in-terafly-datasets
        + swc file
        + apo file
          + center of the cropped image block
      + para
        + crop size x
        + crop size y
        + crop size z
    + `UpsampleImage`
      + upsample image
      + para
        + up factor x
        + up factor y
        + up factor z
  + SWC processing
    + `BoutonSWC_pruning`
      + step1: near bouton removing
        + if two boutons from one branch are too close to each other, remove the one with small radius
      + step2: internode pruning
        + remove the regular nodes and keep the following nodes
          + soma,tip,bouton,branch nodes
        + node-2-node distance is more than 5 pixels.
      + step3: redendancy tip node pruning
    + `Scale_registered_swc`
      + scaleup registered swc, for mapping the registered file into CCF
        + x-20; {x,y,z}*25  
      + map onto bouton swc file
      + input file list
        + unregistered-swc file
        + registered swc file
## update log
1. update at 2020-08-01
    > Basic idea: get the intensity value of the reconstruction nodes and consider big intensity changes among child-parent nodes as bouton.
2. update at 2020-08-28
   1. split swc into block, mask the signal,refine the swc into center line: refinement
   2. get the candidate bouton
3. update at 2021-04-20
   >  peak detection was used for getting the initial axonal boutons
4. update at 2021-07~2021-08
   > add radius feature to bouton detection
5. update at 2021-08-10
   > reorganized code