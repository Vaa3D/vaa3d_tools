function [trn_ftrs,samples_idx] = collect_pos_neg_OOF(fn)

pos_feature1 = [];

pos_samp_idx = [];

neg_samp_idx = [];

neg_feature1 = [];

pos_p = [];

neg_p = [];


for i_img = 1 : length(fn.train.gts.X)
        
   [I,gt_2D] = import_3D_data(fn.train.imgs.X{i_img});
   
   est_fn = sprintf('%s_%d',fn.data_str,i_img);
   
   est = load_OOF(est_fn,I);

   gt = import_3D_data(fn.train.gts.X{i_img});
    
   [pos_seed_img,neg_seed_img,pos_ind,neg_ind] = generate_gt_seeds_neuron_3D(gt,est);
    
   CC_pos = bwconncomp(pos_seed_img,6);
   
   [SeedMapP,propsSeedP] = get_frag_sta_3D(est,CC_pos);
   
   if(~isempty(propsSeedP))
   
       nPos = size(propsSeedP,1);
       
       pos_i = zeros(nPos,4);
       
       pos_i(:,1) = i_img;
       
       pos_i(:,2:4) = propsSeedP(:,6:8);
       
       pos_i = round(pos_i);
       
       pos_ind = sub2ind(size(pos_seed_img),pos_i(:,2),pos_i(:,3),pos_i(:,4));
       
       pos_samp_idx = [pos_samp_idx; pos_i];
       
       hog_pos = HOG3D_frag(pos_seed_img,pos_ind);
       
       pos_p1 = GetPatches3DGT(I,pos_ind,3);
       
       pos_p = cat(4,pos_p,pos_p1);
       
       pos_feature = [propsSeedP(:,1:5),hog_pos];
       
       pos_feature1 = [pos_feature1; pos_feature];
       
   end   

   
   CC_neg = bwconncomp(neg_seed_img,6);
   
   [SeedMapN,propsSeedN] = get_frag_sta_3D(est,CC_neg);
   
   if(~isempty(propsSeedN))
       
       nNeg = size(propsSeedN,1);
       
       neg_i = zeros(nNeg,4);
       
       neg_i(:,1) = i_img;
       
       neg_i(:,2:4) = propsSeedN(:,6:8);
       
       neg_i = round(neg_i);
       
       neg_ind = sub2ind(size(neg_seed_img),neg_i(:,2),neg_i(:,3),neg_i(:,4));
       
       hog_neg = HOG3D_frag(neg_seed_img,neg_ind);
       
       neg_p1 = GetPatches3DGT(I,neg_ind,3);
       
       neg_p = cat(4,neg_p,neg_p1);
       
       
       neg_feature = [propsSeedN(:,1:5), hog_neg];
       
       neg_feature1 = [neg_feature1; neg_feature];
       
       neg_samp_idx = [neg_samp_idx; neg_i];
       
       
   end
     
end

nPos = size(pos_samp_idx,1);

nNeg = size(neg_samp_idx,1);

pos_ftrs = pos_feature1;

neg_ftrs = neg_feature1;

% balance the number of positive and negative instances

if(0)
    
    
    nNeg1 = min(2 * nPos,nNeg);
    
    
    neg_samp = randperm(nNeg,nNeg1);
    
    neg_samp = sort(neg_samp);
    
    neg_samp_idx = neg_samp_idx(neg_samp,:);
    
    neg_ftrs = neg_ftrs(neg_samp,:);
    
    label = ones(nPos + nNeg1,1);
    
    label(nPos + 1 : end) = -1;
    
else
    
    %     samples_idx = [pos_samp_idx;neg_samp_idx];
    
    label = ones(nPos + nNeg,1);
    
    label(nPos + 1 : end) = -1;
    
end

samples_idx = [pos_samp_idx;neg_samp_idx];

samples_idx = [samples_idx,label];

trn_ftrs = [pos_ftrs; neg_ftrs];

trn_patches = cat(4,pos_p,neg_p);

trn_patches = reshape(trn_patches,[],size(trn_patches,4));

trn_patches = trn_patches';

trn_ftrs = [trn_patches ,trn_ftrs ];
