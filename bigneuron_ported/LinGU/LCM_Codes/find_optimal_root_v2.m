function ri = find_optimal_root_v2(adjmatT,TN)

n = size(adjmatT,1);

if(nargin < 2)
    
   TN{1} = 1 : n;
    
end


for i_t = 1 : length(TN)
    
    
    if(length(TN{i_t}) == 1)

        ri(i_t) = TN{i_t};
     
    else
        
            
    node_zero = setdiff(1:n,TN{i_t});
    
    adjmatT1 = adjmatT;
    
    adjmatT1(node_zero,:) = 0;
    
    adjmatT1(:,node_zero) = 0;
        
        ri(i_t) = search_optimal_root_tree(adjmatT1);
        
    end
end




end







function ri = search_optimal_root_tree(adjmatT)


degreeT = full(sum(double(adjmatT),2));

root_ind_cand = find(degreeT == max(degreeT(:)));
% 
% root_ind_cand(root_ind_cand < 101) = [];
% 
% if(isempty(root_ind_cand))
%     
%     root_ind_cand = find(degreeT == max(degreeT(:)));
%     
% end



for i_r = 1 : length(root_ind_cand)
    
    ri = root_ind_cand(i_r);
    
    [children,pedigree_node,pedigree_mat,depths,parents] =...
        tree_traversal_df(adjmatT,ri);
    
    dir_children =  children{ri};
    
    l_os = zeros(length(dir_children),1);
    
    for i_d = 1 : length(dir_children)
       
        os = accum_offspring(pedigree_mat,dir_children(i_d));
        
        l_os(i_d) = length(os);
        
        
    end
    
    dis_os = l_os / sum(l_os);
    
    ent_os(i_r) = -sum(dis_os .* log(dis_os));
    
        
end

[~,max_ent] = max(ent_os);

ri = root_ind_cand(max_ent);

end
