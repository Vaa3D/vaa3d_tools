%atlas guided auto C.elegant cell segementaion and annotation by deterministic annealing
%where the distortion is modeled by affine
%by Lei Qu 20100713
clc
clear all
close all

%read image
for file_ind=9:9
switch file_ind
    case 1  
        filename_img='/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/data/cnd1threeL1_1213061_crop_straight.raw';
        b_upsidedown=0;
    case 2
        filename_img='/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/data/cnd1threeL1_1228061_crop_straight.raw';
        b_upsidedown=0;
    case 3
        filename_img='/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/data/cnd1threeL1_1229061_crop_straight.raw';  %target
        b_upsidedown=0;
    case 4
        filename_img='/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/data/cnd1threeL1_1229062_crop_straight.raw';
        b_upsidedown=1;
    case 5
        filename_img='/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/data/cnd1threeL1_1229063_crop_straight.raw';
        b_upsidedown=1;
    case 6
        filename_img='/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/data/jkk1SD1439L1_0904072_crop_straight.raw';
        b_upsidedown=1;
    case 7
        filename_img='/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/data/jkk1SD1439L1_0904073_crop_straight.raw';
        b_upsidedown=1;
    case 8
        filename_img='/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/data/jkk1SD1439L1_0904074_crop_straight.raw';
        b_upsidedown=0;
    case 9
        filename_img='/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/data/jkk1SD1439L1_0904075_crop_straight.raw';
        b_upsidedown=0;
end
I_ori=loadRaw2Stack(filename_img);
I_ori=double(I_ori);

%correct xy axis
I_ori_xy=permute(I_ori,[2,1,3,4]);
clear I_ori

%extract green channel (contain muscle cells)
I_g=I_ori_xy(:,:,:,2);
sz_I_g=size(I_g);
clear I_ori_xy

%%
%read atlas apo file
filename_apo_atlas='/Users/qul/work/v3d_2.0_bk/sub_projects/celegant_atlas_builder/atlas.apo';
% filename_apo_atlas='/Volumes/MyBook/markeralign/Peng/cnd1threeL1_1229062.apo';
apo_atlas=load_v3d_apo_file(filename_apo_atlas);

%extract the muscle cells (BWM*)
n_musclecell=0;
for i=1:size(apo_atlas,2)
    cellname=apo_atlas{i}.name;
    %format string
    cellname=strtrim(cellname);
    cellname=upper(cellname);
    cellname(strfind(cellname,' '))='_';
    
    ind_BWM=strfind(cellname,'BWM');
    ind_SPH=strfind(cellname,'SPH');
    ind_DEP=strfind(cellname,'DEP');
    if(isempty(ind_BWM) && isempty(ind_SPH) && isempty(ind_DEP))
        continue;
    end
    n_musclecell=n_musclecell+1;
    arr_musclecell(1,n_musclecell)=apo_atlas{i}.x;
    arr_musclecell(2,n_musclecell)=apo_atlas{i}.y;
    arr_musclecell(3,n_musclecell)=apo_atlas{i}.z;
    arr_musclecellname{n_musclecell}=apo_atlas{i}.name;
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
%rotate the atlas points (since the worm upside down sometimes)
if(b_upsidedown)
    offset_yz=repmat([0;sz_I_g_s(1);sz_I_g_s(3)]/2,1,size(arr_musclecell_s,2));
    degree=180/180*pi;
    Rx=[1       0            0;
        0  cos(degree) -sin(degree);
        0 -sin(degree)  cos(degree)];
    arr_musclecell_s=Rx*(arr_musclecell_s-offset_yz)+offset_yz;
end

arr_musclecell_s_bk=arr_musclecell_s;

save_v3d_raw_img_file(uint8(I_g_s),['img_',num2str(file_ind),'.raw']);
save_arr2apo(arr_musclecell_s,arr_musclecellname,'atlas.apo');

%%
%generate foreground mask (kappa-gamma thresholding)
%low threshold (should include as many as possible foreground)
f=3;
ind=1:(sz_I_g_s(1)*sz_I_g_s(2)*sz_I_g_s(3));
for i=1:1
    bg_mean=mean(I_g_s(ind));
    bg_std=std(I_g_s(ind));
    threshold=bg_mean+f*bg_std;
    ind=find(I_g_s<=threshold);
end
clear ind
ind_fg=find(I_g_s>threshold);
[mask_y,mask_x,mask_z]=ind2sub(sz_I_g_s,ind_fg);
n_maskpixels=length(mask_x);


%%
% load('1.mat');
I_inv=ones(size(I_g_s))*255-I_g_s;
dis_norm=max(size(I_g_s));  %normalize the dis between [0~1], so that when image size change, we do not need to change tmperature

%initialize the temperature (in proportion to the energy)
temperature=5;
annealing=0.95;

%build cell sphere xyz_range array
radius_cell=10;
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

fig=figure; 
mat_matching=zeros(length(ind_fg),n_musclecell);
arr_musclecell_s_last=arr_musclecell_s;
for iter_annealing=1:1000
    
    %plot cell point
    clf(fig); grid on; axis on; axis equal; hold on
    for i=1:n_musclecell
        plot3(arr_musclecell_s_bk(1,i),arr_musclecell_s_bk(2,i),arr_musclecell_s_bk(3,i),'or');
        plot3(arr_musclecell_s(1,i),arr_musclecell_s(2,i),arr_musclecell_s(3,i),'+b');
    end
    hold off; drawnow;
    
    arr_musclecell_tmp=arr_musclecell_s;
    n_innermaxiter=1;
    if(temperature<0.2)
        n_innermaxiter=10;
        temperature=0.1;
    end
    for iter=1:n_innermaxiter
