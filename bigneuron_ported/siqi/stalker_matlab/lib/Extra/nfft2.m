function [varargout] = nfft2(varargin)
%
% NFFT2 Two-dimensional discrete Fourier Transform.
%
%    Y = NFFT2(X) returns the two-dimensional Fourier transform of matrix X 
%    performing the visualization correction.
% 
%    Y = NFFT2(X,MROWS,NCOLS) pads matrix X with zeros to size MROWS-by-NCOLS
%    before transforming.
%
%    [Y,W1,W2] = NFFT2(X) returns also the normalized frequencies W1 and W2.
% 
%    NFFT2(X) makes the surf plot of NFFT2(X).
%
%    Y = NFFT2(X,...,'normalize') normalizes the transform [0 - 1].
%
%    See also FFT2, IFFT2, FFT, FFTSHIFT.
%
    
[X, MROWS, NCOLS, normalize] = parse_inputs(varargin{:});

R2 = floor(MROWS/2);
C2 = floor(NCOLS/2);

temp = fft2(X, MROWS, NCOLS);
%r = floor(size(X,1)/2);
%c = floor(size(X,2)/2);
%X = X(1:r, 1:c);
%Y = zeros(2*r, 2*c);
%Y(r+1:2*r, c+1:2*c) = X;
%Y(1:r,     c+1:2*c) = flipud(X);
%Y(1:r, 1:c) = fliplr(Y(1:r,c+1:2*c));
%Y(r+1:2*r, 1:c) = fliplr(X);
Y = fft2fix(temp);
clear temp

abs_Y = abs(Y);
max_abs_Y = max(max(abs_Y));

if normalize == 1;
   Y = Y/max_abs_Y;
end
   
switch nargout
case 0
   [W1,W2]=freqspace(size(Y),'meshgrid');
   if normalize == 0 
      surf(W1,W2,abs_Y)
      zlabel('Magnitude');
   else
      surf(W1,W2,abs_Y/max_abs_Y)
      zlabel('Normed Magnitude');
   end
      if prod(size(Y)) > 7500
      shading interp
   end
   xlabel('Wn1');
   ylabel('Wn2');

case 1
   varargout(1) = {Y};
   
case 2
   varargout(1) = {Y};
   varargout(2) = {W1};
   
case 3
   varargout(1) = {Y};
   varargout(2) = {W1};
   varargout(3) = {W2};
   
case 4
   error('Incorrect number of output parameters!')
   return
end

   
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [X, MROWS, NCOLS, normalize] = parse_inputs(varargin)
switch nargin
case 0
   error('Missing variable or function')
   
case 1
   X = varargin{1};
   MROWS = size(X,1);
   NCOLS = size(X,2);
   normalize = 0;
   
case 2
   X = varargin{1};
   if strcmp(varargin{2},'normalize')
      MROWS = size(X,1);
      normalize = 1;
   else
      MROWS = varargin{2};
      normalize = 0;
   end
   NCOLS = size(X,2);
   
case 3
   X = varargin{1};
   MROWS = varargin{2};
   if strcmp(varargin{3},'normalize')
      NCOLS = size(X,2);
      normalize = 1;
   else
      NCOLS = varargin{2};
      normalize = 0;
   end

case 4
   X = varargin{1};
   MROWS = varargin{2};
   NCOLS = varargin{3};
   if strcmp('normalize', varargin{4})
      normalize = 1;
   else
      error('Unknown parameter.')
      return
   end
   
case 5
   error('Too many inputs')
end



   