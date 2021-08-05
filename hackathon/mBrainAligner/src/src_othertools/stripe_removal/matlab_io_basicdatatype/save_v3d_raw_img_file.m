function save_v3d_raw_img_file(img, filename)
%function save_v3d_raw_img_file(img, filename)
%
% Write an image stack (4D array of UINT8, UINT16, or SINGLE type) to
% the .RAW or .TIF image stack files (depending on the suffix given) that are supported by V3D
% 
%
% V3D website: see software page of http://penglab.janelia.org
%
% by Hanchuan Peng
% 20090724

saveStack2File_c(img, filename);
