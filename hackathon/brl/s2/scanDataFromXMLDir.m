function s2ScanData  = scanDataFromXMLDir(xmldir)

%  Brian Long
%  2016.03.28
%
% function to  collect some scan data from a PrairieView .xml 
%

%  input:    xmldir

%  this directory contains the all the xml files for a single scan sequence
%  

%  


scanXMLfiles = dir([xmldir,'/*.xml']);

% folder name
s2ScanData.folderName = xmldir;
% number of tiles
s2ScanData.nTiles= numel(scanXMLfiles);



%  From each xml file, I need
%  first read the file using xml2struct package


for i = 1:numel(scanXMLfiles)
 ['processing tile ', num2str(i)]    
    xmlStructi = xml2struct([xmldir,'/',scanXMLfiles(i).name]);

stackStateShard = xmlStructi.PVScan.PVStateShard;
%  SINGLE TILE DATA:
tileData.tileString = [xmldir,'/',scanXMLfiles(i).name];
% active mode
tileData.activeMode = stackStateShard.PVStateValue{1}.Attributes.value;  % string "Resonant Galvo" or "Galvo"
clockDateString = datestr(xmlStructi.PVScan.Attributes.date);
tileData.clockStartTime = 24*60*60*(floor(datenum(clockDateString))+datenum(xmlStructi.PVScan.Sequence.Attributes.time));
% % optical zoom 
 tileData.opticalZoom = str2double(stackStateShard.PVStateValue{14}.Attributes.value);
% tile dimensions;
pixelsPerLine = str2double(stackStateShard.PVStateValue{15}.Attributes.value);
linesPerFrame =    str2double( stackStateShard.PVStateValue{7}.Attributes.value);
if ~isfield(xmlStructi.PVScan.Sequence, 'Frame')
framesPerTile = 0;
tileData.tileTime = 0;

else  
framesPerTile = numel(xmlStructi.PVScan.Sequence.Frame); 
tileData.tileTime = str2double( xmlStructi.PVScan.Sequence.Frame{end}.Attributes.absoluteTime);

end

tileData.tileDimensions =[pixelsPerLine ,linesPerFrame , framesPerTile];
% % number of pixels per tile (x*y*z)

 tileData.voxelsPerTile = pixelsPerLine*linesPerFrame*framesPerTile;

 
 % total time per tile


 xMicronsPerPixel = str2double(stackStateShard.PVStateValue{9}.IndexedValue{1}.Attributes.value);
 yMicronsPerPixel = str2double(stackStateShard.PVStateValue{9}.IndexedValue{1}.Attributes.value);
 
 % tile microns per pixel
 tileData.micronsPerPixel =  [xMicronsPerPixel, yMicronsPerPixel];
 %tile size in microns
 tileData.tileSizeMicrons=[pixelsPerLine*xMicronsPerPixel,linesPerFrame*yMicronsPerPixel];
 
 xMinVolts = str2double(stackStateShard.PVStateValue{10}.IndexedValue{1}.Attributes.value);
 xMaxVolts = str2double(stackStateShard.PVStateValue{8}.IndexedValue{1}.Attributes.value);
 yMinVolts = str2double(stackStateShard.PVStateValue{10}.IndexedValue{2}.Attributes.value);
 yMaxVolts = str2double(stackStateShard.PVStateValue{8}.IndexedValue{2}.Attributes.value);
% ;
tileData.tileLocationVolts=[xMinVolts,yMinVolts,xMaxVolts,yMaxVolts];  %  upper left corner is scan center - scanSize/2
% 

tileData.tileLocationVoltsPixels = tileData.tileLocationVolts.*[pixelsPerLine/(xMaxVolts-xMinVolts),linesPerFrame/(yMaxVolts-yMinVolts),pixelsPerLine/(xMaxVolts-xMinVolts), linesPerFrame/(yMaxVolts-yMinVolts)];
tileData.tileLocationVoltsMicrons = tileData.tileLocationVoltsPixels.*[xMicronsPerPixel, yMicronsPerPixel,xMicronsPerPixel,yMicronsPerPixel];
% 
% 
% 
% 
% %  tile time (average or all or one?)
 s2ScanData.allTileTimes(i) =  tileData.tileTime ;  % all tile times if applicable
% % tile start times
% 
 s2ScanData.tileStartTimes(i) = tileData.clockStartTime;     
% % tile locations and sizes
 s2ScanData.tileLocations{i}=tileData.tileLocationVoltsPixels;
s2ScanData.allTileInfo{i} = tileData;


end
% 
% 
% s2ScanData(1).averageTileTime = 0;
% s2ScanData(1).stdTileTime = 0;  % standard deviation of tile times
% % overlap (changed from 10% to 15% at some point...)
% s2ScanData(1).nominalOverlap = 0;
% 
% % total time for the whole acquisition
% s2ScanData(1).totalTime=0;

