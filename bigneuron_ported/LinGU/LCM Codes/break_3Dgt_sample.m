function [patch_stack,hd_stack,cl_ind] = break_3Dgt_sample(gt)

% break the groundtruth filament into several fragments

p_sz = 15;

CC = bwconncomp(gt);

numPixels = cellfun(@numel,CC.PixelIdxList);

[biggest,idx] = max(numPixels);

gt1 = zeros(size(gt));

% get rid of the groundtruth on the peripheral area

gt1(CC.PixelIdxList{idx}) = 1;

gt = gt1;

clear gt1;

centre_line = Skeleton3D(gt);

cl_ind = find(centre_line);


if(0)
    
    cl_idx = find(centre_line > 0);
    
    [clx,cly,clz] = ind2sub(size(centre_line),cl_idx);
    
    scatter3(clx,cly,clz);
    
end

n_cl = length(cl_ind);

% sample the HOG centred on the centreline

% first calculate the direction on 3D 

[Fx,Fy,Fz] = gradient(gt);

Gmag = sqrt(Fx .^ 2 + Fy .^ 2 + Fz .^ 2);

Gind = find(Gmag > 0);

Gmag1 = Gmag(Gind);

Fx1 = Fx(Gind) ./ Gmag1;

Fy1 = Fy(Gind) ./ Gmag1;

Fz1 = Fz(Gind) ./ Gmag1;

% calculate the unit vectors for the actual gradient vectors 

Fphi1 = atan2(Fx1,Fy1) / pi * 180;

Ftheta1 = acos(Fz1) / pi * 180;

clear Fx Fy Fz;

Fphi = zeros(size(gt));

Ftheta = zeros(size(gt));

Fphi(Gind) = Fphi1;

Ftheta(Gind) = Ftheta1;

patch_mag = GetPatches3DGT(Gmag,cl_ind,p_sz);

patch_phi = GetPatches3DGT(Fphi,cl_ind,p_sz);

patch_theta = GetPatches3DGT(Ftheta,cl_ind,p_sz);

patch_stack = GetPatches3DGT(gt,cl_ind,p_sz);

vec_phi = reshape(patch_phi,p_sz * p_sz * p_sz,[]);

vec_theta = reshape(patch_theta,p_sz * p_sz * p_sz,[]);

vec_mag = reshape(patch_mag,p_sz * p_sz * p_sz,[]);

vec_mag = vec_mag';

% here achieve a weighted 2D histogram

% 
% vec_phi(vec_phi > 174.5) = -175;
% 
% vec_theta(vec_theta > 174.5) = 1;

vec_phi_ind = round((vec_phi + 180) / 45) ;

vec_theta_ind = round(vec_theta / 10) + 1;


sz_hist = [8,19];

vec_hog_ind = sub2ind(sz_hist,vec_phi_ind',vec_theta_ind');

ind_incre = 0 : (sz_hist(1) * sz_hist(2)) : (sz_hist(1) * sz_hist(2)) * (n_cl - 1);

ind_incre = ind_incre';

ind_incre = repmat(ind_incre,[1, p_sz ^ 3]);

vec_hog_ind = vec_hog_ind + ind_incre;

hd_stack = zeros(sz_hist(1) * sz_hist(2),n_cl);

for i_v = 1 : n_cl

    hog_2D = zeros(sz_hist(1) * sz_hist(2),p_sz ^ 3);

    hog_2D(vec_hog_ind(i_v,:)) = vec_mag(i_v,:);

    hd_stack(:,i_v) = sum(hog_2D,2);

end

hd_stack = hd_stack';







% 
% ind_incre = 0 : (sz_hist(1) * sz_hist(2)) : (sz_hist(1) * sz_hist(2)) * (n_cl - 1);
% 
% ind_incre = ind_incre';
% 
% ind_incre = repmat(ind_incre,[1, p_sz ^ 3]);
% 
% vec_hog_ind = vec_hog_ind + ind_incre;




%[bc_dir,ind_dir] = ndhist(vec_phi,vec_theta,'intbins',10);
% 
% 
% 
% 
% 
% 
% width_c = bwdist(~gt1);
% 
% tf = width_c .* centre_line;
% 
% tf = (tf > 0) & (tf < 2);
% 
% bp = bwmorph(tf,'branchpoints');
% 
% bp = bwmorph(bp,'thicken',2);
% 
% tf(bp) = 0;
% 
% tf = bwmorph(tf,'spur');
% 
% [Gmag,Gdir] = imgradient(centre_line,'CentralDifference');
% 
% 
% 
% 
% 
% 
% 
% 
% 
% 
% 
% 
% 
% 
% 
% 
% 
% 
% 
% 
% 
% 
% CCtf = bwconncomp(tf);
% 
% numPixels = cellfun(@numel,CCtf.PixelIdxList);
% 
% [nP_ske,nP_idx_ske] = sort(numPixels,'descend');
% 
% f_label = zeros(size(gt));
% 
% 
% 
% 
% f_tip = bwmorph(tf,'endpoints');
% 
% tip_dist = bwdistgeodesic(tf,f_tip);
% 
% % determine the size of core part of the patch
% 
% p_step =  5 : 5 : 30;
% 
% 
% 
% for i_p = 1 : length(p_step)
%     
%     patch_stack{i_p} = [];
%     
%     patch_idx{i_p} = [];
%     
%     Gmag_stack{i_p} = [];
%     
%     Gdir_stack{i_p} = [];
%     
%     hdc_stack{i_p} = [];
%     
%     hd_stack{i_p} = [];
%     
% end
% 
% for k = 1 : length(nP_ske)
%     
%     si = nP_idx_ske(k);
%     
%     n_v = nP_ske(k);
%     
%     f_label(CCtf.PixelIdxList{si}) = n_v;
%     
%     cf = zeros(size(gt));
%     
%     cf(CCtf.PixelIdxList{si}) = 1;
%     
%     for i_p = 1 : length(p_step)
%           
%         psz = p_step(i_p);
%         
%         if(n_v > psz + 1)
%         
%             sp_m = cf .* tip_dist;
%             
%             sp_m = sp_m > ceil(psz / 2);
%             
%             bpsz = psz * 3;
%             
%             [ps,idx_ps] = GetPatchesGT_large_window_v2(cf,sp_m,bpsz);
%             
%             hdc = hist_descriptor_2(cf,sp_m,psz);
%                         
%             hd = hist_descriptor_2(cf,sp_m,bpsz);
%            
%             patch_stack{i_p} = cat(3,patch_stack{i_p},ps); 
%                         
%             patch_idx{i_p} = [patch_idx{i_p};idx_ps];
%             
%             hdc_stack{i_p} = [hdc_stack{i_p};hdc];
%  
%             hd_stack{i_p} = [hd_stack{i_p};hd];
%              
%             
%         end
%             
%     end
%     
% end
% 
% 









