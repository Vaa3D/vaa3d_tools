function [feaVec] = compute_neuron_GMI(b, centerpos, radius_thres)
%function [feaVec] = compute_neuron_GMI(b, centerpos, radius_thres)
%
% Compute the GMI for an input neuron in swc format
%
% b - the neuron in swc format
% centerpos - the centerpos (origin), a 3x1 vector or 1x3 vector, of the current computation. So that
% every neuronal node will subtract this pos
% radius_thres - the threshold of radius for computation, Any node outside
% this radius will not be counted
%
% by Hanchuan Peng
% 2007-08-08
% last update: 2008-11-01: add centerpos and radius_thres
% last update: 2008-11-01: remove the first value which is the number of lines in the input data
%
% see also: the original computeGMI() in the fly.brain.Julie directory I
% wrote last year.
%

if nargin<3,
    radius_thres=[]; %set as an invalid value
end;

if nargin<2,
    centerpos = [];
else,
    if (length(centerpos(:))~=3),
        error('The second parameter centerpos does not have three elements.');
    end;
end;

if ~isempty(centerpos) & length(centerpos(:))==3,
    b(:,3:5) = b(:,3:5)-repmat(centerpos(:)', size(b,1), 1);
end;

%% The actual computation

feaVec=[];

c000 = compute_moments_neuron(b, [0 0 0], radius_thres); 

c200 = compute_moments_neuron(b, [2 0 0], radius_thres); 
c020 = compute_moments_neuron(b, [0 2 0], radius_thres); 
c002 = compute_moments_neuron(b, [0 0 2], radius_thres); 
c110 = compute_moments_neuron(b, [1 1 0], radius_thres); 
c101 = compute_moments_neuron(b, [1 0 1], radius_thres); 
c011 = compute_moments_neuron(b, [0 1 1], radius_thres); 

c300 = compute_moments_neuron(b, [3 0 0], radius_thres); 
c030 = compute_moments_neuron(b, [0 3 0], radius_thres); 
c003 = compute_moments_neuron(b, [0 0 3], radius_thres); 
c120 = compute_moments_neuron(b, [1 2 0], radius_thres); 
c102 = compute_moments_neuron(b, [1 0 2], radius_thres); 
c210 = compute_moments_neuron(b, [2 1 0], radius_thres); 
c201 = compute_moments_neuron(b, [2 0 1], radius_thres); 
c012 = compute_moments_neuron(b, [0 1 2], radius_thres); 
c021 = compute_moments_neuron(b, [0 2 1], radius_thres); 
c111 = compute_moments_neuron(b, [1 1 1], radius_thres); 


feaVec(:,1) = size(b,1); %%double(b(pixInd)); %081101, Note this value will be removed right before the finishing of this function.
                                              % kept here for consistency
                                              % of previous versions
feaVec(:,2) = c000;
feaVec(:,3) = c200+c020+c002;
feaVec(:,4) = c200.*c020+c020.*c002+c002.*c200-c101.^2-c011.^2-c110.^2; 
feaVec(:,5) = c200.*c020.*c002-c002.*c110.*c110+2.*c110.*c101.*c011-c020.*c101.*c101-c200.*c011.*c011; 

spi = sqrt(pi);

v_0_0 = (2*spi/3).*(c200+c020+c002);

v_2_2 = spi*sqrt(2/15) .* (c200-c020+2j.*c110);
v_2_1 = spi*sqrt(2/15) .* (-2.*c101-2j.*c011);
v_2_0 = spi*sqrt(4/45) .* (2.*c002-c200-c020);
v_2_m1 = spi*sqrt(2/15) .* (2.*c101-2j.*c011);
v_2_m2 = spi*sqrt(2/15) .* (c200-c020-2j.*c110);

v_3_3 = spi*sqrt(1/35) .* ((-c300+3.*c120) + i.*(c030-3.*c210));
v_3_2 = spi*sqrt(6/35) .* ((c201-c021) + 2i.*c111);
v_3_1 = spi*sqrt(3/175) .* ((c300+c120-4.*c102) + i.*(c030+c210-4.*c012));
v_3_0 = spi*sqrt(4/175) .* (2.*c003 - 3.*c201 - 3.*c021);
v_3_m1 = spi*sqrt(3/175) .* ((-c300-c120+4.*c102) + i.*(c030+c210-4.*c012));
v_3_m2 = spi*sqrt(6/35) .* ((c201-c021) - 2i.*c111);
v_3_m3 = spi*sqrt(1/35) .* ((c300-3.*c120) + i.*(c030-3.*c210));

v_1_1 = spi.*sqrt(6/25) .* ((-c300-c120-c102) - j.*(c030+c210+c012));
v_1_0 = spi.*sqrt(12/25) .* (c003+c201+c021);
v_1_m1 = spi.*sqrt(6/25) .* ((c300+c120+c102) - j.*(c030+c210+c012));

v_g33_2_2 = sqrt(10/21).*v_3_3.*v_3_m1 - sqrt(20/21).*v_3_2.*v_3_0 + sqrt(2/7).*v_3_1.*v_3_1;
v_g33_2_1 = sqrt(25/21).*v_3_3.*v_3_m2 - sqrt(5/7).*v_3_2.*v_3_m1 + sqrt(2/21).*v_3_1.*v_3_0;
v_g33_2_0 = sqrt(25/21).*v_3_3.*v_3_m3 - sqrt(3/7).*v_3_1.*v_3_m1 + sqrt(4/21).*v_0_0.*v_0_0;
v_g33_2_m1 = sqrt(25/21).*v_3_m3.*v_3_2 - sqrt(5/7).*v_3_m2.*v_3_1 + sqrt(2/21).*v_3_m1.*v_3_0;
v_g33_2_m2 = sqrt(10/21).*v_3_m3.*v_3_1 - sqrt(20/21).*v_3_m2.*v_3_0 + sqrt(2/7).*v_3_m1.*v_3_m1;

v_g31_2_2 = -sqrt(1/105).*v_3_2.*v_1_0 + sqrt(1/35).*v_3_3.*v_1_m1 + sqrt(1/525).*v_3_1.*v_1_1;
v_g31_2_1 = sqrt(2/105).*v_3_2.*v_1_m1 + sqrt(1/175).*v_3_0.*v_1_1 - sqrt(4/525).*v_3_1.*v_1_0;
v_g31_2_0 = -sqrt(3/175).*v_3_0.*v_1_0 + sqrt(2/175).*v_3_1.*v_1_m1 + sqrt(2/175).*v_3_m1.*v_1_1;
v_g31_2_m1 = sqrt(2/105).*v_3_m2.*v_1_1 + sqrt(1/175).*v_3_0.*v_1_m1 -sqrt(4/525).*v_3_m1.*v_1_0;
v_g31_2_m2 = -sqrt(1/105).*v_3_m2.*v_1_0 + sqrt(1/35).*v_3_m3.*v_1_1 + sqrt(1/525).*v_3_m1.*v_1_m1;

v_g11_2_2 = 0.2.*v_1_1.*v_1_1;
v_g11_2_1 = sqrt(2/25).*v_1_0.*v_1_1;
v_g11_2_0 = sqrt(2/75).*(v_1_0.*v_1_0 + v_1_1.*v_1_m1);
v_g11_2_m1 = sqrt(2/25).*v_1_0.*v_1_m1;
v_g11_2_m2 = 0.2.*v_1_m1.*v_1_m1;


% % feaVec(:,6) = 7.^(-0.5) .* (...
% %               + 2*pi/35 * (-(c300-3*c120).*2-(c030-3.*c210).^2) ...
% %               - 12*pi/35 * ((c201-c021).^2+4.*(c111.^2)) ...
% %               + feaVec(:,6) ...
% %               + 6*pi/175 * (-(c300+c120-4.*c102).^2-(c030+c210-4.*c012).^2) ...
% %               - 4*pi/175 * ((2.*c003-3.*c201-3.*c021).^2)...
% %               ) ./ ...
% %               (v00.^(12/5)); %% common coefficient can be omitted
% % 
% % feaVec(:,7) = 3.^(-0.5) .* (...
% %               + 6*pi/25 * (-(c300+c120+c102).^2-(c030+c210+c012).^2) ...
% %               - 12*pi/25 * ((c003+c201+c021).^2) ...
% %               )./ ...
% %               (v00.^(12/5)); %% common coefficient can be omitted

tmp = v_0_0.^(12/5);
feaVec(:,6) = 7.^(-0.5) .* (2.*v_3_3.*v_3_m3 - 2.*v_3_2.*v_3_m2 + 2.*v_3_1.*v_3_m1 - v_3_0.^2) ./ tmp; 
feaVec(:,7) = 3.^(-0.5) .* (2.*v_1_1.*v_1_m1 - v_1_0.^2) ./ tmp; 

tmp = v_0_0.^(24/5);
feaVec(:,8) = 5.^(-0.5) .* (2.*v_g33_2_m2.*v_g33_2_2 - 2.*v_g33_2_m1.*v_g33_2_1 + v_g33_2_0.*v_g33_2_0) ./ tmp;
feaVec(:,9) = 5.^(-0.5) .* (2.*v_g31_2_m2.*v_g31_2_2 - 2.*v_g31_2_m1.*v_g31_2_1 + v_g31_2_0.*v_g31_2_0) ./ tmp;
feaVec(:,10) = 5.^(-0.5) .* (v_g33_2_m2.*v_g31_2_2 - v_g33_2_m1.*v_g31_2_1 + v_g33_2_0.*v_g31_2_0 - v_g33_2_1.*v_g31_2_m1 + v_g33_2_2.*v_g31_2_m2) ./ tmp;
feaVec(:,11) = 5.^(-0.5) .* (v_g31_2_m2.*v_g11_2_2 - v_g31_2_m1.*v_g11_2_1 + v_g31_2_0.*v_g11_2_0 - v_g31_2_1.*v_g11_2_m1 + v_g31_2_1.*v_g11_2_m2) ./ tmp;

tmp = v_0_0.^(17/5);
feaVec(:,12) = 5.^(-0.5) .* (v_g33_2_m2.*v_2_2 - v_g33_2_m2.*v_2_1 + v_g33_2_0.*v_2_0 - v_g33_2_1.*v_2_m1 + v_g33_2_2.*v_2_m2) ./ tmp;
feaVec(:,13) = 5.^(-0.5) .* (v_g31_2_m2.*v_2_2 - v_g31_2_m2.*v_2_1 + v_g31_2_0.*v_2_0 - v_g31_2_1.*v_2_m1 + v_g31_2_2.*v_2_m2) ./ tmp;
feaVec(:,14) = 5.^(-0.5) .* (v_g11_2_m2.*v_2_2 - v_g11_2_m2.*v_2_1 + v_g11_2_0.*v_2_0 - v_g11_2_1.*v_2_m1 + v_g11_2_2.*v_2_m2) ./ tmp;

feaVec = real(feaVec); %% in case there is any rounding error to get the imaginary part to be non-zero, then only use the real.

%feaVec = feaVec./repmat(max(feaVec), length(pixInd), 1);
%feaVec = feaVec(:, 1:5);

%%===

feaVec = feaVec(:,2:end); %081101, remove the first value, as it is not a moment value

return;
