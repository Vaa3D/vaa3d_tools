%atlas guided auto C.elegant cell segementaion and annotation by deterministic annealing
%where the distortion is modeled by TPS=affine+nonrigid
%by Lei Qu 20100721
clc
clear all
close all

%read image
for file_ind=1:1
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
% filename_apo_atlas='/Users/qul/work/v3d_2.0/sub_projects/atlasguided_seganno/data/jkk1SD1439L1_0904074.apo';
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

%change the atlas (for test)
% atlas_shift=[50;50;50];
% arr_musclecell_s=arr_musclecell_s+repmat(atlas_shift,1,n_musclecell);

I_inv=ones(size(I_g_s))*255-I_g_s;
dis_norm=max(size(I_g_s));  %normalize the dis between [0~1], so that when image size change, we do not need to change tmperature

%initialize the temperature (in proportion to the energy)
temperature=3;
temperature_stop=0.2;
annealing=0.95;


fig=figure; 
mat_matching=zeros(length(ind_fg),n_musclecell);
mat_disIinv=mat_matching;
arr_musclecell_s_last=arr_musclecell_s;
x=arr_musclecell_s';
vx=x; z=x;
lamda1=1000;
lamda2=0;
for iter_annealing=1:1000
    
    %plot cell point
    clf(fig); grid on; axis on; axis equal; hold on
    for i=1:n_musclecell
        plot3(arr_musclecell_s_bk(1,i),arr_musclecell_s_bk(2,i),arr_musclecell_s_bk(3,i),'or');
        plot3(arr_musclecell_s(1,i),arr_musclecell_s(2,i),arr_musclecell_s(3,i),'+b');
    end
    hold off; drawnow;
    
    arr_musclecell_tmp=arr_musclecell_s;
    n_innermaxiter=5;
    if(temperature<=temperature_stop)
        n_innermaxiter=10;
        temperature=0.1;
        lamda1=1;
    end
    for iter=1:n_innermaxiter
