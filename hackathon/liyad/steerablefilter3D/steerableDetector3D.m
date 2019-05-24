%[res, theta, nms] = steerableDetector3D(vol, M, sigma, zxRatio) performs curve/surface detection using 3D steerable filters
%
% Inputs: 
%         vol : input volume
%           M : filter type
%               1: curve detector
%               2: surface detector
%       sigma : standard deviation of the Gaussian kernel on which the filters are based
%   {zxRatio} : correction factor for z anisotropy (default: 1).
%               Example: if the z sampling step is 5x larger than xy-sampling, set this value to 5.
%
% Outputs: 
%         res : response to the filter
%       theta : orientation vector component structure:
%               .x1, .x2, .x3 fields
%         nms : non-maximum-suppressed response
%
% Memory usage: ~17x size of 'vol'
%
% For more information, see:
% F. Aguet et al., IEEE Proc. ICIP'05, pp. II 1158-1161, 2005.

% Francois Aguet, 08/2012 (last modified 08/28/2012).

function [res, theta, nms] = steerableDetector3D(vol, M, sigma, zxRatio) %#ok<STOUT,INUSD>