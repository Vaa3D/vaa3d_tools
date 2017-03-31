% % 2016.11.10
% %  plotting results of s2 scans from s2summary data and the output of ZZ's
% %  tip finder
% %
% %  currently there are 3 neurons.  the relevant data from each one is
% %  basically the last entry in the S2summary.txt  
% %  Since there are only 3 cells, I'm not going to go through the trouble of parsing the data automatically
% %  instead I'm just going to copy-and-paste it here and generate plots by hand.
% 
% 

%  desired output:
%   1. scan area efficiency, with and without overlap, compared to tight
%   boundingbox and integer max field bounding box.
%   2. time efficiency compared to both above
%   3. tip analysis and comparison to chance. out of ~100 tips, 20 ended at
%   tile boundaries, which is 10 above chance,  so estimate 90% accurate in tip
%   identification


%    background info, calculated below
%   1.  max-size imaging is 1.56s/slice +/-.01    1730x1730 0.23um/pixel
%   This is set based on keeping the laser power constant and acheiving
%   similar signal by averaging.  One could also increase laser power, but
%   that would result in more bleaching.
%   2. total tile time for me :  (1.3-1.7s) my overhead per tile + tile setup time (PV)  + true scan  time derived from resonant freq. + per-plane-overhead + total image .tif conversion 
%   for single tiles  4.8s (actual imaging +.tif) -> 6.1s  for my "imaging
%   time" for a tile ->  1.3s overhead

%  TIP calculation input is based on Zhi's plugin which identifies true
%  tips (i.e. not just extra little spines, etc see S2 analysis plugin for details)  and how many of those true
%  tips are within 5% of tile edges.
%  so given that each tile has 5% edges, each tile area is 5% + 5% +
%  2*(5%*(1-10%)) = 10%+9% = 19% of area counted as 'edge'

%  so the estimated number of real tips that should be counted as an edge
%  regardless of S2 scanning would be  N_total_tips * .19
%  the remaining tips can be interpreted as failures or artifacts. The
%  relevant denominator is the total number of tips so 

%  S2 quality =  1- (N_edge_tips - N_total_tips * edgeFraction )/ N_total_tips



% overall, we're talking about 5s per tile and 1.Xs per tile my overhead
% for the 501 stack. something like 2.8s + 1.X for 250slices


%%  neuron 1 /local2/data/2016_11_03_Thu_14_02/2016_11_03_Thu_14_02/
% @@  running totals ;    @@
% imaged area ;1.46162e+06
% sum of tile areas ;1.67613e+06
% boundingBoxX ;1850
% boundingBoxY ;3401
% boundingBox Area ;6.29185e+06
% tiles in queue ;0
% tiles imaged ;68
% total imaging time ;329499
% total analysis time ;245889
% ms since start ;370054
% average ms between tile starts ;5417.66
% z = 169 slices



%  SPATIAL SCAN EFFICIENCY:
nSlices1= 169
bigZoomTimePerSlice = 1.56
imArea1 = 1.46162e+06
sumTiles1 = 1.67613e+06
bbX1 =1850
bbY1 = 3401
bbA1 = bbX1*bbY1
areaEfficiency1 = imArea1/bbA1
tileEfficiency1 = sumTiles1/bbA1
percentOverlap1 = sumTiles1/imArea1


%  TIME EFFICIENCY:
nTiles1 = 68
myTotalImagintTime1 = 329.499
totalAnalysisTime1 =  245.889
totalS2Time1 = 370.054
timeBetweeTiles1 =5.41766

% time to image tight BB at max image size
tightBB1 = bbA1/(1730*1730)
tightBBtime1 = bigZoomTimePerSlice*nSlices1*tightBB1
% integer max
xIntBB1 = ceil(bbX1/1730)
yIntBB1 = ceil(bbY1/1730)
integerBBtime1 = bigZoomTimePerSlice*nSlices1*xIntBB1*yIntBB1

%  s2 speedup:
s2SpeedTightBB1 = tightBBtime1/totalS2Time1
s2SpeedIntBB1 = integerBBtime1/totalS2Time1

% TIPS:
%swc file /local2/s2Data_II/2016_11_03_Thu_14_02/scanData.txt_tips.swc has been generated, size: 3586
% edge (11) to tip (57) ratio is 0.192982,
edgeFraction = 0.19;
edge1 = 11
tip1 = 57
s2Q1 = 1- (edge1-(tip1*edgeFraction))/tip1

