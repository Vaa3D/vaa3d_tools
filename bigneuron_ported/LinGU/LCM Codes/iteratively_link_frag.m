function linkage = iteratively_link_frag(est_KB)

% Starting from the largest fragment, attempt to absort the nearest fragments 


sz_I = size(est_KB);

CC = bwconncomp(est_KB > 0);

numPixels = cellfun(@numel,CC.PixelIdxList);

[nP,nP_idx] = sort(numPixels,'descend');

for is = 1 : length(nP_idx)
   
    [x{is},y{is},z{is}] = ind2sub(sz_I,CC.PixelIdxList{nP_idx(is)});
    
end


nC = CC.NumObjects;

min_dist = zeros(nC,nC);

min_f1 = zeros(nC,nC,3);

min_f2 = zeros(nC,nC,3);



for s1 = 1 : length(nP_idx)
    
    for s2 = (s1 + 1) : length(nP_idx)
        
        distM = pdist2([x{s1},y{s1},z{s1}],[x{s2},y{s2},z{s2}]);
        
        [min_dist(s1,s2),ppair_id] = min(distM(:));
        
        [ppair_idx,ppair_idy] = ind2sub([length(x{s1}),length(x{s2})],ppair_id);
        
        min_f1(s1,s2,1) = x{s1}(ppair_idx);
        
        min_f1(s1,s2,2) = y{s1}(ppair_idx);
        
        min_f1(s1,s2,3) = z{s1}(ppair_idx);

        
        
        min_f2(s1,s2,1) = x{s2}(ppair_idy);
        
        min_f2(s1,s2,2) = y{s2}(ppair_idy);
        
        min_f2(s1,s2,3) = z{s2}(ppair_idy);        
        
    end
    
end


% span the minimal forest in a naive way

min_dist(min_dist < 1) = max(min_dist(:)) + 1;


[min_dst,min_path_idx] = sort(min_dist(:));

[mp_x,mp_y] = ind2sub(size(min_dist),min_path_idx);


tree_idx = zeros(length(nP_idx),1);

tree_idx_mat = zeros(length(nP_idx),length(nP_idx));


new_tree = 1;

lk_idx = 1;

linkage = {};




for i_p = 1 : length(min_path_idx)
    
    f1 = mp_x(i_p);
    
    f2 = mp_y(i_p);
    
    % check whether the component has been checked or not
    
    f_checkedout = sum(tree_idx([f1,f2]) > 0);
    
    switch f_checkedout
        
        case 0
            
            tree_idx_mat(f1,f2) = new_tree;
            
            tree_idx_mat(f2,f1) = new_tree;
            
            tree_idx(f1) = new_tree;
            
            tree_idx(f2) = new_tree;
            
            new_tree = new_tree + 1;
            
            linkage{lk_idx}.dst = min_dst(i_p);
            
            p1 = min_f1(f1,f2,:);
            
            linkage{lk_idx}.p1 = p1(:);
            

            p2 = min_f2(f1,f2,:);
            
            linkage{lk_idx}.p2 = p2(:);
            
            lk_idx = lk_idx + 1;
            
        case 1
            
            f12 = [f1,f2];
            
            f_old = f12(find(tree_idx(f12)));
            
            f_new = f12(find(tree_idx(f12) == 0));

            f_tree = tree_idx(f_old);
            
            tree_idx(f_new) = f_tree; 
            

            linkage{lk_idx}.dst = min_dst(i_p);
            
            p1 = min_f1(f1,f2,:);
            
            linkage{lk_idx}.p1 = p1(:);
            

            p2 = min_f2(f1,f2,:);
            
            linkage{lk_idx}.p2 = p2(:);
            
            lk_idx = lk_idx + 1;
            
            
            
        case 2
            
            if(tree_idx(f1) ~= tree_idx(f2))
                
                t1 = tree_idx(f1);
                
                t2 = tree_idx(f2);
                
                [tnew,fnew] = min([t1,t2]);
                
                [told,fold] = max([t1,t2]);
                
                tree_idx(tree_idx == told) = tnew;
                
                linkage{lk_idx}.dst = min_dst(i_p);
                
                p1 = min_f1(f1,f2,:);
                
                linkage{lk_idx}.p1 = p1(:);
                
                
                p2 = min_f2(f1,f2,:);
                
                linkage{lk_idx}.p2 = p2(:);
                
                lk_idx = lk_idx + 1;
                
            end
                
        otherwise
            
    end
    
    
    
%     
%     
%     if(tree_idx(i_p))
%     
%     
%     tree_idx()
%     
%     
%     end
    
    
    
    
end


% tree_idx;






%[Fdist,F] = sort(distM(:));




