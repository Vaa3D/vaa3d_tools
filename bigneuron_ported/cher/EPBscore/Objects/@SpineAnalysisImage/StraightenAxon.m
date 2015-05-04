function obj = StraightenAxon(obj)
 if (obj.state.display.ch1==1) && isfield(obj.data.ch(1),'filteredArray')
                ch=1;
            end
            if (obj.state.display.ch2==1) && isfield(obj.data.ch(2),'filteredArray')
                ch=2;
            end
if (obj.state.display.showprojection==1) && isfield(obj.gh.projectionGUI,'Figure') && obj.gh.projectionGUI.Figure 
    [xv,yv]=getline(get(obj.gh.projectionGUI.Figure,'Children'),'closed');
    if (obj.state.display.showprojection==1) && isfield(obj.data.ch(ch),'filteredArray') && size(obj.data.ch(ch).filteredArray,2)>0
        maxx=size(obj.data.ch(ch).filteredArray,1);
        maxy=size(obj.data.ch(ch).filteredArray,2);
        allpointsy=ones(maxx,1)*(1:maxy);
        allpointsx=(1:maxx)'*ones(maxy,1)';
        in=inpolygon(allpointsy(:),allpointsx(:),xv,yv);
        ind=find(in);
        axonnumber=obj.state.display.axonnumber;
        
        % find the correct axon to work on
        inden=[];
        for i=1:size(obj.data.dendrites,2)
            indden=double(obj.data.dendrites(i).voxel(2,:))*maxy+double(obj.data.dendrites(i).voxel(1,:));
            if (sum(ismember(indden,ind))>0)
                inden=[inden i];
            end
        end
        if ~ismember(axonnumber,inden) & (size(inden,2)==1)
            axonnumber=inden(1);
        end
        
        indden=double(obj.data.dendrites(axonnumber).voxel(2,:))*maxy+double(obj.data.dendrites(axonnumber).voxel(1,:));
        inden=~ismember(indden,ind);
        startden=find(diff(inden)==1);
        endden=find(diff(inden)==-1);
        if (size(endden,2)>0 & size(startden,2)==0)
            startden=[1 startden];
        elseif (size(endden,2)>0 & endden(1)<startden(1))
            startden=[1 startden];
        end
        if (size(startden,2)>0 & size(endden,2)==0)
            endden=[endden size(indden,2)];
        elseif (size(startden,2)>0 &  startden(end)>endden(end))
            endden=[endden size(indden,2)];
        end
        voxel1=obj.data.dendrites(axonnumber).voxel(:,startden(1):endden(1));
        voxel2=obj.data.dendrites(axonnumber).voxel(:,startden(end):endden(end));
        
        voxel1=double(voxel1);
        voxel2=double(voxel2)+0.00001;
        % figure out the linear link between the two axons
        dis=-double(voxel1(:,end))+double(voxel2(:,1));
        voxel=[voxel1(1,end):dis(1)/max(abs(dis)):voxel2(1,1); voxel1(2,end):dis(2)/max(abs(dis)):voxel2(2,1); voxel1(3,end):dis(3)/max(abs(dis)):voxel2(3,1); voxel1(4,end):dis(4)/max(abs(dis)):voxel2(4,1)];
        voxel=round(voxel);
        %smoothing around the area of linkage
        
        voxel(1,:)=smooth(voxel(1,:),7)';
        voxel(2,:)=smooth(voxel(2,:),7)';
        voxel(3,:)=smooth(voxel(3,:),7)';
        voxel=[voxel1'; voxel(:,2:end-1)'; voxel2']';
        
        obj.data.dendrites(axonnumber).voxel=double(round(floor(voxel)));
        obj=ShiftDendriteMaxObj(obj,1);
        obj=CorrectDendrites(obj);
    end
end