%%  neuron 2 /local2/data/2016_11_08_Tue_13_12/2016_11_08_Tue_13_13
% @@  running totals ;    @@
% imaged area ;2.67096e+06
% sum of tile areas ;3.10577e+06
% boundingBoxX ;3823
% boundingBoxY ;3541
% boundingBox Area ;1.35372e+07
% tiles in queue ;0
% tiles imaged ;126
% total imaging time ;767682
% total analysis time ;529085
% ms since start ;790554
% average ms between tile starts ;6241.48
% @@@@@@@@@@@;@@@@@@@@@@@
% z = 251 slices
nSlices2 = 251
imArea2 = 2.67096e+06
sumTiles2 = 3.10577e+06
bbX2 =3823
bbY2 = 3541
bbA2 = bbX2*bbY2
areaEfficiency2 = imArea2/bbA2
tileEfficiency2 = sumTiles2/bbA2
percentOverlap2 = sumTiles2/imArea2


%  TIME EFFICIENCY:
nTiles2 = 126
myTotalImagintTime2 = 767.682
totalAnalysisTime2 =  529.085
totalS2Time2 = 790.554
timeBetweeTiles2 =6.24148

% time to image tight BB at max image size
tightBB2 = bbA2/(1730*1730)
tightBBtime2 = bigZoomTimePerSlice*nSlices2*tightBB2
% integer max
xIntBB2 = ceil(bbX2/1730)
yIntBB2 = ceil(bbY2/1730)
integerBBtime2 = bigZoomTimePerSlice*nSlices2*xIntBB2*yIntBB2

%  s2 speedup:
s2SpeedTightBB2 = tightBBtime2/totalS2Time2
s2SpeedIntBB2 = integerBBtime2/totalS2Time2



% TIPS
%swc file /local2/s2Data_II/2016_11_08_Tue_13_13/scanData.txt_tips.swc has been generated, size: 6160
%edge (31) to tip (101) ratio is 0.306931,

edgeFraction = 0.19;
edge2 = 31
tip2 = 101
s2Q2 = 1- (edge2-(tip2*edgeFraction))/tip2














%%  neuron 3 /local2/data/2016_11_08_Tue_15_36/2016_11_08_Tue_16_07/
%  @@  running totals ;    @@
% imaged area ;1.84117e+06
% sum of tile areas ;2.11981e+06
% boundingBoxX ;2414
% boundingBoxY ;3400
% boundingBox Area ;8.2076e+06
% tiles in queue ;0
% tiles imaged ;86
% total imaging time ;867298
% total analysis time ;761183
% ms since start ;984383
% average ms between tile starts ;11381
nSlices3 = 501
imArea3 = 1.84117e+06
sumTiles3 = 2.11981e+06
bbX3 =2414
bbY3 = 3400
bbA3 = bbX3*bbY3
areaEfficiency3 = imArea3/bbA3
tileEfficiency3 = sumTiles3/bbA3
percentOverlap3 = sumTiles3/imArea3


%  TIME EFFICIENCY:
nTiles3 = 86
myTotalImagintTime3 = 867.298
totalAnalysisTime3 =  761.183
totalS2Time3 = 984.383
timeBetweeTiles3 =11.381

% time to image tight BB at max image size
tightBB3 = bbA3/(1730*1730)
tightBBtime3 = bigZoomTimePerSlice*nSlices3*tightBB3
% integer max
xIntBB3 = ceil(bbX3/1730)
yIntBB3 = ceil(bbY3/1730)
integerBBtime3 = bigZoomTimePerSlice*nSlices3*xIntBB3*yIntBB3

%  s2 speedup:
s2SpeedTightBB3 = tightBBtime3/totalS2Time3
s2SpeedIntBB3 = integerBBtime3/totalS2Time3

% TIP CORRECTNESS: 
% run on 
% swc file /local2/s2Data_II/2016_11_08_Tue_16_07/scanData.txt_tips.swc has been generated, size: 3620
% edge (17) to tip (74) ratio is 0.22973,

edgeFraction = 0.19;
edge3 = 17
tip3 = 74
s2Q3 = 1- (edge3-(tip3*edgeFraction))/tip3


%%  scan 4 is a multineuron scan

% 
% tile filename ;/local2/data/2016_11_04_Fri_09_38/2016_11_04_Fri_09_39/x_894_y_3294_ZSeries-11042016-0914-29521_Cycle00001_Ch2_000001.ome.tifCh2.v3draw
% tile event 0 ;2016_11_04_Fri_09_55_35_177
% tile event 1 ;2016_11_04_Fri_09_55_37_053
% tile event 2 ;2016_11_04_Fri_09_55_42_141
% tile event 3 ;2016_11_04_Fri_09_55_44_991
% elapsed time 0 ;0
% elapsed time 1 ;1876
% elapsed time 2 ;5088
% elapsed time 3 ;2850
% @@  running totals ;    @@
% imaged area ;3.71268e+06
% sum of tile areas ;4.23963e+06
% boundingBoxX ;4246
% boundingBoxY ;5938
% boundingBox Area ;2.52127e+07
% tiles in queue ;0
% tiles imaged ;176
% total imaging time ;900668
% total analysis time ;544481
% ms since start ;967815
% average ms between tile starts ;5477.32


