function obj= manualtracespines(obj)
seriesdata= getappdata(gcbf,'seriesdata');
data=getappdata(seriesdata.currentimage,'data');
prevdata=getappdata(seriesdata.previmage,'data');
newspinenum=max([prevdata.spines.label])+1;
newspinenum=max(newspinenum,prevdata.newspinenum);
data.newspinenum=newspinenum;
%newspinenum=max([prevdata.spines.label])+1;
%newspinenum=max(newspinenum,prevdata.newspinenum);
%data.newspinenum=newspinenum;
numstr=char(get(handles.inputtext,'String'));
rem=numstr;
labels=[data.spines.label];
while(rem)
[token,rem]=strtok(rem);
[first,secondrem]=strtok(token,'=');
second=strtok(secondrem,'=');
numfirst=str2double(first);
numsecond=str2double(second);
if (numsecond==-2)      % indicating a spine on previous day is out of picture this day
    j=size(data.spines,2)+1;
    data.spines(j).type=-2;
    data.spines(j).color=[0.7,0.7,0.7];
    data.spines(j).voxels=3;
    data.spines(j).voxel=[-1 -1 -1;-1 -1 -1; -1 -1 -1]';
    data.spines(j).den_ind=1;
    data.spines(j).dendis=-2;
    data.spines(j).label=numfirst;
else
firstlocs=find(labels==numfirst);
data.spines(firstlocs(1)).label=numsecond;
end
end

displayImage(guidata(seriesdata.currentimage),data);
displayProjectionImage(guidata(seriesdata.currentimage),data);
setappdata(seriesdata.currentimage,'data',data);
