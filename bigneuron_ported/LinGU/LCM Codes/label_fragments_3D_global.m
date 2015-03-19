function [node_idx_map,nSeeds] = label_fragments_3D_global(est_KB)

% give each component an index number

sz_I = size(est_KB);


if(nargin < 2)
    
    mask = ones(sz_I);
    
end

CC = bwconncomp(est_KB > 0);


numPixels = cellfun(@numel,CC.PixelIdxList);

[nP,nP_idx] = sort(numPixels,'descend');

node_idx_map = zeros(sz_I);

for is = 1 : length(nP)
   
    node_idx_map(CC.PixelIdxList{nP_idx(is)}) = is;
    
end

nSeeds = length(nP_idx);


% n_zero = sum(mask(:)) - sum(est_KB(:));
% 
% uncharted_idx  = find((mask - est_KB) > 0);
% 
% 
% node_idx_map((mask - est_KB) > 0) = (1:n_zero)' + length(nP_idx);
% 
% 


