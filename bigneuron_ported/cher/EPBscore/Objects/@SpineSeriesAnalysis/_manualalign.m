function obj=manualalign(obj)
seriesdata= getappdata(gcbf,'seriesdata');
data=getappdata(seriesdata.currentimage,'data');
prevdata=getappdata(seriesdata.previmage,'data');
numstr=char(get(handles.inputtext,'String'));
rem=numstr;
total=zeros(1,size(data.dendrites,2));
n=zeros(1,size(data.dendrites,2));
while(rem)
[token,rem]=strtok(rem);
[first,secondrem]=strtok(token,'=');
second=strtok(secondrem,'=');
numfirst=str2double(first);
numsecond=str2double(second);
k=data.spines(find([data.spines.label]==numfirst)).den_ind;
total(k)=total(k)+data.spines(find([data.spines.label]==numfirst)).dendis-prevdata.spines(find([prevdata.spines.label]==numsecond)).dendis;
n(k)=n(k)+1;
end
offset=-total./n;

% start assigning numbers
for k=1:size(data.dendrites,2)
   prevleftspines{k}=find(([prevdata.spines.side]<0)&([prevdata.spines.edittype]==0)&([prevdata.spines.den_ind]==k));
   prevrightspines{k}=find(([prevdata.spines.side]>0)&([prevdata.spines.edittype]==0)&([prevdata.spines.den_ind]==k));
end
newspinenum=max([prevdata.spines.label])+1;
newspinenum=max(newspinenum,prevdata.newspinenum);
data.newspinenum=newspinenum;
newspinenum=-10;

for j=1:size(data.spines,2)
          if (data.spines(j).edittype==0)  
            %find distances to spines
            k=data.spines(j).den_ind;
            if (data.spines(j).side<0)
                dis=abs(data.spines(j).dendis-[prevdata.spines(prevleftspines{k}).dendis]+offset(k));
                [mindis,loc]=min(dis);
                if (abs(mindis)<15)
                    data.spines(j).label=prevdata.spines(prevleftspines{k}(loc)).label;
                    prevleftspines{k}(loc)=[];
                else
                    data.spines(j).label=newspinenum;
                    newspinenum=newspinenum-1;
                end
            else
                dis=abs(data.spines(j).dendis-[prevdata.spines(prevrightspines{k}).dendis]+offset(k));
                [mindis,loc]=min(dis);
                if mindis<15
                    data.spines(j).label=prevdata.spines(prevrightspines{k}(loc)).label;
                    prevrightspines{k}(loc)=[];
                else
                    data.spines(j).label=newspinenum;
                    newspinenum=newspinenum-1;
                end
            end
          end  
        end        
displayImage(guidata(seriesdata.currentimage),data);
displayProjectionImage(guidata(seriesdata.currentimage),data);
setappdata(seriesdata.currentimage,'data',data);

