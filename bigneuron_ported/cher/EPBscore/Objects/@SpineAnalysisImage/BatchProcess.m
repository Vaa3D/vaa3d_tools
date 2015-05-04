function obj=batchProcess(obj,reanalyze)
try
    set(0,'Units','Pixels')
    screensize=get(0,'ScreenSize');
    x=screensize(3);
    y=screensize(4);
    obj=set(obj,'state.display.windowpos',struct('mainGUI',[0.005*x 0.03*y 0.35*x 0.15*y],'profileGUI',[0.005*x 0.22*y 0.35*x 0.18*y],'projectionGUI',[0.005*x 0.44*y 0.495*x 0.53*y],'imageGUI',[0.005*x 0.44*y 0.495*x 0.53*y]));
    obj=set(obj,'state.display.showprofile',0);
    obj=set(obj,'state.display.showprojection',1);
    obj=set(obj,'state.display.showparameters',0);
    obj=set(obj,'state.display.showstack',0);
    obj=updateWindowPosByObj(obj);
    obj.state.display.colormap=colormap(makeColorMap('gray', 8));
    obj=showprojection(obj);
        
%     % recalculate ROI intensity
%     maxx=size(obj.data.imageArray,1);
%     maxy=size(obj.data.imageArray,2);
%     if isfield(obj.data,'ROI')
%         obj.data.ROI.meanintensity=mean(obj.data.imageArray(obj.data.ROI.ind+(obj.data.ROI.z-1)*maxx*maxy))-obj.data.imagemedian;
%     end

    obj=set(obj,'state.display.displaybinary',0);
    obj=set(obj,'state.display.displayfiltered',0);
    obj=set(obj,'state.display.displayraw',1);
    obj=set(obj,'state.display.displaydendrites',1);
    obj=set(obj,'state.display.displayspines',1);    
    displayProjectionImage(obj);

    if (reanalyze==0) || (reanalyze==2)
        display('median filtering');
        obj=medianFilter(obj);
        %   obj=findprocess(obj);
        %   obj=ShiftDendriteMaxObj(obj,0);
        %   obj=CorrectDendrites(obj);
    end
    if (reanalyze==1) || (reanalyze==2)
        obj=ShiftDendriteMaxObj(obj,1);
        obj=CorrectDendrites(obj);
        obj=AxonProfile(obj);
    end
    
    obj=set(obj,'state.display.displaybinary',0);
    obj=set(obj,'state.display.displayfiltered',1);
    obj=set(obj,'state.display.displaydendrites',1);
    obj=set(obj,'state.display.displayraw',0);
    obj=set(obj,'state.display.displayspines',1);    
    displayProjectionImage(obj);
    obj=set(obj,'state.display.showprofile',1);
    obj=showprofile(obj);
    displayProfile(obj);  
catch
    disp(['error in batchprocessing']);
    disp(lasterr);
    disp(obj.state.rawImageName);
end
