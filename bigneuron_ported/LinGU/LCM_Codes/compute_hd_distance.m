function distance = compute_hd_distance(hds)

N = size(hds,1);

distance = zeros(N);

for iN = 1 : N

   for jN = iN + 1 : N
       
       distance(iN,jN) = sum((hds(iN,:) - hds(jN,:)) .^ 2); 
       
   end
    
end

distance = distance + distance';

distance = distance / mean(distance(:)) * 5;