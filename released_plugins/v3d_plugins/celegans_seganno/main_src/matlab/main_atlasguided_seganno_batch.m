%(batch)atlas guided auto C.elegant cell segementaion and annotation
%by Lei Qu 20100708
clc
clear all
close all
% 
% for file_ind=6:6
% 
% %read image
% switch file_ind
%     case 1  
%         filename_img='/Volumes/MyBook/markeralign/Peng/cnd1threeL1_1213061_crop_straight.raw';
%         b_upsidedown=0;
%     case 2
%         filename_img='/Volumes/MyBook/markeralign/Peng/cnd1threeL1_1228061_crop_straight.raw';
%         b_upsidedown=0;
%     case 3
%         filename_img='/Volumes/MyBook/markeralign/Peng/cnd1threeL1_1229061_crop_straight.raw';  %target
%         b_upsidedown=0;
%     case 4
%         filename_img='/Volumes/MyBook/markeralign/Peng/cnd1threeL1_1229062_crop_straight.raw';
%         b_upsidedown=1;
%     case 5
%         filename_img='/Volumes/MyBook/markeralign/Peng/cnd1threeL1_1229063_crop_straight.raw';
%         b_upsidedown=1;
%     case 6
%         filename_img='/Volumes/MyBook/markeralign/Peng/jkk1SD1439L1_0904072_crop_straight.raw';
%         b_upsidedown=1;
%     case 7
%         filename_img='/Volumes/MyBook/markeralign/Peng/jkk1SD1439L1_0904073_crop_straight.raw';
%         b_upsidedown=1;
%     case 8
%         filename_img='/Volumes/MyBook/markeralign/Peng/jkk1SD1439L1_0904074_crop_straight.raw';
%         b_upsidedown=0;
%     case 9
%         filename_img='/Volumes/MyBook/markeralign/Peng/jkk1SD1439L1_0904075_crop_straight.raw';
%         b_upsidedown=0;
% end
% 
% I_ori=loadRaw2Stack(filename_img);
% sz_I_ori=size(I_ori);
% 
% %correct xy axis
% I_ori_xy=zeros(sz_I_ori(2),sz_I_ori(1),sz_I_ori(3),sz_I_ori(4));
% for zz=1:sz_I_ori(3)
%     for cc=1:sz_I_ori(4)
%         I_ori_xy(:,:,zz,cc)=I_ori(:,:,zz,cc)';
%     end
% end
% clear zz cc I_ori sz_I_ori
% 
% %extract green channel (contain muscle cells)
% I_g=I_ori_xy(:,:,:,2);
% sz_I_g=size(I_g);
% clear I_ori_xy
% 
% %%
% %read atlas apo file
% filename_apo_atlas='/Users/qul/work/v3d_2.0/sub_projects/celegant_atlas_builder/atlas.apo';
% apo_atlas=load_v3d_apo_file(filename_apo_atlas);
% 
% %extract the muscle cells (BWM*)
% n=0;
% for i=1:size(apo_atlas,2)
%     cellname=apo_atlas{i}.name;
%     %format string
%     cellname=strtrim(cellname);
%     cellname=upper(cellname);
%     cellname(strfind(cellname,' '))='_';
%     
%     ind=strfind(cellname,'BWM');
%     if(isempty(ind))
%         continue;
%     end
%     n=n+1;
%     arr_musclecell(1,n)=apo_atlas{i}.x;
%     arr_musclecell(2,n)=apo_atlas{i}.y;
%     arr_musclecell(3,n)=apo_atlas{i}.z;
%     arr_musclecellname{n}=apo_atlas{i}.name;
% end
% 
% %%
% %resize image and cell pos
% %resize image
% resize_ratio=1/4;
% I_g_s=imresize_3D(I_g,sz_I_g*resize_ratio);
% sz_I_g_s=size(I_g_s);
% %resize cell pos
% arr_musclecell_s=arr_musclecell*resize_ratio;
% 
% %%
% %rotate the atlas points (since the worm upside down sometimes)
% if(b_upsidedown)
%     offset_yz=repmat([0;sz_I_g_s(1);sz_I_g_s(3)]/2,1,size(arr_musclecell_s,2));
%     degree=180/180*pi;
%     Rx=[1       0            0;
%         0  cos(degree) -sin(degree);
%         0 -sin(degree)  cos(degree)];
%     arr_musclecell_s=Rx*(arr_musclecell_s-offset_yz)+offset_yz;
% end
% 
% arr_musclecell_s_bk=arr_musclecell_s;
% 
% save_v3d_raw_img_file(uint8(I_g_s),['img_',num2str(file_ind),'.raw']);
% save_arr2apo(arr_musclecell_s,arr_musclecellname,'atlas.apo');
% 
% %%
% %generate foreground mask for Voronoi (kappa-gamma thresholding)
% %low threshold (should include as many as possible foreground)
% f=3;
% ind=1:(sz_I_g_s(1)*sz_I_g_s(2)*sz_I_g_s(3));
% for i=1:2
%     bg_mean=mean(I_g_s(ind));
%     bg_std=std(I_g_s(ind));
%     threshold=bg_mean+f*bg_std;
%     ind=find(I_g_s<=threshold);
% 
% %     mask=I_g_s;
% %     mask(mask>threshold)=255;
% %     mask(mask<=threshold)=0;
% %     save_v3d_raw_img_file(uint8(mask),['mask_vor',num2str(file_ind),'.raw']);
% end
% clear ind
% ind_fg=find(I_g_s>threshold);
% [mask_y,mask_x,mask_z]=ind2sub(sz_I_g_s,ind_fg);
% n_maskpixels=length(mask_x);
% 
% %%
% %global alingment mask and atlas
% %generate foreground mask for altlas initialization
% %high threshold (should exclude as many as possible background)
% fg_mean=mean(I_g_s(ind_fg));
% ind_ini=find(I_g_s>=fg_mean);
% 
% mask=zeros(size(I_g_s));
% mask(ind_ini)=255;
% save_v3d_raw_img_file(uint8(mask),['mask_ini',num2str(file_ind),'.raw']);
% 
% [tmp,mask_x_ini,tmp]=ind2sub(sz_I_g_s,ind_ini);
% 
% x_min_mask=min(mask_x_ini(:));
% x_max_mask=max(mask_x_ini(:));
% x_min_atlas=min(arr_musclecell_s(1,:));
% x_max_atlas=max(arr_musclecell_s(1,:));
% 
% offset_mask=ones(1,length(arr_musclecell_s(1,:)))*x_min_mask;
% offset_atlas=ones(1,length(arr_musclecell_s(1,:)))*x_min_atlas;
% ratio=(x_max_mask-x_min_mask)/(x_max_atlas-x_min_atlas);
% arr_musclecell_s(1,:)=(arr_musclecell_s(1,:)-offset_atlas)*ratio+offset_mask;
% 
% save_arr2apo(arr_musclecell_s,arr_musclecellname,'atlas_ini.apo');
% 
% %%
% %global alignment: Voronoi + affine
% arr_musclecell_s_last=arr_musclecell_s;
% 
% for iter_2=1:2
%     %use the bounding box of the first align result to initialize the atlas
%     if(iter_2==2)
%         x_min_0=min(arr_musclecell_s_bk(1,:));
%         x_max_0=max(arr_musclecell_s_bk(1,:));
%         x_min_1=min(arr_musclecell_tmp(1,:));
%         x_max_1=max(arr_musclecell_tmp(1,:));
%         
%         ratio=(x_max_1-x_min_1)/(x_max_0-x_min_0);
%         offset_0=ones(1,length(arr_musclecell_s(1,:)))*x_min_0;
%         offset_1=ones(1,length(arr_musclecell_s(1,:)))*x_min_1;
%         
%         arr_musclecell_s=arr_musclecell_s_bk;
%         arr_musclecell_s(1,:)=(arr_musclecell_s(1,:)-offset_0)*ratio+offset_1;
%         
%         save_arr2apo(arr_musclecell_s,arr_musclecellname,'atlas_ini.apo');
%     end
%     
%     %compute distance matrix for every valid point to marker points
%     for iter=1:100
%         iter
% 
%         arr_musclecell_tmp=arr_musclecell_s;
%         arr_musclecell_tmp_last=arr_musclecell_tmp;
%         for iter_vor=1:100
% 
%             %compute distance matrix of foreground pixels to atlas points
%             for i=1:size(arr_musclecell_s,2)
%                 d(i,:)=sum(([mask_x';mask_y';mask_z']-repmat(arr_musclecell_tmp(:,i),1,n_maskpixels)).^2);
%             end
%             %find the nearest marker index for every mask point
%             [YY,II]=min(d); %index of ind_fg
%             clear YY d
% 
%             for i=1:size(arr_musclecell_s,2)
%                 %find the voronoi region for every marker
%                 ind_i=find(II==i);
%                 [y,x,z]=ind2sub(sz_I_g_s,ind_i);
% 
%                 if(length(ind_i)~=0)
%                     D=sum(repmat(I_g_s(ind_fg(ind_i)),1,3).*[mask_x(ind_i),mask_y(ind_i),mask_z(ind_i)],1);%sum of intensity weighted coordinate in current voronoi region
%                     Nk=sum(I_g_s(ind_fg(ind_i))); %sum of intensity in current voronoi region
%                     if(Nk~=0)
%                         E_image=D./Nk;
%                     end
%                     
%                     arr_musclecell_tmp(:,i)=E_image';
%                 end
% 
%             end
%             
% %             %deal with the nonvalid point (null Voronoi region)
% %             for i=1:size(arr_musclecell_tmp,2)
% %                 if(isnan(arr_musclecell_tmp(1,i)))
% %                     % method 1: 
% %                     arr_musclecell_tmp(:,i)=arr_musclecell_tmp_last(:,i);
% %                     % method 2:
% % %                     dis_i2all=sum((repmat(arr_musclecell_tmp_last(:,i),1,size(arr_musclecell_s,2))-arr_musclecell_tmp_last).^2,1);
% % %                     [YY,ind_i2nearest]=sort(dis_i2all);  clear YY
% % %                     for k=2:size(arr_musclecell_tmp,2)
% % %                         if(~isnan(arr_musclecell_tmp(1,ind_i2nearest(k))))
% % %                             arr_musclecell_tmp(:,i)=arr_musclecell_tmp(:,ind_i2nearest(k));
% % %                             break;
% % %                         end
% % %                     end
% %                 end
% %             end
% 
%             %compute the total change of Voronoi mass center
%             diff=arr_musclecell_tmp-arr_musclecell_tmp_last;
%             change_vor=sum(sum(diff.^2,1).^0.5);
%             fprintf('\t[%d/%d]: change_vor=%f\n',iter,iter_vor,change_vor);
%             %exit voronoi mass center estimation loop if total pos change is small enough
%             if(change_vor<1e-3)
%                 break;
%             end
% 
%             arr_musclecell_tmp_last=arr_musclecell_tmp;
% 
%         end
% 
%         %estimate the best fitting affine transform
%         T=affine3D_model(arr_musclecell_s,arr_musclecell_tmp);  %T*X_sub=X_tar
% 
%         %update the cell postion
%         tmp=T*[arr_musclecell_s;ones(1,size(arr_musclecell_s,2))];
%         arr_musclecell_s=tmp(1:3,:);
% 
%         %save img and apo
%         save_arr2apo(arr_musclecell_s,arr_musclecellname,['affine_',num2str(file_ind),'.apo']);
%         save_arr2apo(arr_musclecell_tmp,arr_musclecellname,['noaffine_',num2str(file_ind),'.apo']);
% 
%         %compute the total pos change
%         diff=arr_musclecell_s-arr_musclecell_s_last;
%         change=sum(sum(diff.^2,1).^0.5);
%         fprintf('[%d]: change=%f\n\n',iter,change);
%         arr_musclecell_s_last=arr_musclecell_s;
%         %exit global alignment loop if total pos change is small enough
%         if(change<1e-3)
%             break;
%         end
% 
%     end
% end
% 
% end


%%
load('3.mat');


arr_musclecellpos=arr_musclecell_tmp/resize_ratio;

totalfidelity=0;
%compute the fidelity of the aligned cells(better performed on the non-downsampled iamge)
radius_cell=8;
%build cell sphere xyz_range array
n_cellpixel=0;
for x=-radius_cell:radius_cell
    for y=-radius_cell:radius_cell
        for z=-radius_cell:radius_cell
            dis=(x*x+y*y+z*z)^0.5;
            if(dis<=radius_cell)
                n_cellpixel=n_cellpixel+1;
                arr_celloffset(:,n_cellpixel)=[x;y;z];
            end
        end
    end
end
%sum of intensity of current cell spheres (union of shpere and Voronoi)
n_cell=size(arr_musclecellpos,2);
for i=1:n_cell
    %index of pixels within current cell sphere
    arr_cellpixelpos_sphere=repmat(arr_musclecellpos(:,i),1,n_cellpixel)+arr_celloffset;
    arr_cellpixelind_sphere=sub2ind(size(I_g),round(arr_cellpixelpos_sphere(2,:)),round(arr_cellpixelpos_sphere(1,:)),round(arr_cellpixelpos_sphere(3,:)));
    %find the index of pixels also within current cell's Voronoi region
    for j=1:n_cell
        d(j,:)=sum((arr_cellpixelpos_sphere-repmat(arr_musclecellpos(:,j),1,n_cellpixel)).^2);
    end
    [YY,II]=min(d); clear YY d
    arr_cellpixelind=arr_cellpixelind_sphere(II==i);
    %compute the fidelity (sum of intensity) of current cell
    fidelity=sum(I_g(arr_cellpixelind));
    arr_musclecell_fidelity(i)=fidelity;
    totalfidelity=totalfidelity+fidelity;
end
fprintf('totalfidelity=%d\n',totalfidelity);
[tmp,fid_ind]=sort(arr_musclecell_fidelity);


%%
% %refine cell position (should performed on non-downsampled iamge)
% %Guassian smooth image
% % I_g=smooth3(I_g,'gaussian');
% save_v3d_raw_img_file(uint8(I_g),'I_smooth.raw');
% 
% %refined point position: reposite the points to their local maixmal
% % (similar to k-mean)
% radius_cell=8;
% %build xyz range array
% n_cellpixel=0;
% for x=-radius_cell:radius_cell
%     for y=-radius_cell:radius_cell
%         for z=-radius_cell:radius_cell
%             dis=(x*x+y*y+z*z)^0.5;
%             if(dis<=radius_cell)
%                 n_cellpixel=n_cellpixel+1;
%                 arr_celloffset(:,n_cellpixel)=[x;y;z];
%             end
%         end
%     end
% end
% %mass center of current cell sphere
% n_cell=size(arr_musclecell_s,2);
% arr_musclecell_refine=arr_musclecell_tmp/resize_ratio;
% arr_musclecell_refine_last=arr_musclecell_refine;
% for iter=1:10
%     iter
%     for i=1:n_cell
%         arr_cellpixelpos=repmat(arr_musclecell_refine(:,i),1,n_cellpixel)+arr_celloffset;
%         arr_cellpixelind=sub2ind(size(I_g),round(arr_cellpixelpos(2,:)),round(arr_cellpixelpos(1,:)),round(arr_cellpixelpos(3,:)));
%         masscenter=sum(repmat(I_g(arr_cellpixelind),3,1).*arr_cellpixelpos,2)/sum(I_g(arr_cellpixelind));
%         arr_musclecell_refine(:,i)=masscenter;
%     %     I_g(arr_cellpixelind)=255;
%     end
%     
%     change=sum((sum((arr_musclecell_refine_last-arr_musclecell_refine).^2,1)).^0.5);
%     arr_musclecell_refine_last=arr_musclecell_refine;
%     fprintf('[%d]: change=%f\n',iter,change);
% end
% save_arr2apo(arr_musclecell_refine,arr_musclecellname,'refine.apo');
% save_arr2apo(arr_musclecell_tmp/resize_ratio,arr_musclecellname,'nonrefine.apo');
% % save_v3d_raw_img_file(uint8(I_g),'tmp.raw');