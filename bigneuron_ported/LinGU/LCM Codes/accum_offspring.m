function os = accum_offspring(pedigree_mat,ri)

is_os_node = sum(pedigree_mat == ri,2);

os = find(is_os_node);

if(isempty(os))
   
    os = ri;
    
end







% for i_n = 1 : length(pedigree_node)
%     
%    pn = pedigree_node{i_n};
%         
%    if(length(find(pn == ri))) 
%        
%         os = [os,i_n];
%        
%        
%        
%    end
%     
% end

