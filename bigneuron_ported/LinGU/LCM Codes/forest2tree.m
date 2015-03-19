function tree_nodes = forest2tree(adjmatT)

n = size(adjmatT,1);

stack = 1 : n;


n_tree = 1;

while ~isempty(stack)

    node = stack(1);
    
    stack = stack(2:end);        % pop off an element

    [children,pedigree_node,pedigree_mat,depths,parents] =...
        tree_traversal_df(adjmatT,node);
    
    forest_n = find(depths > 0);
    
    tree_nodes{n_tree} = [forest_n,node];
    
    stack = setdiff(stack,forest_n);
    
    n_tree = n_tree + 1;
    

end