nSlices4 = 191
imArea4 = 3.71268e+06
sumTiles4 = 4.23963e+06
bbX4 =4246
bbY4 = 5938
bbA4 = bbX4*bbY4
areaEfficiency4 = imArea4/bbA4
tileEfficiency4 = sumTiles4/bbA4
percentOverlap4 = sumTiles4/imArea4


%  TIME EFFICIENCY:
nTiles4 = 176
myTotalImagintTime4 = 900.668
totalAnalysisTime4 =  544.481
totalS2Time4 = 967.815
timeBetweeTiles4 =5.477

% time to image tight BB at max image size
tightBB4 = bbA4/(1730*1730)
tightBBtime4 = bigZoomTimePerSlice*nSlices4*tightBB4
% integer max
xIntBB4 = ceil(bbX4/1730)
yIntBB4 = ceil(bbY4/1730)
integerBBtime4 = bigZoomTimePerSlice*nSlices4*xIntBB4*yIntBB4

%  s2 speedup:
s2SpeedTightBB4 = tightBBtime4/totalS2Time4
s2SpeedIntBB4 = integerBBtime4/totalS2Time4




% % TIP CORRECTNESS:
% swc file /local2/s2Data_II/multi_neuron_data/2016_11_04_Fri_09_39/scanData.txt_tips.swc has been generated, size: 7390
% edge (46) to tip (123) ratio is 0.373984,
edgeFraction = 0.19
edge4 = 46
tip4 = 123
s2Q4 = 1- (edge4 - (tip4*edgeFraction))/tip4

%%  scan 5  multineuron 
% @@@@@@@@@@@;@@@@@@@@@@@
% tile number ;142
% tile mode ;Resonant Galvo
% tile zoom ;13.0446
% microns per pixel ;0.228737
% tracing algorithm ;NeuTube
% tile filename ;/local2/data/2016_12_06_Tue_15_51/2016_12_06_Tue_16_13/x_1821_y_-2054_ZSeries-12062016-1102-33624_Cycle00001_Ch2_000001.ome.tifCh2.v3draw
% tile event 0 ;2016_12_06_Tue_16_37_41_113
% tile event 1 ;2016_12_06_Tue_16_37_41_413
% tile event 2 ;2016_12_06_Tue_16_37_51_045
% tile event 3 ;2016_12_06_Tue_16_38_02_545
% elapsed time 0 ;0
% elapsed time 1 ;300
% elapsed time 2 ;9632
% elapsed time 3 ;11500
% @@  running totals ;    @@
% imaged area ;2.98766e+06
% sum of tile areas ;3.52481e+06
% boundingBoxX ;3259
% boundingBoxY ;2696
% boundingBox Area ;8.78626e+06
% tiles in queue ;0
% tiles imaged ;143
% total imaging time ;1.38284e+06
% total analysis time ;1.91911e+06
% ms since start ;1453476
% average ms between tile starts ;10084.8



nSlices5 = 457
imArea5 =2.98766e+06
sumTiles5 = 3.52481e+06
bbX5 =3259
bbY5 = 2696
bbA5 = bbX5*bbY5
areaEfficiency5 = imArea5/bbA5
tileEfficiency5 = sumTiles5/bbA5
percentOverlap5 = sumTiles5/imArea5


%  TIME EFFICIENCY:
nTiles5 = 142
myTotalImagintTime5 = 1382.84
totalAnalysisTime5 =  1919.11
totalS2Time5 = 1453.476
timeBetweeTiles5 =10.085

% time to image tight BB at max image size
tightBB5 = bbA5/(1730*1730)
tightBBtime5 = bigZoomTimePerSlice*nSlices5*tightBB5
% integer max
xIntBB5 = ceil(bbX5/1730)
yIntBB5 = ceil(bbY5/1730)
integerBBtime5 = bigZoomTimePerSlice*nSlices5*xIntBB5*yIntBB5

%  s2 speedup:
s2SpeedTightBB5 = tightBBtime5/totalS2Time5
s2SpeedIntBB5 = integerBBtime5/totalS2Time5




% % TIP CORRECTNESS:
%_tips.marker : 167
%_edgeTips.marker : 72


edgeFraction = 0.19
edge5 = 72
tip5 = 167
s2Q5 = 1- (edge5 - (tip5*edgeFraction))/tip5


%% scan 6 multineuron

