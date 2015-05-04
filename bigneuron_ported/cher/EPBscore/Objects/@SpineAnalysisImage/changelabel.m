function obj=changelabel(obj)
numstr=char(obj.state.display.editbox);
if (numstr)
    rem=numstr;
    while(rem)
        [token,rem]=strtok(rem);
        [first,secondrem]=strtok(token,'=');
        second=strtok(secondrem,'=');
        numfirst=str2double(first);
        numsecond=str2double(second);
        if (numsecond==0)      % indicating a spine on previous day is out of picture this day
            j=size(obj.data.spines,2)+1;
            obj.data.spines(j).type=-2;
            obj.data.spines(j).color=[0.7,0.7,0.7];
            obj.data.spines(j).voxels=3;
            obj.data.spines(j).voxel=[-1 -1 -1;-1 -1 -1; -1 -1 -1]';
            obj.data.spines(j).den_ind=0;
            obj.data.spines(j).dendis=0;
            obj.data.spines(j).label=numfirst;
        else
            firstlocs=find([obj.data.spines.label]==numfirst);
            currentaxon=obj.state.display.axonnumber;
            if currentaxon==0
                firstloc=firstlocs(1);
            else
                firstloc=firstlocs(find([obj.data.spines(firstlocs).den_ind]==currentaxon));
                firstloc=firstloc(1);
            end
            obj.data.spines(firstloc).label=numsecond;
        end
    end
end