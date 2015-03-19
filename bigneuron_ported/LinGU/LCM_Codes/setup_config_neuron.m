function [params] = setup_config_neuron(dataset_name)

% Configuration file for the DRIVE dataset
%
%  authors: Carlos Becker, Roberto Rigamonti, CVLab EPFL
%  e-mail: name <dot> surname <at> epfl <dot> ch
%  web: http://cvlab.epfl.ch/
%  date: February 2014

%% Dataset parameters
params.dataset_name = dataset_name;

%% Boosting parameters
params.loss_type = 'exp';  % generally exp loss does a good job
params.wl_no = 2000;  	   % number of weak learners to learn
params.tree_depth = 4;	   % maximum tree depth
params.shrinkage_factor = 0.1;

%% Sampling parameters
params.sample_size = [21;21];     % patch window size, centered on pixel of interest
params.border_size = 2*(params.sample_size(1)-1);
params.border_skip_size = 4*params.sample_size(1);

params.pos_samples_no = 300000;  % how many pos/neg samples to get from training images
params.neg_samples_no = 300000;  %  at random. Sampling done at the beginning.

% how many samples to use (at random)
% to learn the convolutional kernels (for each kernel learned)
params.rand_samples_no = 10000;

% do not touch this
params.use_qws = false;
params.use_2nd_deriv = false;
params.use_uniform_random_sampling = true;

%% Channels. In this case we use the image itself only.
params.ch_list = {'imgs'};
params.ch_no = length(params.ch_list);

%% Image channel, min and max kernel size (square)
params.imgs.min_f_size = 4;
params.imgs.max_f_size = 19;

% how many filters to learn to train one weak learner
params.imgs.filters_no = 100;

% list of lambda filter smoothing factors
params.imgs.conv_reg_values = [100;500;1000];

%% Operations
params.ops = {'train','test'};
params.ops_no = length(params.ops);

%% Define codename
% params.codename = input('    Please enter a valid codename for the simulation: ','s');
% while (~all(isstrprop(params.codename,'alphanum')|params.codename=='_')||isempty(params.codename))
%     fprintf('      INVALID INPUT! Only alphanumeric strings are accepted!\n');
%     params.codename = input('      Please enter again a valid codename for the simulation: ','s');
% end
params.codename = [date '_1'];


end
