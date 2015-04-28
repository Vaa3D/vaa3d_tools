#!/bin/bash
# batch build all bigneuron related plugins for v2.940
# by Zhi Zhou

svn up

cd zhijiang_zn_bjut/fastmarching_spanningtree_vaa3d/
make clean
qmake
make
cd ../../

cd qul/anisodiffusion/general/
make clean
qmake
make
cd ../../../

cd qul/anisodiffusion/littlequick/
make clean
qmake
make
cd ../../../

cd zhouhang_hust/NeuroGPSTree/
make clean
qmake
make
