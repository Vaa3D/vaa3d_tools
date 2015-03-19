function [params,fn] = load_fn_OPF(params,trn,op)

% Load program's filename
%
%  authors: Carlos Becker, Roberto Rigamonti, CVLab EPFL
%  e-mail: name <dot> surname <at> epfl <dot> ch
%  web: http://cvlab.epfl.ch/
%  date: February 2014

dirn = '../Data/Olfactory Projection Fibers/OPF_dataset/';


for i_img = 1 : length(trn)

    fn.(op).imgs.X{i_img} = [dirn 'OP_' trn{i_img} '.mat'];

    fn.(op).gts.X{i_img} = [dirn 'OP_' trn{i_img} '_gt.mat'];
    
end


params.gts.(op).imgs_no = length(trn);

params.imgs.(op).imgs_no = length(trn);

params.masks.(op).imgs_no = length(trn);



end

