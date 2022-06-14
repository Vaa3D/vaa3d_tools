% This image preprocessing program supports .nii.gz and .v3draw file formats, used for data
% downsampling, brightness optimization, removing artifact and stripe.

clc;clear;
addpath(genpath(pwd))
foldername_input='.../../../examples/subject/preprocess'; % Input filepath
foldername_output='../../../examples/subject/preprocess'; % Output filepath
curdir=dir(foldername_input);

ch_s=1; % stripe_removal switch, 0:closed; 1:open;

DOWN=2; % Downsample size
Artifact=13; % pixel of artifact,[0~255];
direction=0; % slice direction to removing stripe, 0:coronal; 1:horizontal; 2:sagittal;
angle=0; % between the filter direction and the horizontal axis [0~360];
cutoff=15; % filter cutoff;
radius=5; % filter width;


n_file=0;
format=0;

for i=1:length(curdir)
    if(curdir(i).isdir && curdir(i).name(1)~='.')
    else
        if(length(curdir(i).name)>3 && curdir(i).name(end-2)=='r'&& curdir(i).name(end-1)=='a'&& curdir(i).name(end)=='w')
            n_file=n_file+1;
            fullfilename=[foldername_input,'/',curdir(i).name];
            fprintf('[%4d]: filename: %s\n',n_file,fullfilename);
            format=1;
            image=loadRaw2Stack(fullfilename);  %%读入该文件

            image=image(2:DOWN:end,2:DOWN:end,2:DOWN:end);
            image = Artifactsremove(image,Artifact);
			image = sharpImage(image);
            image = brightnessimage(image);
            if ch_s==1
                image=striperemove(image,direction,angle,cutoff,radius);
                image=uint8(image);
            end
        elseif(length(curdir(i).name)>3 && curdir(i).name(end-2)=='n'&& curdir(i).name(end-1)=='i'&& curdir(i).name(end)=='i')
            n_file=n_file+1;
            fullfilename=[foldername_input,'/',curdir(i).name];
            fprintf('[%4d]: filename: %s\n',n_file,fullfilename);
            format=2;
            info = load_untouch_nii(fullfilename);
            image = info.img;

            image=image(2:DOWN:end,2:DOWN:end,2:DOWN:end);
            image = Artifactsremove(image,Artifact);
			image = sharpImage(image);
            image = brightnessimage(image);
            if ch_s==1
                image=striperemove(image,direction,angle,cutoff,radius);
                image=uint8(image);
            end
        elseif(length(curdir(i).name)>3 && curdir(i).name(end-1)=='g'&& curdir(i).name(end)=='z')
            n_file=n_file+1;
            fullfilename=[foldername_input,'/',curdir(i).name];
            fprintf('[%4d]: filename: %s\n',n_file,fullfilename);
            format=3;
            files = gunzip(fullfilename);
            info = load_untouch_nii(files{1});
            image = info.img;

            image=image(2:DOWN:end,2:DOWN:end,2:DOWN:end);
            image = Artifactsremove(image,Artifact);
			image = sharpImage(image);
            image = brightnessimage(image);
            if ch_s==1
                image=striperemove(image,direction,angle,cutoff,radius);
                image=uint8(image);
            end
        end

        if format==1
            filename_output=[foldername_output,'/',curdir(i).name(1:end-7),'_preprocess.v3draw'];
            save_v3d_raw_img_file(image,filename_output);
            clear image
        elseif format==2 || format==3
            nii=make_nii(image);
            filename_output=[foldername_output,'/',curdir(i).name(1:end-(3*format-2)),'_preprocess.nii'];
            save_nii(nii,filename_output);
            if format==3
                gzip(filename_output)
            end
            clear image
        end
    end
end
