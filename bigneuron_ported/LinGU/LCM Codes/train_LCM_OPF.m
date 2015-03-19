function LTClassifier = train_LCM_OPF(fn,n_wl)
% train the LCM on the given dataset fn to determine the 3D fragment from the probablity map

% Author: Lin GU

if(nargin < 3)
    
   n_wl = 10;
    
end


% build the CLRG tree from the training 3D data

LTClassifier = Learn_CLRG_Classifier_3D(fn);
% collect the positive and negative samples

[trn_ftrs,samples_idx] = collect_pos_neg_OOF(fn);


params = setup_params_wl(samples_idx(:,end));

params.wl_no = n_wl;


LTC_validation = train_LCM_validation(trn_ftrs,samples_idx,LTClassifier);

LTClassifier.LTC_validation = LTC_validation;




end

