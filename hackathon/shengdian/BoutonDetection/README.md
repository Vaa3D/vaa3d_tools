# Bouton Detection
> author: Shengdian Jiang; date: 2020-2021
---
## Introduction
This plugin can detect axonal boutons based on axonal morphology. 
1. from image block
2. from whole-brain terafly datasets
## update log
1. update at 2020-08-01
  > Basic idea: get the intensity value of the reconstruction nodes and consider big intensity changes among child-parent nodes as bouton.
2. update at 2020-08-28
   1. split swc into block, mask the signal,refine the swc into center line: refinement
   2. get the candidate bouton
3. update at 2021-04-20
   >  peak detection was used for getting the initial axonal boutons