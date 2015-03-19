function [params] = setup_directories_L(params)

% Setup program's directories
%
%  authors: Carlos Becker, Roberto Rigamonti, CVLab EPFL
%  e-mail: name <dot> surname <at> epfl <dot> ch
%  web: http://cvlab.epfl.ch/
%  date: February 2014

params.dataset_dir = fullfile('../Results',params.dataset_name,'KB');
params.results_dir = fullfile('../results',params.dataset_name,'KB',params.codename);

% if (exist(params.results_dir,'dir'))
%     fprintf('!!! A previous results directory corresponding to the same experiment codename exists !!!\n');
%     
%     user_choice = input('  Should I continue? (yes/no) ','s');
%     if (~strcmpi(user_choice,'yes'))
%         error('  Previous directory present on disk, but user chose not to continue');
%     end
% end
[status,message,messageid] = mkdir(params.results_dir); %#ok<*ASGLU,*NASGU>

params.test_subdir_path = fullfile(params.results_dir,'final_results');
[status,message,messageid] = mkdir(params.test_subdir_path);

end
