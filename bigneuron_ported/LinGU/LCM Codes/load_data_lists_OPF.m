function [trn,tst] = load_data_lists_OPF

% Load program's data
%
%  authors: Carlos Becker, Roberto Rigamonti, CVLab EPFL
%  e-mail: name <dot> surname <at> epfl <dot> ch
%  web: http://cvlab.epfl.ch/
%  date: February 2014

dirn = '../Data/Neuron/Neuron_Segmentation/';

% fl = dir('../Data/HRF/images/*.jpg');

trn{1} = '1';

trn{2} = '3';

trn{3} = '4';


tst{1} = '5';

tst{2} = '6';

tst{3} = '7';

%  img_ids  =1:3;
% 
% for i_img = 1 :  3    
%     trn{i_img} = num2str(i_img);
%     
% end
% 
% for i_img =  4:6
%     
%     tst{i_img - 3} = num2str(i_img);
%     
% end


end

%     op = 'train';
% 
% for i_img = 1 : length(trn)
%     
%     data.(op).gts.X{i_img} = imread([dirn 'manual1/' trn{i_img} '.tif']);
%     
%     data.(op).gts.X{i_img} = expand_img(data.(op).gts.X{i_img},params) > 0;
%     
%     I = double(imread([dirn 'images/' trn{i_img} '.jpg'])) / 256;
%         
%     for i_b = 1 : 3
%                
%         data.(op).imgs.X{i_img,i_b} = I(:,:,i_b);
%         
%         data.(op).imgs.X{i_img,i_b}  = expand_img(data.(op).imgs.X{i_img,i_b} ,params);
%         
%     end
%    
%     data.(op).imgs.X{i_img,4} = lap_gaussian_F(sum(I,3));        
%     
%     data.(op).imgs.X{i_img,4}  = expand_img(data.(op).imgs.X{i_img,4} ,params);
%     
%     mask = imread([dirn 'mask/' trn{i_img} '_mask.tif']);
%     
%     
%     data.(op).masks.X{i_img} = mask(:,:,1) > 0;
%      
%     data.(op).masks.X{i_img}  = expand_img(data.(op).masks.X{i_img} ,params);
% end
% 
% 
% params.gts.(op).imgs_no = 15;
% 
% params.imgs.(op).imgs_no = 15;
% 
% params.masks.(op).imgs_no = 15;
% 
% 
%     op = 'test';
% 
% for i_img = 1 : length(tst)
%     
%     data.(op).gts.X{i_img} = imread([dirn 'manual1/' tst{i_img} '.tif']);
% 
%     data.(op).gts.X{i_img} = expand_img(data.(op).gts.X{i_img},params) > 0;    
%     
%     
%     I = double(imread([dirn 'images/' tst{i_img} '.jpg'])) / 256;
%     
%     
%     
%     for i_b = 1 : 3
%        
%         data.(op).imgs.X{i_img,i_b} = I(:,:,i_b);
%         
%         data.(op).imgs.X{i_img,i_b}  = expand_img(data.(op).imgs.X{i_img,i_b} ,params);
%         
%     end
%    
%     data.(op).imgs.X{i_img,4} = lap_gaussian_F(sum(I,3));  
%     
%     data.(op).imgs.X{i_img,4}  = expand_img(data.(op).imgs.X{i_img,4} ,params);
%     
%     mask = imread([dirn 'mask/' tst{i_img} '_mask.tif']);
%     
%         data.(op).masks.X{i_img} = mask(:,:,1) > 0;
%     
%     data.(op).masks.X{i_img}  = expand_img(data.(op).masks.X{i_img} ,params);
% end
% 
% 
% params.gts.(op).imgs_no = 15;
% 
% params.imgs.(op).imgs_no = 15;
% 
% params.masks.(op).imgs_no = 15;
% 
% 
% 
% 
% end
% 
% function I = expand_img(I,params)
% 
%     I = padarray(I,[params.border_size,params.border_size],'symmetric');
% end
