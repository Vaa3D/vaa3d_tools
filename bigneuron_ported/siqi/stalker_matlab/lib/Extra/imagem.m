function varargout = imagem(varargin)
% IMAGEM   Displays image and marker.
%
%    IMAGEM(X,M) displays the grayscale or RGB image X and superimposes the 
%    b/w image M to it. The image M is intended to be a marker image of
%    some characteristics of X. X and M must have the same size.
%
%    IMAGEM(X,M1,M2,..,Mn) superimposes each new image at a different color.
%
%    IMAGEM(X,M1,M2,..,Mn,'color',C1,C2,..,Cn) uses the colors specified as
%    a RGB color vector. Alternativelly its possible to pass colors as strings.
%    The following color strings are accepted:
%
%    'r':red,      'g':green,     'b':blue,     'y':yellow, 
%    'm':magenta,  'c':cyan,      'w':white,     'k':black
%                        
%    Y = IMAGEM(...) returns the new superimposed image as an RGB image.
%

if nargout > 1
   error('Too many outputs.')
end

[X, MARK, COLOR] = parse_inputs(varargin{:});
if isempty(MARK)
   image(X)
   if nargout == 1
      varargout{1} = X;
   end
   return
end

if size(X,3) == 1 % normalizing image
   xmax = max(X(:));
   xmin = min(X(:));
   if ~(isa(X,'uint8') | (xmax<=1 & xmin>=1))
      X = (X-xmin)./(xmax-xmin);
   end
   X = repmat(X,[1 1 3]);
end

Y = mountim(X,MARK,COLOR);  % Creates mounted image

if nargout == 1
   varargout{1} = Y;
elseif nargout == 0
   image(Y);
   figure(gcf)
else
   error('Too many outputs.')
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [X, MARK, COLOR] = parse_inputs(varargin)
if nargin==0
   error('Too few inputs')
   
elseif nargin==1
   X = varargin{1};
   MARK = [];
   COLOR = [];
   
else  
   X = varargin{1};
   flag = 0;
   for i = 2 : nargin
      if strcmp(varargin{i},'color')
         MARK = varargin(2:i-1);
         COLOR = varargin(i+1:end);
         flag = 1;
         break
      end
   end
   
   if flag == 0
      MARK = varargin(2:end);
      COLOR = [];
   end
   
   sizechk(X,MARK{:}); % Checks if all images have the same size
   
   if length(COLOR)~=length(MARK) & ~isempty(COLOR)
      error('Number of image markers and colors must be equal.')
   end
     
   MARK = cat(3,MARK{:}); % Convert mark from cell to matrix
   MARK = logical(uint8(MARK~=0));
   COLOR = convcolor(COLOR);
   
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function sizechk(X,varargin)
S = size(X(:,:,1));
for i = 1 : nargin-1
   if size(varargin{i},3) > 1
      error('Marker images must be bidimensional.')
   end
   if size(varargin{i}) ~= S
      error('All images must have the same size.')
   end
end
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function Y = mountim(X,MARK,COLOR)
if isempty(COLOR)
   COLOR = [1 0 0;0 1 0;0 0 1;  1 1 0;1 0 1;0 1 1; .75 .25 0; .75 0 .25;.25 .75 0; .25 0 .75];
   if isa(X,'uint8')
      COLOR = 255*COLOR;
   end
end   
r = X(:,:,1); g = X(:,:,2); b = X(:,:,3);
for i = 1 : size(MARK,3)
   j = mod(i-1,length(COLOR))+1;
   r(MARK(:,:,i)) = COLOR(j,1);
   g(MARK(:,:,i)) = COLOR(j,2);
   b(MARK(:,:,i)) = COLOR(j,3);
end

Y = X;
Y(:,:,1) = r; Y(:,:,2) = g; Y(:,:,3) = b;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function Y = convcolor(X)
COLOR = [1 0 0; 0 1 0; 0 0 1; 1 1 0; 1 0 1; 0 1 1; 1 1 1; 0 0 0];
Y = zeros(length(X),3);
for i = 1 : length(X)
   if ischar(X{i})
      k = findstr('rgbymcwk', X{i});
      Y(i,:) = COLOR(k,:);
   else
      Y(i,:) = X{i};
   end
end