# Neuron Morphology Processing Lib
> author: Shengdian Jiang (SD-Jiang); date: 2023-03
- [Neuron Morphology Processing Lib](#neuron-morphology-processing-lib)
  - [Introduction](#introduction)
  - [Usage](#usage)
  - [Functions](#functions)

## Introduction
This plugin is designed for the processing of neuronal morphology (SWC/ESWC)

## Usage
> command line only
> 
NOTE: make sure neuron is a single connected tree with
1. one soma (type=1, pn=-1),
2. no loop,
3. no three bifurcations
## Functions            
- lm_feas: measure neuron morphological features
  - input file: swc file
  - output file: csv file
  - input para: 
    - nt_check: default is 0, no checking of the topological error
    - process_3_bifs: default is 0, ignore multiple bifurcations
  - features  (statistics: min, max, std, mean)
    - the number of stems
    - (overall) width, height, depth, volume, length
    - branch
      - number
      - path length
      - contraction / tortuosity
    - branching point:
      - {path,Euclidean}dist2soma
      - partition asymmetry
      - order
    - Angle:
        - bif_ampl_local and big_ampl_remote
        - bif_tilt_local and bif_tilt_remote
- branch_feas: measure branch level features
  - input file: swc file
  - output: csv file
  - input para: 
    - nt_check: default is 0, no checking of the topological error
    - process_3_bifs: default is 0, ignore multiple bifurcations
    - with_bouton: default is 0, not a bouton-swc file
  - features:
    - id,parent_id,x,y,z,type,level,dist2soma,path_dist2soma,angle,angle_remote,angle_io1,angle_io1_remote,angle_io2,angle_io2_remote,radius,lcradius,rcradius,length,pathlength,contraction,lclength,lcpathlength,rclength,rcpathlength,width,height,depth,lslength,lspathlength,rslength,rspathlength,lstips,rstips
    - if input is bouton swc file, will append the following features
      - bnum,pbnum,lcbnum,rcbnum,dist2nb,neigborbs,interb_dist,uniform_bdist
- swc2branches: split neuron tree into branches
  - input file: swc file
  - output: branch swc file
  - input para: 
    - type: default is 0, process to all type of branches
- qc
  - preprocess:
     * soma checking
     * loop checking
     * multiple bifurcation checking 
     * pruning small tip branch (tip branch length less than 6 pixels)
     * pruning internal node (distance less than 1.5 pixels)
     * resample node (4 pixels)
- nt_check
     * single tree checking
     * soma checking
     * loop checking
     * multiple bifurcation checking
- neuron_split: split neuron into dendrite,axon,apical_dendrite
  - input file: swc file
  - output: different types of swc file
- crop_swc_terafly_image_block: Base on swc boundingbox, crop image block and swc-in-block
  - input file: 
    - terafly image file path
    - swc file
  - output: output dir
  - input para
    - crop_neighbor_voxels
    - cropx
    - cropy
    - cropz
- bswcTo
  - Bouton swc file conversion
     * filetype=1, to apo file
     * filetype=2, to image marker file
     * otherwise, to apo and marker file


  