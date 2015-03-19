function hd_stack = HOG3D_frag(gt,cl_ind)
% collect the HOG from 3D fragments

if(nargin < 3)
   
    p_sz = 15;
    
end



n_cl = length(cl_ind);

[Gmag,Fphi,Ftheta] = gradient_3D(gt);

% collect the gradient information centred on objects like cl_ind

patch_mag = GetPatches3DGT(Gmag,cl_ind,p_sz);

patch_phi = GetPatches3DGT(Fphi,cl_ind,p_sz);

patch_theta = GetPatches3DGT(Ftheta,cl_ind,p_sz);

vec_phi = reshape(patch_phi,p_sz * p_sz * p_sz,[]);

vec_theta = reshape(patch_theta,p_sz * p_sz * p_sz,[]);

vec_mag = reshape(patch_mag,p_sz * p_sz * p_sz,[]);

vec_mag = vec_mag';

% cpmpress the gradient information

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












