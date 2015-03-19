function [patch_stack,hdc_stack,patch_idx,patch_stack1] = collect_fragment_3D(fn)

% collect the 3D fragments for the training tokens


% data_str = fn.data_str;



%for i_p = 1 : 6
    
patch_stack = [];

hdc_stack = [];

%hd_stack{i_p} = [];

patch_idx = [];
%
% end
patch_stack1 = [];


for i_img = 1 : length(fn.train.gts.X)
    
    %gt = fn.train.gts.X{i_img};
    
    % currently, the 3D groundtruth is not avaliable, therefore, use the hybrid
 % groundtruth

    [I,gt_2D] = import_3D_data(fn.train.imgs.X{i_img});
    
    gt = import_3D_data(fn.train.gts.X{i_img});
    


%     est_fn = sprintf('%s_%d',fn.data_str,i_img);
% 
%     est = load_OOF(est_fn,I);
% 
%     % the transform of groundtruth is only necessary for EPFL OOF 
% 
%     gt = (est > 0.6) .* repmat(gt_2D, [1 1 size(est,3)]);
    
    [ps,hdcs,pidx] = break_3Dgt_sample(gt);
    
    patch_stack = cat(4,patch_stack,ps); 
   
    
    ps1 = GetPatches3DGT(gt,pidx,3);

    patch_stack1 = cat(4,patch_stack1,ps1);     
    
    hdc_stack = [hdc_stack; hdcs];

    sz_I = size(gt);

    [samp_x,samp_y,samp_z] = ind2sub(sz_I,pidx);


    pidx1 = i_img * ones(length(pidx),4);
    
    pidx1(:,2) = samp_x;
    
    pidx1(:,3) = samp_y;

    pidx1(:,4) = samp_z;


    patch_idx = [patch_idx; pidx1];
    
end