%         iter
        %Given cellpos Update match
        for i=1:n_musclecell
            dis_x2yi=repmat(arr_musclecell_tmp(:,i),1,length(ind_fg))-[mask_x';mask_y';mask_z'];
            dis_x2yi=(sum(dis_x2yi.^2,1)).^0.5/dis_norm;
            dis_x2yi=I_inv(ind_fg)'.*dis_x2yi;

            exp_tmp=exp(-dis_x2yi/temperature);
            mat_matching(:,i)=exp_tmp';%Gibbs distribution: P(yi/x)
        end
        mat_matching=mat_matching./repmat(sum(mat_matching,2),1,n_musclecell);
        
        %update centroid (intensity weighted)
        for i=1:n_musclecell
            N=sum(mat_matching(:,i));
            arr_musclecell_tmp(1,i)=sum(mat_matching(:,i).*mask_x)/N;
            arr_musclecell_tmp(2,i)=sum(mat_matching(:,i).*mask_y)/N;
            arr_musclecell_tmp(3,i)=sum(mat_matching(:,i).*mask_z)/N;
%             N=sum(I_g_s(ind_fg).*mat_matching(:,i));
%             arr_musclecell_tmp(1,i)=sum(I_g_s(ind_fg).*mat_matching(:,i).*mask_x)/N;
%             arr_musclecell_tmp(2,i)=sum(I_g_s(ind_fg).*mat_matching(:,i).*mask_y)/N;
%             arr_musclecell_tmp(3,i)=sum(I_g_s(ind_fg).*mat_matching(:,i).*mask_z)/N;
        end
        
        save_arr2apo(arr_musclecell_tmp,arr_musclecellname,['noaffine',num2str(file_ind),'.apo']);
    end
           
    %update transform
    %T=affine3D_model(X_sub,X_tar) <=> T*X_sub=X_tar
    T=affine3D_model(arr_musclecell_s,arr_musclecell_tmp);

    %update constrainted cellpos
    tmp=T*[arr_musclecell_s;ones(1,n_musclecell)];
    arr_musclecell_s=tmp(1:3,:);
    save_arr2apo(arr_musclecell_s,arr_musclecellname,['affine',num2str(file_ind),'.apo']);
    
    %compute the total pos change (affine)
    diff=arr_musclecell_s-arr_musclecell_s_last;
    sum_poschange=sum(sum(diff.^2,1).^0.5);
    arr_musclecell_s_last=arr_musclecell_s;
    
%     %compute the energy (sum of intensity of current cell spheres (union of shpere and Voronoi))
%     totalfidelity=0;
%     for i=1:n_musclecell
%         %index of pixels within current cell sphere
%         arr_cellpixelpos_sphere=repmat(arr_musclecell_tmp(:,i),1,n_cellpixel)+arr_celloffset;
%         arr_cellpixelind_sphere=sub2ind(size(I_g_s),round(arr_cellpixelpos_sphere(2,:)),round(arr_cellpixelpos_sphere(1,:)),round(arr_cellpixelpos_sphere(3,:)));
%         %find the index of pixels also within current cell's Voronoi region
%         for j=1:n_musclecell
%             d(j,:)=sum((arr_cellpixelpos_sphere-repmat(arr_musclecell_tmp(:,j),1,n_cellpixel)).^2);
%         end
%         [YY,II]=min(d); clear YY d
%         arr_cellpixelind=arr_cellpixelind_sphere(II==i);
%         %compute the fidelity (sum of intensity) of current cell
%         fidelity=sum(I_g(arr_cellpixelind));
%         arr_musclecell_fidelity(i)=fidelity;
%         totalfidelity=totalfidelity+fidelity;
%     end
%     fprintf('[%d]: T=%f, sum_poschange=%f, energy=%f\n',iter_annealing,temperature,sum_poschange,totalfidelity);
    fprintf('[%d]: T=%f, sum_poschange=%f\n',iter_annealing,temperature,sum_poschange);
    
    %exit global alignment loop if total pos change is small enough
    if(sum_poschange<0.1)
        temperature=0.2;
    end

    %decrease temperature
    if(temperature<0.2)
        break;
    end
    temperature=temperature*annealing;

end


% %compute the final energy (sum of intensity of current cell spheres (union of shpere and Voronoi))
% totalfidelity=0;
% arr_musclecell=arr_musclecell_tmp/resize_ratio;%cell pos on the full resolution image
% for i=1:n_musclecell
%     %index of pixels within current cell sphere
%     arr_cellpixelpos_sphere=repmat(arr_musclecell(:,i),1,n_cellpixel)+arr_celloffset;
%     arr_cellpixelind_sphere=sub2ind(size(I_g),round(arr_cellpixelpos_sphere(2,:)),round(arr_cellpixelpos_sphere(1,:)),round(arr_cellpixelpos_sphere(3,:)));
%     %find the index of pixels also within current cell's Voronoi region
%     for j=1:n_musclecell
%         d(j,:)=sum((arr_cellpixelpos_sphere-repmat(arr_musclecell(:,j),1,n_cellpixel)).^2);
%     end
%     [YY,II]=min(d); clear YY d
%     arr_cellpixelind=arr_cellpixelind_sphere(II==i);
%     %compute the fidelity (sum of intensity) of current cell
%     fidelity=sum(I_g(arr_cellpixelind));
%     arr_musclecell_fidelity(i)=fidelity;
%     totalfidelity=totalfidelity+fidelity;
% end
% fprintf('FinalEnergy=%f\n',totalfidelity);


end

