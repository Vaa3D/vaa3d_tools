clear;

N = 100;
img = uint8( rand([N,N,N]) * 255 );

[res, numlabels] = slic_supervoxel(img, 10, 40);

%% Histograms
histMinVal = uint8(0);
histMaxVal = uint8(255);
histNBins = uint32(20);

[ownHist, nbHist] = slic_genhistograms( img, res, numlabels, histMinVal, histMaxVal, histNBins );

%% compute map to test histograms
map = slic_slicmap( res, numlabels );

%% check some random numbers
P = min(1000, numlabels);
idx = randperm(numlabels);
idx = idx(1:P) - 1;

d = [];
z = [];
for I=1:P
    svIdx = map{idx(I)+1} + 1;
    
    mH = histc( double(img( svIdx )), [linspace(0,255,21)] );
    zH = mH(end);
    mH = mH(1:(end-1));
    oH = double(ownHist(:,idx(I)+1));
    d(I) = max(oH ./ mH);
    z(I) = max(mH ./ oH);
    if (sum(oH) ~= sum(mH) + zH)
        error('err');
    end
end