% 
% @@@@@@@@@@@;@@@@@@@@@@@
% tile number ;504
% tile mode ;Resonant Galvo
% tile zoom ;13.0446
% microns per pixel ;0.228737
% tracing algorithm ;NeuTube
% tile filename ;/local2/data/2016_12_07_Wed_14_58/2016_12_07_Wed_17_18/x_2958_y_-3161_ZSeries-12072016-1455-34929_Cycle00001_Ch2_000001.ome.tifCh2.v3draw
% tile event 0 ;2016_12_07_Wed_18_42_14_067
% tile event 1 ;2016_12_07_Wed_18_42_16_070
% tile event 2 ;2016_12_07_Wed_18_42_21_507
% tile event 3 ;2016_12_07_Wed_18_42_24_205
% elapsed time 0 ;0
% elapsed time 1 ;2003
% elapsed time 2 ;5437
% elapsed time 3 ;2698
% @@  running totals ;    @@
% imaged area ;1.03286e+07
% sum of tile areas ;1.24477e+07
% boundingBoxX ;5234
% boundingBoxY ;6926
% boundingBox Area ;3.62507e+07
% tiles in queue ;0
% tiles imaged ;505
% total imaging time ;4.88781e+06
% total analysis time ;7.80681e+06
% ms since start ;5008927
% average ms between tile starts ;9917.63
% 
% 


nSlices6 = 213
imArea6 = 1.03286e+07
sumTiles6 = 1.24477e+07
bbX6 =5234
bbY6 = 6926
bbA6 = bbX6*bbY6
areaEfficiency6 = imArea6/bbA6
tileEfficiency6 = sumTiles6/bbA6
percentOverlap6 = sumTiles6/imArea6


%  TIME EFFICIENCY:
nTiles6 =504
myTotalImagintTime6 = 4887.81
totalAnalysisTime6 = 7806.81
totalS2Time6 = 5008.927
timeBetweeTiles6 =9.918

% time to image tight BB at max image size
tightBB6 = bbA6/(1730*1730)
tightBBtime6 = bigZoomTimePerSlice*nSlices6*tightBB6
% integer max
xIntBB6 = ceil(bbX6/1730)
yIntBB6 = ceil(bbY6/1730)
integerBBtime6 = bigZoomTimePerSlice*nSlices6*xIntBB6*yIntBB6

%  s2 speedup:
s2SpeedTightBB6 = tightBBtime6/totalS2Time6
s2SpeedIntBB6 = integerBBtime6/totalS2Time6






% % TIP CORRECTNESS:

%tips.marker: 1328
%edgeTips.marker: 516

edgeFraction = 0.19
edge6 = 516
tip6 = 1328
s2Q6 = 1- (edge6 - (tip6*edgeFraction))/tip6

%% human@@@@@@@@@@@;@@@@@@@@@@@
% tile number ;120
% tile mode ;Resonant Galvo
% tile zoom ;13.0446
% microns per pixel ;0.228737
% tracing algorithm ;NeuTube
% tile filename ;/local2/s2Data_II/2017_02_15_Wed_18_38/2017_02_15_Wed_18_39/x_706_y_1475_ZSeries-02142017-0858-35365_Cycle00001_Ch2_000001.ome.tifG-R.v3draw
% tile event 0 ;2017_02_15_Wed_18_52_24_741
% tile event 1 ;2017_02_15_Wed_18_52_25_394
% tile event 2 ;2017_02_15_Wed_18_52_31_347
% tile event 3 ;2017_02_15_Wed_18_52_36_218
% elapsed time 0 ;0
% elapsed time 1 ;653
% elapsed time 2 ;5953
% elapsed time 3 ;4871
% @@  running totals ;    @@
% imaged area ;2.53151e+06
% sum of tile areas ;2.98253e+06
% boundingBoxX ;1849
% boundingBoxY ;2555
% boundingBox Area ;4.7242e+06
% tiles in queue ;0
% tiles imaged ;121
% total imaging time ;725683
% total analysis time ;914035
% ms since start ;768230
% average ms between tile starts ;6310.27 neuron 



nSlices7 = 235
imArea7 = 2.53151e+06
sumTiles7 = 2.98253e+06
bbX7 =1849
bbY7 =2555
bbA7 = bbX7*bbY7
areaEfficiency7 = imArea7/bbA7
tileEfficiency7 = sumTiles7/bbA7
percentOverlap7 = sumTiles7/imArea7


%  TIME EFFICIENCY:
nTiles7 =120
myTotalImagintTime7 = 725.683
totalAnalysisTime7 = 914.035
totalS2Time7 = 768.230
timeBetweeTiles7 =6.31027 

% time to image tight BB at max image size
tightBB7 = bbA7/(1730*1730)
tightBBtime7 = bigZoomTimePerSlice*nSlices7*tightBB7
% integer max
xIntBB7 = ceil(bbX7/1730)
yIntBB7 = ceil(bbY7/1730)
integerBBtime7 = bigZoomTimePerSlice*nSlices7*xIntBB7*yIntBB7

%  s2 speedup:
s2SpeedTightBB7 = tightBBtime7/totalS2Time7
s2SpeedIntBB7 = integerBBtime7/totalS2Time7






% % TIP CORRECTNESS:

