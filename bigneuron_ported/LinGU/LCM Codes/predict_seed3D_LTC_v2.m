function [est_seed_img,SeedMap,est_seed,y_p,propsSeed] = predict_seed3D_LTC_v2(X,prob_img,seed_img,LTClassifier)

% predict whether the seed belong to true vessel or not

% in accordance with the draft, the fragment is validated in the leaf node
% of CLRG tree

if(sum(seed_img(:)))

CC = bwconncomp(seed_img);
    
[SeedMap,propsSeed,SeedList] = get_frag_sta_3D(prob_img,CC);

seed_i = propsSeed(:,6:8);

seed_i = round(seed_i);

seed_ind = sub2ind(size(seed_img),seed_i(:,1),seed_i(:,2),seed_i(:,3));

hog_seed = HOG3D_frag(double(seed_img),seed_ind);

ftrs = [propsSeed(:,1:5),hog_seed];

patch_I = GetPatches3DGT_v2(X,seed_i,3);

patch_I = reshape(patch_I,[],size(patch_I,4));

patch_I = patch_I';

ftrs = [patch_I,ftrs];

sidx = propsSeed(:,6:8);

sidx = round(sidx);

params = setup_params;





% assign each training data to the leaf node of the CLRG tree

n_d = size(propsSeed,1);
   
X1{1} = sum(X / 256 ,3);
   
y_p = predict_RF_seed_v3(ftrs,LTClassifier.CLRG_tree,...
        LTClassifier.node_classifier);

est_seed = zeros(n_d,1);    

istest_y = zeros(n_d,1); 

for i_p = 1 : max(y_p)
    
    if(~(isempty(y_p == i_p) || isempty(LTClassifier.LTC_validation.weak_learners{i_p})))
        
        est_seed(y_p == i_p) = predict_ftrs_3D_v3(X1,ftrs(y_p == i_p,:),sidx...
            (y_p == i_p,1:2),LTClassifier.weak_learners{i_p}) - 1;
        
        istest_y(y_p == i_p) = 1;
        
    end
    
end

if(~(isempty(istest_y == 0) || isempty(LTClassifier.LTC_validation.weak_learner_default)))
    
    est_seed(istest_y == 0) = predict_ftrs_3D_v3(X,ftrs(istest_y == 0,:),sidx...
        (istest_y == 0,1:2),LTClassifier.LTC_validation.weak_learner_default) - 1;
    
end


est_seed_img = zeros(size(seed_img));

for i_s = 1 : length(est_seed)

    est_seed_img(SeedList{i_s}) = est_seed(i_s);

end

else
    
    est_seed_img = seed_img;
    
    SeedMap = seed_img;
    
    est_seed = [];
    
end







end

function params = setup_params



DATASET_NAME = 'DRIVE';

params = setup_config_L(DATASET_NAME);

params.codename = date;

params = setup_lists(params);

params = setup_directories_L(params);

params.wl_no = 100;

params.use_qws = 0;

n_samp = 1000;

%     params.wl_depth = 5;

% number to sample for filter search (T1) and tree learning (T2)

% n_samp = length(label);


params.T1_size = round(n_samp / 3);

params.T2_size = round(n_samp / 3);

params.rand_samples_no = round(params.T1_size / 2);

% nPos = sum(label == 1);
% 
% nNeg = sum(label == -1);
% 
% 
% params.pos_to_sample_no = round(nPos / 2);
% 
% params.neg_to_sample_no = round(nNeg / 2);

   
    
    
end
