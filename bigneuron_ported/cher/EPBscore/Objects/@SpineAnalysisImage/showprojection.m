function obj=showprojection(obj)
global self;
global ogh;
global objects;
obj=recordWindowPos(obj);
self=struct(obj);
if (self.state.display.showprojection==1)
    if ~(isfield(self.gh.projectionGUI,'Figure'))
        self.gh.projectionGUI=struct('Figure',figure);                % open new window for the image
        set(gca,'Position',[0 0 1 1]);
        set(gcf,'MenuBar','none','NumberTitle','off');
        set(self.gh.projectionGUI.Figure,'doublebuffer','on');
        % move window to appropriate place
        setappdata(self.gh.projectionGUI.Figure,'object',['objects([objects.index]==' num2str(self.ID) ').object']);           % associate the new window this this data structure
        basename=get(self.gh.mainGUI.Figure,'name');                   % associate it with the correct Name  
        set(self.gh.projectionGUI.Figure,'name',['projection:' basename]);
        
        % handle window closing gracefully
        set(self.gh.projectionGUI.Figure,'closeRequestFcn','global objects;obj=eval(getappdata(gcf,''object''));obj=set(obj,''state.display.showprojection'',0);showprojection(obj);');
        
        % handle window resizing gracefully
        %    set(self.gh.projectionGUI.Figure,'ResizeFcn','global objects;obj=eval(getappdata(gcf,''object''));obj=recordWindowPos(obj); eval([getappdata(gcf,''object'') ''=obj;'']);');
        
        obj=set(obj,'struct',self);
        % move it to the correct position
        obj=updateWindowPosByObj(obj);
        obj=recordWindowPos(obj);
    end
else
    if (isfield(self.gh.projectionGUI,'Figure'))
        obj=recordWindowPos(obj);
        delete(obj.gh.projectionGUI.Figure)
    end
    obj.gh.projectionGUI=[];    
end
ogh=self.gh;
updateGUIbyglobal('self.state.display.showprojection');
objects([objects.index]==obj.ID).object=obj;
