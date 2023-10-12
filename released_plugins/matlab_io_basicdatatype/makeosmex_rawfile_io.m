%% This file is generated in case I forget the right way to compile next
%% time, :-)
%%
%%by Hanchuan Peng
%% 2007-03-06
% last update: 090722: by Hanchuan Peng. This makes it easier to compile all mex files under this directory
%% 2011-03-23: change to -lv3dtiff

mex loadRaw2Stack_c.cpp mg_image_lib.cpp mg_utilities.cpp -L../v3d_main/common_lib/lib -lv3dtiff
mex saveStack2File_c.cpp mg_image_lib.cpp mg_utilities.cpp -L../v3d_main/common_lib/lib -lv3dtiff

mex checkMachineEndian.cpp