% tip
%swc file /local2/s2Data_II/humanCell/2017_02_15_Wed_18_38/2017_02_15_Wed_18_39/scanData.txt_tips.swc has been generated, size: 5354
%edge (9) to tip (56) ratio is 0.160714,
edgeFraction = 0.16
edge7 = 9
tip7 = 56
s2Q7 = 1- (edge7 - (tip7*edgeFraction))/tip7




%% PLOTTING


% area efficiency
s2AreaData = [[imArea1, imArea2, imArea3, imArea7];...
    [sumTiles1, sumTiles2, sumTiles3, sumTiles7];...
    [bbA1, bbA2, bbA3, bbA7];...
    [xIntBB1*yIntBB1, xIntBB2*yIntBB2, xIntBB3*yIntBB3, xIntBB7*yIntBB7]*1730*1730;...
    [nTiles1,nTiles2, nTiles3, nTiles7];...
    [nSlices1, nSlices2, nSlices3, nSlices7]]


% time speedup
s2TimeData = [[totalS2Time1, totalS2Time2, totalS2Time3, totalS2Time7];...
    [tightBBtime1, tightBBtime2, tightBBtime3, tightBBtime7];...
    [integerBBtime1, integerBBtime2, integerBBtime3, integerBBtime7]]



% microscope efficiency and analysis overhead
s2InternalTimeData = [[totalS2Time1, totalS2Time2, totalS2Time3, totalS2Time7];...
    [myTotalImagintTime1, myTotalImagintTime2, myTotalImagintTime3, myTotalImagintTime7];...
    [totalAnalysisTime1, totalAnalysisTime2, totalAnalysisTime3, totalAnalysisTime7];...
    [timeBetweeTiles1, timeBetweeTiles2, timeBetweeTiles3, timeBetweeTiles7]]


%  tip correctness
s2TipData = [[edge1, edge2, edge3, edge7];...
    [tip1, tip2, tip3, tip7];...
    [s2Q1, s2Q2, s2Q3, s2Q7]]




%% summary statistics:

% area efficiency:

sumTilesVsTightBB = s2AreaData(3,:)./s2AreaData(2,:)
imAreaVsTightBB = s2AreaData(3,:)./s2AreaData(1,:)

sumTilesVsIntBB = s2AreaData(4,:)./s2AreaData(2,:)
imAreaVsIntBB = s2AreaData(4,:)./s2AreaData(1,:)




%% table of data, including scale in mm and multi-neuronScan 4-6
%  duplicated columns are to facilitate merging data in excel for layout
%  purposes, specifically to include pixel and mm size data under a single
%  heading. 
dataTable = [[1,2, 3, 7, 4, 5, 6];...
   [imArea1, imArea2, imArea3,imArea7, imArea4, imArea5, imArea6];...
    [sumTiles1, sumTiles2, sumTiles3,sumTiles7, sumTiles4, sumTiles5, sumTiles6];...
    [bbX1, bbX2, bbX3,bbX7, bbX4, bbX5, bbX6]*.229;...
        [bbY1, bbY2, bbY3,bbY7, bbY4, bbY5, bbY6]*.229;...
    [xIntBB1, xIntBB2, xIntBB3,xIntBB7, xIntBB4, xIntBB5, xIntBB6]*1730*0.229;...
    [yIntBB1, yIntBB2, yIntBB3, yIntBB7,yIntBB4, yIntBB5, yIntBB6]*1730*0.229;...
%%  area efficiency 
    ( [xIntBB1*yIntBB1, xIntBB2*yIntBB2, xIntBB3*yIntBB3,xIntBB7*yIntBB7,xIntBB4*yIntBB4, xIntBB5*yIntBB5, xIntBB6*yIntBB6]*1730*1730)./[sumTiles1, sumTiles2, sumTiles3, sumTiles7, sumTiles4, sumTiles5, sumTiles6];...
%%  time benefit
    [integerBBtime1/totalS2Time1,integerBBtime2/totalS2Time2,integerBBtime3/totalS2Time3,integerBBtime7/totalS2Time7, integerBBtime4/totalS2Time4,integerBBtime5/totalS2Time5,integerBBtime6/totalS2Time6];...
    [tightBBtime1/totalS2Time1,tightBBtime2/totalS2Time2,tightBBtime3/totalS2Time3,tightBBtime7/totalS2Time7, tightBBtime4/totalS2Time4, tightBBtime5/totalS2Time5,tightBBtime6/totalS2Time6];...
    [nTiles1,nTiles2, nTiles3, nTiles7, nTiles4, nTiles5, nTiles6];...
    [nSlices1, nSlices2, nSlices3,nSlices7, nSlices4, nSlices5, nSlices6];...
    [1.0, 1.0, 0.5, 1.0, 1.0, .5, 1.0];... //z step
    [totalS2Time1, totalS2Time2, totalS2Time3, totalS2Time7 totalS2Time4, totalS2Time5, totalS2Time6];...
    [myTotalImagintTime1, myTotalImagintTime2, myTotalImagintTime3, myTotalImagintTime7 myTotalImagintTime4, myTotalImagintTime5, myTotalImagintTime6];...
    [totalAnalysisTime1, totalAnalysisTime2, totalAnalysisTime3,totalAnalysisTime7, totalAnalysisTime4, totalAnalysisTime5, totalAnalysisTime6];...
       [tip1, tip2, tip3,tip7, tip4, tip5, tip6];...
        [edge1, edge2, edge3,edge7, edge4, edge5, edge6];...

    ceil(edgeFraction*[tip1, tip2, tip3,tip7, tip4, tip5, tip6]);...   
    [s2Q1, s2Q2, s2Q3,s2Q7, s2Q4, s2Q5, s2Q6];...
]
dataTable = [dataTable,dataTable];
dataTable(4:7,8:14) = [[bbX1, bbX2, bbX3,bbX7, bbX4, bbX5, bbX6];...
        [bbY1, bbY2, bbY3,bbY7, bbY4, bbY5, bbY6];...
    [xIntBB1, xIntBB2, xIntBB3, xIntBB7, xIntBB4, xIntBB5, xIntBB6]*1730;...
    [yIntBB1, yIntBB2, yIntBB3, yIntBB7, yIntBB4, yIntBB5, yIntBB6]*1730;]

