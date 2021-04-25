%remove stripe 
%by Lei Qu @ 20190415

function main
clc
clear all
close all

foldername_input='G:\stripremove\17787';
foldername_output='G:\stripremove\17787';
curdir=dir(foldername_input);
n_file=0;
% h1=figure();h2=figure;
for i=1:length(curdir)
    if(curdir(i).isdir && curdir(i).name(1)~='.')
        ;
    else
        if(length(curdir(i).name)>3 && curdir(i).name(end-2)=='r'&& curdir(i).name(end-1)=='a'&& curdir(i).name(end)=='w')
            n_file=n_file+1;
            fullfilename=[foldername_input,'/',curdir(i).name];
            fprintf('[%4d]: filename: %s\n',n_file,fullfilename);
            
            I=loadRaw2Stack(fullfilename);
            
            %检视图像频谱并确定参数(filter_axis_dir, filter_cutoff, filter_radius)
            imgsize=size(I);
            I_slice=double(I(:,:,round(imgsize(3)/2)));
            figure;imshow(uint8(I_slice));impixelinfo 
            %显示对比度增强后图像
            c=1;    gamma=0.3;%0.6,3
            I_tmp=c*(I_slice/255).^gamma * 255;
            figure;imshow(uint8(I_tmp)); impixelinfo 
            %显示频谱
            Fc=fftshift(fft2(I_slice));
            S=abs(Fc);
            S2=log(1+S);
            figure;imshow(S2,[]); impixelinfo
            
            %construct filter
%             %构建矢状限波滤波器(效果不好！)
%             centerpos=round(imgsize(1:2)/2);
%             filter_axis_dir=0;%滤波器方向同横轴夹角
%             filter_cutoff=25;%滤波器通带截止频率
%             filter_axis_deg=2;%矢状滤波器夹角的一半
%             H=ones(imgsize(1:2));
%             for row=1:imgsize(1)
%                 for col=1:imgsize(2)
%                     offset=[row,col]-centerpos;
%                     dis2center=sqrt(sum(offset.^2));
%                     degree2x=atan(offset(1)/offset(2))/pi*180;
%                     degree2axis=abs(filter_axis_dir-degree2x);
%                     if(dis2center>=filter_cutoff && degree2axis<=filter_axis_deg)
%                         H(row,col)=0;
%                     end
%                 end
%             end
%             figure; imshow(H);
            %构建带状限波滤波器
            filter_axis_dir=175;%滤波器方向同横轴夹角(应按照条纹方向手工选择合适的角度)
            filter_cutoff=10;%截止频率太高将导致低频条纹不能完全消除,太低则会出现虚假纹理!越小离中心越近
            filter_radius=5;%滤波器宽度半径(该值较大时某些高亮区域周围会出现虚假低频纹理!)越小越窄
            %构建水平带状滤波器
            dim_max2=max(imgsize(1:2))*2;
            H_max=ones(dim_max2,dim_max2);
            centerpos_max=round(dim_max2/2);
            H_max(centerpos_max-filter_radius:centerpos_max+filter_radius,:)=0;
            H_max(:,centerpos_max-filter_cutoff:centerpos_max+filter_cutoff)=1;
            %旋转水平带状滤波器
            H_max=imrotate(H_max,filter_axis_dir,'crop');
            %截取同输入相同大小的滤波器
            H=H_max(centerpos_max-round((imgsize(1)-1)/2):centerpos_max-round((imgsize(1)-1)/2)+imgsize(1)-1,...
            centerpos_max-round((imgsize(2)-1)/2):centerpos_max-round((imgsize(2)-1)/2)+imgsize(2)-1);
            %将理想滤波器转换为高斯滤波器(消除滤波振铃)
%             h_gaussian=fspecial('gaussian',[filter_radius*3,filter_radius*3],filter_radius);
            h_gaussian=fspecial('gaussian',[filter_radius*3,filter_radius*3],5);
            H=imfilter(H,h_gaussian,'replicate');
            %figure; imshow(H);
            figure;imshow(H.*S2,[]); impixelinfo
            figure; imshow(log(1+H.*Fc),[]);   %%去条纹后的切片
             pause;
            %do filtering slice by slice
            I_output=stripremove_stack(I,H);


            %save image
            filename_output=[foldername_output,'/',curdir(i).name(1:end-7),'_remove.v3draw'];
            save_v3d_raw_img_file(uint8(I_output),filename_output);
        end
    end
end

function I_output=stripremove_stack(I,H)
I=double(I);
imgsize=size(I);
%滤波
I_output=I;
%h1=figure;h2=figure;
for z=1:imgsize(3)
    fprintf('%d/%d\n',imgsize(3),z);
    I_slice=double(I(:,:,z));
    Fc=fftshift(fft2(I_slice));
    G=H.*Fc;
    g=real(ifft2(ifftshift(G)));
    I_output(:,:,z)=uint8(g);
    
    %figure(h1);imshow(uint8(I_slice));
    %figure(h2);imshow(uint8(g));
    drawnow;
end
%close(h1);close(h2);
