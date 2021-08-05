%average all the image in given directory
clc
clear all
close all

% function imgavg_allindir(foldername,filename_avg)
foldername='D:\Allen\result_images\ssd';
outpath='D:\Allen\result_images';

%I_avg=[];

curdir=dir(foldername);
n_file=0;
for i=1:length(curdir)
    if(curdir(i).isdir && curdir(i).name(1)~='.')
        ;
    else
        if(length(curdir(i).name)>3 && curdir(i).name(end-2)=='r'&& curdir(i).name(end-1)=='a'&& curdir(i).name(end)=='w')
            filename=curdir(i).name;
           n_file=n_file+1;
            fullfilename=[foldername,'/',curdir(i).name];
            fprintf('[%4d]: filename: %s\n',n_file,fullfilename);
                 
            %load image stack
            I=loadRaw2Stack(fullfilename);
            [m n z]=size(I);
            %%·­×ª,t=b,b=a,a=t
          
           
			    for k=1:z
                    tmp=I(i,j,k);
                    I(i,j,k)=I(i,j,z-k+1);
                    I(i,j,z-k)=tmp;
				 end
			
            
             filename=filename(1:end-6);
            newname = strcat(filename,'_f','.v3draw');
            save_v3d_raw_img_file(uint8(I), newname);
            
           
            
        end
    end
  
  
end
%I_avg=I_avg/n_file;

%save image

% return;
