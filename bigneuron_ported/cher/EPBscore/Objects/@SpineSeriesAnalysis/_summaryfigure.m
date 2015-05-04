function obj= summaryfigure(obj)
seriesdata= getappdata(gcbf,'seriesdata');
filenames=get(handles.listbox,'String');
pathname=seriesdata.pathname;
figurehandle=figure;
intfigurehandle=figure;
rows=2;
columns=floor(size(filenames,1)/2-0.01)+1;
for day=1:size(filenames,1)
    subplot(columns,rows,day);
    [path,name,ext] = fileparts([pathname char(filenames(day))]);
    if (strcmp(ext,'.spi'))
        try
            load('-mat',[path '\' name ext]);
        catch
            print('Could not open data');
            continue;
        end

        %extract intensity information
        
        axonnumber=1;
voxel=double(data.dendrites(axonnumber).voxel);
maxx=size(data.filteredArray,2);
maxy=size(data.filteredArray,1);
maxz=size(data.filteredArray,3);
%index=voxel(1,:)+(voxel(2,:)-1)*maxy+(voxel(3,:)-1)*maxy*maxx;
%calculate the instantaneous slope in X-Y plane use neighborhood of 3 pixels to each side
sizeofaxon=size(data.dendrites(axonnumber).voxel,2);


dy=([voxel(1,1) voxel(1,1) voxel(1,1) voxel(1,1:sizeofaxon-3)]-[voxel(1,4:sizeofaxon) voxel(1,sizeofaxon) voxel(1,sizeofaxon) voxel(1,sizeofaxon)]);
dx=([voxel(2,1) voxel(2,1) voxel(2,1) voxel(2,1:sizeofaxon-3)]-[voxel(2,4:sizeofaxon) voxel(2,sizeofaxon) voxel(2,sizeofaxon) voxel(2,sizeofaxon)]);
maxint=zeros(1,sizeofaxon);
length=sqrt(dx.*dx+dy.*dy);
%change length of 0 to 1 to avoid division by zero
length(find(length==0))=1;
dx=dx./length;
dy=dy./length;
for i=1:5 %crosssection of 3 on each side
    crossdx{i}=round((i-3).*dx);
    crossdy{i}=round((i-3).*dy);
    for j=-1:1
        x=min(max(voxel(2,:)+crossdy{i},0),maxx-1);
        y=min(max(voxel(1,:)+crossdx{i},0),maxy-1);
        z=min(max(voxel(3,:)+j,0),maxz-1);
        maxint=max(maxint,data.filteredArray(y+x*maxy+z*maxy*maxx));    
    end
end
thres=min(maxint(threshold(maxint,'background')));
meanback=mean(maxint(threshold(maxint,'background')==0));

%calculate axon length
locy=double(data.dendrites(axonnumber).voxel(1,1:10:size(data.dendrites(axonnumber).voxel(1,:),2)))*0.083;
locx=double(data.dendrites(axonnumber).voxel(2,1:10:size(data.dendrites(axonnumber).voxel(2,:),2)))*0.083;
length=sum(sqrt(diff(locx).*diff(locx)+diff(locy).*diff(locy)));

if (data.GUI.displaybinary==1)
        data.GUI.displaybinary=0;
end;
if (data.GUI.displayraw==1)
        data.GUI.displayraw=0;
end;
data.GUI.displayfiltered=1;
data.GUI.displaydendrites=0;
data.GUI.displayspines=1;
data.GUI.showprojection=1;
data.GUI.projectionhandle=figurehandle;
data.GUI.clow=0; 
data.GUI.chigh=meanback*3;
data.GUI.displayrejectedspine=0;
figure(figurehandle);
subplot(columns,rows,day);
displayProjectionImage(handles,data);
title(name);
xlabel(['meanback=' num2str(meanback)]);
figure(intfigurehandle);
subplot(columns,rows,day);
plot(double(maxint)/meanback);
hold on;
plot(double(thres)*ones(size(maxint))/meanback,'r');
axis([0 size(maxint,2) 0 ceil(max(double(maxint)/meanback))]);
title(name);
peaks=data.spines(find([data.spines.label]>0 ));
for i=1:size(peaks,2)
    text(max(1,peaks(i).dendis-5),0.2+maxint(peaks(i).dendis)/meanback,num2str(peaks(i).label));
end
end
end
setappdata(gcbf,'seriesdata',seriesdata);
