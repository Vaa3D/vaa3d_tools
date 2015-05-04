for z=1:max(size(directorynames)) %get the list of directories
    [path,name,ext] = fileparts([char(directorynames(z))]);
    pathnames=dir([path '\*edited.spi']);     
    
    % Add Series. Each directory is one series
    if (size(db.series,2)==0)
        seriesuid=1;
    else
        seriesuid=max([db.series.uid])+1;
    end
    db.series(seriesuid).uid=seriesuid;
    db.series(seriesuid).experiments=[];
