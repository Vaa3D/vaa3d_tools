%average all the image in given directory
clc
clear all
close all

% function imgavg_allindir(foldername,filename_avg)
foldername='D:\Allen\result_images\jba';

curdir=dir(foldername);
n_file=0;
for i=1:length(curdir)
    if(curdir(i).isdir && curdir(i).name(1)~='.')
        ;
    else
        if(length(curdir(i).name)>3 && curdir(i).name(end-2)=='r'&& curdir(i).name(end-1)=='a'&& curdir(i).name(end)=='w')
            filename=curdir(i).name;
%             ind=strfind(filename,'_');
%             if(length(ind)<2 || ~strcmp(filename(ind(2)+1:ind(2)+2),'01'))
%                 continue;
%             end
            
            n_file=n_file+1;
            fullfilename=[foldername,'/',curdir(i).name];
            fprintf('[%4d]: filename: %s\n',n_file,fullfilename);
                     
            %load image stack
            I=loadRaw2Stack(fullfilename);
            
            %flip z axis
            sz_I=size(I);
            I_flipz=zeros(sz_I);
            for j=1:sz_I(3)
                I_flipz(:,:,j)=I(:,:,sz_I(3)-j+1);
            end
            
            %save image
            filename_output=[foldername,'/',curdir(i).name(1:end-7),'_flipz.v3draw'];
            save_v3d_raw_img_file(uint8(I_flipz),filename_output);
            
        end
    end
end

% return;
