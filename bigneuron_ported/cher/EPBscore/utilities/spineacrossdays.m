function results=spineacrossdays(name)
global db;
ind=[];
for i=1:size(db.experiments,2)
    ind=[ind strcmp(db.experiments(i).rawImageName,name)];
end
ind=find(ind);
ind=ind(end);
seriesuid=db.experiments(ind).seriesuid;
results=struct('spineacrossdays',db.series([db.series.uid]==seriesuid).spineacrossday,'dendrites',[db.dendrites([db.dendrites.seriesuid]==seriesuid)]);