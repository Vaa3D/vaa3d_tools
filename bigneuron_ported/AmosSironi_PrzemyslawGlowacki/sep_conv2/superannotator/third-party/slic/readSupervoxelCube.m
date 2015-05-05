%function L = readSupervoxelCube(labelFilenm, s)
%% readSupervoxelCube reads supervoxels data files
%   
%   L = readSupervoxelCube(labelFilenm, size) reads a supervoxel label data file 
%   and converts it into a matlab label matrix. Size of the image
%   must be specified in SIZE.
%

%   Copyright © 2009 Computer Vision Lab, 
%   École Polytechnique Fédérale de Lausanne (EPFL), Switzerland.
%   All rights reserved.
%
%   Authors:    Kevin Smith         http://cvlab.epfl.ch/~ksmith/
%               Aurelien Lucchi     http://cvlab.epfl.ch/~lucchi/
%
%   This program is free software; you can redistribute it and/or modify it 
%   under the terms of the GNU General Public License version 2 (or higher) 
%   as published by the Free Software Foundation.
%                                                                     
% 	This program is distributed WITHOUT ANY WARRANTY; without even the 
%   implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
%   PURPOSE.  See the GNU General Public License for more details.

labelFilenm = 'desktop'
%s = [1024 768];
s= [512 512];
%s= [192 130];
%s= [300 400];

fid = fopen(labelFilenm,'r');

for i=1:30
    i
    L{i} = fread(fid,[s(1) s(2)],'int');
    L{i} = double(L{i});
    %L = L+1;
    %imagesc(L{i});
    %pause;
end

fclose(fid);
