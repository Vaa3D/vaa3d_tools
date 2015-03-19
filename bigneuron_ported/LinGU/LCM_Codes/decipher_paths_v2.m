function [pathsxy,Seed_m] = decipher_paths_v2(node_idx_map,mask,paths,nSeeds)

if(~iscell(paths))
    
    paths1{1} = paths;
    
end


paths = paths1;

uncharted_idx  = find((mask - (node_idx_map < nSeeds + 1) ) > 0);

for i_p = 1 : length(paths)
    
    
    
    
    paths_idx = paths{i_p};
    
    if(sum(isnan(paths_idx)))
    
        pathsxy{i_p} = 0;
        
        Seed_m{i_p} = 1;
        
        continue;
    
    end
        
    path_xy = paths_idx;
    
    isSeed = paths_idx < nSeeds + 1;
    
    path_xy(~isSeed) = uncharted_idx(paths_idx(~isSeed) - nSeeds);
        
    pathsxy{i_p} = path_xy;
        
    Seed_m{i_p} = isSeed;
    
    
    
end