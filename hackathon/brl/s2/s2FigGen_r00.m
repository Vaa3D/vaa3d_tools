%  s2FigGen_r00
%  revised figure generation after re-reimaging and subsequent data
%  curation.
%  current data set from 50um sections is 20 neurons as of July 5, 2016.
%  numbering scheme is in the data directories, so there are skipped
%  'numbers'.

%  main dataset (cell027 - cell048, omitting 42, 40)  includes several
%  neurons that were imaged twice. in the first run-through, these
%  different imaging conditions will be shown together.  actual figure
%  generation and statistics will be generated only from most current
%  scans because they were all imaged under the same conditions.





%Figure generation for smartscope 2 data
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

batch = dir(fullfile(batchTopDirectory,'cell0*'))
tic


for i = 1:numel(batch)


   celliDir = dir(fullfile(batchTopDirectory, batch(i).name));

    celliDir = celliDir([celliDir(:).isdir]') ; % just get scan directories
    keepList = true(1,numel(celliDir));
   for j = 1:numel(celliDir)  %  all scans, even aborted ones
       if (sum(findstr(celliDir(j).name, '2016_'))==0)  % not a normal scan folder
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
% 2.  Analysis

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
            lowerx = floor(min(allLocations(:,1)));
            upperx = ceil(max(allLocations(:,3)));
            lowery =floor(min(allLocations(:,2)));
            uppery = ceil(max(allLocations(:,4)));
            
            if (upperx-lowerx) < (uppery-lowery)
                upperxS = lowerx+uppery-lowery;
                upperyS = uppery;
            else
                upperxS = upperx;
                upperyS = lowery+upperx-lowerx;
            end
            
            [bigRectx, bigRecty] = meshgrid(lowerx:upperx,lowery:uppery);
            bigRect = false(size(bigRectx));
            for k = 1:nTiles
                bigRect(:) = bigRect(:)| ((bigRectx(:)>allLocations(k,1)) &( bigRectx(:)<=allLocations(k,3) )& (bigRecty(:)>allLocations(k,2)) &( bigRecty(:)<=allLocations(k,4) ));
                
                
            end
            
               [bigRectxS, bigRectyS] = meshgrid(lowerx:upperxS,lowery:upperyS);
            bigRectS = false(size(bigRectxS));
            for k = 1:nTiles
                bigRectS(:) = bigRectS(:)| ((bigRectxS(:)>allLocations(k,1)) &( bigRectxS(:)<=allLocations(k,3) )& (bigRectyS(:)>allLocations(k,2)) &( bigRectyS(:)<=allLocations(k,4) ));
                
                
            end         
            
            
            cd(fullfile(cellData{i}(j).folderName,'..'))
            dString = pwd;
            
            nnTry = str2double(dString(end-2:end))+1
            if isnan(nnTry)
                nnTry = normalScans+2+(j>nBeforeGrid) % extra number here because there's one number missing.  files should be organized better
            end
            cellData{i}(j).neuronNumber = nnTry;
 
            
            cellData{i}(j).boundingBoxArea = numel(bigRect);
            cellData{i}(j).boundingBoxAreaSquare = numel(bigRectS);
            cellData{i}(j).imagedArea = sum(bigRect(:));
            cellData{i}(j).tileAreas = (allLocations(:,4)-allLocations(:,2)).*(allLocations(:,3)-allLocations(:,1));
            cellData{i}(j).totalTileArea = sum(cellData{i}(j).tileAreas);
            cellData{i}(j).extraScanning = cellData{i}(j).totalTileArea-cellData{i}(j).imagedArea;
            cellData{i}(j).boundingBoxSparsity = cellData{i}(j).totalTileArea/numel(bigRect);
            cellData{i}(j).boundingBoxSparsityS = cellData{i}(j).totalTileArea/numel(bigRectS);
            
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
            
            
        if       cellData{i}(j).neuronNumber >= 28  % add BB scans for most recent dataset.

            xmlFile = dir(fullfile(pwd, 'ZSeries*'));
            if numel(xmlFile)>0
            if xmlFile(1).isdir
                xmlDir = xmlFile(1);
            else
                xmlDir.name = '';
            end
            cellData{i}(j).BBdata = scanDataFromXMLDir(fullfile(batchTopDirectory, batch(i).name, xmlDir.name))
            % now check the .tif conversion time using the timestamps on
            % the .tif files in the original directory.  the latest time
            % on a .tif file - the earliest time stamp on a .tif file gives a
            % very good estimate of the total .tif conversion time.
            zSeriesName = dir(fullfile(fileparts(cellData{i}(1).folderName),'ZSeries*'))
            [ig, cellData{i}(j).BBName, nore] = fileparts(zSeriesName(1).name)
            oDataDir = dir(fullfile('/local3/testNAS/data000/', cellData{i}(j).BBName, '*.tif'));
            cellData{i}(j).BBTiffTime = 24*60*60*(max([oDataDir(:).datenum]')-min([oDataDir(:).datenum]'));
            end
            
        end
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
    neuronData{a(ii)}=[0,0,0,0,0,0,0,0,0,0]
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
                
                neuronData{nn} = [neuronData{nn} ; [mean(sqrt(cellData{i}(j).tileAreas)),cellData{i}(j).imagedArea,mean(sqrt(cellData{i}(j).tileAreas)),cellData{i}(j).totalTime, cellData{i}(j).totalTileArea, cellData{i}(j).boundingBoxArea,i,j,cellData{i}(j).zDepthVoxels, cellData{i}(j).boundingBoxAreaSquare]]
                
                timeSummary{nn}= [timeSummary{nn}; [[cellData{i}(j).estimatedGridTime,cellData{i}(j).minTotalTime , cellData{i}(j).totalTime]/( mean((cellData{i}(j).allTileTimes(:)+cellData{i}(j).estimatedMinLag))),cellData{i}(j).estimatedGridTime,cellData{i}(j).boundingBoxImagingOnly]];
                neuronScale{nn} = [neuronScale{nn}; cellData{i}(j).micronsPerPixel];
                
                if cellData{i}(j).isGridScan
                    thisMode = 2;
                elseif cellData{i}(j).isAdaptive
                    thisMode = 1;neuronScale
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

allData = -1*ones(1,19)
for i = 1:numel(a)
    allData = [allData; [neuronData{a(i)}, neuronData{a(i)}(:,1).*neuronScale{a(i)},neuronData{a(i)}(:,2).*neuronScale{a(i)}.*neuronScale{a(i)},neuronData{a(i)}(:,6).*neuronScale{a(i)}.*neuronScale{a(i)}, timeSummary{a(i)},scanMode{a(i)}]];



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
%  cellData{i}(j).estimatedGridTime
%  cellData{i}(j).imagingOnlyGridTime 

% scanMode ;

%                                                                         
                


s2 = allData(allData(:,end)==0,:)
s2A= allData(allData(:,end)==1,:)
s2G = allData(allData(:,end)==2,:)
%% then manually plot over with the s2A scans as grey circles:

subplot(4,1,1)
hold all, plot(s2A(:,11), s2A(:,12),'o', 'markersize',10, 'color', [.5,.5,.5])

subplot(4,1,2)
hold all, plot(s2A(:,11), s2A(:,4),'o', 'markersize',10, 'color', [.5,.5,.5])
subplot(4,1,3)
hold all, plot(s2A(:,11), s2A(:,16)./s2A(:,14),'o', 'markersize',10, 'color', [.5,.5,.5])
subplot(4,1,4)
hold all, plot(s2A(:,11), s2A(:,16)./s2A(:,15)-1,'o', 'markersize',10, 'color', [.5,.5,.5])


%%  now go into detail on the standardized data set with square boundingbox acquisitions.



bbCells = 27:46
summaryData = zeros(numel(bbCells),19);
for i = 1:numel(bbCells)
    if numel(cellData{bbCells(i)})==2  % this is ONLY LOOKING AT THE SECOND s2 acquisition if there are 2. otherwise look only at the first
        bbDatai = cellData{bbCells(i)}(2);
    else
        
    bbDatai = cellData{bbCells(i)}(1);
    end
    nBBTiles = numel( bbDatai.BBdata.allTileInfo)
    totalBBAreai = nBBTiles*bbDatai.BBdata.allTileInfo{1}.tileDimensions(1)*bbDatai.BBdata.allTileInfo{1}.tileDimensions(2); % true only for grid acquisitions
    zDepth = bbDatai.BBdata.allTileInfo{1}.tileDimensions(3);
    totalBBTimei  =  nBBTiles* bbDatai.BBdata.allTileInfo{1}.tileTime;
    totalBBTimeWithTif = totalBBTimei+bbDatai.BBTiffTime;
    
    %  summary information:
    
    %   |1: rectangular boundingBoxArea |2: Square BB Area| 3: Actual BB area | 4: total BB time (imaging only) |
    %    5: z depth | 6: total s2 scan time | 7: s2 imaged Area | 8: mean   sqrt(S2 tile area)
    %    9: s2 total tile area|10: microns per pixel |11: index i for  cellData{i}| 12: index j for cellData{i}(j) |
    %    13: estimated Rectangular BB Time |14: minimum S2 time (imaging
    %    and min. lag) | 15: Rectangular BB Imaging Only time | 16: actual imaging time
    %    per tile area of BB |
    %   17: S2 time per tile area Imaging Only |18: S2 estimated time per tile  area |19: BB imaging + Tif conversion 
    s2Infoi = [bbDatai.boundingBoxArea, bbDatai.boundingBoxAreaSquare,totalBBAreai, totalBBTimei,...
                zDepth , bbDatai.totalTime, bbDatai.imagedArea,  mean(sqrt(bbDatai.tileAreas)), ...
                bbDatai.totalTileArea,bbDatai.micronsPerPixel,i,j,...
               bbDatai.estimatedGridTime,bbDatai.minTotalTime ,  bbDatai.boundingBoxImagingOnly  , totalBBTimei/totalBBAreai       ,...
               bbDatai.imagingOnlyTimePerTileArea, bbDatai.estimatedTimePerTileArea, totalBBTimeWithTif];

    
    summaryData(i,:) = s2Infoi;
    
end


%%   plot stuffs

%  1. first, let's look at actual s2 scan time / BB scan time vs sparseness

%  added 'basic fitting' for imaging only data and saved fig.



figure, 
subplot(3,1,1)
hold all, plot(summaryData(:,7)./summaryData(:,3), summaryData(:,6)./summaryData(:,19),'o')  %plot  (S2 imaging time / BB Imaging Time)   vs imagedArea/BBArea
%  the line of y = 1  is the break-even point.
hold all, plot([0, .5], [1, 1])
title('s2 is faster than boundingbox scans for sparse structures')
xlabel('fraction of boundingbox scanned')
ylabel('S2 / boundingbox scan time')
% fit this with linear fit basic fitting, and leave fit equation on the plot.
% also plot what would happen if the imaging were perfect:
%  from subplot 3 we know that there is 2.4x slow down compared to a
%  maximum tile size. we can say ideal imaging would not slow down at all,
%  regaining a factor of 2.4 in the imaging time. this is an approximation,
%  basically saying that the total time (including analysis time) is
%  proportional to the imaged area.  since we have lots of tiles (sometimes
%  hundreds), this is probably reasonable.
hold all, plot([0 .5], [0 .5]*1.833 + .0061, 'DisplayName', 'estimated scaling for ideal imaging: y = 1.8*x + 0.0061')


%  2. now lets see why we only break even at sparsity around .25
subplot(3,1,2)

hold all,
plot(summaryData(:,19), summaryData(:,13).*(summaryData(:,2)./summaryData(:,1)), '*', 'DisplayName', 'BB Scan Time with s2 tiles')  % estimated boundingbox time.   plotted vs. actual imaging time for square BB

% I fit this with the mean slope y/x = 3.19 and plotted that too:
hold all, plot([0 900], [0 900]*3.1859)

hold all, plot([0 1000], [0 1000])
xlabel('bounding box scan time (s)')
ylabel('small-tile scan of bounding box (s)')



subplot(3,1,3)

% 3.  show just how bad this by plotting the 
%   plot scan effect from other script, scanCheckData.m



hold all, plot(dt1p2(:,1), dt1p2(:,6)./(dt1p2(:,5)), 'DisplayName', '1.2 us dwell time')
 plot(dt2p8(:,1), dt2p8(:,6)./(dt2p8(:,5)), 'DisplayName', '2.8 us dwell time')
 plot(dt10(:,1), dt10(:,6)./(dt10(:,5)), 'DisplayName', '10.0 us dwell time')
 

 
xlabel('x pixels')
ylabel('slowdown factor normalized to nominal dwell time')
legend toggle
title({'performance decline for smaller tiles',['nSlices = ',num2str(nSlices), ', piezo delay = ', num2str(piezoDelay), ' ms'] })
% I added datatips on the 2.8us dwell time line, and the text result (2.4x)
% is 2.611/1.089.




%%  ok,  final figures characterizing sparsity and S2 advantages:

%  data for PHC (summaryData above) with all the column information

%  1. bar plots with growing sparsity (volume/scanned volume)
%  

sparsityMetric = summaryData(:,3)./summaryData(:,7);
bbArea = summaryData(:,3);
bbTotalTime = summaryData(:,19);  % sort by this?
s2Time = summaryData(:,6); % plot this
s2Area = summaryData(:,7); % and this.


figure, 

p1data = [bbArea, bbTotalTime, s2Time, s2Area, sparsityMetric];
p1DataSorted = sortrows(p1data, 5);

plotyy(1:20, p1DataSorted(:,3)./p1DataSorted(:,2), 1:20, p1DataSorted(:,5))

%%hold all, plot(p1DataSorted(:,5))

% for supplemental:
% 
%  explanation of S2 taking longer than bounding box:
%

%  time per area  estimate for S2 (including image conversion)
%  imaging alone ratio (no conversion)
%  full ratio (conversion on both)  (distribution of conversion times for
%  BB?   19-21%

%   tif conversion for small tiles is less than 1s, hard to tell because it's
%   only based on system file dates.  

% other times (turnaround, sensing file change, initializing and starting
% stack) are about 0.6s

% in any case, the minimum lag (non-imaging time) is about 1.6s

%  note that my system can never acheive this because we have to convert
%  the file BEFORE I KNOW THE IMAGING IS DONE!  so the .tif conversion
%  should be counted into the imaging time, not the 'lag' time.  
%  so that lag includes .tif conversion (before trigger) and time to sense
%  the .tif conversion is done (cycles through a long list, could be 10s of
%  ms) and internal state processing (is there a new location? move to a
%  new position, adjust the zoom, initiate the z stack. all after the
%  trigger).







%  for small tiles, 1.6s is too slow and comes entirely from
%  post-acquisition, pre-read-in delays.  (time between startZstack and
%  finishedZstack signals is ~4.28s

%   startzstack is actually very close in time to the stack start
%   signal. basically emits the startzstack event string and then 2 lines
%   later emits the startzstack signal to myController with a delay set by
%   the spinbox in the configuration gui.  The only explanation is that the
%   s2controller and tile setup are combining with .tif time to take up
%   1.6s

% two factors in the overall slowdown:
%  1. image conversion and processing:  minimal processing time
%  adds 1.4747x on average.
%  2. scan losses due to piezo and flyback times, which don't scale
%  linearly with tile size:
%   2.6x  estimate for 2.8us dwell time, 30ms delay time and 50-slice
%   stacks.





%% old notes:
% the time/area is about 4.3x higher for small tiles than the whole BB,
% which is even more than the worst estimates from single tiles (2.8us adds
% about ~1.75x slowdown per line for small tiles).  there is still a factor
% of 2.45x missing WITHOUT COUNTING ANY TIME TO PROCESS INDIVIDUAL TILES.  
%  part of the problem is that file transfer, stack setup time and .tif conversion probably both
%  increase as a fraction of acquisition time as you get to smaller tiles.

% this doesn't correlate with the small variation in z depth (whew)  
%  but may be correlated somehow with imaging session?


%  yes. it turns out that 12 of these were imaged with minimum tile size of
%  50, while the remaining 10 of them were imaged with minimum tile size of
%  100.  The time per area is so nonlinear at these small tiles that the
%  change in tile size results in a ~3x worse time/area compared to BB
%  scanning when you go from 50 to 100pixel tiles. 

%  how did my other calculations miss this dramatic increase?  I
%  double-checked the displayed frame rate and there is no appreciable lag
%  for each frame (i.e. the frame period is within a few percent of
%  N_lines*linePeriod even for small tiles)
% what about z wait times?  
%  yes. of course.  it's the 30ms delay waiting for the piezo to settle.
%  This time is necessary to get a nice clean frame and accounts for the
%  rest of the discrepancy between scan times / area.

%  There would be room to do this differently with a
%  fixed tile size, but with changing tile size, the piezo has to calculate
%  a new pattern for each tile.  perhaps 20ms would be ok, but I want to
%  keep data collection as consistent as possible.


% which cells need to be re-imaged?



% cell032 - cell039
6/27 16:43
6/27 17:15
6/28 11:05
6/28 11:24
6/28 13:06
6/28 13:42
6/28 14:30
6/28 14:41



% cell041 cell042
6/28 16:52
6/28 17:01

% cell047-cell048
6/28 17:28
6/28 18:12



%%  for Hanchuan,simplified matrices with all scan data:


% 
s2PHC = s2(:,[1, 2, 5, 6, 11,12,13, 17,18, 7,8, 9,4,10])
% |1: average tile side|2: imaged area (no doublecounting) |3: total tile area (incl doublecounting) |4: bounding box area |5: average tile side (microns) 
% |6: imaged area (microns) |7: bounding box area (microns) |8:
% estimatedGridTime |9: imagingOnlyGridTime |10: ignore |11: ignore |12:
% zDepthVoxels |13: actual time | 14: square boundingbox area (voxels)
% 
%  "estimatedGridTime" includes the time for .tif conversion, (which can  be comparable to imaging times), and does not include any image overlap.  
% "imagingOnlyGridTime" would be if there is no time for .tif conversion.
%
%  
s2APHC = s2A(:,[1, 2, 5, 6, 11,12,13, 17,18, 7,8, 9,4,10])



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



%%  
%%  scaling analysis

figure,
subplot(2,1,1)
plot(log10(s2(:,9)),log10(s2(:,10)),'.')
subplot(2,1,2)
plot((s2(:,9)),(s2(:,10)),'.')


%% other  previous calculations:
% 
% 
% % how much additional time is contributed by analysis?  
% 
% %  timeSummary 
% 
% smartScanTimes = [timeSummary{1};timeSummary{2};timeSummary{3};timeSummary{4}]
% adaptiveScanTimes = timeSummary{aScanNN}
% gridScanTimes5 = [timeSummary{gScanNN}]
% 
% 
% ssAnalysisTimes = smartScanTimes(:,3)./smartScanTimes(:,2)-1
% mean(ssAnalysisTimes)
% std(ssAnalysisTimes)
% 
% ssScanTimes = smartScanTimes(:,3)./smartScanTimes(:,1)
% mean(ssScanTimes)
% std(ssScanTimes)
% 
% aScanTimes = adaptiveScanTimes(:,3)./adaptiveScanTimes(:,1)
% mean(aScanTimes)
% std(aScanTimes)
% 
% ssND = [neuronData{1}; neuronData{2}; neuronData{3}; neuronData{4}]
% aND = neuronData{aScanNN};
% gND = neuronData{gScanNN};
% ssTotalVolume = ssND(:,2);
% aTotalVolume = aND(:,2);
% gTotalVolume5 = gND(1,2);
% gTotalVolume7 = gND(2,2);
% mean(ssTotalVolume)
% std(ssTotalVolume)
% mean(aTotalVolume)
% std(aTotalVolume)
% 
% ssTotalTime = ssND(:,4)
% gTotalTime = gND(:,4)
% aTotalTime = aND(:,4)
% g5nD = neuronData{gScanNN}(1,:)
% 
% 
% mean(ssTotalTime)
% std(ssTotalTime)
% gTotalTime
% gTotalTime(2)/mean(aTotalTime)
% gTotalVolume7/mean(aTotalVolume)
% mean(gTotalVolume7)/mean(ssTotalVolume)
% mean(gTotalVolume5)/mean(aTotalVolume)
% mean(gTotalVolume7)/mean(aTotalVolume)
% mean(gTotalTime)/mean(aTotalTime)




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