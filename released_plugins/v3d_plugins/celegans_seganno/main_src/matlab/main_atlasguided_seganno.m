%atlas guided auto C.elegant cell segementaion and annotation
%by Lei Qu 20100705
clc
clear all
close all

%%
%read image
% filename_img='/Volumes/MyBook/markeralign/Peng/cnd1threeL1_1213061_crop_straight.raw';
% filename_img='/Volumes/MyBook/markeralign/Peng/sub2tar_warped/cnd1threeL1_1213061_sub2tar.raw'; %sub2tar
% filename_img='/Volumes/MyBook/markeralign/Peng/cnd1threeL1_1228061_crop_straight.raw';
% filename_img='/Volumes/MyBook/markeralign/Peng/cnd1threeL1_1229061_crop_straight.raw';  %target
% filename_img='/Volumes/MyBook/markeralign/Peng/cnd1threeL1_1229062_crop_straight.raw';
% filename_img='/Volumes/MyBook/markeralign/Peng/cnd1threeL1_1229063_crop_straight.raw';
filename_img='/Volumes/MyBook/markeralign/Peng/jkk1SD1439L1_0904072_crop_straight.raw';
% filename_img='/Volumes/MyBook/markeralign/Peng/jkk1SD1439L1_0904073_crop_straight.raw';
% filename_img='/Volumes/MyBook/markeralign/Peng/jkk1SD1439L1_0904074_crop_straight.raw';
% filename_img='/Volumes/MyBook/markeralign/Peng/jkk1SD1439L1_0904075_crop_straight.raw';
I_ori=loadRaw2Stack(filename_img);
sz_I_ori=size(I_ori);

%correct xy axis
I_ori_xy=zeros(sz_I_ori(2),sz_I_ori(1),sz_I_ori(3),sz_I_ori(4));
for zz=1:sz_I_ori(3)
    for cc=1:sz_I_ori(4)
        I_ori_xy(:,:,zz,cc)=I_ori(:,:,zz,cc)';
    end
end
clear zz cc I_ori sz_I_ori

%extract green channel (contain muscle cells)
I_g=I_ori_xy(:,:,:,2);
sz_I_g=size(I_g);
clear I_ori_xy

%%
%read atlas apo file
% filename_apo_atlas='/Users/qul/work/v3d_2.0/sub_projects/celegant_atlas_builder/atlas.apo';
filename_apo_atlas='/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/result_downsample4/affine_6.apo';
apo_atlas=load_v3d_apo_file(filename_apo_atlas);

%extract the muscle cells (BWM*)
n=0;
for i=1:size(apo_atlas,2)
    cellname=apo_atlas{i}.name;
    %format string
    cellname=strtrim(cellname);
    cellname=upper(cellname);
    cellname(strfind(cellname,' '))='_';
    
    ind=strfind(cellname,'BWM');
    if(isempty(ind))
        continue;
    end
    n=n+1;
    arr_musclecell(1,n)=apo_atlas{i}.x;
    arr_musclecell(2,n)=apo_atlas{i}.y;
    arr_musclecell(3,n)=apo_atlas{i}.z;
end

%%
%resize image and cell pos
%resize image
resize_ratio=1/4;
I_g_s=imresize_3D(I_g,sz_I_g*resize_ratio);
sz_I_g_s=size(I_g_s);
%resize cell pos
arr_musclecell_s=arr_musclecell*resize_ratio;


%%
% load('1.mat');

%%
%generate foreground mask (kappa-gamma thresholding)
f=3;
ind=1:(sz_I_g_s(1)*sz_I_g_s(2)*sz_I_g_s(3));
for i=1:3
    bg_mean=sum(I_g_s(ind))/length(ind);
    bg_std=std(I_g_s(ind));
    threshold=bg_mean+f*bg_std;
    ind=find(I_g_s<=threshold);

%     tmp=I_g_s;
%     tmp(tmp>threshold)=255;
%     save_v3d_raw_img_file(uint8(tmp),'mask.raw');
end
clear ind
ind_fg=find(I_g_s>threshold);
[mask_y,mask_x,mask_z]=ind2sub(sz_I_g_s,ind_fg);
n_maskpixels=length(mask_x);

