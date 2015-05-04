%MEDFILT1	One-dimensional median filter
%
%  	y = MEDFILT(x)
%  	y = MEDFILT(x, w)
%
% 	median filter the signal with window of width W (default is 5).
%
%
%	Copyright (c) Peter Corke, 1999  Machine Vision Toolbox for Matlab

%	pic 6/93
% vectorized version 8/95  pic

function m = medfilt1cir(s, w)
	if nargin == 1,
		w = 5;
	end
	
	s = s(:)';
	w2 = floor(w/2);
	w = 2*w2 + 1;

	n = length(s);
	m = zeros(w,n+w-1);
	s0 = s(1); sl = s(n);

	for i=0:(w-1)
            % changed code to do circular filtering
% 		m(i+1,:) = [s0*ones(1,i) s sl*ones(1,w-i-1)];
        m(i+1,:) = [s(end-i+1:end) s s(1:w-i-1)];
	end
	m = median(m);
	m = m(w2+1:w2+n);
