function [conv_features,sep_features] = convolve_sep_comb(img,separable_filters,weights,scale_factor)
% conv_features = convolve_sep_comb(I,S,W)
% 
% convolves one 2D or 3D image with a bank of separable filters S,
% multiplies result by weights W to obtain convolutions with full rank filter bank F = { Fj }_j=1^J ,
% where Fj = sum_k( W(k,j)Sk ) and Sk is the k-th separable filter
%
%output: - conv_features = a matrix of size numel(I) x J:
%           conv_features(:,j) = ( I**F{j}  )(:) 
%   - sep_features = a matrix of size numel(I) x K:
%           sep_features(:,k) = ( I**Sk  )(:), for k = 1,...,K
%input: -I = 2d or 3d image
%   -S = matrix containing separable filters, each column corresponds to
%        the separable components of one filter
%        it assumes squared (or cubic) filters 
%        for example, for 2D filetrs Sk = S(1:end/2,k)*S([end/2+1:end],k)'; 
%   - W = wheights matrix 
%
%
% to obatain convolution I**F{j} with same size of input image:
%  conv_features_j_size_im = reshape( conv_features(:,j) ,size(img));
% and the same for separable convolution I**Sk : 
% sep_features_k_size_im = reshape(sep_features(:,k) ,size(img));



if(nargin<4)
    scale_factor = 1;
end

size_img = size(img);
dimension = numel(size_img);

%single precision
%img = im2single(img);
%separable_filters = single(separable_filters);
%weights = single(weights);
%sep_features = zeros(prod(size_img), size(weights,1),'single');

%double precision
img = im2double(img);
separable_filters = double(separable_filters);
weights = double(weights);
sep_features = zeros(prod(size_img), size(weights,1),'double');

if( scale_factor ~= 1)
    separable_flag = 1;
    [separable_filters ,size_filters_scaled]= scale_filter_bank_mat(separable_filters,scale_factor,dimension,separable_flag);
end

%% separable convolution
switch dimension
    case 2                                
            for i_kernel = 1:size(separable_filters,2),
                kernel = separable_filters(:,i_kernel);
                filters_size  = ones(1,2)*length(kernel)/2; % assume squared filter
                size_half_kernel = ceil((filters_size-1)/2);
                %size_img_extended = size_img+2*size_half_kernel;

                central_part_fm_x = size_half_kernel(1)+(1:size_img(1));
                central_part_fm_y = size_half_kernel(2)+(1:size_img(2));

                 % assume sqyared filter
                A = kernel(1:end/2);
                B = kernel(end/2+1:end);

               %symmetric boundary conditions
                fm_temp = conv2(padarray(img,size_half_kernel,'symmetric','both'), A(end:-1:1),'same'); %convolution in x
                fm_temp = conv2(fm_temp', B(end:-1:1),'same')'; %convolution in y
                fm_temp=fm_temp(central_part_fm_x,central_part_fm_y);

                sep_features(:,i_kernel) =fm_temp(:);        
            end
    case 3
            
            
            for i_kernel = 1:size(separable_filters,2),
                kernel = separable_filters(:,i_kernel);
                filters_size  = ones(1,3)*length(kernel)/3; % assume cubic filter
                size_half_kernel = ceil((filters_size-1)/2);
                size_img_extended = size_img+2*size_half_kernel;

                central_part_fm_x = size_half_kernel(1)+(1:size_img(1));
                central_part_fm_y = size_half_kernel(2)+(1:size_img(2));
                central_part_fm_z = size_half_kernel(3)+(1:size_img(3));


                %extract separable components : filter_i = A(:,i)oB(:,i)oC(:,i)
                %A = fb(1:filters_size(1),:);
                %B = fb(filters_size(1)+1:filters_size(1)+filters_size(2),:);
                %C = fb(filters_size(1)+filters_size(2)+1:filters_size(1)+filters_size(2)+filters_size(3),:);

                 % assume cubic filter
                A = kernel(1:end/3);
                B = kernel(end/3+1:2*end/3);
                C = kernel(2*end/3+1:end);

              %for i_fm = 1:sep_filters_no, % number of separable filters
                % no boundary conditions
               % fm_temp = conv2(reshape(img,size_img(1),size_img(2)*size_img(3)), A(end:-1:1),'same'); %convolution in x
               % fm_temp = conv2(reshape(fm_temp.',size_img(2),size_img(1)*size_img(3)), B(end:-1:1),'same'); %convolution in y
               % fm_temp = conv2(reshape(fm_temp.',size_img(3),size_img(1)*size_img(2)), C(end:-1:1),'same'); %convolution in z
               % fm_temp= reshape(fm_temp.',size_img);

                %symmetric boundary conditions
                fm_temp = conv2(reshape(padarray(img,size_half_kernel,'symmetric','both'),size_img_extended(1),size_img_extended(2)*size_img_extended(3)), A(end:-1:1),'same'); %convolution in x
                fm_temp = conv2(reshape(fm_temp.',size_img_extended(2),size_img_extended(1)*size_img_extended(3)), B(end:-1:1),'same'); %convolution in y
                fm_temp = conv2(reshape(fm_temp.',size_img_extended(3),size_img_extended(1)*size_img_extended(2)), C(end:-1:1),'same'); %convolution in z
                fm_temp= reshape(fm_temp.',size_img_extended);
                fm_temp = fm_temp(central_part_fm_x,central_part_fm_y,central_part_fm_z);

                sep_features(:,i_kernel) = fm_temp(:);        
            end
    otherwise
        error('unsupported dimension: %i', dimension)
end


%% full rank convolution
conv_features = sep_features*weights;

conv_features = conv_features/(scale_factor.^(dimension)); % normalize by area 


