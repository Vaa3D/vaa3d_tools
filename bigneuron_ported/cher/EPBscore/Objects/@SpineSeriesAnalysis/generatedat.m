% generate .dat file
function self=generatedat(db,experimentuid)

self=struct();
% Add experiment
db.experiments(experimentuid).uid=experimentuid;
self.data.ch(1).imagemedian=db.experiments(experimentuid).imagemedian;
self.data.ch(1).imagestd=db.experiments(experimentuid).imagestd;
if isfield(db.experiments(experimentuid),'m')
    self.data.ch(1).m=db.experiments(experimentuid).m;
    self.data.ch(1).s=db.experiments(experimentuid).s;
end
self.data.path=db.experiments(experimentuid).rawImagePath;
self.data.name=db.experiments(experimentuid).rawImageName;
self.state.rawImageName=db.experiments(experimentuid).rawImageName;
self.state.rawImagePath=db.experiments(experimentuid).rawImagePath;
self.parameters=db.experiments(experimentuid).parameters;
self.data.parameters=self.parameters;
if isfield(db.experiments(experimentuid),'ROI')
    self.data.ROI=struct();
    self.data.ROI.meanintensity=db.experiments(experimentuid).ROIintensity;
end

% Add dendrites
self.data.marks=[];
self.data.dendrites=[];
self.data.spines=[];
for i=1:max(db.experiments(experimentuid).seriesuid)
    % find the corresponding dendrite
    suid=db.experiments(experimentuid).seriesuid(i)
    for j=1:length(db.series(suid).names)
        if strcmp(db.experiments(experimentuid).rawImageName,db.series(suid).names(j))
            uid=db.series(1).dendrites(i,j)
        end
    end
    
    names=fieldnames(db.dendrites(uid));
    self.data.dendrites(i)=struct()
    for j=1:size(names,1)
        self.data.dendrites(i).(char(names(j)))=db.dendrites(uid).(char(names(j)));
    end
    self.data.dendrites(i).index=i;
    
    % generate marks
    dis=db.dendrites(uid).meanmarks+db.dendrites(uid).stdmarks*[-1 -0.5 0 0.5 1]/0.7906;
    for j=1:length(dis)
        ind=length(self.data.marks)+1;
        self.data.marks(ind).type=0;
        self.data.marks(ind).color=[0.7 0.7 0.7];
        self.data.marks(ind).den_ind=i;
        self.data.marks(ind).voxels=1;
        self.data.marks(ind).label=j;
        self.data.marks(ind).dendis=dis(j);
        self.data.marks(ind).voxel=self.dendrites(i).voxel(1:3,dis(j));
    end
end

