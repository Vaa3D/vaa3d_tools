%  Figure generation for smartscope 2 data
% based on scanAnalysisScripts, this script will be updated as changes are
% made to output figures, but figure generation should be accomplished
% simply by running this script. 
%   Sun April 10 2016

%  reviewing data again 4/11 further reduced the number of legitimate scans
%  (e.g. not crazy overtracing due to low threshold or mislocated starting
%  tiles due to xy stage movement after preview was taken.)


%  one or two adaptive tiles were also removed 4/12- they didnt' start on
%  the cell body or they only contained one tile and so would have been
%  skipped anyway below.

%Data analysis for smartscope 2 
% starting 2016.03.28

% 1. Data Entry



%   only a subset of data is worth analyzing for s2 performance.  
%  these data will be copied into a special directory, /local2/s2Data/  and
%  organized into directories for each cell imaged.

%  the post-PV files are stored on my workstation, including .xml files transfered
%  from the rig machine.
batchTopDirectory = '/local2/s2Data'
cd(fullfile(batchTopDirectory))

batch = dir(fullfile(batchTopDirectory,'cell*'))
tic


for i = 1:numel(batch)


   celliDir = dir(fullfile(batchTopDirectory, batch(i).name));

    celliDir = celliDir([celliDir(:).isdir]') ; % just get scan directories
    keepList = true(1,numel(celliDir));
   for j = 1:numel(celliDir)  %  all scans, even aborted ones
       if (sum(findstr(celliDir(j).name, '2016'))==0)  % not a normal scan folder
           keepList(j) = false;
       end
   end
   celliDir = celliDir(keepList);
    for j = 1:numel(celliDir)  %  all scans, even aborted ones    
        scanjDir = dir(fullfile(batchTopDirectory, batch(i).name, celliDir(j).name,'*.xml'));
        if numel(scanjDir)>0
            cellData{i}(j) = scanDataFromXMLDir(fullfile(batchTopDirectory, batch(i).name, celliDir(j).name));
            
        end
        
   end
   

end

% 914s for 25 cells
normalScans = numel(cellData)
% now add the adaptive and grid scans for comparison

%

adaptiveScans = dir(fullfile(batchTopDirectory, 'cell001','adaptive'));
for i = 1:numel(adaptiveScans)
    if findstr(adaptiveScans(i).name, '.')
                adaptiveScans(i).isdir = false;
        continue
        else
    end
adaptiveScans(i).fullDirPath = fullfile(batchTopDirectory, 'cell001','adaptive',adaptiveScans(i).name)
end


adaptiveScans2 = dir(fullfile(batchTopDirectory, 'cell002','adaptive'));
for i = 1:numel(adaptiveScans2)
    if findstr(adaptiveScans2(i).name, '.')
                adaptiveScans2(i).isdir = false;
        continue
    end
adaptiveScans2(i).fullDirPath = fullfile(batchTopDirectory, 'cell002','adaptive',adaptiveScans2(i).name)
end


adaptiveScans3 = dir(fullfile(batchTopDirectory, 'cell003','adaptive'));
for i = 1:numel(adaptiveScans3)
    if findstr(adaptiveScans3(i).name, '.')
                adaptiveScans3(i).isdir = false;
        continue
    end
adaptiveScans3(i).fullDirPath = fullfile(batchTopDirectory, 'cell003','adaptive',adaptiveScans3(i).name)
end



adaptiveScans = [adaptiveScans;adaptiveScans2; adaptiveScans3]

adaptiveScans = adaptiveScans([adaptiveScans(:).isdir]')


gridScans = dir(fullfile(batchTopDirectory, 'cell001', 'grid'));
for i = 1:numel(gridScans)
    if findstr(gridScans(i).name, '.')
        gridScans(i).isdir = false;
        continue
        
    end
gridScans(i).fullDirPath = fullfile(batchTopDirectory, 'cell001','grid',gridScans(i).name)
end

gridScans = gridScans([gridScans(:).isdir]')


gridScans2 = dir(fullfile(batchTopDirectory, 'cell003', 'grid'));
for i = 1:numel(gridScans2)
    if findstr(gridScans2(i).name, '.')
        gridScans2(i).isdir = false;
        continue
        
    end
gridScans2(i).fullDirPath = fullfile(batchTopDirectory, 'cell003','grid',gridScans2(i).name)
end

gridScans2 = gridScans2([gridScans2(:).isdir]')

otherScans = [adaptiveScans; gridScans]
nBeforeGrid = numel(adaptiveScans)

%




%

for i = 1:numel(otherScans)


            cellData{1+normalScans}(i) = scanDataFromXMLDir(fullfile(otherScans(i).fullDirPath));
       
  
   

end


aScanNN = normalScans+2
gScanNN = normalScans+3
toc
%% 2.  Analysis

% determine difference between summed tile volume and the actual scanned
% volume [estimate from (whole micron?) binary images] 



% for a set of tile locations, build a binary array to model the scan area:

for i = 1:numel(cellData)
    for j = 1:numel(cellData{i})
        
        tileSetij = cellData{i}(j).tileLocations;
        nTiles = numel(tileSetij)
        cellData{i}(j).ignore = false;
        
        
        if ( nTiles<3 )|| isempty(cellData{i}(j).folderName) || sum(strfind(cellData{i}(j).folderName, '.'))>0
            cellData{i}(j).ignore = true;
            continue
        else
            allLocations = cell2mat(tileSetij');
            [bigRectx, bigRecty] = meshgrid(floor(min(allLocations(:,1))):ceil(max(allLocations(:,3))),floor(min(allLocations(:,2))):ceil(max(allLocations(:,4))));
            bigRect = false(size(bigRectx));
            for k = 1:nTiles
                bigRect(:) = bigRect(:)| ((bigRectx(:)>allLocations(k,1)) &( bigRectx(:)<=allLocations(k,3) )& (bigRecty(:)>allLocations(k,2)) &( bigRecty(:)<=allLocations(k,4) ));
                
                
            end
            cd(fullfile(cellData{i}(j).folderName,'..'))
            dString = pwd;
            
            nnTry = str2double(dString(end-2:end))+1
            if isnan(nnTry)
                nnTry = normalScans+2+(j>nBeforeGrid) % extra number here because there's one number missing.  files should be organized better
            end
            cellData{i}(j).neuronNumber = nnTry;
 
            
            cellData{i}(j).boundingBoxArea = numel(bigRect);
            cellData{i}(j).imagedArea = sum(bigRect(:));
            cellData{i}(j).tileAreas = (allLocations(:,4)-allLocations(:,2)).*(allLocations(:,3)-allLocations(:,1));
            cellData{i}(j).totalTileArea = sum(cellData{i}(j).tileAreas);
            cellData{i}(j).extraScanning = cellData{i}(j).totalTileArea-cellData{i}(j).imagedArea;
            cellData{i}(j).boundingBoxSparsity = cellData{i}(j).totalTileArea/numel(bigRect);
            cellData{i}(j).lagTimes =  diff(cellData{i}(j).tileStartTimes)-cellData{i}(j).allTileTimes(1:end-1);
            %  early on, there were some extraneous delays due to
            %  instabilities of the code on Windows.  The result was an
            %  error message that popped up and stopped the scan unless it
            %  was clicked.  this can be seen in /local2/s2Data/cell003/grid/2016_04_15_Fri_11_13
            % where tiles ZSeries-04152016-0942-3972.xml and
            % ZSeries-04152016-0942-3971.xml differ by ~30 min, completely
            % screwing up the timing data.   
            %  also, if the .xml file of an overview tile gets stuck in this folder, all the results are off.
            % The following lines at least warn you
            checkLags = cellData{i}(j).lagTimes > 300;
            longLagLocs = find(checkLags);
            longLags = cellData{i}(j).lagTimes(checkLags);
            if sum(checkLags) >0
                'LAG TIME PROBLEM!!!!!!'
                'REVISING LAG TIMES!!!!'
                i
                j
                cellData{i}(j).folderName
                for jjj = 1:numel(longLagLocs)  % fix the lags
                    cellData{i}(j).lagTimes(longLagLocs(jjj)) = mean(cellData{i}(j).lagTimes(~checkLags));
                end
                % and fix the total time..
                
            end   
            cellData{i}(j).totalTime = cellData{i}(j).tileStartTimes(end)-cellData{i}(j).tileStartTimes(1)+cellData{i}(j).allTileTimes(end)+min(cellData{i}(j).lagTimes);
            cellData{i}(j).minTotalTime = sum(cellData{i}(j).allTileTimes(:)+min(cellData{i}(j).lagTimes));
            cellData{i}(j).minImagingOnly = sum(cellData{i}(j).allTileTimes(:));
            cellData{i}(j).estimatedMinLag = min( cellData{i}(j).lagTimes);
            cellData{i}(j).estimatedTimePerTileArea = mean((cellData{i}(j).allTileTimes(:)+cellData{i}(j).estimatedMinLag)./cellData{i}(j).tileAreas);
            cellData{i}(j).estimatedGridTime = numel(bigRect)*cellData{i}(j).estimatedTimePerTileArea;
            cellData{i}(j).micronsPerPixel = cellData{i}(j).allTileInfo{1}.micronsPerPixel(1);
            cellData{i}(j).imagingOnlyTimePerTileArea = mean((cellData{i}(j).allTileTimes(:))./cellData{i}(j).tileAreas);
            cellData{i}(j).boundingBoxImagingOnly = numel(bigRect)*cellData{i}(j).imagingOnlyTimePerTileArea;
            cellData{i}(j).zDepthVoxels = cellData{i}(j).allTileInfo{1}.tileDimensions(3);
        end
    end
end

cd(fullfile(batchTopDirectory))

% extract lag times = (difference between sequential tiles) - tiletime.
% this will include convert/load times but should also show initial big lag
% followed by minimal lags in continuous imaging mode. tough to extract necessary from unnecessary
% delays, though.



% extract 'extra' time (difference between tiletime*N and total time)

%  total time vs tile size for each neuron (N  = 3)

%  total volume vs tile size for each neuron (N = 3)


%%  OK, there are two data points that need to be removed-  the 7x7 grid scan time data is way wrong due to the
% Windows glitch where PV stops accepting new commands until a dialog box
% is clicked. the gaps in the xml dates is obvious in the 7x7 grid  AND in
% the largest tile scan of Neuron 4  (cell003 in the data).  BOTH of these
% datapoints are eliminated below:
%  

%cellData{27} = cellData{27}(1:5)
%cellData{4} = cellData{4}(1:2)

% there were a few other problems due to .xml files from previews getting
% stuck in the data directories, but those files were moved, so that
% shouldnt be a problem any longer

% this data was curated out of the s2data directory june 14 along with some
% failed experiments where imaging or unusual tracing errors resulted in missing a
% key process (or outright missing a stem from the soma) so that the
% resulting stitched images were very small


%% plotting
figure
neuronNumbers = []
neuronData={}
timeSummary = {}
neuronScale = {}
scanMode={}
for i = 1:numel(cellData)
    for j = 1:numel(cellData{i})
                          if isfield(cellData{i}(j),'neuronNumber')    
                nn  = cellData{i}(j).neuronNumber;
neuronNumbers = [neuronNumbers; nn];

                          end
    end
end


a = unique(neuronNumbers)
for ii = 1:numel(a)
    neuronData{a(ii)}=[0,0,0,0,0,0,0,0,0]
                timeSummary{a(ii)} = [0,0,0,0,0]
                neuronScale{a(ii)} = [];
                scanMode{a(ii)}= []
end

myCmap = colormap(jet(numel(a)+1));
for i = 1:numel(cellData)
    for j = 1:numel(cellData{i})
        
        tileSetij = cellData{i}(j).tileLocations;
        nTiles = numel(tileSetij);
        if cellData{i}(j).ignore
            continue
        else
            if isfield(cellData{i}(j),'neuronNumber')
                nn  = cellData{i}(j).neuronNumber;
                
                neuronData{nn} = [neuronData{nn} ; [mean(sqrt(cellData{i}(j).tileAreas)),cellData{i}(j).imagedArea,mean(sqrt(cellData{i}(j).tileAreas)),cellData{i}(j).totalTime, cellData{i}(j).totalTileArea, cellData{i}(j).boundingBoxArea,i,j,cellData{i}(j).zDepthVoxels]]
                
                timeSummary{nn}= [timeSummary{nn}; [[cellData{i}(j).estimatedGridTime,cellData{i}(j).minTotalTime , cellData{i}(j).totalTime]/( mean((cellData{i}(j).allTileTimes(:)+cellData{i}(j).estimatedMinLag))),cellData{i}(j).estimatedGridTime,cellData{i}(j).boundingBoxImagingOnly]];
                neuronScale{nn} = [neuronScale{nn}; cellData{i}(j).micronsPerPixel];
                
                if cellData{i}(j).isGridScan
                    thisMode = 2;
                elseif cellData{i}(j).isAdaptive
                    thisMode = 1;
                else
                    thisMode = 0;
                end
                scanMode{nn} = [scanMode{nn}; thisMode];
                %                     subplot(2,1,1), hold all, plot(mean(sqrt(cellData{i}(j).tileAreas)),cellData{i}(j).imagedArea,'o-', 'DisplayName', cellData{i}(j).folderName, 'color', myCmap(nn,:))
                %                     subplot(2,1,2), hold all, plot(mean(sqrt(cellData{i}(j).tileAreas)),cellData{i}(j).totalTime, '*-', 'DisplayName', cellData{i}(j).folderName, 'color', myCmap(nn,:))
                %                     plot(mean(sqrt(cellData{i}(j).tileAreas)),cellData{i}(j).minTotalTime, '*-','color', myCmap(nn,:))
            end
            
        end
    end
end
%
a = unique(neuronNumbers)
for ii = 1:numel(a)
neuronData{a(ii)} = neuronData{a(ii)}(neuronData{a(ii)}(:,1)~=0,:)
[rows, cs] = size(neuronData{a(ii)});
rs= 1:rows;
%if a(ii)~=aScanNN
[ neuronData{a(ii)}, rs] = sortrows(neuronData{a(ii)},1)
%end

timeSummary{a(ii)} = timeSummary{a(ii)}(timeSummary{a(ii)}(:,1)~=0,:);
timeSummary{a(ii)} = timeSummary{a(ii)}(rs,:);


scanMode{a(ii)} = scanMode{a(ii)}(rs,:);

% if there are multiple scans at the same tile size, I'll plot the mean and
% min-max as errorbars.
xVals = unique(neuronData{a(ii)}(:,1))
xToPlot = xVals
y1ToPlot = zeros(size(xToPlot))
yErrorU1 = y1ToPlot
yErrorL1 = yErrorU1
yErrorU2= yErrorU1
yErrorL2= yErrorU2
y2ToPlot = yErrorU1
yErrorU3 = y1ToPlot
yErrorL3 = yErrorU1
yErrorU3= yErrorU1
yErrorL3= yErrorU2
y3ToPlot = yErrorU1
yErrorU4 = y1ToPlot
yErrorL4 = yErrorU1
yErrorU4= yErrorU1
yErrorL4= yErrorU2
y4ToPlot = yErrorU1
for jj = 1:numel(xVals)
    y1ToPlot(jj)   = mean(neuronData{a(ii)}(neuronData{a(ii)}(:,1)==xVals(jj),2))
    yMax1 = max(neuronData{a(ii)}(neuronData{a(ii)}(:,1)==xVals(jj),2));
    yMin1 = min(neuronData{a(ii)}(neuronData{a(ii)}(:,1)==xVals(jj),2));
    yErrorU1(jj) = yMax1-y1ToPlot(jj)
    yErrorL1(jj) = y1ToPlot(jj)-yMin1
    
    y2ToPlot(jj)   = mean(neuronData{a(ii)}(neuronData{a(ii)}(:,1)==xVals(jj),4))
    yMax2 = max(neuronData{a(ii)}(neuronData{a(ii)}(:,1)==xVals(jj),4));
    yMin2 = min(neuronData{a(ii)}(neuronData{a(ii)}(:,1)==xVals(jj),4));
    yErrorU2(jj) = yMax2-y2ToPlot(jj)
    yErrorL2(jj) = y2ToPlot(jj)-yMin2 
    
    y3ToPlot(jj)   = mean(timeSummary{a(ii)}(neuronData{a(ii)}(:,1)==xVals(jj),3)./timeSummary{a(ii)}(neuronData{a(ii)}(:,1)==xVals(jj),1))
    yMax3 = max(timeSummary{a(ii)}(neuronData{a(ii)}(:,1)==xVals(jj),3)./timeSummary{a(ii)}(neuronData{a(ii)}(:,1)==xVals(jj),1));
    yMin3 = min(timeSummary{a(ii)}(neuronData{a(ii)}(:,1)==xVals(jj),3)./timeSummary{a(ii)}(neuronData{a(ii)}(:,1)==xVals(jj),1));
    yErrorU3(jj) = yMax3-y3ToPlot(jj)
    yErrorL3(jj) = y3ToPlot(jj)-yMin3 
    
        y4ToPlot(jj)   = mean(timeSummary{a(ii)}(neuronData{a(ii)}(:,1)==xVals(jj),3)./timeSummary{a(ii)}(neuronData{a(ii)}(:,1)==xVals(jj),2)-1)
    yMax4 = max(timeSummary{a(ii)}(neuronData{a(ii)}(:,1)==xVals(jj),3)./timeSummary{a(ii)}(neuronData{a(ii)}(:,1)==xVals(jj),2)-1);
    yMin4 = min(timeSummary{a(ii)}(neuronData{a(ii)}(:,1)==xVals(jj),3)./timeSummary{a(ii)}(neuronData{a(ii)}(:,1)==xVals(jj),2)-1);
    yErrorU4(jj) = yMax4-y4ToPlot(jj)
    yErrorL4(jj) = y4ToPlot(jj)-yMin4 
end

subplot(4,1,1),  hold all,   plot(neuronData{a(ii)}(:,1),neuronData{a(ii)}(:,2), '*-','color', myCmap(ii,:),'DisplayName', cellData{neuronData{a(ii)}(end,7)}(neuronData{a(ii)}(end,8)).folderName);
errorbar(xToPlot,y1ToPlot, yErrorL1, yErrorU1,'color', myCmap(ii,:))
bip
xlim([0,450])
xlabel('tile size (pixels)')
ylabel('imaged area (pixels^2)')
subplot(4,1,2),    hold all, plot(neuronData{a(ii)}(:,3),neuronData{a(ii)}(:,4), '*-','color', myCmap(ii,:),'DisplayName',cellData{neuronData{a(ii)}(end,7)}(neuronData{a(ii)}(end,8)).folderName);
errorbar(xToPlot,y2ToPlot, yErrorL2, yErrorU2,'color', myCmap(ii,:))
bip
xlim([0,450])
xlabel('tile size (pixels)')
ylabel('imaging time (s)')

subplot(4,1,3) ,   hold all, plot(neuronData{a(ii)}(:,1), timeSummary{a(ii)}(:,3)./timeSummary{a(ii)}(:,1),'*-','color', myCmap(ii,:))
errorbar(xToPlot,y3ToPlot, yErrorL3, yErrorU3,'color', myCmap(ii,:))
 bip
 xlim([0,450])
xlabel('tile size (pixels)')
ylabel('normalized imaging time')



subplot(4,1,4), hold all, plot(neuronData{a(ii)}(:,1), timeSummary{a(ii)}(:,3)./timeSummary{a(ii)}(:,2)-1,'*-','color', myCmap(ii,:))
errorbar(xToPlot,y4ToPlot, yErrorL4, yErrorU4,'color', myCmap(ii,:))

bip
xlim([0,450])
xlabel('tile size (pixels)')
ylabel('normalized analysis time')
end
subplot(4,1,3) ,  plot([0 450], [1 1],'k')
xlim([0,450])

%subplot(2,2,3) ,   hold all, plot(neuronData{a(ii)}(:,5),neuronData{a(ii)}(:,6), '*-','color', myCmap(ii,:),'DisplayName',cellData{neuronData{a(ii)}(end,7)}(neuronData{a(ii)}(end,8)).folderName);
%subplot(2,2,4) ,   hold all, plot(neuronData{a(ii)}(:,2),neuronData{a(ii)}(:,6), '*-','color', myCmap(ii,:),'DisplayName',cellData{neuronData{a(ii)}(end,7)}(neuronData{a(ii)}(end,8)).folderName);

%%  now repeat with microns on the x axis.  and y axis where applicable
figure
for ii = 1:numel(a)

%xVals = unique(neuronData{a(ii)}(:,1))
[xVals, indx] = unique(neuronData{a(ii)}(:,1).*neuronScale{a(ii)})
scaleToUse = neuronScale{a(ii)}(indx);
y1ToPlot = zeros(size(xVals))
yErrorU1 = y1ToPlot;
yErrorL1 = yErrorU1;
yErrorU2= yErrorU1;
yErrorL2= yErrorU2;
y2ToPlot = yErrorU1;
yErrorU3 = y1ToPlot;
yErrorL3 = yErrorU1;
yErrorU3= yErrorU1;
yErrorL3= yErrorU2;
y3ToPlot = yErrorU1;
yErrorU4 = y1ToPlot;
yErrorL4 = yErrorU1;
yErrorU4= yErrorU1;
yErrorL4= yErrorU2;
y4ToPlot = yErrorU1;
for jj = 1:numel(xVals)
    y1ToPlot(jj)   = mean(neuronData{a(ii)}(neuronData{a(ii)}(:,1).*neuronScale{a(ii)}==xVals(jj),2));
    yMax1 = max(neuronData{a(ii)}(neuronData{a(ii)}(:,1).*neuronScale{a(ii)}==xVals(jj),2));
    yMin1 = min(neuronData{a(ii)}(neuronData{a(ii)}(:,1).*neuronScale{a(ii)}==xVals(jj),2));
    yErrorU1(jj) = yMax1-y1ToPlot(jj);
    yErrorL1(jj) = y1ToPlot(jj)-yMin1;
    
    y2ToPlot(jj)   = mean(neuronData{a(ii)}(neuronData{a(ii)}(:,1).*neuronScale{a(ii)}==xVals(jj),4));
    yMax2 = max(neuronData{a(ii)}(neuronData{a(ii)}(:,1).*neuronScale{a(ii)}==xVals(jj),4));
    yMin2 = min(neuronData{a(ii)}(neuronData{a(ii)}(:,1).*neuronScale{a(ii)}==xVals(jj),4));
    yErrorU2(jj) = yMax2-y2ToPlot(jj);
    yErrorL2(jj) = y2ToPlot(jj)-yMin2 ;
    
    y3ToPlot(jj)   = mean(timeSummary{a(ii)}(neuronData{a(ii)}(:,1).*neuronScale{a(ii)}==xVals(jj),3)./timeSummary{a(ii)}(neuronData{a(ii)}(:,1).*neuronScale{a(ii)}==xVals(jj),1));
    yMax3 = max(timeSummary{a(ii)}(neuronData{a(ii)}(:,1).*neuronScale{a(ii)}==xVals(jj),3)./timeSummary{a(ii)}(neuronData{a(ii)}(:,1).*neuronScale{a(ii)}==xVals(jj),1));
    yMin3 = min(timeSummary{a(ii)}(neuronData{a(ii)}(:,1).*neuronScale{a(ii)}==xVals(jj),3)./timeSummary{a(ii)}(neuronData{a(ii)}(:,1).*neuronScale{a(ii)}==xVals(jj),1));
    yErrorU3(jj) = yMax3-y3ToPlot(jj);
    yErrorL3(jj) = y3ToPlot(jj)-yMin3 ;
    
        y4ToPlot(jj)   = mean(timeSummary{a(ii)}(neuronData{a(ii)}(:,1).*neuronScale{a(ii)}==xVals(jj),3)./timeSummary{a(ii)}(neuronData{a(ii)}(:,1).*neuronScale{a(ii)}==xVals(jj),2)-1);
    yMax4 = max(timeSummary{a(ii)}(neuronData{a(ii)}(:,1).*neuronScale{a(ii)}==xVals(jj),3)./timeSummary{a(ii)}(neuronData{a(ii)}(:,1).*neuronScale{a(ii)}==xVals(jj),2)-1);
    yMin4 = min(timeSummary{a(ii)}(neuronData{a(ii)}(:,1).*neuronScale{a(ii)}==xVals(jj),3)./timeSummary{a(ii)}(neuronData{a(ii)}(:,1).*neuronScale{a(ii)}==xVals(jj),2)-1);
    yErrorU4(jj) = yMax4-y4ToPlot(jj);
    yErrorL4(jj) = y4ToPlot(jj)-yMin4 ;
end

subplot(4,1,1),  hold all,   plot(neuronData{a(ii)}(:,1).*neuronScale{a(ii)},neuronData{a(ii)}(:,2).*neuronScale{a(ii)}.^2, '*-','color', myCmap(ii,:),'DisplayName', cellData{neuronData{a(ii)}(end,7)}(neuronData{a(ii)}(end,8)).folderName);
errorbar(xVals,y1ToPlot.*scaleToUse.^2, yErrorL1.*scaleToUse.^2, yErrorU1.*scaleToUse.^2,'color', myCmap(ii,:))
bip
xlim([0,120])
xlabel('tile size (microns)')
ylabel('imaged area (microns^2)')
subplot(4,1,2),    hold all, plot(neuronData{a(ii)}(:,3).*neuronScale{a(ii)},neuronData{a(ii)}(:,4), '*-','color', myCmap(ii,:),'DisplayName',cellData{neuronData{a(ii)}(end,7)}(neuronData{a(ii)}(end,8)).folderName);
errorbar(xVals,y2ToPlot, yErrorL2, yErrorU2,'color', myCmap(ii,:))
bip
xlim([0,120])
xlabel('tile size (microns)')
ylabel('imaging time (s)')

subplot(4,1,3) ,   hold all, plot(neuronData{a(ii)}(:,1).*neuronScale{a(ii)}, timeSummary{a(ii)}(:,3)./timeSummary{a(ii)}(:,1),'*-','color', myCmap(ii,:))
errorbar(xVals,y3ToPlot, yErrorL3, yErrorU3,'color', myCmap(ii,:))

 bip
 xlim([0,120])
xlabel('tile size (microns)')
ylabel('normalized imaging time')
subplot(4,1,4), hold all, plot(neuronData{a(ii)}(:,1).*neuronScale{a(ii)}, timeSummary{a(ii)}(:,3)./timeSummary{a(ii)}(:,2)-1,'*-','color', myCmap(ii,:))
errorbar(xVals,y4ToPlot, yErrorL4, yErrorU4,'color', myCmap(ii,:))

bip
xlim([0,120])
xlabel('tile size (microns)')
ylabel('normalized analysis time')
end
subplot(4,1,3) ,  plot([0 120], [1 1],'k')
xlim([0,120])
bip
%subplot(2,2,3) ,   hold all, plot(neuronData{a(ii)}(:,5),neuronData{a(ii)}(:,6), '*-','color', myCmap(ii,:),'DisplayName',cellData{neuronData{a(ii)}(end,7)}(neuronData{a(ii)}(end,8)).folderName);
%subplot(2,2,4) ,   hold all, plot(neuronData{a(ii)}(:,2),neuronData{a(ii)}(:,6), '*-','color', myCmap(ii,:),'DisplayName',cellData{neuronData{a(ii)}(end,7)}(neuronData{a(ii)}(end,8)).folderName);
%% 2016.06.07 now organize everything by scan mode field

allData = -1*ones(1,18)
for i = 1:numel(a)
    allData = [allData; [neuronData{a(i)}, neuronData{a(i)}(:,1).*neuronScale{a(i)},neuronData{a(i)}(:,2).*neuronScale{a(i)}.*neuronScale{a(ii)},neuronData{a(i)}(:,6).*neuronScale{a(i)}.*neuronScale{a(ii)}, timeSummary{a(i)},scanMode{a(i)}]];
end

%[mean(sqrt(cellData{i}(j).tileAreas)),cellData{i}(j).imagedArea,mean(sqrt(cellData{i}(j).tileAreas)),cellData{i}(j).totalTime,
%                                                                         cellData{i}(j).totalTileArea,
%                                                                         cellData{i}(j).boundingBoxArea,i,j,
%                                                                         cellData{i}(j).zDepthVoxels
%                                                                          
%    micron versions of columns 1, 2, and 6
%  cellData{i}(j).estimatedGridTime/( mean((cellData{i}(j).allTileTimes(:)+cellData{i}(j).estimatedMinLag))),
%  cellData{i}(j).minTotalTime /( mean((cellData{i}(j).allTileTimes(:)+cellData{i}(j).estimatedMinLag))), 
%  cellData{i}(j).totalTime]/( mean((cellData{i}(j).allTileTimes(:)+cellData{i}(j).estimatedMinLag))),
% cellData{i}(j).estimatedGridTime
%  cellData{i}(j).imagingOnlyGridTime 

% scanMode ;

%                                                                         
                


s2 = allData(allData(:,end)==0,:)
s2A= allData(allData(:,end)==1,:)
s2G = allData(allData(:,end)==2,:)
%% then manually plot over with the s2A scans as grey circles:

subplot(4,1,1)
hold all, plot(s2A(:,10), s2A(:,11),'o', 'markersize',10, 'color', [.5,.5,.5])

subplot(4,1,2)
hold all, plot(s2A(:,10), s2A(:,4),'o', 'markersize',10, 'color', [.5,.5,.5])
subplot(4,1,3)
hold all, plot(s2A(:,10), s2A(:,15)./s2A(:,13),'o', 'markersize',10, 'color', [.5,.5,.5])
subplot(4,1,4)
hold all, plot(s2A(:,10), s2A(:,15)./s2A(:,14)-1,'o', 'markersize',10, 'color', [.5,.5,.5])


%%  for Hanchuan,simplified matrices with all scan data:


% 
s2PHC = s2(:,[1, 2, 5, 6, 10,11,12, 16,17, 7,8, 9])
% |1: average tile side|2: imaged area (no doublecounting) |3: total tile area (incl doublecounting) |4: bounding box area |5: average tile side (microns) 
% |6: imaged area (microns) |7: bounding box area (microns) |8:
% estimatedGridTime |9: imagingOnlyGridTime|10: ignore|11: ignore|12:
% zDepthVoxels
% 
%  "estimatedGridTime" includes the time for .tif conversion, (which can  be comparable to imaging times), and does not include any image overlap.  
% "imagingOnlyGridTime" would be if there  is no time for .tif conversion.
%
%  
s2APHC = s2A(:,[1, 2, 5, 6, 10,11,12, 16,17, 7,8,9])



%  add string with file name.
[r,c] = size(s2PHC);
for i = 1:r
s2PHCNames{i} = cellData{s2PHC(i,10)}(s2PHC(i,11)).folderName;
end

[r,c] = size(s2APHC);
for i = 1:r
s2APHCNames{i} = cellData{s2APHC(i,10)}(s2APHC(i,11)).folderName;
end




%% sorted versions:

[s2APHCSorted, rowsA] = sortrows(s2APHC,4)
[s2PHCSorted, rows] = sortrows(s2PHC,4)


% now the lowest, middle and highest boundingbox areas for adaptive scans
lowestAName = s2APHCNames{rowsA(1)}
middleAName = s2APHCNames{rowsA(10)}
highestAName = s2APHCNames{rowsA(end)}

% now the lowest, middle and highest boundingbox areas for regular scans
lowestName = s2PHCNames{rows(1)}
middleName = s2PHCNames{rows(15)}
highestName = s2PHCNames{rows(end)}






%%
mean(s2(:,14)./s2(:,12))  % normalized imaging times
std(s2(:,14)./s2(:,12))

mean(s2A(:,14)./s2A(:,12))
std(s2A(:,14)./s2A(:,12))

mean(s2G(:,14)./s2G(:,12))
std(s2G(:,14)./s2G(:,12))

mean(s2(:,14)./s2(:,13))-1 % percent of tile time devoted to analysis
std(s2(:,14)./s2(:,13))

mean(s2A(:,14)./s2A(:,13))-1
std(s2A(:,14)./s2A(:,13))

mean(s2G(:,14)./s2G(:,13))-1
std(s2G(:,14)./s2G(:,13))

%%  and new plots based on the scan mode tag


%  distribution or histogram or violin of scanned / minGrid  for s2 and s2A

[h,v] = hist(s2(:,10))
figure, plot(v,h)
[h2, v] = hist(s2A(:,10),v)
hold all, plot(v,h2)
%  same for  analysistime/mingrid-1

% do complete fractal dimension analysis of grid scans vs micron tile size



%%  bar plots with errorbars showing area and time 
figure
subplot(2,1,1)
errorbar([mean(s2(:,10)./s2(:,11)),mean(s2A(:,10)./s2A(:,11)) ],[std(s2(:,10)./s2(:,11)), std(s2A(:,10)./s2A(:,11))])
hold all
bar([1,2], [mean(s2(:,10)./s2(:,11)),mean(s2A(:,10)./s2A(:,11)) ])
text(.6,.9,['S2: ',num2str(mean(s2(:,10)./s2(:,11))), ' \pm ', num2str(std(s2(:,10)./s2(:,11))), ' N = ', num2str(numel(s2(:,1))) ])
text(1.6,.9,['S2Adaptive: ',num2str(mean(s2A(:,10)./s2A(:,11))), ' \pm ', num2str(std(s2A(:,10)./s2A(:,11))), ' N = ', num2str(numel(s2A(:,1))) ])
title('S2 Scan Area Reduction')
ylim([0,1])
xlim([0.5, 2.5])

bip



subplot(2,1,2)
errorbar([mean(s2(:,14)./s2(:,12)),mean(s2A(:,14)./s2A(:,12)) ],[std(s2(:,14)./s2(:,12)), std(s2A(:,14)./s2A(:,12))])
hold all
bar([1,2], [mean(s2(:,14)./s2(:,12)),mean(s2A(:,14)./s2A(:,12)) ])
text(.6,.9,['S2: ',num2str(mean(s2(:,14)./s2(:,12))), ' \pm ', num2str(std(s2(:,14)./s2(:,12))), ' N = ', num2str(numel(s2(:,1))) ])
text(1.6,.9,['S2Adaptive: ',num2str(mean(s2A(:,14)./s2A(:,12))), ' \pm ', num2str(std(s2A(:,14)./s2A(:,12))), ' N = ', num2str(numel(s2A(:,1))) ])
title('S2 Imaging Time Reduction')
ylim([0,1])
xlim([0.5, 2.5])

bip
%  and of course the requisite 'fold' version
figure
subplot(2,1,1)
errorbar([mean(s2(:,11)./s2(:,10)),mean(s2A(:,11)./s2A(:,10)) ],[std(s2(:,11)./s2(:,10)), std(s2A(:,11)./s2A(:,10))])
hold all
bar([1,2], [mean(s2(:,11)./s2(:,10)),mean(s2A(:,11)./s2A(:,10)) ])
text(.6,6,['S2: ',num2str(mean(s2(:,11)./s2(:,10))), ' \pm ', num2str(std(s2(:,11)./s2(:,10))), ' N = ', num2str(numel(s2(:,1))) ])
text(1.6,6,['S2Adaptive: ',num2str(mean(s2A(:,11)./s2A(:,10))), ' \pm ', num2str(std(s2A(:,11)./s2A(:,10))), ' N = ', num2str(numel(s2A(:,1))) ])
title('S2 Scan Area Improvement')
ylim([0,7])
xlim([0.5, 2.5])

bip



subplot(2,1,2)
errorbar([mean(s2(:,12)./s2(:,14)),mean(s2A(:,12)./s2A(:,14)) ],[std(s2(:,12)./s2(:,14)), std(s2A(:,12)./s2A(:,14))])
hold all
bar([1,2], [mean(s2(:,12)./s2(:,14)),mean(s2A(:,12)./s2A(:,14)) ])
text(.6,7,['S2: ',num2str(mean(s2(:,12)./s2(:,14))), ' \pm ', num2str(std(s2(:,12)./s2(:,14))), ' N = ', num2str(numel(s2(:,1))) ])
text(1.6,7,['S2Adaptive: ',num2str(mean(s2A(:,12)./s2A(:,14))), ' \pm ', num2str(std(s2A(:,12)./s2A(:,14))), ' N = ', num2str(numel(s2A(:,1))) ])
title('S2 Imaging Time Improvement')
xlim([0.5, 2.5])
ylim([0,8])

bip


figure, 
plot(s2A(:,11), s2A(:,10),'.')
hold all,
plot(s2(:,11), s2(:,10),'.')
 plot([0, max(s2A(:,11))], [0, max(s2A(:,11))])
xlabel('minimal scan area bounding box (\mum^2)')
ylabel('actual scanned area (\mum^2)')
bip
figure, 
plot(s2A(:,12), s2A(:,14),'.')
hold all,
plot(s2(:,12), s2(:,14),'.')

 plot([0, max(s2A(:,12))], [0, max(s2A(:,12))])
xlabel('estimated minimal imaging time (average tile time)')
ylabel('actual imaging time (average tile time)')
bip

%%  scaling analysis

figure,
subplot(2,1,1)
plot(log10(s2(:,9)),log10(s2(:,10)),'.')
subplot(2,1,2)
plot((s2(:,9)),(s2(:,10)),'.')


%% other calculations:


% how much additional time is contributed by analysis?  

%  timeSummary 

smartScanTimes = [timeSummary{1};timeSummary{2};timeSummary{3};timeSummary{4}]
adaptiveScanTimes = timeSummary{aScanNN}
gridScanTimes5 = [timeSummary{gScanNN}]


ssAnalysisTimes = smartScanTimes(:,3)./smartScanTimes(:,2)-1
mean(ssAnalysisTimes)
std(ssAnalysisTimes)

ssScanTimes = smartScanTimes(:,3)./smartScanTimes(:,1)
mean(ssScanTimes)
std(ssScanTimes)

aScanTimes = adaptiveScanTimes(:,3)./adaptiveScanTimes(:,1)
mean(aScanTimes)
std(aScanTimes)

ssND = [neuronData{1}; neuronData{2}; neuronData{3}; neuronData{4}]
aND = neuronData{aScanNN};
gND = neuronData{gScanNN};
ssTotalVolume = ssND(:,2);
aTotalVolume = aND(:,2);
gTotalVolume5 = gND(1,2);
gTotalVolume7 = gND(2,2);
mean(ssTotalVolume)
std(ssTotalVolume)
mean(aTotalVolume)
std(aTotalVolume)

ssTotalTime = ssND(:,4)
gTotalTime = gND(:,4)
aTotalTime = aND(:,4)
g5nD = neuronData{gScanNN}(1,:)


mean(ssTotalTime)
std(ssTotalTime)
gTotalTime
gTotalTime(2)/mean(aTotalTime)
gTotalVolume7/mean(aTotalVolume)
mean(gTotalVolume7)/mean(ssTotalVolume)
mean(gTotalVolume5)/mean(aTotalVolume)
mean(gTotalVolume7)/mean(aTotalVolume)
mean(gTotalTime)/mean(aTotalTime)




%%  notes and comments.

% 2016.04.27  figure work
% in illustrator I modified colors and plot labels (now cell0000 is labeled
% as neuron 1, etc)  and linked the adaptive plot symbols to the
% corresponding neuron plot color. I also eliminated the grid plots for the
% normalized plots -the 7x7 values are way out of whack probably because of
% a prairieview error or something.  

%%


%  cell000 neuronNumber = 1
%  2016_03_27_Sun_09_45  background was too high. but the tile size is the
%  same as 9_46.  for some reason some preview files got put in the data
%  directory, causing an overestimation of the tile size (which is taken
%  from the average of the tile sizes generated from the xml files)  9_45
%  and the problem tiles in 9_46 were removed to ./other/


% cell001 neuronNumber = 8

% cell002 neuronNumber = 9





% FORMER NAMES! stored in /data
% cell001  neuronNumber = 3 removed looks like crap. MOST tracing, discontinuous tile boundaries,
% poor labeling.

% cell002 (neuronNumber = 4)  THESE ARE ALL TEST SCANS OF NOISE




%%   notes and comments



%  A. /local2/2016_03_27_Sun_09_00/2016_03_27_Sun_10_18  ran away and kept
%  imaging forever.  initial threshold was 10,  189 slices 
% B. /local2/2016_03_27_Sun_09_00/2016_03_27_Sun_10_50   threshold was 15,  169 slices 
%  C. /local2/2016_03_27_Sun_09_00/2016_03_27_Sun_10_55   threshold was 10.  169 slices 
% imaging power was the same for all, but clear drop in fluorescence from
% A. to B.

%10_12 power 100
%10_14 power 110
%10_18-10_55 power 130

%  I loaded the center tile of all 5 scans of this neuron into FIJI and
%  generated the mean value of each slice. those data were loaded into
%  matlab as matrices 

% % plot the peak values of each:
% 
% peakData = [max(mean4022(:,2)), max(mean4026(:,2)),max(mean4036(:,2)),max(mean4159(:,2)),max(mean4170(:,2))]
% 
% backgroundData = [max(mean4022(1:50,2)), max(mean4026(1:50,2)),max(mean4036(1:50,2)),max(mean4159(1:50,2)),max(mean4170(1:50,2))]
% 
% plot(peakData), hold all, plot(backgroundData)
% 
% 
% %  very clear from looking at the data again that tile-to-tile stitching is
%  incorrect. specifically, it looks like the left and right edges of the
%  tiles are not correctly located in space.  we think we have 10-15%
%  overlap, but the image data is NOT actually overlapped.  since the field
%  size is taken directly from PV, it's hard to see how this can be
%  happening.  either the field size is wrong or the distance metric varies
%  across the field along the fastscan axis.  

%