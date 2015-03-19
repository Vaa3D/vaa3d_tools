function [fn,params_trn,params_tst] = GetFNIdxs_OPF

rng(2014);

DATASET_NAME = 'OPF';

params = setup_config_neuron(DATASET_NAME);

params.codename = date;

params = setup_lists(params);
params = setup_directories_L(params);

% save(fullfile(params.results_dir,'params.mat'),'params','-v7.3');

%% Data loading
fprintf('  Loading data...\n');

[trn,tst] = load_data_lists_OPF;

[params_trn,fn1] = load_fn_OPF(params,trn,'train');

[params_tst,fn2] = load_fn_OPF(params,tst,'test');


fn.train = fn1.train;

fn.test = fn2.test;




% load('data_DRIVE_v2.mat');


%% Sample acquisition

%[params_trn,samples_idx_trn] = acquire_sample_DRIVE(params,data,'train');

%[params_tst,samples_idx_tst] = acquire_sample_DRIVE(params,data,'test');


fn.train.imgs.sub_ch_no = 1;

fn.train.imgs.idxs = 1; 


fn.test.imgs.sub_ch_no = 1;

fn.test.imgs.idxs = 1; 



