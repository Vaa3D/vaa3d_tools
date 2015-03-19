function [label,prob_l] = predict_RF_vector_v2(hd,weak_learners)

% employ gradient boost based ramdom forest to predict the label assignment
% of each individual seed


% score_features = zeros(size(sample_idx,1),1);

wl_no = length(weak_learners);

prob_l = [];

features = hd; 

for i_w = 1 : wl_no
    
%     t_wl = tic;
%     
%     ch = 'imgs';
%     
     wl = weak_learners(i_w);
%     
%     idxs = find(cellfun(@(x)(strcmp(x.ch_name,ch)),wl.kernel_params));
%     
%     features = zeros(numel(tmp_image),length(wl.kernels));
%     
%     ch_wl = wl;
%     
%     ch_wl.kernels = ch_wl.kernels(idxs);
%     
%     ch_wl.kernel_params = ch_wl.kernel_params(idxs);
%     
%     fprintf('    Start performing prediction ...\n');
%     
%     t_kf = tic;
%     
%     
%     for i_k = 1:length(ch_wl.kernels)
%         
%         kernel_params = ch_wl.kernel_params{i_k};
%         
%         k_s = kernel_params.filter_size;
%         
%         r_k = kernel_params.start_row;
%         
%         c_k = kernel_params.start_col;
%         
%         kernel = zeros(params.sample_size(1),params.sample_size(2));
%         
%         kernel(r_k:r_k+k_s-1,c_k:c_k+k_s-1) = reshape(ch_wl.kernels{i_k},[k_s,k_s]);
%         
%         response = imfilter(X{ch_wl.kernel_params{i_k}.sub_ch_no},kernel,'same');
%         
%         features(:,idxs(i_k)) = response(:);
%         
%         
%     end
%     
%     kf_time = toc(t_kf);
%     
%     fprintf('Complete extracting %d kernel features, took %d seconds ...\n',...
%         i_k, kf_time);
    
%     sample_idx1d = sub2ind(size(tmp_image),sample_idx(:,1),sample_idx(:,2));
%     
%     features = features(sample_idx1d,:);
%         
%     features = [features,hd];
%     
    
    fprintf('Performing classification ...\n');
    
    t_f = tic;
    
    [hs,ps] = forestApply(single(features), wl.tree);
    
%     prob_l = ps;
    
    f_time = toc(t_f);
    
    fprintf('Finished classification, took %d seconds ...\n',f_time);
    
    
    if(isempty(prob_l))
       
        prob_l = zeros(size(ps));
        
    end
    
    prob_l = prob_l + ps / wl_no;
%     
%     wl_time = toc(t_wl);
%     
%     fprintf('  Predicting WL %d of %d took %f seconds\n---------------------\n\n',i_w,wl_no,wl_time);
    
end

prob_l = double(prob_l);

prob_l = prob_l ./ repmat(sum(prob_l,2),[1,size(prob_l,2)]);

[~,label] = max(prob_l,[],2);



% score_image(sub_mask(:)) = score_features;




