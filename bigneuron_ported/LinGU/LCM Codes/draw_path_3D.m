function path_img = draw_path_3D(node_idx_map,pathsxy,Seed_m,drawlist)

% draw the path on the image given the pathsxy

if(nargin < 4)
    
   drawlist = ones(length(pathsxy)); 
    
end

path_img = zeros(size(node_idx_map));

for k = 1 : length(drawlist)
    
    if(drawlist(k))
        
        pc = pathsxy{k};
        
        pc(Seed_m{k}) = [];
       
        path_img(pc) = 1;
        
    end
    
end

