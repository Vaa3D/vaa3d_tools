function params = setup_params_wl(label)



DATASET_NAME = 'DRIVE';

params = setup_config_L(DATASET_NAME);

params.codename = date;

params = setup_lists(params);

params = setup_directories_L(params);

params.wl_no = 10;

params.use_qws = 0;

%     params.wl_depth = 5;

% number to sample for filter search (T1) and tree learning (T2)

n_samp = length(label);


params.T1_size = round(n_samp / 3);

params.T2_size = round(n_samp / 3);

params.rand_samples_no = round(params.T1_size / 2);

nPos = sum(label == 1);

nNeg = sum(label == -1);


params.pos_to_sample_no = round(nPos / 2);

params.neg_to_sample_no = round(nNeg / 2);

   
    
    
end
