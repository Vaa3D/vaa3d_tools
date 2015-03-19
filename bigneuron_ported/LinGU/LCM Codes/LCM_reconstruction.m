function [p90,frag,rec_path] = LCM_reconstruction(X,prob_img,LTClassifier)

% apply the LCM to reconstruct the neuron

%author : Lin Gu

mask = ones(size(X));


if(nargin < 4)

    pe = 0;

end

sz_I = size(prob_img);


p90 = prob_img > 0.6;

%  [mbp90,mbp90_idx]  = find_mbp3D(prob_img,p90,450);
%  
%  baselist = 1 : sum(mbp90_idx);
%  
% 
% seed_img = p90 - mbp90;

% at first, the main branch is pre-linked to speed up the linking time  


path_img_pre = pre_link_branch3D(prob_img,p90,mask,450,2,0.8);

frag{1} = find(p90 > 0);


path_img_pre = imdilate(path_img_pre,ones(7,7,7));

rec_path{1} = find(path_img_pre > 0);


p90 = path_img_pre + p90;

if(1)
    
   mip_p90 = sum(p90,3);
    
   mip_path = sum(path_img_pre,3); 
    
end




mbp90 = preserve_main_branch(p90);

seed_img = (p90 - mbp90) > 0;

% npi = prob_img - 0.1;
% 
% npi(npi < 0.01) = 0.01;


[est_seed_img,SeedMap,est_seed,frag_token,frag_prop] = predict_seed3D_LTC_v2(X,prob_img,seed_img,LTClassifier);

frag{2} = find(seed_img);


dds.est_seed_img{1} = est_seed_img;

dds.mbp90= mbp90;




% [est_seed_img,SeedMap,est_seed] = predict_seed(X,prob_img,seed_img,weak_learners);

if(0)

%    pot_img = predict_vessel_potential(X,LTClassifier,seed_img);
    

    pot_img = generate_3D_potential(X,est_seed,frag_token,frag_prop(:,6:8),LTClassifier);

    new_prob_img = prob_img + pot_img;
    
    

else

    new_prob_img = prob_img;
    
    pot_img = zeros(size(prob_img));

end

new_prob_img(new_prob_img > 0.999) = 0.999;


% link the high confidence part - p90

% if(length(est_seed) > 220)

est_seed_img = est_seed_img > 0;

p90 = est_seed_img + mbp90;

%seed_img =   est_seed_img > 0;


%     
% end

p90 = p90 > 0;%seed_img

fprintf('Start processing the seeds of probablity 0.9 \n');


new_prob_img = X;

% p90 = clean_small_seeds(p90);

[costs,pathsxy,Seed_m,node_idx_map,nSeeds] = connect_seed_3D(new_prob_img,p90,50);

if(~isempty(costs))

sz_I = size(node_idx_map);

path_img = draw_path_3D(node_idx_map,pathsxy,Seed_m);

% enlarge the path to form a neuron

path_img = imdilate(path_img,ones(7,7,7));

rec_path{2} = find(path_img > 0);

end

p90 = p90 + path_img;

%p90prior = p90;


sz_I = size(p90);

CC = bwconncomp(p90 > 0,6);

numPixels = cellfun(@numel,CC.PixelIdxList);

[biggest,idx] = max(numPixels);

[~,nPidx] = sort(numPixels,'descend');

p90 = zeros(sz_I);

p90(CC.PixelIdxList{idx}) = 1;

for i_c = 2 : length(numPixels)
    
    est_seed_img(CC.PixelIdxList{nPidx(i_c)}) = 0;
    
end


frag{3} = find(est_seed_img > 0);



if(1)
    
    p90ind = find(p90 > 0);
    
    [p90x,p90y,p90z] = ind2sub(size(p90),p90ind);
    
    figure;

    scatter3(p90x,p90y,p90z);
    
end

% p90 = preserve_main_branch(p90);


% link the high confidence part - p60
% 
% fprintf('Start processing the seeds of probablity 0.6 \n');
% 
% seed_img = diff_comp(p90,prob_img > 0.6);
% 
% 
% [est_seed_img,SeedMap,est_seed] = predict_seed(X,prob_img,seed_img,weak_learners);
% 
% seed_img = est_seed_img > 0;
% 
% seed_img = remove_small_seeds(seed_img);
% 
% 
% if((pe) & (issinglecomp(seed_img + p90) > 1))
% 
%     pot_img = predict_vessel_potential(X,LTClassifier,seed_img);
%     
%     new_prob_img = prob_img + pot_img;
%     
%     
% 
% else
% 
%     new_prob_img = prob_img;
% 
% end
% 
% 
% 
% 
% 
% if(issinglecomp(seed_img + p90) > 1)
% 
% [path_img(:,:,2),~,~,path_data{2},bN{2}] = link_seed_neuron(new_prob_img,(seed_img + p90) > 0,...
%     mask,2,0.8,pot_img,length(baselist));
% 
% 
% else
%     
%     path_img(:,:,2) = zeros(sz_I);
%     
%     
% end
% 
% dds.path_data = path_data;
% 
% smap(:,:,2) = seed_img;
% 
% 
% p60mb = p90 - smap(:,:,1) - path_img(:,:,1);
% 
% p86 = prob_img > 0.86;
% 
% aug_p = diff_comp(p60mb,p86);
% 
% aug_p = p86 - aug_p;
% 
% p60mb = p60mb + aug_p;
% 
% nb = sum(mbp90_idx);
% 
% RF = paste_neuron(p60mb,path_img,smap,nb);
% 
% RF = RF > 0;



end



















function p60 = remove_small_seeds(p60)


sz_I = size(p60);

CC = bwconncomp(p60);

numPixels = cellfun(@numel,CC.PixelIdxList);

p60 = zeros(sz_I);

for i_n = 1 : length(numPixels)
    
    if(numPixels(i_n) > 5)
        
        p60(CC.PixelIdxList{i_n}) = 1;
        
    end
    
end





end

function p60 = clean_small_seeds(p60)


sz_I = size(p60);

CC = bwconncomp(p60);

numPixels = cellfun(@numel,CC.PixelIdxList);

p60 = zeros(sz_I);


[~,nPi] = sort(numPixels,'descend');


for i_n = 1 : min(length(nPi),200)
        
        p60(CC.PixelIdxList{nPi(i_n)}) = 1;
    
end


end



