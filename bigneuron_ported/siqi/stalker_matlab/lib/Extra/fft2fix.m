function Y = fft2fix(X)
% FFT2FIX2  Fixes FFT2 graphic
%
%    Using Y = FFT2FIX(X), where X is a FFT2 graphic returns Y
%    as the same graphic as X, but the central point of Y will
%    represent the origin of the graphic.
%    This changes the FFT2 graphic aspect of having 4 separate
%    pieces and put them together for better visualization
%

r = floor(size(X,1)/2);
c = floor(size(X,2)/2);

X = X(1:r, 1:c);
Y = zeros(2*r, 2*c);

Y(r+1:2*r, c+1:2*c) = X;
Y(1:r,     c+1:2*c) = flipud(X);
Y(1:r, 1:c) = fliplr(Y(1:r,c+1:2*c));
Y(r+1:2*r, 1:c) = fliplr(X);