%  column headings:
headings = {'Scan Number', 'Imaged Area', 'Sum of Tile Areas', 'X_TightBB', 'Y_TightBB', 'X_IntBB', 'Y_IntBB', 'Area Efficiency', ...
    'S2 Speedup (Integer BB)', 'S2 Speedup (Tight BB)', 'n_tiles', 'n_z_planes', 'z step (microns)', ...
    'total S2 Time', 'total imaging time', 'total analysis time', 'number of tips', 'number of edge tips', 'expected number of tips', 'S2 Quality Index'}

%csvwrite('/data/mat/BRL/s2_human/S2_with_human/tableDataHeadings.csv', headings)
%writetable(table(headings),'/data/mat/BRL/s2_human/S2_with_human/tableDataHeadings.txt')

%%
figure;
bar( fliplr(s2AreaData(1:4,:)'))
xlabel('S2 Scan')
ylabel('tile area (total voxels/depth)')
bip(gca)
title('S2 Scan Area')

figure;
bar([s2AreaData(3,:)./s2AreaData(1,:);s2AreaData(3,:)./s2AreaData(2,:);  s2TimeData(2,:)./s2TimeData(1,:)]')
xlabel('S2 Scan')
ylabel('fold improvement')
bip(gca)
title('S2 Scan Speed vs Tight Bounding Box')

figure;
bar([s2AreaData(4,:)./s2AreaData(1,:);s2AreaData(4,:)./s2AreaData(2,:); s2TimeData(3,:)./s2TimeData(1,:)]')
xlabel('S2 Scan')
ylabel('fold improvemnent')
bip(gca)
title('S2 Scan Speed vs Integer Bounding Box')

figure;
bar([[tightBBtime1, tightBBtime2, tightBBtime3, tightBBtime7]; [s2InternalTimeData(2,:)+s2InternalTimeData(3,:)]; s2InternalTimeData(1,:)]')
xlabel('S2 Scan')
ylabel('time (s)')
bip(gca)
title('S2 Scan Time')


figure;
bar([s2TipData(1,:); s2TipData(2,:)]')
hold all, plot([1,2,3, 4], s2TipData(3,:)*100)
xlabel('S2 Scan')
ylabel('N_{tips}')
title('S2 Quality')
bip(gca)

%  the above plots were modified to enter display names and resized.  the
%  last one the axes were duplicated on the right side to indicate a
%  dual-plot. Fortuitously, the percentage and the max tips are very
%  similar so the axis scale and range are the same between the bar plot
%  and the line plot.

% plots were saved as .fig, .eps and .png files 11/17 and 2/21/17


% % outputs:
% s2AreaData =
% 
%      1461620     2670960     1841170
%      1676130     3105770     2119810
%      6291850    13537243     8207600
%     11971600    26936100    11971600
%           68         126          86
%          169         251         501
% 
% 
% s2TimeData =
% 
%    1.0e+03 *
% 
%     0.3701    0.7906    0.9844
%     0.5542    1.7711    2.1433
%     1.0546    3.5240    3.1262
% 
% 
% s2InternalTimeData =
% 
%   370.0540  790.5540  984.3830
%   329.4990  767.6820  867.2980
%   245.8890  529.0850  761.1830
%     5.4177    6.2415   11.3810
% 
% 
% s2TipData =
% 
%    11.0000   31.0000   17.0000
%    57.0000  101.0000   74.0000
%     0.9970    0.8831    0.9603

% %%
%  fliplr(s2AreaData(1:3,:)')
% 
% ans =
% 
%      6291850     1676130     1461620
%     13537243     3105770     2670960
%      8207600     2119810     1841170
% %ylabel('tile area (total voxels/depth)')
% %title('S2 Scan Area')
% 
% 
% 
% [s2AreaData(3,:)./s2AreaData(1,:);s2AreaData(3,:)./s2AreaData(2,:);  s2TimeData(2,:)./s2TimeData(1,:)]'
% ans =
% 
%     4.3047    3.7538    1.4977
%     5.0683    4.3587    2.2403
%     4.4578    3.8719    2.1773
% %ylabel('fold improvement')
% %title('S2 Scan Speed vs Tight Bounding Box')
% 
% [s2AreaData(4,:)./s2AreaData(1,:);s2AreaData(4,:)./s2AreaData(2,:); s2TimeData(3,:)./s2TimeData(1,:)]'
% 
% ans =
% 
%     8.1906    7.1424    2.8497
%    10.0848    8.6729    4.4577
%     6.5022    5.6475    3.1758
% %ylabel('fold improvemnent')
% %title('S2 Scan Speed vs Integer Bounding Box')
% 
% [[tightBBtime1, tightBBtime2, tightBBtime3]; [s2InternalTimeData(2,:)+s2InternalTimeData(3,:)]; s2InternalTimeData(1,:)]'
% 
% ans =
% 
%    1.0e+03 *
% 
%     0.5542    0.5754    0.3701
%     1.7711    1.2968    0.7906
%     2.1433    1.6285    0.9844
% %ylabel('time (s)')
% %title('S2 Scan Time')
% 
% [s2TipData(1,:); s2TipData(2,:)]'
% 
% 
%     11    57
%     31   101
%     17    74
% s2TipData(3,:)*100
% 
%    99.7018   88.3069   96.0270
%title('S2 Quality')
%bip(gca)

%% multi-neuron scans

% 
% marker num = 29131, save swc file to /local2/data/2016_11_30_Wed_15_30/2016_11_30_Wed_15_33/scanData.txt_tips.swc
% -------------------------------------------------------
% ---------------------read 29131 lines, 29131 remained lines
% done with saving file: /local2/data/2016_11_30_Wed_15_30/2016_11_30_Wed_15_33/scanData.txt_tips.marker
% done with saving file: /local2/data/2016_11_30_Wed_15_30/2016_11_30_Wed_15_33/scanData.txt_edgeTips.marker
% swc file /local2/data/2016_11_30_Wed_15_30/2016_11_30_Wed_15_33/scanData.txt_tips.swc has been generated, size: 22214
% edge (190) to tip (460) ratio is 0.413043,
% 






%%
% %%
% getting maximum scan 1730x1730 with 8x averaging although should probably be 11x...
% 
% 12x  4min:56s   = 3552s for 191 slices
%     max imaging is 1.56s/slice +/-.01    1730x1730 0.23um/pixel
% %%  going over these numbers shows that for the 8x averaging, the 8khz scan speed underestimates the imaging time by a few percent.  
%     it pencils out to 3ms/frame -> 25ms/ averaged frame.  it's in the imaging time, not the .tif conversion

%  actually there's 4.7s of setup time for this big stack!  8.3 is
%  unaccounted for- that is 17ms per plane, 
% time between planes is 0.88s but it should be 0.864s , so 15ms per plane
% + 4.7s setup time.
%

% data : a max-scan stack 1730x1730 at zoom 1.15 with 0.235x0.235x0.5um voxels and 8x averaging takes 446s imaging and 342s Tif conversion = 788s.
%  pure imaging time is 433s, not 446.  
% %%  
% for the unaveraged single tiles:  4.9s pure imaging for 501 z slices, but
% 5.39s from the .xml file.  this is ~9% slower than pure imaging
%  201ms setup time (relative v absolute) + extra .6ms per plane (!) 

% so the imaging is actually slightly less efficient for multiplane
% averaging stacks of large images than for unavaraged small tiles.
% haven't checked .tif conversion.  


% time per pixel:  big scan, with 8x frame averaging: 0.5us per pixel
% [including .tif]
%  157pix tile, no averaging: .677us per pixel  including rough .tif
%  estimation from file modified dates.
%  so possibly slightly worse per pixel, but only by 26%  (ROUGH estimate!
%  probably 1 sig fig!)
% also imaging + tif for 501-slice tiles is ~8.3s, average 'imaging time'
% from s2 is 10.08s.  We know there is 1s hangout time between configure
% and sending the signal to start the stack.  1.7s is then "tile
% configuration and sending the updated string name"
%  the latter could be reduced to ~10ms with a dedicated checker of the
%  current index
% the former could maybe be reduced with some kind of polling of the
% readiness?  I'm worried about a big stack of waiting requests if I
% constantly poll the current index, but it could probably be tried easily
% enough.




% @@@@@@@@@@@;@@@@@@@@@@@
% tile number ;67
% tile mode ;Resonant Galvo
% tile zoom ;13.0446
% microns per pixel ;0.228737
% tracing algorithm ;NeuTube
% tile filename ;/local2/data/2016_11_03_Thu_14_02/2016_11_03_Thu_14_02/x_1029_y_-2809_ZSeries-11032016-0851-28741_Cycle00001_Ch2_000001.ome.tifCh2.v3draw
% tile event 0 ;2016_11_03_Thu_14_08_46_754
% tile event 1 ;2016_11_03_Thu_14_08_47_366
% tile event 2 ;2016_11_03_Thu_14_08_52_086
% tile event 3 ;2016_11_03_Thu_14_08_54_242
% elapsed time 0 ;0
% elapsed time 1 ;612
% elapsed time 2 ;4720
% elapsed time 3 ;2156
% @@  running totals ;    @@
% imaged area ;1.46162e+06
% sum of tile areas ;1.67613e+06
% boundingBoxX ;1850
% boundingBoxY ;3401
% boundingBox Area ;6.29185e+06
% tiles in queue ;0
% tiles imaged ;68
% total imaging time ;329499
% total analysis time ;245889
% ms since start ;370054
% average ms between tile starts ;5417.66
% z = 169 slices
% 
% 
% %%
% @@@@@@@@@@@;@@@@@@@@@@@
% tile number ;125
% tile mode ;Resonant Galvo
% tile zoom ;13.0446
% microns per pixel ;0.228737
% tracing algorithm ;NeuTube
% tile filename ;/local2/data/2016_11_08_Tue_13_12/2016_11_08_Tue_13_13/x_-2240_y_485_ZSeries-11082016-0946-29876_Cycle00001_Ch2_000001.ome.tifCh2.v3draw
% tile event 0 ;2016_11_08_Tue_13_26_27_068
% tile event 1 ;2016_11_08_Tue_13_26_29_597
% tile event 2 ;2016_11_08_Tue_13_26_35_514
% tile event 3 ;2016_11_08_Tue_13_26_39_047
% elapsed time 0 ;0
% elapsed time 1 ;2529
% elapsed time 2 ;5917
% elapsed time 3 ;3533
% @@  running totals ;    @@
% imaged area ;2.67096e+06
% sum of tile areas ;3.10577e+06
% boundingBoxX ;3823
% boundingBoxY ;3541
% boundingBox Area ;1.35372e+07
% tiles in queue ;0
% tiles imaged ;126
% total imaging time ;767682
% total analysis time ;529085
% ms since start ;790554
% average ms between tile starts ;6241.48
% @@@@@@@@@@@;@@@@@@@@@@@
% z = 251 slices
% 
% 
% 
% %%  
% %  this is the .5um z step scan:
% @@@@@@@@@@@;@@@@@@@@@@@
% tile number ;85
% tile mode ;Resonant Galvo
% tile zoom ;13.0446
% microns per pixel ;0.228737
% tracing algorithm ;NeuTube
% tile filename ;/local2/data/2016_11_08_Tue_15_36/2016_11_08_Tue_16_07/x_328_y_-3380_ZSeries-11082016-0946-30095_Cycle00001_Ch2_000001.ome.tifCh2.v3draw
% tile event 0 ;2016_11_08_Tue_16_23_21_156
% tile event 1 ;2016_11_08_Tue_16_23_21_385
% tile event 2 ;2016_11_08_Tue_16_23_31_435
% tile event 3 ;2016_11_08_Tue_16_23_38_155
% elapsed time 0 ;0
% elapsed time 1 ;229
% elapsed time 2 ;10050
% elapsed time 3 ;6720
% @@  running totals ;    @@
% imaged area ;1.84117e+06
% sum of tile areas ;2.11981e+06
% boundingBoxX ;2414
% boundingBoxY ;3400
% boundingBox Area ;8.2076e+06
% tiles in queue ;0
% tiles imaged ;86
% total imaging time ;867298
% total analysis time ;761183
% ms since start ;984383
% average ms between tile starts ;11381
% @@@@@@@@@@@;@@@@@@@@@@@
% also, the last file from 11/8 was collected at 0.5um step size instead of 1.0
% for that scan,  a max-scan stack 1730x1730 at zoom 1.15 with 0.235x0.235x0.5um voxels and 8x averaging takes 446s imaging and 342s Tif conversion = 788s.
% 86 tiles
% total imaging time ;867298
% total analysis time ;761183
% ms since start ;984383
% average ms between tile starts ;11381
% 984s
% 
% 2.74x max-scan area needed, so absolute scanning is something like 2161s,  or 2.2x faster for s2scan with perfect BB scan.  
% integer N tiles 2x2 ->  4x max scan  = 3152s  -> 3.2x faster 
% time/pixel:  
% 
% 
% z = 501 slices  