function path_img = pre_link_branch3D(prob_img,seed_img,mask,mt,t1,t2)

% this function serves to link the sevral major branch to speed up the
% processing

sz_I = size(seed_img);

CC = bwconncomp(seed_img > 0,6);

numPixels = cellfun(@numel,CC.PixelIdxList);

seed_plist = CC.PixelIdxList;

[nP,nP_idx] = sort(numPixels,'descend');

[SeedMap,propsSeed] = get_frag_sta_3D(prob_img,CC);

large_c1 = propsSeed(:,1) > mt;

% large_c2 = (propsSeed(:,2) ./ propsSeed(:,3)) > 3;

large_c2(1) = 1;

large_c2 = large_c1 & large_c2;

BW1 = zeros(sz_I);

for k = 1 : length(large_c2)
    
    if(large_c1(k) > 0)
        
        BW1(SeedMap(:) == k) = 1;
        
    end
    
end

%     ske_w_v(k) = mean(ske_dist(CC_Eoc.PixelIdxList{nP_idx_ske(k)}));
%     
%     l_w(k) = nP_ske(k) / ske_w_v(k);
    
            


if(sum(large_c1) > 1)
    
    [costs,pathsxy,Seed_m,node_idx_map,nSeeds] = connect_seed_3D(prob_img,BW1 > 0);
    
    sz_I = size(node_idx_map);
    
    % filter out the path which is too artificial
    
%     diff_np = trace_path(sz_I,pathsxy,Seed_m);
%     
%     iss = is_straight(diff_np);
%     
%     
%     
%     %  gradually absorb the positive node into the
%     % source node and update the cost for each seed and then remove the one which
%     % is above the threshold
%     
%     [pos_path,path_cost] = iterate_absorb_seed_v3(costs,l_w,paths,pathsxy, 1 - new_prob_img,length(l_w),t1);
%     
%     pos_path = pos_path & (iss' < t2);
% 
%     p90 = node_idx_map > 0;
%     
%     mip_p90 = sum(p90,3) > 0;
%     
%     mip_BW1 = sum(BW1,3); 
    
    path_img = draw_path_3D(node_idx_map,pathsxy,Seed_m);
    
       
else
    
    path_img = zeros(sz_I);
    
end

end