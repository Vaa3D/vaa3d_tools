function [weak_learners,params] = train_3Dseed_learner_RF_v2(localfeature,params,hd,sample_idx,l_new,n_l)
    
% extend the function from 2D to 3D dataset

% abandon the kernel boost features


        
        
        sample_idx1 = sample_idx;
        
        sample_idx1(:,5) = l_new;
        
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
        
        localfeature = reshape(localfeature,[],size(localfeature,4));
        
        localfeature = localfeature';
        
        localfeature(l_new == 0,:) = [];
        
        features = [localfeature,hd1];
        
        weak_learners = train_RF(params,features,sample_idx1,n_l);

end

function [weak_learners] = train_RF(params,features,samples_idx,n_l)

% Train a classifier on the given samples

% the classifier combine the histogram discriptor
%



labels1 = samples_idx(:,5);

labels = samples_idx(:,5) / n_l;

labels = 2 * labels - 1;

n_sample = size(samples_idx,1);





samples_no = size(samples_idx,1);

labels2 = ones(samples_no,1);

weak_learners(params.wl_no).alpha = 0;

samples_idx = samples_idx(:,1:3);

samples_idx(:,2:3) = samples_idx(:,2:3) + params.border_size;

current_response = zeros(samples_no,1);

% [compute_wi1,compute_ri1,compute_loss1,compute_indiv_loss1,compute_2nd_deriv1,mex_loss_type1] = select_fncts(params,labels);
% 
% [compute_wi,compute_ri,compute_loss,compute_indiv_loss,compute_2nd_deriv,mex_loss_type] = select_fncts(params,labels2);

% 
% W = compute_wi(current_response);
% R = compute_ri1(current_response);
% 
% train_scores = zeros(params.wl_no,3);
% 
% alpha_cum = 0;

for i_w = 1:params.wl_no
    t_wl = tic;
    fprintf('  Learning WL %d/%d\n',i_w,params.wl_no);
    
    % Indexes of the two training subparts
    T1_idx = sort(randperm(length(labels),params.T1_size),'ascend');
    T2_idx = sort(randperm(length(labels),params.T2_size),'ascend');
%     [wr_idxs,wr_responses,wr_weights] = compute_wr(params,T1_idx,W,R,compute_indiv_loss,compute_2nd_deriv,labels2,current_response);
%     
%     [wr_idxs1,wr_responses1,wr_weights1] = compute_wr(params,T1_idx,W,R,compute_indiv_loss1,compute_2nd_deriv1,labels,current_response);
%     
%     s_T1 = samples_idx(wr_idxs,1:3);
    s_T2 = samples_idx(T2_idx,1:3);
    
%     features = cell(params.ch_no,1);
%     kernels = cell(params.ch_no,1);
%     kernel_params = cell(params.ch_no,1);
    
%     for i_ch = 1:params.ch_no
%         ch = params.ch_list{i_ch};
%         fprintf('    Learning channel %s (%d/%d)\n',ch,i_ch,params.ch_no);
%         X = data.train.(ch).X(:,data.train.(ch).idxs);
%         
%         X = expand_img(X,params);
%         
%         X_idxs = data.train.(ch).idxs;
%         
%         sub_ch_no = data.train.(ch).sub_ch_no;
%         features{i_ch} = cell(sub_ch_no,1);
%         kernels{i_ch} = cell(sub_ch_no,1);
%         kernel_params{i_ch} = cell(sub_ch_no,1);
%         
%         % Learn the filters
%         fprintf('      Learning filters on the sub-channels\n');
%         for i_s = 1:sub_ch_no
%             t_sch = tic;
%             fprintf('        Learning on subchannel %d/%d of channel %s\n',i_s,sub_ch_no,ch);
%             [kernels{i_ch}{i_s},kernel_params{i_ch}{i_s}] = mexMultipleSmoothRegression(params,params.(ch),X(:,i_s),X_idxs,s_T1,wr_responses1,wr_weights,i_ch,i_s,ch);
%             sch_time = toc(t_sch);
%             fprintf('        Completed, learned %d filters in %f seconds\n',length(kernels{i_ch}{i_s}),sch_time);
%             
%             t_ev = tic;
%             fprintf('        Evaluating the filters learned on the subchannel\n');
%             features{i_ch}{i_s} = mexEvaluateKernels(X(:,i_s),s_T2(:,1:3),params.sample_size,kernels{i_ch}{i_s},kernel_params{i_ch}{i_s});
%             ev_time = toc(t_ev);
%             fprintf('        Evaluation completed in %f seconds\n',ev_time);
%         end
%     end
    
%     fprintf('    Merging features and kernels...\n');
%     [kernels,kernel_params,features] = merge_features_kernels(kernels,kernel_params,features);
%     fprintf('    Done!\n');
%     
    % add the histogram discriptor
    
%     hd1 = hd(T2_idx,:);
    
    features1 = features(T2_idx,:);
    
