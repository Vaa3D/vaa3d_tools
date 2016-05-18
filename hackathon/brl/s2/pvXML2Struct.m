function pvXML2Struct(xmlfilepath)
% function to  collect data from  .xml files 


% new struct array to house this information

%  scan folder name
s2ScanData(1).folderName = '';


%  tile time (average or all or one?)
s2ScanData(1).allTileTimes = [];  % all tile times if applicable
s2ScanData(1).averageTileTime = [];

% number of tiles
s2ScanData(1).nTiles=0;
% tile dimensions
s2ScanData(1).tileDimensions = [0,0,0];
% number of pixels per tile (x*y*z)
s2ScanData(1).voxelsPerTile = 0;

% overlap (changed from 10% to 15% at some point...)
s2ScanData(1).nominalOverlap = 0;
% optical zoom 
s2ScanData(1).opticalZoom = 0;
% 
s2ScanData(1).tileSizeMicrons=[0,0];

% total time for the whole acquisition
s2ScanData(1).totalTime=0;

% tile locations and sizes
s2ScanData(1).tileLocations={[0,0,10,10], [9,0,19,10]};

% tile start times

s2ScanData(1).tileStartTimes=[0,24.4];     
