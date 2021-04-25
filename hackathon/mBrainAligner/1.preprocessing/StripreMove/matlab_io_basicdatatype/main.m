function main
clc
clear all
close all
foldername_input='D:\Allen\data\新建文件夹';
foldername_output='D:\Allen\data';
% foldername_input='./545_ori';
% foldername_output='./545_stripremoved';
curdir=dir(foldername_input); %输入文件夹名称，输入文件夹里放的是2D切片%结构体
n_file=0;
%h1=figure();h2=figure;   %创建两个新的窗口
for i=1:length(curdir)   %从第一个切片到最后一个切片
    if(curdir(i).isdir && curdir(i).name(1)~='.')%%不是要处理的文件什么都不做
        ;
    else     %%如果是所需文件，进行下述操作
        if(length(curdir(i).name)>3 && curdir(i).name(end-2)=='r'&& curdir(i).name(end-1)=='a'&& curdir(i).name(end)=='w')
		%%判断是不是tif文件，是则执行下述操作
            filename=curdir(i).name;            
            n_file=n_file+1;
            fullfilename=[foldername_input,'/',curdir(i).name];%%取得输入路径文件夹下第n_file个tif文件
            fprintf('[%4d]: filename: %s\n',n_file,fullfilename);%%打印文件名及是第几个
            
% %             if(~strcmpi(filename,'mouse17545redl5_s20100.png')) continue; end
%             if(~strcmpi(filename,'mouse17302l5_s20100.png')) continue; end
            I = loadRaw2Stack(fullfilename);  %%读入该文件
            I_output=stripremove(I);    %%对该文件进行stripremove后输出，这是2D的操作
            
            figure(h1);imshow(I);        %%展示去条纹前的图像
            figure(h2);imshow(I_output);  %%展示去条纹后的图像
            drawnow;
            save_v3d_raw_img_file(I_output,[foldername_output,'/',filename]);
            %imwrite(I_output,[foldername_output,'/',filename]);   %%将去过后的图像保存
        end
    end
end

function I_output=stripremove(I)
I=double(I);
imgsize=size(I);
%中心化的频谱
    Fc=fftshift(fft2(I1));    
%%fftshift将零频变量移到频谱中心
    S=abs(Fc);
 %对数增强并居中后的频谱
    S2=log(1+S/max(S(:))*1000);% S2=log(1+S);
     figure;imshow(S2,[])
 impixelinfo 
%构建限波滤波器，阻带很窄的带阻滤波器，阻止某一频率点的信号通过
  radius=30;%60
  radius_notch=3;
  centerpos=round(imgsize/2);
  H=ones(size(I1)); %%构建全1矩阵
%构建带状限波滤波器（效果更好）
H(centerpos(1)-radius_notch:centerpos(1)+radius_notch,1:centerpos(2)-radius+radius_notch)=0;
H(centerpos(1)-radius_notch:centerpos(1)+radius_notch,centerpos(2)+radius-radius_notch:end)=0;
G=H.*Fc;
 figure;imshow(log(1+abs(G)/max(abs(G(:)))*1000),[])
g=real(ifft2(ifftshift(G)));
I_output=uint8(g);


%save_v3d_raw_img_file(uint8(I_new),filename_avg);
% figure;imshow(uint8(g));
