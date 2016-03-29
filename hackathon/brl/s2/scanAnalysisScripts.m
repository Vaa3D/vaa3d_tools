% Data analysis for smartscope 2 
% starting 2016.03.28

% 1. Data Entry





%  the post-PV files are stored on my workstation, including .xml files transfered
%  from the rig machine.
batch1directory = '/local2/'
batch1 = dir([batch1directory,'2016*Su*'])  % images collected on Sunday.
batch1 = batch1([batch1(:).isdir]')
    
tic

for i = 1:numel(batch1)
   sessioniDir = dir([batch1directory,'/', batch1(i).name])
    sessioniDir = sessioniDir([sessioniDir(:).isdir]')  % just get scan directories
   for j = 1:numel(sessioniDir)  %  all scans, even aborted ones
        scanjDir = dir([batch1directory, '/', batch1(i).name,'/', sessioniDir(j).name,'/','*.xml'])
        if numel(scanjDir)>0
            sessionData{i}(j) = scanDataFromXMLDir([batch1directory, '/', batch1(i).name,'/', sessioniDir(j).name]);
        end
        
   end
   

end
toc
% 914s for 25 sessions






%% 2.  Analysis

% determine difference between summed tile volume and the actual scanned
% volume [estimate from (whole micron?) binary images] 






% for a set of tile locations, build a binary array to model the scan area:

for i = 1:numel(sessionData)
    for j = 1:numel(sessionData{i})
        
        tileSetij = sessionData{i}(j).tileLocations
        nTiles = numel(tileSetij)
        sessionData{i}(j).ignore = False
        
        if ( nTiles<3 )| (strfind(sessionData{i}(j).folderName, '.'))
            sessionData{i}(j).ignore = False
            continue
        else
            
            allLocations = cell2mat(tileSetij');
            [bigRectx, bigRecty] = meshgrid(floor(min(allLocations(:,1))):ceil(max(allLocations(:,3))),floor(min(allLocations(:,2))):ceil(max(allLocations(:,4))));
            bigRect = false(size(bigRectx));
            for k = 1:nTiles
                bigRect(:) = bigRect(:)| ((bigRectx(:)>allLocations(k,1)) &( bigRectx(:)<=allLocations(k,3) )& (bigRecty(:)>allLocations(k,2)) &( bigRecty(:)<=allLocations(k,4) ));
                
                
            end
            %figure, imshow(bigRect);
            if strfind(sessionData{i}(j).folderName,'Sun_09' )
                sessionData{i}(j).neuronNumber = 1;
            end
                if strfind(sessionData{i}(j).folderName,'Sun_10' )  %  this is bad form... I need a cell index that comes from the s2 program.
                sessionData{i}(j).neuronNumber = 2;
                end
                
                if strfind(sessionData{i}(j).folderName,'Sun_11' )
                sessionData{i}(j).neuronNumber = 3;
            end
            
            sessionData{i}(j).imagedArea = sum(bigRect(:));
            sessionData{i}(j).tileAreas = (allLocations(:,4)-allLocations(:,2)).*(allLocations(:,3)-allLocations(:,1));
            sessionData{i}(j).totalTileArea = sum(sessionData{i}(j).tileAreas);
            sessionData{i}(j).extraScanning = sessionData{i}(j).totalTileArea-sessionData{i}(j).imagedArea;
            sessionData{i}(j).boundingBoxSparsity = sessionData{i}(j).totalTileArea/numel(bigRect);            
            sessionData{i}(j).lagTimes =  diff(sessionData{i}(j).tileStartTimes)-sessionData{i}(j).allTileTimes(1:end-1);
            sessionData{i}(j).totalTime = sessionData{i}(j).tileStartTimes(end)-sessionData{i}(j).tileStartTimes(1)+sessionData{i}(j).allTileTimes(end)+min(sessionData{i}(j).lagTimes);
            sessionData{i}(j).minTotalTime = sum(sessionData{i}(j).allTileTimes(:)+min(sessionData{i}(j).lagTimes));
            sessionData{i}(j).minImagingOnly = sum(sessionData{i}(j).allTileTimes(:));
            sessionData{i}(j).estimatedMinLag = min( sessionData{i}(j).lagTimes);
            sessionData{i}(j).estimatedTimePerTileArea = mean((sessionData{i}(j).allTileTimes(:)+sessionData{i}(j).estimatedMinLag)./sessionData{i}(j).tileAreas);
            sessionData{i}(j).estimatedGridTime = numel(bigRect)*sessionData{i}(j).estimatedTimePerTileArea;
        end
    end
end



% extract lag times = (difference between sequential tiles) - tiletime.
% this will include convert/load times but should also show initial big lag
% followed by minimal lags in continuous imaging mode. tough to extract necessary from unnecessary
% delays, though.



% extract 'extra' time (difference between tiletime*N and total time)

%  total time vs tile size for each neuron (N  = 3)

%  total volume vs tile size for each neuron (N = 3)
%% plotting
figure
timeSummary{1} = [0,0,0]
timeSummary{2} = [0,0,0]
timeSummary{3} = [0,0,0]

neuronData{1} = [0,0, 0,0]
neuronData{2} = [0,0,0,0]
neuronData{3} = [0,0,0,0]
for i = 1:numel(sessionData)
    for j = 1:numel(sessionData{i})
        
        tileSetij = sessionData{i}(j).tileLocations;
        nTiles = numel(tileSetij);
        if sessionData{i}(j).ignore
            continue
        else
            if isfield(sessionData{i}(j),'neuronNumber')
                if sessionData{i}(j).neuronNumber == 1
                    neuronData{1} = [neuronData{1} ; [mean(sqrt(sessionData{i}(j).tileAreas)),sessionData{i}(j).imagedArea],i,j]
                    neuronData{1} = [neuronData{1} ; [mean(sqrt(sessionData{i}(j).tileAreas)),sessionData{i}(j).totalTime],i,j]
                    timeSummary{1}= [timeSummary{1}; [sessionData{i}(j).estimatedGridTime,sessionData{i}(j).minTotalTime , sessionData{i}(j).totalTime]/( mean((sessionData{i}(j).allTileTimes(:)+sessionData{i}(j).estimatedMinLag)))];
                end
                
                if sessionData{i}(j).neuronNumber == 2
                    neuronData{2} = [neuronData{2} ; [mean(sqrt(sessionData{i}(j).tileAreas)),sessionData{i}(j).imagedArea],i,j]
                    neuronData{2} = [neuronData{2} ; [mean(sqrt(sessionData{i}(j).tileAreas)),sessionData{i}(j).totalTime],i,j]
                    timeSummary{2}= [timeSummary{2}; [sessionData{i}(j).estimatedGridTime,sessionData{i}(j).minTotalTime , sessionData{i}(j).totalTime]/( mean((sessionData{i}(j).allTileTimes(:)+sessionData{i}(j).estimatedMinLag)))];
                end
                
                if sessionData{i}(j).neuronNumber == 3
                    neuronData{3} = [neuronData{3} ; [mean(sqrt(sessionData{i}(j).tileAreas)),sessionData{i}(j).imagedArea],i,j]
                    neuronData{3} = [neuronData{3} ; [mean(sqrt(sessionData{i}(j).tileAreas)),sessionData{i}(j).totalTime],i,j]
                    timeSummary{3}= [timeSummary{3}; [sessionData{i}(j).estimatedGridTime,sessionData{i}(j).minTotalTime , sessionData{i}(j).totalTime]/( mean((sessionData{i}(j).allTileTimes(:)+sessionData{i}(j).estimatedMinLag)))];
                end
                
            end
            
            subplot(2,1,1), hold all, plot(mean(sqrt(sessionData{i}(j).tileAreas)),sessionData{i}(j).imagedArea,'o-', 'DisplayName', sessionData{i}(j).folderName)
            subplot(2,1,2), hold all, plot(mean(sqrt(sessionData{i}(j).tileAreas)),sessionData{i}(j).totalTime, '*-', 'DisplayName', sessionData{i}(j).folderName)
            plot(mean(sqrt(sessionData{i}(j).tileAreas)),sessionData{i}(j).minTotalTime, '*-')
            
            
        end
    end
end


for i = 1:3
subplot(2,1,2),    plot(neuronData{i}(1:2:end,1),neuronData{i}(1:2:end,2));
subplot(2,1,1),    plot(neuronData{i}(2:2:end,1),neuronData{i}(2:2:end,2));
end
