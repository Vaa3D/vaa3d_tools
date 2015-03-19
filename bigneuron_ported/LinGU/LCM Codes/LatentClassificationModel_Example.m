% This is an example for the Latent Classification Model(LCM) on Neuron
% Tracing

% Use MIT Licences

% Author : Lin Gu



clear;

addpath('../3rd Party/OOF/');

addpath(genpath('../3rd Party/KernelBoost-v0.1/'));

addpath(genpath('../3rd Party/latentTree/'));

addpath(genpath('../3rd Party/Dollar toolbox/'));

addpath(genpath('../3rd Party/Skeleton3D/'));

addpath(genpath('../3rd Party/vaa3d_matlab_io/'));


% first of all, collect the necessary data (OPF in Diadiem) for training the LCM
% 

[fn,params_trn,params_tst] = GetFNIdxs_OPF;

fn.data_str = 'OPF_train';

LTC_classifier = train_LCM_OPF(fn,10);

fn.data_str = 'OPF_test';

for i_img = 1: length(fn.test.imgs.X)
    
%     mask = data.test.masks.X{i_img};
    
    [I,gt_2D] = import_3D_data(fn.test.imgs.X{i_img});
    
    sz_I = size(I);
    
    est_fn = sprintf('%s_%d',fn.data_str,i_img);
    
    est = load_OOF(est_fn,I);
    
    gt = import_3D_data(fn.test.gts.X{i_img});
    
    
    
    fprintf('start processing image... %d \n',i_img);
    
    recon_neuron = LCM_reconstruction(double(I),est,LTC_classifier);
    
%     RFM_img = RFMatting_di(I,p90,RFM,0,17);

    visualise_3D_img(recon_neuron > 0);
    
end