%         iter
        %Given cellpos Update match
        for i=1:n_musclecell
            dis_x2yi=repmat(arr_musclecell_tmp(:,i),1,length(ind_fg))-[mask_x';mask_y';mask_z'];
            ang_x2yi=(dis_x2yi(2,:).^2+dis_x2yi(3,:).^2).^0.5;  %(y^2+z^2)^0.5
            dis_x2yi=(sum(dis_x2yi.^2,1)).^0.5;
            ang_x2yi=ang_x2yi./dis_x2yi;
%             ang_x2yi=acosd(ang_x2yi);                           %angle between x-yi and X axis
            mat_disIinv(:,i)=(dis_x2yi/dis_norm);
            dis_x2yi=I_inv(ind_fg)'.*(dis_x2yi/dis_norm);
%             mat_disIinv(:,i)=dis_x2yi';

            exp_tmp=exp(-dis_x2yi/temperature);
%             exp_tmp=exp(-dis_x2yi/temperature).*ang_x2yi;
            mat_matching(:,i)=exp_tmp';%Gibbs distribution: P(yi/x)
        end
        mat_matching=mat_matching./repmat(sum(mat_matching,2),1,n_musclecell);
        
        %update centroid (intensity weighted)
        for i=1:n_musclecell
            %without intensity weighted
            N=sum(mat_matching(:,i));
            arr_musclecell_tmp(1,i)=sum(mat_matching(:,i).*mask_x)/N;
            arr_musclecell_tmp(2,i)=sum(mat_matching(:,i).*mask_y)/N;
            arr_musclecell_tmp(3,i)=sum(mat_matching(:,i).*mask_z)/N;
%             %with intensity weighted
%             N=sum(I_g_s(ind_fg).*mat_matching(:,i));
%             arr_musclecell_tmp(1,i)=sum(I_g_s(ind_fg).*mat_matching(:,i).*mask_x)/N;
%             arr_musclecell_tmp(2,i)=sum(I_g_s(ind_fg).*mat_matching(:,i).*mask_y)/N;
%             arr_musclecell_tmp(3,i)=sum(I_g_s(ind_fg).*mat_matching(:,i).*mask_z)/N;
        end
        
%         save_arr2apo(arr_musclecell_tmp,arr_musclecellname,['noTPS',num2str(file_ind),'_1.apo']);
% %         save_arr2apo(arr_musclecell_tmp,arr_musclecellname,['noTPS',num2str(file_ind),'_',num2str(iter_annealing),'.apo']);
    end
           
    %update transform
    vy=arr_musclecell_tmp';
    y=vy;
    [c_tps,d_tps]   =ctps_gen(x,vy,lamda1,lamda2);

    %update constrainted cellpos
    vx=ctps_warp_pts(x,z,c_tps,d_tps); 
    arr_musclecell_s=vx';
    save_arr2apo(arr_musclecell_s,arr_musclecellname,['TPS',num2str(file_ind),'_1.apo']);
% %     save_arr2apo(arr_musclecell_s,arr_musclecellname,['TPS',num2str(file_ind),'_',num2str(iter_annealing),'.apo']);
    
    
    %compute the total pos change (affine)
    diff=arr_musclecell_s-arr_musclecell_s_last;
    sum_poschange=sum(sum(diff.^2,1).^0.5);
    arr_musclecell_s_last=arr_musclecell_s;
    fprintf('[%d]: T=%f, sum_poschange=%f\n',iter_annealing,temperature,sum_poschange);
    
    %exit global alignment loop if total pos change is small enough
    if(sum_poschange<=0.1)
        temperature=temperature_stop;
    end

    %decrease temperature
    if(temperature<=temperature_stop)
        break;
    end
    temperature=temperature*annealing;
    lamda1=lamda1*annealing;

end


end


% load('4.mat');
% 
% %build cell sphere xyz_range array
% radius_cell=5;
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
% 
% %%
% %find the index of 4 cell stripes
% ind_VL=zeros(1,19);
% for i=1:n_musclecell
%     cellname=arr_musclecellname{i};
%     tmp=strfind(cellname,'BWMVL');
% %     tmp=strfind(cellname,'BWMVR');
%     if(isempty(tmp))
%         continue;
%     end
%     indnum=str2num(cellname(tmp+5:end));
%     if(indnum==20)
%         indnum=17;
%     elseif(indnum>=22)
%         indnum=indnum-4;
%     end
%     ind_VL(indnum)=i;
% end
% % ind_VL=zeros(1,21);
% % for i=1:n_musclecell
% %     cellname=arr_musclecellname{i};
% % %     tmp=strfind(cellname,'BWMDL');
% %     tmp=strfind(cellname,'BWMDR');
% %     if(isempty(tmp))
% %         continue;
% %     end
% %     indnum=str2num(cellname(tmp+5:end));
% %     if(indnum>=17 && indnum<=20)
% %         indnum=indnum-2;
% %     elseif(indnum>=22 && indnum<=24)
% %         indnum=indnum-3;
% %     end
% %     ind_VL(indnum)=i;
% % end
% 
% arr_musclecell_VL=arr_musclecell_tmp(:,ind_VL);
% arr_musclecell_VL_affine=arr_musclecell_s(:,ind_VL);
% 
% %compute the best fitting quadratic curve 
% %y=ax^2+bx+c   |    |a b c| |x^2|  |y|
% %z=ox^2+px+q   ==>  |o p q|*| x |= |z| ==> para_curv*X=YZ
% %                           | 1 |
% X2=[arr_musclecell_VL(1,:).^2;arr_musclecell_VL(1,:);ones(1,size(arr_musclecell_VL,2))];
% YZ=[arr_musclecell_VL(2,:);arr_musclecell_VL(3,:)];
% A=(X2'\YZ')';
% % A=(pinv(X')*YZ')';
% 
% %compute the intensity profile along the curve
% %get the point along the curve
% x_min=min(arr_musclecell_VL(1,:));
% x_span=max(arr_musclecell_VL(1,:))-min(arr_musclecell_VL(1,:));
% n_profilepoint=500;
% X=(0:n_profilepoint-1)*(x_span/n_profilepoint)+x_min*ones(1,n_profilepoint);
% X2=[X.^2;X;ones(1,length(X))];
% YZ=A*X2;
% XYZ=[X;YZ];
% XYZ=XYZ/resize_ratio;
% % for i=1:size(XYZ,2)
% %     cellarr{i}.n=i;
% %     cellarr{i}.orderinfo=num2str(i);
% %     cellarr{i}.name='';
% %     cellarr{i}.comment='';
% %     cellarr{i}.pixmax=0;
% %     cellarr{i}.intensity=79;
% %     cellarr{i}.sdev=18;
% %     cellarr{i}.volsize=5;
% %     cellarr{i}.mass=34223;
% %     color.r=255;color.g=0;color.b=0;
% %     cellarr{i}.color=color;
% % %     cellarr{i}.x=cellarr_save(1,i);
% % %     cellarr{i}.y=cellarr_save(2,i);
% %     cellarr{i}.y=XYZ(1,i);
% %     cellarr{i}.x=XYZ(2,i);
% %     cellarr{i}.z=XYZ(3,i);
% % end
% % save_v3d_apo_file(cellarr,'tmp.apo');
% %the profile value is the sum of the pixel intensity within the sphere at certain position along the curve
% %only search along the curve
% % for i=1:n_profilepoint
% %     %index of pixels within current cell sphere
% %     arr_cellpixelpos_sphere=repmat(XYZ(:,i),1,n_cellpixel)+arr_celloffset;
% %     arr_cellpixelind_sphere=sub2ind(size(I_g),round(arr_cellpixelpos_sphere(2,:)),round(arr_cellpixelpos_sphere(1,:)),round(arr_cellpixelpos_sphere(3,:)));
% %     %compute the fidelity (sum of intensity) of current cell
% %     fidelity=sum(I_g(arr_cellpixelind_sphere));
% %     arr_sumintensity(i)=fidelity;
% % end
% %search the region along the curve
% radius_search=5;
% n_searchpixels=0;
% for x=-radius_search:radius_search
%     for y=-radius_search:radius_search
%         dis=(x*x+y*y)^0.5;
%         if(dis<=radius_search)
%             n_searchpixels=n_searchpixels+1;
%             arr_diskpixel(:,n_searchpixels)=[0;x;y];
%         end
%     end
% end
% for i=1:n_profilepoint
%     maxintensity=0;
%     for j=1:n_searchpixels
%         %index of pixels within current cell sphere
%         arr_cellpixelpos_sphere=repmat(XYZ(:,i)+arr_diskpixel(:,j),1,n_cellpixel)+arr_celloffset;
%         arr_cellpixelind_sphere=sub2ind(size(I_g),round(arr_cellpixelpos_sphere(2,:)),round(arr_cellpixelpos_sphere(1,:)),round(arr_cellpixelpos_sphere(3,:)));
%         %compute the fidelity (sum of intensity) of current cell
%         fidelity=sum(I_g(arr_cellpixelind_sphere));
%         if(maxintensity<fidelity)
%             maxintensity=fidelity;
%         end
%     end
%     arr_sumintensity(i)=maxintensity;
% end
% 
% %%
% % span=30;
% % window=ones(span,1)/span; 
% % arr_sumintensity=convn(arr_sumintensity,window,'same');
% % %find peaks
% % radius=1;
% % n_pks=0;
% % for i=1:n_profilepoint
% %     left_ind=[-radius:-1]+ones(1,radius)*i;
% %     right_ind=[1:radius]+ones(1,radius)*i;
% %     left_ind(left_ind<1)=1;
% %     right_ind(right_ind>n_profilepoint)=n_profilepoint;
% %     
% %     left_mean=sum(arr_sumintensity(left_ind))/radius;
% %     right_mean=sum(arr_sumintensity(right_ind))/radius;
% %     
% %     if(arr_sumintensity(i)>=left_mean && arr_sumintensity(i)>=right_mean)
% %         n_pks=n_pks+1;
% %         pks_ind(n_pks)=i;
% %         pks_value(n_pks)=arr_sumintensity(i);
% %     end
% % end
% % find the peaks (has bug!)
% th=1e+4;
% [pks_value,pks_ind]=findpeaks(arr_sumintensity,'minpeakheight',th);
% % %find the peak by mean shift
% % pks_ind=[];
% % radius=5;
% % window=-radius:radius;
% % for i=1:10:n_profilepoint
% % % for i=1:1
% %     tmp=i;
% %     for j=1:500
% %         ind_span=round(ones(1,length(window))*tmp+window);
% %         ind_span(ind_span<=0)=1;
% %         ind_span(ind_span>n_profilepoint)=n_profilepoint;
% % 
% %         tmp=sum(arr_sumintensity(ind_span).*ind_span)/sum(arr_sumintensity(ind_span));
% %     end
% %     pks_ind(round(i/10+1))=round(tmp);
% % %     pks_ind(i)=tmp;
% % end
% 
% 
% 
% %%
% 
% %%
% figure; grid on; axis equal; hold on
% for i=1:size(arr_musclecell_VL,2)
%     plot3(arr_musclecell_VL(1,i),arr_musclecell_VL(2,i),arr_musclecell_VL(3,i),'or');
% end
% plot3(X',YZ(1,:)',YZ(2,:)');
% hold off; drawnow;
% 
% figure; grid on; hold on
% plot(arr_sumintensity);
% for i=1:size(arr_musclecell_VL,2)
%     %plot no-affine point
%     x=(arr_musclecell_VL(1,i)-arr_musclecell_VL(1,1))/(x_span/n_profilepoint);
%     if(x<=0) x=1; end
%     if(x>n_profilepoint) x=n_profilepoint; end
%     plot(x,arr_sumintensity(round(x)),'*r');
%     %plot affine point
%     x=(arr_musclecell_VL_affine(1,i)-arr_musclecell_VL_affine(1,1))/(x_span/n_profilepoint);
%     if(x<=0) x=1; end
%     if(x>n_profilepoint) x=n_profilepoint; end
%     plot(x,arr_sumintensity(round(x)),'*g');
% end
% for i=1:length(pks_ind)
%     %plot peak point
%     x=pks_ind(i);
%     plot(x,arr_sumintensity(round(x)),'ob');
% end
% hold off; drawnow;
% 
% %%
% %find the miss matched high peak
% th_pks=mean(pks_value)-std(pks_value);
% missed_pksind=[];
% n=0;
% for i=1:length(pks_ind)
%     tmp=pks_ind(i)*(x_span/n_profilepoint)+arr_musclecell_VL(1,1);
%     dis=abs(repmat(tmp,1,length(arr_musclecell_VL(1,:)))-arr_musclecell_VL(1,:));
%     mindis=min(dis);
%     fprintf('mindis=%f, value=%f\n',mindis,pks_value(i));
%     
%     if(mindis>3 && pks_ind(i)<450)%should exclude SPH and EFP
%         n=n+1;
%         missed_pksind(n)=i;
%     end
% end
% 
% %assign the left or right cell to the miss matched peak and re-run the
% %final optimum
% left_pos_new=
% %find the configuration with minimum energy and rerun the final
% %optimization untill converge