%     features = [localfeature,hd1];
    
    features1 = single(features1);
    
    %    still employ the kernel method to extract the features for the later
%    classification

    
    % employ the standard random forest to train the forest
    
        % here adopt a 01 loss to estimate simulate the boosting effect
    
       
    fprintf('    Training classification tree on learned features...\n');
    t_tr = tic;
    
    weak_learners(i_w).tree = forestTrain(features1,labels1(T2_idx),'maxDepth',params.wl_depth,'H',n_l);
    
%     fprintf('    Removing ueseless kernels...\n');
%     
%     [weak_learners(i_w).kernels,weak_learners(i_w).kernel_params,...
%        weak_learners(i_w).tree] = remove_useless_filters_sl(tree,kernels,kernel_params);
%     
    
    
    
 
    
%     tree.fids(tree.child>0) = fids(tree.fids(tree.child>0)+1)-1; 
%     
     time_tr = toc(t_tr);
    fprintf('    Done! (took %f seconds)\n',time_tr);
  
    
%     weak_learners(i_w).tree = tree; %#ok<NASGU>
%     
%     weak_learners(i_w).kernels = kernels;
%     
%     weak_learners(i_w).kernel_params = kernel_params;
   
%     t_ev = tic;
%     
%     fprintf('    Evaluating the learned kernels on the whole training set...\n');
%     features = zeros(length(labels),length(weak_learners(i_w).kernels));
%     for i_ch = 1:params.ch_no
%         ch = params.ch_list{i_ch};
%         sub_ch_no = data.train.(ch).sub_ch_no;
%         
%         X = data.train.(ch).X(:,data.train.(ch).idxs);
%         
%         X = expand_img(X,params);
%         
%         for i_s = 1:sub_ch_no
%             idxs = find(cellfun(@(x)(x.ch_no==i_ch && x.sub_ch_no==i_s),weak_learners(i_w).kernel_params));
%             if (~isempty(idxs))
%                 features(:,idxs) = mexEvaluateKernels(X(:,i_s),samples_idx(:,1:3),params.sample_size,weak_learners(i_w).kernels(idxs),weak_learners(i_w).kernel_params(idxs));
%             end
%         end
%     end
%     ev_time = toc(t_ev);
%     fprintf('      Evaluation completed in %f seconds\n',ev_time);
    
    % add the hd feature
    
%     hd_aug = hd(:,weak_learners(i_w).hd_feature);
    
%     features = [localfeature,hd];
    
       fprintf('    Performing prediction on the whole training set...\n');
    
%            t_pr = tic;
       
    
%     [hs,ps] = forestApply(single(features), weak_learners(i_w).tree);
    
    
%     cached_responses = double(prob_dist(ps,labels1));
    
%     time_pr = toc(t_pr);
    
%     fprintf('    Prediction finished, took %f seconds\n',time_pr);
%     clear features;
%     
%      
%     fprintf('    Finding alpha through line search...\n');
%     t_alp = tic;
%     alpha = mexLineSearch(current_response,cached_responses,labels2,mex_loss_type);
%     time_alp = toc(t_alp);
%     fprintf('    Good alpha found (alpha=%f), took %f seconds\n',alpha,time_alp);
%     alpha = alpha * params.shrinkage_factor;
%     
%     alpha_cum = alpha_cum + alpha;
%     
%     
%     current_response = current_response + alpha * cached_responses / alpha_cum;
%     
%     W = compute_wi(current_response);
% %     R = compute_ri1(current_response);
%     
%     weak_learners(i_w).alpha = alpha;
%     
%     
%     
%     
%     MR = sum((current_response < 0.5)) / length(labels);
%     fprintf('    Misclassif rate: %.2f | Loss: %f\n',100*MR,compute_loss(current_response));
%     
%     train_scores(i_w,1) = 100*MR;
%     train_scores(i_w,2) = compute_loss(current_response);
%     train_scores(i_w,3) = alpha;
%     
%     wl_time = toc(t_wl);
%     fprintf('  Learning WL %d took %f seconds\n------------------------------------------------\n\n',i_w,wl_time);
%     
%     
    
end
    
   
end


function X1 = expand_img(X,params)

for xi = 1 : size(X,1)
    
    for xj = 1 : size(X,2)
        
        X1{xi,xj} = padarray(X{xi,xj},[params.border_size,params.border_size],...
            'symmetric');
        
    end
    
end



end



function pt = prob_dist(ps,labels1)
    
    
%     pt = zeros(length(labels1),1);
    

    sz_ps = size(ps);
    
    ind_label = sub2ind(sz_ps,(1 : sz_ps(1))',labels1);
    
    pt = ps(ind_label);
    

% 
%     n_l = max(labels1(:));
%     
%     
%     for i_l = 1 : n_l
%         
%         pt(labels1 == i_l) = ps(labels1 == i_l,i_l);
%         
%     end
    
    
end

