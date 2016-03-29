% Data analysis for smartscope 2 
% starting 2016.03.28

% 1. Data Entry





%  the post-PV files are stored on my workstation, including .xml files transfered
%  from the rig machine.
batch1directory = '/local2/'
batch1 = dir([batch1directory,'2016*S*'])  % these were collected on Saturday and Sunday.
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






% 2.  Analysis

% determine difference between summed tile volume and the actual scanned
% volume [estimate from (whole micron?) binary images] 






% for a set of tile locations, build a binary array to model the scan area:

for i = 1:numel(sessionData)
    for j = 1:numel(sessionData{i})
        
        tileSetij = sessionData{i}(j).tileLocations
        nTiles = numel(tileSetij)
        if nTiles<2
            continue
        else
            
            allLocations = cell2mat(tileSetij');
            [bigRectx, bigRecty] = meshgrid(floor(min(allLocations(:,1))):ceil(max(allLocations(:,3))),floor(min(allLocations(:,2))):ceil(max(allLocations(:,4))));
            bigRect = false(size(bigRectx));
            for k = 1:nTiles
                bigRect(:) = bigRect(:)| ((bigRectx(:)>allLocations(k,1)) &( bigRectx(:)<=allLocations(k,3) )& (bigRecty(:)>allLocations(k,2)) &( bigRecty(:)<=allLocations(k,4) ));
                
                
            end
            %figure, imshow(bigRect);
            sessionData{i}(j).imagedArea = sum(bigRect(:));
            sessionData{i}(j).tileAreas = (allLocations(:,4)-allLocations(:,2)).*(allLocations(:,3)-allLocations(:,1));
            sessionData{i}(j).totalTileArea = sum(sessionData{i}(j).tileAreas);
            sessionData{i}(j).extraScanning = sessionData{i}(j).totalTileArea-sessionData{i}(j).imagedArea;
            sessionData{i}(j).boundingBoxSparsity = sessionData{i}(j).totalTileArea/numel(bigRect);            
            sessionData{i}(j).lagTimes =  diff(sessionData{i}(j).tileStartTimes)-sessionData{i}(j).allTileTimes(2:end);
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

