#!/bin/sh
cd `dirname $0` 
export LD_LIBRARY_PATH=/lustre/atlas/proj-shared/nro101/BigNeuron/Vaa3D_BigNeuron_version1
/lustre/atlas/proj-shared/nro101/BigNeuron/Vaa3D_BigNeuron_version1/vaa3d $*

