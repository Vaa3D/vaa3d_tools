# Bouton Detection
> author: Shengdian Jiang; date: 2020-2021
---
- [Bouton Detection](#bouton-detection)
  - [> author: Shengdian Jiang; date: 2020-2021](#author-shengdian-jiang-date-2020-2021)
  - [Introduction](#introduction)
  - [Functions](#functions)
  - [update log](#update-log)
## Introduction
This plugin can detect axonal boutons based on axonal arbors. 
1. from image block
2. from whole-brain terafly datasets
## Functions
+ BoutonDetection
  + `BoutonDetection_terafly`
  + `BoutonDetection_image`
  + `Bouton_filter`
+ Refinement
  + Reconstruction refinement
    + `Refinement_terafly`
    + `Refinement_image`
  + Node refinement
    + `NodeRefinement_terafly`
    + `NodeRefinement_image`
+ Image related
  + `TeraImage_SWC_Crop`
  + `UpsampleImage`
+ SWC processing
  + `BoutonSWC_Compress`
  + `Scale_registered_swc`
+ Other
  + `Scale_registered_swc`
  + `SWC_Analysis`
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