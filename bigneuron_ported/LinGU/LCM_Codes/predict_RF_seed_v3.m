function [img_y,prob_y] = predict_RF_seed_v3(hd,CLRG_tree,node_classifier)


% msz = min(size(sample_idx));
% 
% if(msz == 1)
%     
%    sz_I = size(X{1});
%    
%    [sidx,sidy] = ind2sub(sz_I,sample_idx);
%    
%    sidxy = [sidx,sidy];
%    
%    sample_idx = sidxy;
%     
% end






stack = CLRG_tree.root_node;

children = CLRG_tree.children;

n_s = size(hd,1);

img_mask{stack} = ones(n_s,1);

img_y = zeros(n_s,1);

n_l = CLRG_tree.n_label;

prob_y = zeros(n_s,n_l);



while ~isempty(stack)
    
    node = stack(1);
    
    stack = stack(2:end);
    
    kids = children{node};
    
    if(~sum(img_mask{node}))
        
        continue;
        
    end
    
    
    if(length(kids) < 1)
        
        img_y(img_mask{node}) = node;
       
    elseif(length(kids) == 1)
        
       img_mask{kids} = img_mask{node};
       
       
    else
        
        weak_learners = node_classifier{node}.wl;
        
        params = node_classifier{node}.params;
                
        hd1 = hd(img_mask{node},:);
        
        fprintf('    Predicting the labels at depth %d ...\n',CLRG_tree.depths(node));
            
        [y,pl] = predict_RF_vector_v2(hd1,weak_learners);
        
        y1 = zeros(n_s,1);
        
        y1(find(img_mask{node})) = y; 
        
        pl1 = zeros(n_s,length(kids));
        
        pl1(find(img_mask{node}),:) = pl;
        
%         y = y .* img_mask{node};
        
        for i_l = 1 : length(kids)
            
            im = y1 == i_l;
            
            pl1(im,i_l) = 0;
            
            img_mask{kids(i_l)} = im;
            
%             prob_y(:,kids(i_l)) = prob_y(:,kids(i_l)) .* (~im);
         
        end
        
        prob_y = substitue_prob(prob_y,pl1,kids); 
                
    end
    
    stack = [stack,kids];
    
end

end


function prob_y = substitue_prob(prob_y,pl,kids)
    
    pl(:,kids > size(prob_y,2)) = [];
    
    kids(:,kids > size(prob_y,2)) = [];

    prob_y1 = zeros(size(prob_y));

    prob_y1(:,kids) = pl;
    
    prob_y = prob_y + prob_y1;
    
end


