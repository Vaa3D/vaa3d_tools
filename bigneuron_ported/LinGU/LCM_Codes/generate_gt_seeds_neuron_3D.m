function [pos_seed_img,neg_seed_img,pos_ind,neg_ind] = generate_gt_seeds_neuron_3D(gt,prob_img)
% sample the 3D fragments based on 2D groundtruth

prob90 = prob_img > 0.3;

CC = bwconncomp(prob90,6);

[SeedMap,propsSeed,CCList] = get_frag_sta_3D(prob_img,CC);

pos_seed_img = zeros(size(prob_img)); 

neg_seed_img = zeros(size(prob_img)); 


pos_ind = {};

neg_ind = {};

for i_c = 1 : length(CCList)
    
   ind3D_cc = CCList{i_c};
   
   [ind2D_ccx,ind2D_ccy,~] = ind2sub(size(prob_img),ind3D_cc);
   
   ind2D = sub2ind(size(prob_img(:,:,1)),ind2D_ccx,ind2D_ccy);
   
   prec(i_c) = mean(gt(ind3D_cc));
   
   if(prec(i_c) < 0.2)
       
       neg_seed_img(ind3D_cc) = 1;
       
       neg_ind{end + 1} = ind3D_cc;
      
   else
       
       pos_seed_img(ind3D_cc) = 1;
       
       pos_ind{end + 1} = ind3D_cc;
   end
       
   
end






% 
% prob90_2D = sum(prob90,3);
% 
% [mbp90,mbp90_idx] = find_mbp3D(prob_img,prob90,150,25);
% 
% 
% seed_img1 = prob90 - mbp90;
% 
% seed_img = diff_comp(prob90,prob_img > 0.6);
% 
% seed_img = seed_img + seed_img1;
% 
% neg_seed_img = diff_comp(gt,seed_img);
% 
% pos_seed_img = seed_img - neg_seed_img;

if(0)

show_img(:,:,1) = pos_img;

show_img(:,:,2) = neg_img;

show_img(:,:,3) = gt;

end