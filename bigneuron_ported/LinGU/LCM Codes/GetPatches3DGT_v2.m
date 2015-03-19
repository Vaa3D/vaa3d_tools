function [imgp,ind_obj] = GetPatches3DGT_v2(gt,obj,p_sz)

% collect local cubic for individual points of obj



if(nargin < 3)
    
    p_sz = 15;
    
end

Isz = size(gt);


sz_obj = size(obj);

min_sz = min(sz_obj);

max_sz = max(sz_obj);

if((sz_obj(1) == size(gt,1)) && (size(sz_obj) > 1) )
    
    if(sz_obj(2) == size(I,2))
        
        ind_obj = find(obj);
        
        [x_obj,y_obj,z_obj] = ind2sub(Isz,ind_obj);
        
    end
    
end



if(0)
    
    



elseif(((min_sz == 1) && (size(obj,1) ~= 1)) | (max_sz == 1))
        
    [x_obj,y_obj,z_obj] = ind2sub(Isz,obj);
    
else
   x_obj = obj(:,1);
   
   y_obj = obj(:,2);

   z_obj = obj(:,3);
    
end

ind_obj = sub2ind(Isz,x_obj,y_obj,z_obj);


% expand the image given the patch size

gt = padarray(gt,[p_sz,p_sz,p_sz],'replicate');


% 
% gt1 = zeros(Isz + p_sz * 2);
% 
% gt1(p_sz + 1: p_sz + Isz(1),p_sz + 1: p_sz + Isz(2)) = gt;
% 
% gt = gt1;

ind_s_img = sub2ind(Isz + p_sz * 2,x_obj + p_sz,y_obj + p_sz,z_obj + p_sz);

n_s = length(ind_s_img);

ps = [];

p_idx = img_patch_idx(size(gt),p_sz);

ind_p = repmat(p_idx',n_s,1);

ind_p = ind_p + repmat(ind_s_img,1,p_sz * p_sz * p_sz);

imgp = gt(ind_p);

imgp = reshape(imgp',p_sz,p_sz,p_sz,n_s);


% for i_b = 1 : size(gt,3)
%     
%     gtb = gt(:,:,i_b);
%     
%     p_idx = img_patch_idx(size(gtb),p_sz);
%     
%     ind_p = repmat(p_idx',n_s,1);
%     
%     ind_p = ind_p + repmat(ind_s_img,1,p_sz * p_sz);
%     
%     imgp = gtb(ind_p);
%     
%     imgp = reshape(imgp',p_sz,p_sz,n_s);
%     
%     ps = cat(3,ps,imgp);
%     
% end


end

function p_idx = img_patch_idx(sze_img,p_sz)

img = zeros(sze_img);

img(1:p_sz,1:p_sz,1:p_sz) = 1;

p_idx = find(img == 1) - (sub2ind(sze_img,ceil(p_sz / 2),ceil(p_sz/2),ceil(p_sz/2)));

end