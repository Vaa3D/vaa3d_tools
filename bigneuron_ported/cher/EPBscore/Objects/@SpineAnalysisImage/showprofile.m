function obj=showprofile(obj)
global self;
global ogh;
global objects;
obj=recordWindowPos(obj);
self=struct(obj);
if (self.state.display.showprofile==1)
    if ~(isfield(self.gh.profileGUI,'Figure'))
        self.gh.profileGUI=struct('Figure',figure);                % open new window for the image
        set(self.gh.profileGUI.Figure,'doublebuffer','on');
        set(gcf,'MenuBar','none','NumberTitle','off');
        % move window to appropriate place
        setappdata(self.gh.profileGUI.Figure,'object',['objects([objects.index]==' num2str(self.ID) ').object']);           % associate the new window this this data structure
        basename=get(self.gh.mainGUI.Figure,'name');                   % associate it with the correct Name  
        set(self.gh.profileGUI.Figure,'name',['profile:' basename]);
        
        % handle window closing gracefully
        set(self.gh.profileGUI.Figure,'closeRequestFcn','global objects;obj=eval(getappdata(gcf,''object''));obj=set(obj,''state.display.showprofile'',0);showprofile(obj);');
        
        % handle window resizing gracefully
        %    set(self.gh.profileGUI.Figure,'ResizeFcn','global objects;obj=eval(getappdata(gcf,''object''));obj=recordWindowPos(obj); eval([getappdata(gcf,''object'') ''=obj;'']);');
        
        obj=set(obj,'struct',self);
        % move it to the correct position
        obj=updateWindowPosByObj(obj);
        obj=recordWindowPos(obj);
    end
else
    if (isfield(self.gh.profileGUI,'Figure'))
        obj=recordWindowPos(obj);
        delete(obj.gh.profileGUI.Figure)
    end
    obj.gh.profileGUI=[];    
end
ogh=self.gh;
updateGUIbyglobal('self.state.display.showprofile');
objects([objects.index]==obj.ID).object=obj;
