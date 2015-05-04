function obj = ROI(obj)
if (obj.state.display.showstack==1) && isfield(obj.gh.imageGUI,'Figure') && obj.gh.imageGUI.Figure 
    [xv,yv]=getline(get(obj.gh.imageGUI.Figure,'Children'),'closed');
    if (obj.state.display.displayraw==1) && isfield(obj.data,'imageArray') && size(obj.data.imageArray,2)>0
        maxx=size(obj.data.imageArray,1);
        maxy=size(obj.data.imageArray,2);
        allpointsy=ones(maxx,1)*(1:maxy);
        allpointsx=(1:maxx)'*ones(maxy,1)';
        in=inpolygon(allpointsy(:),allpointsx(:),xv,yv);
        ind=find(in);
        obj.data.ROI.ind=ind;
        obj.data.ROI.xv=xv;
        obj.data.ROI.yv=yv;
        obj.data.ROI.z=obj.state.display.currentz;
        obj.data.ROI.meanintensity=mean(obj.data.imageArray(ind+(obj.data.ROI.z-1)*maxx*maxy))-obj.data.imagemedian;
    end
end