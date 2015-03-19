function node_classifier = train_RF_3Dseed_v2(CLRG_tree,localfeature,hd,sample_idx)

% train a classifier based on the token from CLRG tree

% This version works on the 3D image

% use the local cubic as the low level feature

label = sample_idx(:,5);

% n_label = max(label);

stack = CLRG_tree.root_node;

children = CLRG_tree.children;

pedigree_mat = CLRG_tree.pedigree_mat;

isleaf = zeros(size(CLRG_tree.depths));

isonlychild = zeros(size(CLRG_tree.depths));


% transfer the patch_idx into the format accepted by KB method

% 
% for i_X = 1:size(data.train.imgs.X,1)
%     
%     for i_b = 1 : min(size(data.train.imgs.X,2),3)
%         
%         img(:,:,i_b) = data.train.imgs.X{i_X,i_b};
%         
%     end
%     
%     img = sum(img,3);
%     
%     data.train.imgs.X{i_X,4} = lap_gaussian_F(img);
%     
% end



params = setup_params;


while ~isempty(stack)
    
    node = stack(1);
    
    stack = stack(2:end);
    
    %     c_children = children{node};
    
    
    [l_new,os] = assign_label_node(pedigree_mat,children,node,label);
    
    n_l = length(os);
    
    if(n_l > 1)

        
        [weak_learners,params] = train_3Dseed_learner_RF_v2(localfeature,params,hd,sample_idx,l_new,n_l);
        
        node_classifier{node}.wl = weak_learners;
              
        node_classifier{node}.params = params;
        
        
    end
    
    node_classifier{node}.os = os;
    
    close all hidden;
    
    stack = [stack,children{node}];
    
end

end

function [l_new,os] = assign_label_node(pedigree_mat,children,node,label)

    l_new = zeros(size(label));
  
    c_children = children{node};
    
    n_l = length(c_children);
    
    os = [];
    
    for i_c = 1 : n_l
        
        os{i_c} = accum_offspring(pedigree_mat,c_children(i_c));
        
        constain_os = ismember(label,os{i_c});
        
        l_new(constain_os) = i_c;
        
    end
    
%     os{i_c + 1} = node;
%     
%     constain_os = ismember(label,node);
%     
%     
%     
%     n_l = n_l + 1;
%     
%     l_new(constain_os) = n_l;
    
end


function params = setup_params


    
    DATASET_NAME = 'DRIVE';
    
    params = setup_config_L(DATASET_NAME);
    
    params.codename = date;
    
    params = setup_lists(params);
    
    params = setup_directories_L(params);
    
    params.wl_no = 10;
    
    params.use_qws = 1;
    
    params.wl_depth = 5;
    
   
    
    
end

function [weak_learners,params] = train_seed_learner(data,params,hd,sample_idx,l_new,n_l)

      

        l_new1 = l_new / n_l;
        
        
        l_new1 = 2 * l_new1 - 1;
        
        
        sample_idx1 = sample_idx;
        
        sample_idx1(:,4) = l_new1;
        
        sample_idx1(l_new == 0,:) = [];
        
        hd1 = hd;
        
        hd1(l_new == 0,:) = [];
        
        
        n_samp = size(sample_idx1,1);
        
        % number to sample for filter search (T1) and tree learning (T2)
        params.T1_size = round(n_samp / 3);
        
        params.T2_size = round(n_samp / 3);
        
        params.rand_samples_no = round(params.T1_size / 2);
        
        params.pos_to_sample_no = round(n_samp / 2.1);
        
        params.neg_to_sample_no = round(n_samp / 2.1);
        
        
        weak_learners = train_boost_hd(params,data,hd1,sample_idx1);



end




    