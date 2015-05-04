function maxima = findmaxima(x)
%FINDMAXIMA  Find location of local maxima
%  From David Sampson
%  See also FINDMINIMA

% Unwrap to vector
x = x(:);
% Identify whether signal is rising or falling
upordown = sign(diff(x));
% Find points where signal is rising before, falling after
maxflags = [upordown(1)<0; diff(upordown)<0; upordown(end)>0];
maxima   = find(maxflags);