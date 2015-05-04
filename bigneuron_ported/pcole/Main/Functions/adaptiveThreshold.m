function bw = adaptiveThreshold(IM,ws,C,tm)
%  adaptiveThreshold is an adaptive thresholding algorithm that seperates the
%  foreground from the background of an image with nonuniform illumination.
%
%  bw = adaptivethreshold(IM,ws,C) outputs a binary image bw with the local 
%  threshold mean-C or median-C to the image IM.
%  ws is the local window size.
%  tm is 0 or 1, a switch between mean and median. tm=0 mean(default); tm=1 median.

if (nargin < 3)
    error('You must provide the image IM, the window size ws, and C.');
elseif (nargin == 3)
    tm = 0;
elseif (tm ~= 0 && tm ~= 1)
    error('tm must be 0 or 1.');
end

IM = mat2gray(IM);

if tm == 0
    mIM = imfilter(IM,fspecial('gaussian',ws),'replicate');
else
    mIM = medfilt2(IM,[ws ws]);
end

sIM = mIM-IM-C;
bw = im2bw(sIM,0);
bw = imcomplement(bw);