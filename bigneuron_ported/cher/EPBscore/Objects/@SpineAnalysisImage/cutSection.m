function obj = cutSection(obj)
if (obj.state.display.showstack==1) && isfield(obj.gh.imageGUI,'Figure') && obj.gh.imageGUI.Figure 
    [xv,yv]=getline(get(obj.gh.imageGUI.Figure,'Children'),'closed');
    if (obj.state.display.displayfiltered==1) && isfield(obj.data,'filteredArray') && size(obj.data.filteredArray,2)>0
        maxx=size(obj.data.filteredArray,1);
        maxy=size(obj.data.filteredArray,2);
        allpointsy=ones(maxx,1)*(1:maxy);
        allpointsx=(1:maxx)'*ones(maxy,1)';
        in=inpolygon(allpointsy(:),allpointsx(:),xv,yv);
        ind=find(in);
        
        for z=1:size(obj.data.filteredArray,3)
            obj.data.filteredArray(ind+(obj.state.display.currentz-1)*maxx*maxy)=0;
        end
    else
        if (obj.state.display.displayraw==1) && isfield(obj.data,'imageArray') && size(obj.data.imageArray,2)>0
            maxx=size(obj.data.imageArray,1);
            maxy=size(obj.data.imageArray,2);
            allpointsy=ones(maxx,1)*(1:maxy);
            allpointsx=(1:maxx)'*ones(maxy,1)';
            in=inpolygon(allpointsy(:),allpointsx(:),xv,yv);
            ind=find(in);
            
            for z=1:size(obj.data.imageArray,3)
                obj.data.imageArray(ind+(obj.state.display.currentz-1)*maxx*maxy)=0;
            end
        end
    end 
end