%%
%global alignment: Voronoi + affine
%compute distance matrix for every valid point to marker points
arr_musclecell_s_last=arr_musclecell_s;
for iter=1:100
    iter

    arr_musclecell_tmp=arr_musclecell_s;
    arr_musclecell_tmp_last=arr_musclecell_tmp;
    for iter_vor=1:100

        %compute distance matrix of foreground pixels to atlas points
        for i=1:size(arr_musclecell_s,2)
            d(i,:)=sum(([mask_x';mask_y';mask_z']-repmat(arr_musclecell_tmp(:,i),1,n_maskpixels)).^2);
        end
        %find the nearest marker index for every mask point
        [YY,II]=min(d); %index of ind_fg
        clear YY d

        for i=1:size(arr_musclecell_s,2)
            %find the voronoi region for every marker
            ind_i=find(II==i);
            [y,x,z]=ind2sub(sz_I_g_s,ind_i);

            if(length(ind_i)~=0)
                D=sum(repmat(I_g_s(ind_fg(ind_i)),1,3).*[mask_x(ind_i),mask_y(ind_i),mask_z(ind_i)],1);%sum of intensity weighted coordinate in current voronoi region
                Nk=sum(I_g_s(ind_fg(ind_i))); %sum of intensity in current voronoi region
                if(Nk==0)
                    E_image=arr_musclecell_tmp(:,i)';
                else
                    E_image=D./Nk;
                end
            end

            arr_musclecell_tmp(:,i)=E_image';
        end
        
        %compute the total change of Voronoi mass center
        diff=arr_musclecell_tmp-arr_musclecell_tmp_last;
        change_vor=sum(sum(diff.^2,1).^0.5);
        fprintf('\t[%d/%d]: change_vor=%f\n',iter,iter_vor,change_vor);
        %exit voronoi mass center estimation loop if total pos change is small enough
        if(change_vor<1e-5)
            break;
        end
        
        arr_musclecell_tmp_last=arr_musclecell_tmp;
    
    end

    %estimate the best fitting affine transform
    T=affine3D_model(arr_musclecell_s,arr_musclecell_tmp);  %T*X_sub=X_tar

    %update the cell postion
    tmp=T*[arr_musclecell_s;ones(1,size(arr_musclecell_s,2))];
    arr_musclecell_s=tmp(1:3,:);
    
    %save img and apo
    save_arr2apo(arr_musclecell_s,'affine.apo');
    save_arr2apo(arr_musclecell_tmp,'noaffine.apo');
    save_v3d_raw_img_file(uint8(I_g_s),'img_2.raw');
    
    %compute the total pos change
    diff=arr_musclecell_s-arr_musclecell_s_last;
    change=sum(sum(diff.^2,1).^0.5);
    fprintf('[%d]: change=%f\n\n',iter,change);
    %exit global alignment loop if total pos change is small enough
    if(change<1e-5)
        break;
    end

    arr_musclecell_s_last=arr_musclecell_s;

end


%%
% load('2.mat');

%%
% %block affine optimization
% for iter=1:10
% sz_block=10;
% cellarr_localaffine=cell(size(arr_musclecell_s,2),1);
% 
% for m=1:size(arr_musclecell_s,2)
%     fprintf('%d,',m);
%     
%     %find n nestest points of current point
%     mat_dis=sum((repmat(arr_musclecell_s(:,m),1,size(arr_musclecell_s,2))-arr_musclecell_s).^2,1);
%     [tmp,ind_nearest]=sort(mat_dis);
%     
%     %compute distance matrix of foreground pixels to atlas points
%     for i=1:size(arr_musclecell_s,2)
%         d(i,:)=sum(([mask_x';mask_y';mask_z']-repmat(arr_musclecell_s(:,i),1,n_maskpixels)).^2);
%     end
%     %find the nearest point index for every point
%     [YY,II1]=min(d); %index of ind_fg
%     clear YY d
% 
%     %compute the mass center of the n nearest point's Voroni region
%     arr_musclecell_tmp=arr_musclecell_s;
%     for i=1:sz_block
%         ind_cellinarr=ind_nearest(i);
%         %find the voronoi region for given points
%         ind_i=find(II==ind_cellinarr);
%         [y,x,z]=ind2sub(sz_I_g_s,ind_i);
% 
%         if(length(ind_i)~=0)
%             D=sum(repmat(I_g_s(ind_fg(ind_i)),1,3).*[mask_x(ind_i),mask_y(ind_i),mask_z(ind_i)],1);%sum of intensity weighted coordinate in current voronoi region
%             Nk=sum(I_g_s(ind_fg(ind_i))); %sum of intensity in current voronoi region
%             if(Nk==0)
%                 E_image=arr_musclecell_tmp(:,ind_cellinarr)';
%             else
%                 E_image=D./Nk;
%             end
%         end
% 
%         arr_musclecell_tmp(:,ind_cellinarr)=E_image';
%     end
%     
%     %estimate the best fitting local affine transform
%     vec_ori=arr_musclecell_s(:,ind_nearest(1:sz_block));
%     vec_cur=arr_musclecell_tmp(:,ind_nearest(1:sz_block));
%     T=affine3D_model(vec_ori,vec_cur);  %T*X_sub=X_tar
% 
%     %affine transform the local cell position
%     vec_new=T*[vec_ori;ones(1,sz_block)];
%     
%     %record the local cell position
%     for i=1:sz_block
%         s.x=vec_new(1,i);
%         s.y=vec_new(2,i);
%         s.z=vec_new(3,i);
%         cellpos=cellarr_localaffine{ind_nearest(i)};
%         npoints=length(cellpos);
%         cellpos{npoints+1}=s;
%         cellarr_localaffine{ind_nearest(i)}=cellpos;
%     end
% end
% fprintf('\n');
% 
% %update cell postion (average all the new position for a certain cell)
% arr_musclecell_tmp=zeros(size(arr_musclecell_s));
% for i=1:length(cellarr_localaffine)
%     for j=1:length(cellarr_localaffine{i})
%         arr_musclecell_tmp(1,i)=arr_musclecell_tmp(1,i)+cellarr_localaffine{i}{j}.x;
%         arr_musclecell_tmp(2,i)=arr_musclecell_tmp(2,i)+cellarr_localaffine{i}{j}.y;
%         arr_musclecell_tmp(3,i)=arr_musclecell_tmp(3,i)+cellarr_localaffine{i}{j}.z;
%     end
%     arr_musclecell_tmp(1,i)=arr_musclecell_tmp(1,i)/length(cellarr_localaffine{i});
%     arr_musclecell_tmp(2,i)=arr_musclecell_tmp(2,i)/length(cellarr_localaffine{i});
%     arr_musclecell_tmp(3,i)=arr_musclecell_tmp(3,i)/length(cellarr_localaffine{i});
% end
% arr_musclecell_s=arr_musclecell_tmp;
% 
% save_arr2apo(arr_musclecell_s,['tmp',num2str(iter),'.apo']);
% 
% end

%%
% %compute distance matrix for every valid point to marker points
% for iter=1:100
%     iter
%     
%     arr_musclecell_s=arr_musclecell_tmp;
% 
%     %compute distance matrix of foreground pixels to atlas points
%     for i=1:size(arr_musclecell_s,2)
%         d(i,:)=sum(([mask_x';mask_y';mask_z']-repmat(arr_musclecell_s(:,i),1,n_maskpixels)).^2);
%     end
%     %find the nearest marker index for every mask point
%     [YY,II]=min(d); %index of ind_fg
%     clear YY d
% 
%     arr_musclecell_tmp=arr_musclecell_s;
%     for i=1:size(arr_musclecell_s,2)
%         %find the voronoi region for every marker
%         ind_i=find(II==i);
%         [y,x,z]=ind2sub(sz_I_g_s,ind_i);
% 
%         if(length(ind_i)~=0)
%             D=sum(repmat(I_g_s(ind_fg(ind_i)),1,3).*[mask_x(ind_i),mask_y(ind_i),mask_z(ind_i)],1);%sum of intensity weighted coordinate in current voronoi region
%             Nk=sum(I_g_s(ind_fg(ind_i))); %sum of intensity in current voronoi region
%             if(Nk==0)
%                 E_image=arr_musclecell_tmp(:,i)';
%             else
%                 E_image=D./Nk;
%             end
%         end
% 
%         arr_musclecell_tmp(:,i)=E_image';
%     end
%     
% %%
% % save_arr2apo(arr_musclecell_s,['noaffine_',num2str(iter),'.apo']);
% save_arr2apo(arr_musclecell_s,['noaffine.apo']);
% 
% end

