function LTC_v = train_LCM_validation(trn_ftrs,samples_idx,LTClassifier)
% train the validation stage of LCM

% author : Lin GU

n_d = size(samples_idx,1);

n_img = max(samples_idx(:,1));

% assign each training data to the leaf node of the CLRG tree

y_p = zeros(n_d,1);

for i_img = 1 : n_img
    
%     X{1} = data.train.imgs.X{i_img};
    
    features_seed = trn_ftrs(samples_idx(:,1) == i_img,:);
        
    y_p(samples_idx(:,1) == i_img) = predict_RF_seed_v3...
        (features_seed,LTClassifier.CLRG_tree,...
        LTClassifier.node_classifier);
    
end


% train the validation phase for each leaf node of the tree, particularly
% the node with enough positive and negative samples

y_istrain = zeros(n_d,1);

for i_y = 1 : max(y_p)
    
    sp_idx = samples_idx(y_p == i_y,:);
    
    n_pos = sum(sp_idx(:,4) == 1);
    
    n_neg = sum(sp_idx(:,4) == -1);
    
    if((n_pos > 5) && (n_neg > 5))
        
        y_istrain(y_p == i_y) = 1;
        
%         weak_learners{i_y} = train_boost_weight_3D_v2(params,trn_ftrs(y_p == i_y,:),sp_idx(:,4),wgt(y_p == i_y));
        
        label = sp_idx(:,5) / 2 + 1.5;
        
        
        weak_learners{i_y} = forestTrain(single(trn_ftrs(y_p == i_y,:)),label,'maxDepth',5);
        
        
    else
        
        weak_learners{i_y} = [];
        
    end
        
end


if(sum(y_istrain == 0))
        
    label = samples_idx(y_istrain == 0,5) / 2 + 1.5;
    
    weak_learner_default = forestTrain(single(trn_ftrs(y_istrain == 0,:)),...
        label,'maxDepth',5);
    
else
    
    weak_learner_default = {};
    
end


LTC_v.weak_learners = weak_learners;

LTC_v.weak_learner_default = weak_learner_default;
