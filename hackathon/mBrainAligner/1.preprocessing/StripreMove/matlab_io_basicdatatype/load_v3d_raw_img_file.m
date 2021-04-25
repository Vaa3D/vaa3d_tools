function img = load_v3d_raw_img_file(filename)
%function img = load_v3d_raw_img_file(filename)
%
% Load the .RAW or .TIF or (even .LSM) image stack files that are supported by V3D
% 
%
% V3D website: see software page of http://penglab.janelia.org
%
% by Hanchuan Peng
% 20090724

img = loadRaw2Stack(filename);
