function [children,pedigree_node,pedigree_mat,depths,parents] = tree_traversal_df(adjmatT,root_node)

n = size(adjmatT,2);

parents = zeros(1,n);          % a list of parents

stack = root_node;

depths = zeros(1,n);


pedigree_node{stack} = stack;

pedigree_mat = zeros(n,n);



while ~isempty(stack)
    
    node = stack(1);
    
    stack = stack(2:end);        % pop off an element
    
    kids = find(adjmatT(node,:));
    
    kids(kids == parents(node)) = [];
    
    for kid = kids
        
        pedigree_node{kid} = [pedigree_node{node},kid];
        
        pedigree_mat(kid,1:length(pedigree_node{kid})) = pedigree_node{kid};
        
    end
    
    
    children{node} = kids;
    
    parents(kids) = node;
    
    depths(kids) = depths(node) + 1;
    
    stack  =  [stack, kids];
    
end

max_depth = max(depths);

pedigree_mat = pedigree_mat(:,1:max_depth + 1);





