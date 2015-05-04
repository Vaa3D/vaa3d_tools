function obj = cutAxons(obj)
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
        
        newdendrites=[];
        curden=1;
        if isfield(obj.data,'dendrites')
            for i=1:size(obj.data.dendrites,2)
                indden=double(obj.data.dendrites(i).voxel(2,:))*maxy+double(obj.data.dendrites(i).voxel(1,:));
                inden=~ismember(indden,ind);
                if (prod(double(inden))==0)
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
                    den=obj.data.dendrites(i);
                    for j=1:size(startden,2)
%                        [voxel, maxint, dx, dy]=shiftDendriteMax(double(den.voxel(:,startden(j):endden(j))),obj);
                        voxel=den.voxel(:,startden(j):endden(j));
                        newdendrites(curden).voxel=den.voxel(:,startden(j):endden(j));
                        newdendrites(curden).dx=[];%dx;
                        newdendrites(curden).dy=[];
                        newdendrites(curden).maxint=[];
                        newdendrites(curden).start=double(voxel(1:3,1))';
                        newdendrites(curden).end=double(voxel(1:3,end))';
                        newdendrites(curden).totalpixel=size(voxel,2);
                        curden=curden+1;
                    end
                else
                    fnames=fieldnames(obj.data.dendrites(i));
                    for j=1:size(fnames,1)
                        newdendrites(curden).(char(fnames(j)))=obj.data.dendrites(i).(char(fnames(j)));
                    end
                    newdendrites(curden).totalpixel=size(obj.data.dendrites(i).voxel,2);
                    curden=curden+1;
                end
            end
            obj.data.dendrites=newdendrites;
       
            %obj=CorrectDendrites(obj);
        end
        obj=ShiftDendriteMaxObj(obj,1);
        obj=CorrectDendrites(obj);
    end
    obj.data.spines=[];          % void the spine data for now because it's totally different now later try to convert it
end