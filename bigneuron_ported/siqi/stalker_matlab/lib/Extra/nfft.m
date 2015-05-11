function y = nfft(varargin)
% NFFT   Discrete Fourier transform.
%    Y = NFFT(X) returns the Fourier transform of matrix X 
%    performing the visualization correction.
% 
%    Y = NFFT(X,MROWS) pads matrix X with zeros to size MROWS before transforming.
% 
%    Y = NFFT(X,MROWS,type) plots single sided(type='s') or double sided(type='d')
%    frequency responce graphic (default = 's' - single sided)
%
%    Y = NFFT(X,MROWS,type,'normalize') normalizes the magnitude of 'Y'.
%
%    See also FFT, IFFT, FFT2, IFFT2, FFT, FFTSHIFT.

[X, MROWS, tipo, normalize] = parse_inputs(varargin{:});

R2 = floor(MROWS/2);


temp = fft(X, MROWS);
temp = temp(1:R2);
if tipo ==1 
   y= zeros(1,R2);
   y=temp;
   w1 = linspace(0,1,length(y));
else
   y = zeros(1,2*R2);
   y(R2+1:2*R2) = temp;
   y(1:R2) = fliplr(temp);
   w1 = linspace(-1,1,length(y));
end

abs_y = abs(y);

if normalize == 1
   plot(w1,abs_y/max(abs_y));
   ylabel('Normed Magnitude')
else
   plot(w1,abs_y);
   ylabel('Magnitude')
end
xlabel('Wn')

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [X, MROWS, tipo, normalize] = parse_inputs(varargin)
switch nargin
case 0
   error('Missing variable or function')
   
case 1
   X = varargin{1};
   MROWS = max(size(X,2),size(X,1));
   tipo = 1;
   normalize = 0;
   
case 2
   X = varargin{1};
   if strcmp(varargin{2},'normalize')
      normalize = 1;
      MROWS = max(size(X,2),size(X,1));
      tipo = 1;
   elseif strcmp(varargin{2},'s')
      normalize = 0;
      MROWS = max(size(X,2),size(X,1));
      tipo = 1;
   elseif strcmp(varargin{2},'d')
      normalize = 0;
      MROWS = max(size(X,2),size(X,1));
      tipo = 0;
   else
      MROWS = varargin{2};
      normalize = 0;
      tipo = 1;
   end
   
case 3
   X = varargin{1};
   flag=[0 0 0]; %[MROWS tipo normalize]
   for ct = 2 : 3
      if strcmp(varargin{ct},'normalize')
         normalize = 1;
         flag(3) = 1;
      elseif strcmp(varargin{ct},'s')
         tipo = 1;
         flag(2) = 1;
      elseif strcmp(varargin{ct},'d')
         tipo = 0;
         flag(2) = 1;
      else  
         if flag(1) == 1;
            error('Unknown parameter.')
            return
         elseif flag(1) == 0
            flag(1) = 1;
         end
         MROWS = varargin{ct};
      end
      if flag(1) == 0
         MROWS = max(size(X,2),size(X,1));
      end
      if flag(2) == 0
         tipo = 1;
      end
      if flag(3) == 0
         normalize = 0;
      end
   end
   
case 4
   X = varargin{1};
   MROWS = varargin{2};
   
   if strcmp(varargin{3},'s')
      tipo = 1;
   elseif strcmp(varargin{3},'d')
      tipo = 0;
   else
      error('Unknown parameter.')
      return
   end
   
   if strcmp(varargin{4},'normalize')
      normalize = 1;
   else
      error('Unknown parameter.')
      return
   end
   
case 5
   error('Too many inputs')
end



