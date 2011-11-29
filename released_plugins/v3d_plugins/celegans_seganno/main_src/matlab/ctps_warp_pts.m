% Robust Point Matching (RPM) Demo (version 20000427):
% ----------------------------------------------------
% Copyright (C) 2000 Haili Chui, Anand Rangarajan
% 
% Authors: Haili Chui and Anand Rangarajan
% Date:    04/27/2000
% 
% Contact Information:
%
% Haili Chui:		chui@noodle.med.yale.edu
% Anand Rangarajan:	anand@noodle.med.yale.edu
% 
% Terms:	  
% 
% The source code (M-files) are provided under the
% terms of the GNU General Public License with an explicit
% clause permitting the execution of the M-files from within
% a MATLAB environment. See the LICENSE file for details.
%
%


% -------------------------------------------------------------------
% ctps_warp_pts.m
% ------------------------------------------------------------------- 
% Purpose: Given TPS [t,c,d], warp pts x --> x1.
%          (z is the basis point set).
%          (x1 is in normal coordinate, not expanded !).
%
% Usage: 
% [pts1] = ctps_warp_pts (pts, x, c, d);
% [pts1] = ctps_warp_pts (pts, x, y, lamda);
% [pts1] = ctps_warp_pts (pts, x, y);        lamda default = 1;
%
% 01/21/00

function [pts1]= ctps_warp_pts (pts, x, c, d);

% check input.
if nargin <= 1 | nargin >= 5
  disp ('# ERROR #: ctps_warp_pts -- wrong input !');
  help ctps_warp_pts; return;
end;


[dim1, dim2] = size(d); % d     -- affine: dim > 1
                        % lamda -- par:    dim = 1.

% --- [pts1] = ctps_warp_pts (pts, x, c, d) --------------------------
if dim1 > 1
  K = ctps_gen (pts,x);

  % Warp pts --> pts1:
  [n,dim] = size(pts); pts = [ones(n,1), pts];
  pts1 = pts*d + K*c;     %TPS=affine+nonrigid
%   pts1 = pts*d;             %only affine
  pts1 = pts1 (:,2:dim+1);
end;


% [pts1] = ctps_warp_pts (pts, x, y, lamda) ---------------------------
if dim1 == 1
  x = x;
  y = c;     % y     is taking c's position as input now.

  if (nargin == 3)
    lamda = 1;
  else
    lamda = d; % lamda is taking d's position as input now.
  end;
  
  K     = ctps_gen (pts,x);
  [c,d] = ctps_gen (x,y,lamda);
  
  % Warp pts --> pts1:
  [n,dim] = size(pts); pts = [ones(n,1), pts];
  pts1 = pts*d + K*c;
  pts1 = pts1 (:,2:dim+1);
end;

