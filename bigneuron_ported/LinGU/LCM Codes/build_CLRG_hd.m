function CLRG_tree = build_CLRG_hd(hds)

% construct a tree based on the histogram discriptor


dist_hd = compute_hd_distance(hds);

adjmatT = CLRG(dist_hd,1,10000);


n_label = size(hds,1);

%if a forest is recovered

TN = forest2tree(adjmatT);

% determine a balanced root index

rootlist = find_optimal_root_v2(adjmatT,TN);

n = size(adjmatT,2);

if(length(rootlist) > 1)
    
    
   
    root_node = n + 1;
   
    adjmaT1 = zeros(n + 1);
    
    adjmaT1(1 : n,1 : n) = adjmatT;
    
    aug_v = zeros(n + 1,1);
    
    aug_v(rootlist) = 1;
    
    adjmaT1(root_node,:) = aug_v';
    
    adjmaT1(:,root_node) = aug_v;
    
    adjmatT = adjmaT1;
    
else
    
  root_node = rootlist;
    
    
end


[children,pedigree_node,pedigree_mat,depths,parents] =...
    tree_traversal_df(adjmatT,root_node);


CLRG_tree.adjmatT = adjmatT;

CLRG_tree.root_node = root_node;

CLRG_tree.children = children;

CLRG_tree.pedigree_node = pedigree_node;

CLRG_tree.pedigree_mat = pedigree_mat;

CLRG_tree.depths = depths;

CLRG_tree.parents = parents;

CLRG_tree.n_label = n_label;






  