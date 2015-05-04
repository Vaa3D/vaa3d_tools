% Parallel medial axis thinning of a 3D binary volume
%
% MATLAB vectorized implementation of the algorithm by
% Ta-Chih Lee, Rangasami L. Kashyap and Chong-Nam Chu
% "Building skeleton models via 3-D medial surface/axis thinning algorithms."
% Computer Vision, Graphics, and Image Processing, 56(6):462–478, 1994.
%
% Inspired by the ITK implementation by Hanno Homann
% http://hdl.handle.net/1926/1292
% and the Fiji/ImageJ plugin by Ignacio Arganda-Carreras
% http://fiji.sc/wiki/index.php/Skeletonize3D

function skel = Skeleton3D(img,spare)

disp('computing medial axis............................');

% pad volume with zeros to avoid edge effects
skel=padarray(img,[1 1 1]);

if(nargin==2)
    spare=padarray(spare,[1 1 1]);
end;

% number of foreground voxels
l_orig = length(find(skel(:)));

% fill lookup table
eulerLUT = FillEulerLUT;

% number of iterations
iter = 1;

global width height depth

width = size(skel,1);
height = size(skel,2);
depth = size(skel,3);

unchangedBorders = 0;

while( unchangedBorders < 6 )  % loop until no change for all six border types
    unchangedBorders = 0;
    for currentBorder=1:6 % loop over all 6 directions
        cands=zeros(width,height,depth);
        switch currentBorder
            case 4,
                x=2:size(skel,1); % identify border voxels as candidates
                cands(x,:,:)=skel(x,:,:) - skel(x-1,:,:);
            case 3,
                x=1:size(skel,1)-1;
                cands(x,:,:)=skel(x,:,:) - skel(x+1,:,:);
            case 1,
                y=2:size(skel,2);
                cands(:,y,:)=skel(:,y,:) - skel(:,y-1,:);
            case 2,
                y=1:size(skel,2)-1;
                cands(:,y,:)=skel(:,y,:) - skel(:,y+1,:);
            case 6,
                z=2:size(skel,3);
                cands(:,:,z)=skel(:,:,z) - skel(:,:,z-1);
            case 5,
                z=1:size(skel,3)-1;
                cands(:,:,z)=skel(:,:,z) - skel(:,:,z+1);
        end;
        
        % if excluded voxels were passed, remove them from candidates
        if(nargin==2)
            cands = cands.*~spare;
        end;
        
        % make sure all candidates are indeed foreground voxels
        cands = intersect(find(cands(:)==1),find(skel(:)==1));
        
        noChange = true;
                    
        if(~isempty(cands))
            % get subscript indices of candidates
            [x y z]=ind2sub([width height depth],cands);
            
            % get 26-neighbourhood of candidates in volume
            nhood = logical(pk_get_nh(skel,cands));
            
            % remove all endpoints (exactly one nb) from list
            di1 = find(sum(nhood,2)==2);
            nhood(di1,:)=[];
            cands(di1)=[];
            x(di1)=[];
            y(di1)=[];
            z(di1)=[];
            
            % remove all non-Euler-invariant points from list
            di2 = find(~p_EulerInv(nhood, eulerLUT'));
            nhood(di2,:)=[];
            cands(di2)=[];
            x(di2)=[];
            y(di2)=[];
            z(di2)=[];
            
            % remove all non-simple points from list
            di3 = find(~p_is_simple(nhood));
            nhood(di3,:)=[];
            cands(di3)=[];
            x(di3)=[];
            y(di3)=[];
            z(di3)=[];
            
            
            % if any candidates left: divide into 8 independent subvolumes
            if(~isempty(x))
                x1 = find(mod(x,2));
                x2 = find(~mod(x,2));
                y1 = find(mod(y,2));
                y2 = find(~mod(y,2));
                z1 = find(mod(z,2));
                z2 = find(~mod(z,2));
                ilst(1).l = intersect(x1,intersect(y1,z1));
                ilst(2).l = intersect(x2,intersect(y1,z1));
                ilst(3).l = intersect(x1,intersect(y2,z1));
                ilst(4).l = intersect(x2,intersect(y2,z1));
                ilst(5).l = intersect(x1,intersect(y1,z2));
                ilst(6).l = intersect(x2,intersect(y1,z2));
                ilst(7).l = intersect(x1,intersect(y2,z2));
                ilst(8).l = intersect(x2,intersect(y2,z2));
                
                idx = [];
                
                % do parallel re-checking for all points in each subvolume
                for i = 1:8                    
                    if(~isempty(ilst(i).l))
                        idx = ilst(i).l;
                        li = sub2ind([width height depth],x(idx),y(idx),z(idx));
                        skel(li)=0; % remove points
                        nh = logical(pk_get_nh(skel,li));
                        di_rc = find(~p_is_simple(nh));
                        if(~isempty(di_rc)) % if topology changed: revert
                            skel(li(di_rc))=1;
                        else
                            noChange = false; % at least one voxel removed
                        end;
                    end;
                end;
            end;
        end;
        
        if( noChange )
            unchangedBorders = unchangedBorders + 1;
        end;
        fprintf('\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b');
        fprintf('removed %3d%% voxels',round(100*(l_orig-length(find(skel(:))))/l_orig));
        
    end;
end;

fprintf('\n');

% get rid of padded zeros
skel = skel(2:end-1,2:end-1,2:end-1);


