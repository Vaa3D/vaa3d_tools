function I=imgaussian(I,sigma,siz,normalize)
% IMGAUSSIAN filters an 1D, 2D color/greyscale or 3D image with an 
% Gaussian filter. This function uses for filtering IMFILTER or if 
% compiled the fast  mex code imgaussian.c . Instead of using a 
% multidimensional gaussian kernel, it uses the fact that a Gaussian 
% filter can be separated in 1D gaussian kernels.
%
% J=IMGAUSSIAN(I,SIGMA,SIZE)
%
% inputs,
%   I: The 1D, 2D greyscale/color, or 3D input image with 
%           data type Single or Double
%   SIGMA: The sigma used for the Gaussian kernel
%   SIZE: Kernel size (single value) (default: sigma*6)
% 
% outputs,
%   J: The gaussian filtered image
%
% note, compile the code with: mex imgaussian.c -v
%
% example,
%   I = im2double(imread('peppers.png'));
%   figure, imshow(imgaussian(I,10));
% 
% Function is written by D.Kroon University of Twente (September 2009)

if(~exist('siz','var')), siz=sigma*6; end

if(sigma>0)

    % Filter each dimension with the 1D Gaussian kernels\
    if(ndims(I)==1)
        % Make 1D Gaussian kernel
        x=-ceil(siz/2):ceil(siz/2);
        H = exp(-(x.^2/(2*sigma^2)));
        H = H/sum(H(:));        
        I=imfilter(I,H, 'same' ,'replicate');
    elseif(ndims(I)==2)
        x=-ceil(siz(1)/2):ceil(siz(1)/2);
        Hx = exp(-(x.^2/(2*sigma(1)^2)));
        Hx = Hx/sum(Hx(:));
        Hx=reshape(Hx,[length(Hx) 1]);
        
        y=-ceil(siz(2)/2):ceil(siz(2)/2);
        Hy = exp(-(y.^2/(2*sigma(2)^2)));
        Hy = Hy/sum(Hy(:));
        Hy=reshape(Hy,[1 length(Hy)]);
        
        I=imfilter(imfilter(I,Hx, 'same' ,'replicate'),Hy, 'same' ,'replicate');
    elseif(ndims(I)==3)
        if(size(I,3)<4) % Detect if 3D or color image
            Hx=reshape(H,[length(H) 1]);
            Hy=reshape(H,[1 length(H)]);
            for k=1:size(I,3)
                I(:,:,k)=imfilter(imfilter(I(:,:,k),Hx, 'same' ,'replicate'),Hy, 'same' ,'replicate');
            end
        else
            x=-ceil(siz(1)/2):ceil(siz(1)/2);
            Hx = exp(-(x.^2/(2*sigma(1)^2)));
            Hx = Hx/sum(Hx(:));
            Hx=reshape(Hx,[length(Hx) 1 1]);
        
            y=-ceil(siz(2)/2):ceil(siz(2)/2);
            Hy = exp(-(y.^2/(2*sigma(2)^2)));
            Hy = Hy/sum(Hy(:));
            Hy=reshape(Hy,[1 length(Hy) 1]);
            
            z=-ceil(siz(3)/2):ceil(siz(3)/2);
            Hz = exp(-(z.^2/(2*sigma(3)^2)));
            Hz = Hz/sum(Hz(:));
            Hz=reshape(Hz,[1 1 length(Hz)]);
            
            if normalize
                M=imfilter(imfilter(imfilter(I,ones(size(Hx))/length(Hx),'same','replicate'),ones(size(Hy))/length(Hy),'same','replicate'),ones(size(Hz))/length(Hz),'same','replicate');
            end
        
            I=imfilter(imfilter(imfilter(I,Hx, 'same' ,'replicate'),Hy, 'same' ,'replicate'),Hz, 'same' ,'replicate');
            if normalize
                I=single(I)./single(M);      
            end
        end
    else
        error('imgaussian:input','unsupported input dimension');
    end
end