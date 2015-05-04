function EulerInv =  p_EulerInv(img,LUT)

% Calculate Euler characteristic for each octant and sum up
eulerChar = zeros(size(img,1),1);
% Octant SWU
n = ones(size(img,1),1);
n(img(:,25)==1) = bitor(n(img(:,25)==1),128);
n(img(:,26)==1) = bitor(n(img(:,26)==1),64);
n(img(:,16)==1) = bitor(n(img(:,16)==1),32);
n(img(:,17)==1) = bitor(n(img(:,17)==1),16);
n(img(:,22)==1) = bitor(n(img(:,22)==1),8);
n(img(:,23)==1) = bitor(n(img(:,23)==1),4);
n(img(:,13)==1) = bitor(n(img(:,13)==1),2);
eulerChar = eulerChar + LUT(n);
% Octant SEU
n = ones(size(img,1),1);
n(img(:,27)==1) = bitor(n(img(:,27)==1),128);
n(img(:,24)==1) = bitor(n(img(:,24)==1),64);
n(img(:,18)==1) = bitor(n(img(:,18)==1),32);
n(img(:,15)==1) = bitor(n(img(:,15)==1),16);
n(img(:,26)==1) = bitor(n(img(:,26)==1),8);
n(img(:,23)==1) = bitor(n(img(:,23)==1),4);
n(img(:,17)==1) = bitor(n(img(:,17)==1),2);
eulerChar = eulerChar + LUT(n);
% Octant NWU
n = ones(size(img,1),1);
n(img(:,19)==1) = bitor(n(img(:,19)==1),128);
n(img(:,22)==1) = bitor(n(img(:,22)==1),64);
n(img(:,10)==1) = bitor(n(img(:,10)==1),32);
n(img(:,13)==1) = bitor(n(img(:,13)==1),16);
n(img(:,20)==1) = bitor(n(img(:,20)==1),8);
n(img(:,23)==1) = bitor(n(img(:,23)==1),4);
n(img(:,11)==1) = bitor(n(img(:,11)==1),2);
eulerChar = eulerChar + LUT(n);
% Octant NEU
n = ones(size(img,1),1);
n(img(:,21)==1) = bitor(n(img(:,21)==1),128);
n(img(:,24)==1) = bitor(n(img(:,24)==1),64);
n(img(:,20)==1) = bitor(n(img(:,20)==1),32);
n(img(:,23)==1) = bitor(n(img(:,23)==1),16);
n(img(:,12)==1) = bitor(n(img(:,12)==1),8);
n(img(:,15)==1) = bitor(n(img(:,15)==1),4);
n(img(:,11)==1) = bitor(n(img(:,11)==1),2);
eulerChar = eulerChar + LUT(n);
% Octant SWB
n = ones(size(img,1),1);
n(img(:,7)==1) = bitor(n(img(:,7)==1),128);
n(img(:,16)==1) = bitor(n(img(:,16)==1),64);
n(img(:,8)==1) = bitor(n(img(:,8)==1),32);
n(img(:,17)==1) = bitor(n(img(:,17)==1),16);
n(img(:,4)==1) = bitor(n(img(:,4)==1),8);
n(img(:,13)==1) = bitor(n(img(:,13)==1),4);
n(img(:,5)==1) = bitor(n(img(:,5)==1),2);
eulerChar = eulerChar + LUT(n);
% Octant SEB
n = ones(size(img,1),1);
n(img(:,9)==1) = bitor(n(img(:,9)==1),128);
n(img(:,8)==1) = bitor(n(img(:,8)==1),64);
n(img(:,18)==1) = bitor(n(img(:,18)==1),32);
n(img(:,17)==1) = bitor(n(img(:,17)==1),16);
n(img(:,6)==1) = bitor(n(img(:,6)==1),8);
n(img(:,5)==1) = bitor(n(img(:,5)==1),4);
n(img(:,15)==1) = bitor(n(img(:,15)==1),2);
eulerChar = eulerChar + LUT(n);
% Octant NWB
n = ones(size(img,1),1);
n(img(:,1)==1) = bitor(n(img(:,1)==1),128);
n(img(:,10)==1) = bitor(n(img(:,10)==1),64);
n(img(:,4)==1) = bitor(n(img(:,4)==1),32);
n(img(:,13)==1) = bitor(n(img(:,13)==1),16);
n(img(:,2)==1) = bitor(n(img(:,2)==1),8);
n(img(:,11)==1) = bitor(n(img(:,11)==1),4);
n(img(:,5)==1) = bitor(n(img(:,5)==1),2);
eulerChar = eulerChar + LUT(n);
% Octant NEB
n = ones(size(img,1),1);
n(img(:,3)==1) = bitor(n(img(:,3)==1),128);
n(img(:,2)==1) = bitor(n(img(:,2)==1),64);
n(img(:,12)==1) = bitor(n(img(:,12)==1),32);
n(img(:,11)==1) = bitor(n(img(:,11)==1),16);
n(img(:,6)==1) = bitor(n(img(:,6)==1),8);
n(img(:,5)==1) = bitor(n(img(:,5)==1),4);
n(img(:,15)==1) = bitor(n(img(:,15)==1),2);
eulerChar = eulerChar + LUT(n);

EulerInv(eulerChar==0) = true;