function Volume=polygon2voxel(FV,VolumeSize)
% polygon2voxel will convert a triangulated mesh into a
% voxel volume which will contain the discretized mesh. Discretization of a 
% polygon is done by splitting/refining the face, until the longest edge
% is smaller than 0.5 voxels. Followed by setting the voxel beneath the vertice 
% coordinates of that small triangle to one.
%
% Volume=polygon2voxel(FV,VolumeSize,Mode,Yxz);
%
% Inputs,
%   FV : A struct containing FV.faces with a facelist Nx3 and FV.vertices
%        with a Nx3 vertice list. Such a structure is created by Matlab
%        Patch function
%   VolumeSize : The size of the output volume, example [100 100 100]
%   Mode : (optional) if set to:
%               'none', The vertices data is directly used as coordinates
%                       in the voxel volume.
%               'wrap', The vertices data is directly used as coordinates
%                       in the voxel volume, coordinates outside are 
%                       circular wrapped to the inside.
%               'auto', The vertices data is translated and 
%                       scaled with a scalar to fit inside the new volume.
%               'center', coordinate 0,0,0 is set as the center of the volume
%                       instead of the corner of the voxel volume.
%               'clamp', The vertices data is directly used as coordinates
%                       in the voxel volume, coordinates outside are 
%                       clamped to the inside.
%   (Optional)
%   Yxz : If true (default) use Matlab convention 1th dimension Y, 
%         2th dimension X, and last dimension Z. Otherwise 1th 
%         dimension is X, 2th Y and last Z.
%
%                        
% Outputs,
%   Volume : The 3D logical volume, with all voxels part of the discretized
%           mesh one, and all other voxels zero.

if ~exist('hollow','var')
    hollow = false;
end

if(nargin<4), Yxz=true; end
    
% Check VolumeSize size
if(length(VolumeSize)==1)
    VolumeSize=[VolumeSize VolumeSize VolumeSize];
end
if(length(VolumeSize)~=3)
    error('polygon2voxel:inputs','VolumeSize must be a array of 3 elements ')
end

% Volume Size must always be an integer value
VolumeSize=round(VolumeSize);

sizev=size(FV.vertices);
% Check size of vertice array
if((sizev(2)~=3)||(length(sizev)~=2))
    error('polygon2voxel:inputs','The vertice list is not a m x 3 array')
end

sizef=size(FV.faces);
% Check size of vertice array
if((sizef(2)~=3)||(length(sizef)~=2))
    error('polygon2voxel:inputs','The vertice list is not a m x 3 array')
end

% Check if vertice indices exist
if(max(FV.faces(:))>size(FV.vertices,1))
    error('polygon2voxel:inputs','The face list contains an undefined vertex index')
end

% Check if vertice indices exist
if(min(FV.faces(:))<1)
    error('polygon2voxel:inputs','The face list contains an vertex index smaller then 1')
end

% Matlab dimension convention YXZ
if(Yxz)
    FV.vertices=FV.vertices(:,[2 1 3]); 
end

switch(lower(mode(1:2)))
    case {'au'} % auto
        % Make all vertices-coordinates positive
        FV.vertices=FV.vertices-min(FV.vertices(:));
        scaling=min((VolumeSize-1)./(max(FV.vertices(:))));
        % Make the vertices-coordinates to range from 0 to 100
        FV.vertices=FV.vertices*scaling+1;
        Wrap=0;
    case {'ce'} % center
        % Center the vertices
        FV.vertices=FV.vertices+repmat((VolumeSize/2),size(FV.vertices,1),1);
        Wrap=0;
    case {'wr'} %wrap
        Wrap=1;
    case{'cl'} % clamp
        Wrap=2;
    otherwise
        Wrap=0;
end

% Separate the columns;
FacesA=double(FV.faces(:,1));
FacesB=double(FV.faces(:,2));
FacesC=double(FV.faces(:,3));
VerticesX=double(FV.vertices(:,1));
VerticesY=double(FV.vertices(:,2));
VerticesZ=double(FV.vertices(:,3));

% Volume size to double
VolumeSize=double(VolumeSize);

% Call the mex function
Volume=polygon2voxel_double(FacesA,FacesB,FacesC,VerticesX,VerticesY,VerticesZ,VolumeSize,Wrap);

if ~hollow
    for i=1:size(Volume,3)
        Volume(:,:,i) = imfill(Volume(:,:,i),'holes');
    end
end