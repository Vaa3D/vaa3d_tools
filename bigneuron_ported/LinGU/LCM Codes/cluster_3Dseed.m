function [c_hdc,p1,hdc_idx] = cluster_3Dseed(ps,hdc)

% get the token by clustering the 3D fragments.


n_km = 3;

w_core = 1;


% assign each point in the training dataset a label

% cluster the the image patch


[hdc_idx,c_hdc] = kmeans(hdc,30,'EmptyAction','singleton');

for i_c = 1 : 30

    p1(:,:,:,i_c) = mean(ps(:,:,:,hdc_idx == i_c),4);

end





% 
% 
% 
% for i_p = 1 : 6
%     
%     % the label procedure first fix the label of the hardcore part and then
%     % works on potential distribution
%     
%     l2 = zeros(size(hdc_stack{i_p},1),1); 
%         
%     ps = patch_stack{i_p};
%     
%     
%     hdc = hdc_stack{i_p};
%     
%     hd = hd_stack{i_p};
%     
%     hd_ep = hd - hdc;
%     
%     [hdc_idx,c_hdc] = kmeans(hdc,10,'EmptyAction','singleton');
%         
%     l1_stack{i_p} = hdc_idx;
%     
%     chdc1_stack{i_p} = c_hdc;
%     
%     p1 = [];
%     
%     p2 = [];
%     
%     centre_hd_sub = [];
%     
%     for i_c = 1 : 10
%         
%        hd1 = hd_ep(hdc_idx == i_c,:);
%        
%        psc = ps(:,:,hdc_idx == i_c);
%        
%        p1(:,:,i_c) = mean(psc,3);
%        
%        
%        [hdc_idx1,c_hd1] = kmeans(hd1,n_km,'EmptyAction','singleton');
%        
%        
%        centre_hd_sub((i_c - 1) * n_km + (1 : n_km),:) =...
%            repmat(c_hdc(i_c,:),n_km,1) * w_core + c_hd1;
%        
%        centre_hd_seed_stack{i_p} = centre_hd_sub;
%        
%        chd_expand_stack{i_p,i_c} = c_hd1;
%        
%        l2_idx = hdc_idx1 + (i_c - 1) * n_km;
%              
%        l2(hdc_idx == i_c) = l2_idx;
%        
%        for i_c1 = 1 : n_km
%           
%            p2(:,:,i_c1 + (i_c - 1) * n_km) = mean(psc(:,:,hdc_idx1 == i_c1),3); 
%            
%        end
%        
%        
%     end
%     
%     l2_stack{i_p} = l2;
%     
%     p2_stack{i_p} = p2;
%     
%     p1_stack{i_p} = p1;
%     
% end
% 
% 
