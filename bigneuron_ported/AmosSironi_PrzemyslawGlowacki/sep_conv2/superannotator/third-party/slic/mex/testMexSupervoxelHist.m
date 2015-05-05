addpath /home/cjb/matlab/
addpath ../

img = readMultiPageTiff('/data/phd/twoexamples/00147.tif');

%img = img(1:50,1:50,1:50);

%img = double(img);

%% cluster
%[res, idxs] = mex_supervoxel(img, 10, 40);
tic;
[res, numlabels] = slic_supervoxel(img, 10, 20);
toc
%[res, numlabels] = slic_supervoxel(img, 2, 2);

%% Histograms
histMinVal = uint8(0);
histMaxVal = uint8(255);
histNBins = uint32(20);

[ownHist, nbHist] = slic_genhistograms( img, res, numlabels, histMinVal, histMaxVal, histNBins );

%% Hist 2 sided
histMinVal = uint8(0);
histMaxVal = uint8(255);
histNBins = uint32(20);

ctroidOrient = single(rand(3, numlabels));  % random orient
tic;
neighborSearchDepth = 1;
[ownHist, frontHist, backHist, sideHist, sideData] = slic_gen_twosided_histograms( img, img, res, numlabels, ctroidOrient, histMinVal, histMaxVal, histNBins, neighborSearchDepth );
toc