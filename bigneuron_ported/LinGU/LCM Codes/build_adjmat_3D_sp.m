function [ia,ja,ind] = build_adjmat_3D_sp(X)

% build the adj matrix for abritary mask image

[nRows,nCols,nBands] = size(X);

mask_idx = find(X > 0);

nNodes = length(mask_idx);

node_img = zeros(size(X));

node_img(mask_idx) = 1 : nNodes;


% shrink the mask and only build the relation within the core part

% se = strel('ball',15,15);

core_nodes = zeros(size(X));

core_nodes(2:end - 1,2:end - 1,2:end - 1) =...
    X(2:end - 1,2:end - 1,2 : end - 1); 

% core_nodes = imerode(X,se);


adj = sparse(nNodes,nNodes);

% Add 6 connected Edges for 3D images

ind = find(core_nodes > 0);

indc(1,:) = ind + 1;

indc(2,:) = ind - 1;

indc(3,:) = ind + nRows;

indc(4,:) = ind - nRows;

indc(5,:) = ind + nRows * nCols;

indc(6,:) = ind - nRows * nCols;

nindc = indc;

nind = ind;

ia = repmat(nind,6,1);

nindc = nindc';

ja = nindc(:);


% 
% 
% for i_c = 1 : length(indc)
%     
%     adj(sub2ind([nNodes nNodes],nind,nindc(i_c,:)')) = 1;
%     
% end




