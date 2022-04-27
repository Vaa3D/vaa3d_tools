% This program is used to remove stripe.
% filter_axis_dir,filter_cutoff,filter_radius :These are the filter parameters to remove image stripe, which can be changed by observing the image stripe phenomenon.

function s = striperemove(image,direction,angle,cutoff,radius)

    imgsize=size(image);
    if direction==1
        temp=imgsize(2);
        imgsize(2)=imgsize(3);
        imgsize(3)=temp;
    elseif direction==2
        temp=imgsize(1);
        imgsize(1)=imgsize(2);
        imgsize(2)=imgsize(3);
        imgsize(3)=temp;
    end
    if direction==0
        I_slice=I_slice_xoy(image);
    elseif direction==1
        I_slice=I_slice_xoz(image);
    else
        I_slice=I_slice_yoz(image);
    end
    figure;imshow(uint8(I_slice));impixelinfo
        
    c=1;    gamma=0.3;%0.6,3
    I_tmp=c*(I_slice/255).^gamma * 255;
    figure;imshow(uint8(I_tmp)); impixelinfo
        
    Fc=fftshift(fft2(I_slice));
    S=abs(Fc);
    S2=log(1+S);
    figure;imshow(S2,[]); impixelinfo
        
    filter_axis_dir=angle;   % The angle between the filter direction and the horizontal axis
    filter_cutoff=cutoff;    % filter_cutoff
    filter_radius=radius;    % Filter width
        
    dim_max2=max(imgsize(1:2))*2;
    H_max=ones(dim_max2,dim_max2);
    centerpos_max=round(dim_max2/2);
    H_max(centerpos_max-filter_radius:centerpos_max+filter_radius,:)=0;
    H_max(:,centerpos_max-filter_cutoff:centerpos_max+filter_cutoff)=1;
        
    H_max=imrotate(H_max,filter_axis_dir,'crop');
        
    H=H_max(centerpos_max-round((imgsize(1)-1)/2):centerpos_max-round((imgsize(1)-1)/2)+imgsize(1)-1,...
            centerpos_max-round((imgsize(2)-1)/2):centerpos_max-round((imgsize(2)-1)/2)+imgsize(2)-1);
        
    h_gaussian=fspecial('gaussian',[filter_radius*3,filter_radius*3],5);
    H=imfilter(H,h_gaussian,'replicate');
        
    %normalize to 0~1
    H=(H-min(H(:)))*(1.0/(max(H(:))-min(H(:))));
    figure; imshow(log(1+H.*Fc),[]);
    %do filtering slice by slice
    if direction==0
        s=stripremove_stack_xoy(image,H);
    elseif direction==1
        s=stripremove_stack_xoz(image,H);
    else
        s=stripremove_stack_yoz(image,H);
    end

function I_output=stripremove_stack_xoy(I,H)
I=double(I);
imgsize=size(I);
I_output=I;
%h1=figure;
h2=figure;
for z=1:imgsize(3)
    fprintf('%d/%d\n',imgsize(3),z);
    I_slice=double(I(:,:,z));
    
    [rows,cols] = size(I_slice);
    I_slice = double(I_slice) + ones(rows,cols);
    I_slice = log(I_slice);
    
    Fc=fftshift(fft2(I_slice));
    G=H.*Fc;
    g=real(ifft2(ifftshift(G)));
    
    g = exp(double(g))-1;
    
    I_output(:,:,z)=uint8(g);
    
    %figure(h1);imshow(uint8(I_slice));
    figure(h2);imshow(uint8(g));
    %drawnow;
end

function I_output=stripremove_stack_xoz(I,H)
I=double(I);
imgsize=size(I);
temp=imgsize(2);
imgsize(2)=imgsize(3);
imgsize(3)=temp;
I_output=I;
%h1=figure;
h2=figure;
for y=1:imgsize(3)
    fprintf('%d/%d\n',imgsize(3),y);
    I_slice0=double(I(:,y,:));
    I_slice=reshape(I_slice0,[imgsize(1) imgsize(2)]);
    %     I_slice=double(I(:,:,z));
    
    [rows,cols] = size(I_slice);
    I_slice = double(I_slice) + ones(rows,cols);
    I_slice = log(I_slice);
    
    Fc=fftshift(fft2(I_slice));
    G=H.*Fc;
    g0=real(ifft2(ifftshift(G)));
    
    g0 = exp(double(g0))-1;
    g=reshape(g0,[imgsize(1),1,imgsize(2)]);
    
    I_output(:,y,:)=uint8(g);
    
    %figure(h1);imshow(uint8(I_slice));
    figure(h2);imshow(uint8(g0));
    %drawnow;
end

function I_output=stripremove_stack_yoz(I,H)
I=double(I);
imgsize=size(I);
temp=imgsize(1);
imgsize(1)=imgsize(2);
imgsize(2)=imgsize(3);
imgsize(3)=temp;
I_output=I;
%h1=figure;
h2=figure;
for x=1:imgsize(3)
    fprintf('%d/%d\n',imgsize(3),x);
    I_slice0=double(I(x,:,:));
    I_slice=reshape(I_slice0,[imgsize(1) imgsize(2)]);
    %     I_slice=double(I(:,:,z));
    
    [rows,cols] = size(I_slice);
    I_slice = double(I_slice) + ones(rows,cols);
    I_slice = log(I_slice);
    
    Fc=fftshift(fft2(I_slice));
    G=H.*Fc;
    g0=real(ifft2(ifftshift(G)));
    
    g0 = exp(double(g0))-1;
    g=reshape(g0,[1,imgsize(1),imgsize(2)]);
    
    I_output(x,:,:)=uint8(g);
    
    %figure(h1);imshow(uint8(I_slice));
    figure(h2);imshow(uint8(g0));
    %drawnow;
end
%close(h1);close(h2);

function I_slice=I_slice_xoy(I)
imgsize=size(I);
I_slice=double(I(:,:,round(imgsize(3)/2)));

function I_slice=I_slice_xoz(I)
imgsize=size(I);
I_slice0=double(I(:,round(imgsize(2)/2),:));
I_slice=reshape(I_slice0,[imgsize(1),imgsize(3)]);
temp=imgsize(2);
imgsize(2)=imgsize(3);
imgsize(3)=temp;

function I_slice=I_slice_yoz(I)
imgsize=size(I);
I_slice0=double(I(round(imgsize(1)/2),:,:));
I_slice=reshape(I_slice0,[imgsize(2),imgsize(3)]);
temp=imgsize(1);
imgsize(1)=imgsize(2);
imgsize(2)=imgsize(3);
imgsize(3)=temp;
