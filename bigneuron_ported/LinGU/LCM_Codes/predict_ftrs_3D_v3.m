function label = predict_ftrs_3D_v3(X,ftrs,sample_idx,weak_learners)

% tmp_image = X(:,:,1);

score_features = zeros(size(ftrs,1),2);

wl_no = length(weak_learners);
% 
% sample_idx = round(sample_idx);


for i_w = 1 : wl_no
    
    ch = 'imgs';
    
    wl = weak_learners(i_w);
    
%     idxs = find(cellfun(@(x)(strcmp(x.ch_name,ch)),wl.kernel_params));
%     
%     features = zeros(numel(tmp_image(:,:,1)),length(wl.kernels));
%     
%     
%     ch_wl = wl;
%     
%     ch_wl.kernels = ch_wl.kernels(idxs);
%     
%     ch_wl.kernel_params = ch_wl.kernel_params(idxs);
    
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
%         X2D = sum(X,3);
%         
%         response = imfilter(X2D,kernel,'same');
%         
%         features(:,idxs(i_k)) = response(:);
%         
%         
%     end
%     
%     sample_idx1d = sub2ind(size(tmp_image),sample_idx(:,1),sample_idx(:,2));
%     
%     features = features(sample_idx1d,:);
%     features = GetPatches3DGT(X,sample_idx,15);
%     
%     features = reshape(features,[],size(features,4));
%     
%     features = features';
%     
%     features = [features,ftrs];
    
    [~,new_score] = forestApply(single(ftrs),wl);
    
    score_features = score_features +  new_score / wl_no;
    
end

[~,label] = max(score_features,[],2);

% score_image(sub_mask(:)) = score_features;




