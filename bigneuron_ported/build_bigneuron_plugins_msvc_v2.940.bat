:: batch build all BigNeuron plugin projects for v2.940 using a VC Studio complier's nmake function
:: (window version) by Zhi Zhou
:: 2015-04-27
:: revised from the original plugin bat file

svn up

cd zhijiang_zn_bjut\fastmarching_spanningtree_vaa3d\
nmake clean
qmake
nmake -f Makefile.Release
cd ..\..\

cd qul\anisodiffusion/general\
nmake clean
qmake
nmake -f Makefile.Release
cd ..\..\..\

cd qul\anisodiffusion\littlequick\
nmake clean
nmake -f Makefile.Release



