%remove stripe 
%by Lei Qu @ 20190415

%****
%filter_axis_dir,filter_cutoff,filter_radius :These are the filter parameters to remove image stripe, which can be changed by observing the image stripe phenomenon
%****

function f = stripremove(foldername_input,foldername_output,angle,cutoff,radius)
%clc
%clear all
%close all
addpath(genpath(pwd))
%foldername_input='D:\Allen\Data\17300\ori';            %Input filepath
%foldername_output='D:\Allen\Data\17300\stripremove';   %Output filepath
curdir=dir(foldername_input);
n_file=0;

for i=1:length(curdir)
    if(curdir(i).isdir && curdir(i).name(1)~='.');
    else
        if(length(curdir(i).name)>3 && curdir(i).name(end-2)=='r'&& curdir(i).name(end-1)=='a'&& curdir(i).name(end)=='w')
            n_file=n_file+1;
            fullfilename=[foldername_input,'/',curdir(i).name];
            fprintf('[%4d]: filename: %s\n',n_file,fullfilename);
            
            I=loadRaw2Stack(fullfilename);  
            imgsize=size(I);
            I_slice=double(I(:,:,round(imgsize(3)/2)));  
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
            I_output=stripremove_stack(I,H);
            %save image
            filename_output=[foldername_output,'/',curdir(i).name(1:end-7),'_stripremove.v3draw'];
            save_v3d_raw_img_file(uint8(I_output),filename_output);
			f = filename_output
        end
    end
end

function I_output=stripremove_stack(I,H)
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
%close(h1);close(h2);
