function [costs,pathsxy,Seed_m,node_map_gb,nSeeds] = connect_seed_3D(prob_img,est_K,dist_cc)

% at first, connect the conponent with the highest confidence 


% given the fact that 3D space is much larger than the 2D counterpart, here
% connects the fragments by implementing the searching locally.
% Particularly, the fragment is connected by starting from the pairwise
% linkage of the closed fragments

if(nargin < 3)
    
    dist_cc = 20;
    
end


% [ia,ja,node_idx_map,nNodes,nSeeds,ind] = construct_map_3D(est_K,mask);
% 
% find the corresponding graph node for both ends of the edge
% 
% ni = node_idx_map(ia);
% 
% nj = node_idx_map(ja);
% 
% 
% % delete the edges between or within the seeds
% 
% edge_inter_seeds = (ni < nSeeds + 1) & (nj < nSeeds + 1);
% 
% ia(edge_inter_seeds) = [];
% 
% ja(edge_inter_seeds) = [];
% 
% max_prob = max(prob_img(:)) + 1;
% 
% % assign the reverse of the probability as the energy function for each end
% % of the edge
% 
% wi = max_prob - prob_img(ia);
% 
% wj = max_prob - prob_img(ja);
% 
% 
% % find the corresponding graph node for both ends of the edge
% 
% ni = node_idx_map(ia);
% 
% nj = node_idx_map(ja);
% 
% 
% wEdge = min(wi,wj);
% 
% edgeN = sparse(ni,nj,wEdge,nNodes,nNodes);

max_prob = max(prob_img(:)) + 1;

% determine the pairwise distance for each fragment


% roughly predict space where linkage exists, search that space to determine
% the shortest path


linkage = iteratively_link_frag(est_K);

if(sum(linkage{1}.p1 == [0 0 0]'))
    
    costs = [];
    
    pathsxy = [];
    
    Seed_m = [];
    
    node_map_gb = [];
    
    nSeeds = [];
    
%     ,pathsxy,Seed_m,node_map_gb,nSeeds
    
    return;
    
end

% if(linkage{1}.dst == 1)
%    
%     
%     
%     
%     return;
%     
%     
% end

[node_map_gb,nSeeds] = label_fragments_3D_global(est_K);

if(0)
   
     mip_est = sum(est_K,3);
     
     mip_est = repmat(mip_est,[1,1,3]);
    
     mip_path = zeros(size(est_K(:,:,1)));
     
     path_3D = zeros(size(est_K));
     
end



% attempt to link together the closed fragments
 
for i_N = 1 : length(linkage)
    
    if(sqrt(linkage{i_N}.dst) > dist_cc)
        
       continue; 
        
    end
    
    p1 = linkage{i_N}.p1;
    
    p2 = linkage{i_N}.p2;
    
%     if(p1 == [0 0 0])
%         
%         p1 = [1 1 1];
%         
%     end
%        
%     
%     if(p2 == [0 0 0])
%         
%         p2 = [0 0 0];
%         
%     end
    
        
    
    p1ind = sub2ind(size(node_map_gb),p1(1),p1(2),p1(3));
    
    p2ind = sub2ind(size(node_map_gb),p2(1),p2(2),p2(3));
        
    
    sp = [p1,p2];
    
    sp_min = min(sp,[],2);
    
    sp_max = max(sp,[],2);
    
    sp_min = max(sp_min - 5,1);
    
    sp_max = min(sp_max + 5,size(est_K)');
    
    sp_mask = zeros(size(est_K));
    
    sp_mask([sp_min(1) : sp_max(1)], [sp_min(2) : sp_max(2)],[sp_min(3) : sp_max(3)]) = 1;
    
    sp_range = [sp_min,sp_max];
    
    
    h_sp = sp_range(1,2) - sp_range(1,1) + 1;
    
    w_sp = sp_range(2,2) - sp_range(2,1) + 1;
    
    b_sp = sp_range(3,2) - sp_range(3,1) + 1;
    
    [iasp,jasp,ind] = build_adjmat_3D_sp(ones(h_sp,w_sp,b_sp));
    
    node_map_sp = node_map_gb(sp_range(1,1) : sp_range(1,2),...
        sp_range(2,1) : sp_range(2,2),sp_range(3,1) : sp_range(3,2));
    
    
    uncharted_idx  = find(node_map_sp == 0);
        
    node_map_sp(uncharted_idx) = nSeeds + (1:length(uncharted_idx))';
    
    nisp = node_map_sp(iasp);
    
    njsp = node_map_sp(jasp);       
    
    edge_inter_seeds = (nisp < nSeeds + 1) & (njsp < nSeeds + 1);
    
    iasp(edge_inter_seeds) = [];
    
    jasp(edge_inter_seeds) = [];    
    
    wisp = max_prob - prob_img(iasp);
    
    wjsp = max_prob - prob_img(jasp);    
    
    nisp = node_map_sp(iasp);
    
    njsp = node_map_sp(jasp);       

    
    wEdgesp = min(wisp,wjsp);
    
    nNodes_sp = nSeeds + length(uncharted_idx);
    
    
    edgeNsp = sparse(nisp,njsp,wEdgesp,nNodes_sp,nNodes_sp);
    
    [costs_path,paths_sp] = dijkstra_v2(edgeNsp > 0,edgeNsp,...
        node_map_gb(p1ind),node_map_gb(p2ind));
    
    costs(i_N) = costs_path;
    
    [pathsxy_sp_cell,Seed_m_sp] = decipher_paths_v2(node_map_sp,ones(size(node_map_sp)),...
        paths_sp,nSeeds);
    
    pathsxy_sp = pathsxy_sp_cell{1};
    
    Seed_m{i_N} = Seed_m_sp{1};
    
    pathxy_sp = pathsxy_sp(~Seed_m{i_N});
    
    [p_sp_x,p_sp_y,p_sp_z] = ind2sub(size(node_map_sp),pathxy_sp);
      
    p_sub_gb_x = p_sp_x +  sp_min(1) - 1;
    
    p_sub_gb_y = p_sp_y +  sp_min(2) - 1;
    
    p_sub_gb_z = p_sp_z +  sp_min(3) - 1;
    
    p_ind_gb = sub2ind(size(node_map_gb),p_sub_gb_x,p_sub_gb_y,...
        p_sub_gb_z);
    
    pathsxy{i_N} = pathsxy_sp_cell{1};
    
    pathsxy{i_N}(~Seed_m{i_N}) = p_ind_gb;
    
    
    % visualise the build path
    
    if(0)
        
%         mip_est = sum(est_K,3);
%
        p_sub_gb_xy = sub2ind(size(node_map_gb(:,:,1)),p_sub_gb_x,p_sub_gb_y);
        
        path_3D(p_ind_gb) = 1;
        
        mip_path_3D = sum(path_3D,3);
        
        mip_path(p_sub_gb_xy) = 1;
        
        mip_show = mip_est;
        
        mip_show(:,:,2) = mip_show(:,:,2) + mip_path_3D;
        
        imshow(mip_show);
        
    end
    
        
end


