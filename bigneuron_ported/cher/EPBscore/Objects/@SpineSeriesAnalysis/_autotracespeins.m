function obj=autotracespines(obj)

seriesdata= getappdata(gcbf,'seriesdata');
prevdata=getappdata(seriesdata.previmage,'data');
data=getappdata(seriesdata.currentimage,'data');
x=xcorr(prevdata.spinogramr,data.squarespinogramr)+xcorr(prevdata.spinograml,data.squarespinograml);
[num,offset]=max(x);
offset=offset-(size(x,2)-1)/2;

        
        % start assigning numbers
        prevleftspines=find(([prevdata.spines.side]<0)&([prevdata.spines.edittype]==0));
        prevrightspines=find(([prevdata.spines.side]>0)&([prevdata.spines.edittype]==0));
        newspinenum=max([prevdata.spines.label])+1;
        newspinenum=max(newspinenum,prevdata.newspinenum);
        data.newspinenum=newspinenum;
        for j=1:size(data.spines,2)
          if (data.spines(j).edittype==0)  
            %find distances to spines
            if (data.spines(j).side<0)
                dis=abs(data.spines(j).dendis-[prevdata.spines(prevleftspines).dendis]+offset);
                [mindis,loc]=min(dis);
                if (abs(mindis)<8)
                    data.spines(j).label=prevdata.spines(prevleftspines(loc)).label;
                    prevleftspines(loc)=[];
                else
                    data.spines(j).label=newspinenum;
                    newspinenum=newspinenum+1;
                end
            else
                dis=abs(data.spines(j).dendis-[prevdata.spines(prevrightspines).dendis]+offset);
                [mindis,loc]=min(dis);
                if mindis<8
                    data.spines(j).label=prevdata.spines(prevrightspines(loc)).label;
                    prevrightspines(loc)=[];
                else
                    data.spines(j).label=newspinenum;
                    newspinenum=newspinenum+1;
                end
            end
          end  
        end        
displayImage(guidata(seriesdata.currentimage),data);
displayProjectionImage(guidata(seriesdata.currentimage),data);
setappdata(seriesdata.currentimage,'data',data);
