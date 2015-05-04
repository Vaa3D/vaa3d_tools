function obj=showstack(obj)
global self;
global ogh;
global objects;
obj=recordWindowPos(obj);
self=struct(obj);
if (self.state.display.showstack==1)
    if ~(isfield(self.gh.imageGUI,'Figure'))
        self.gh.imageGUI=struct('Figure',figure);                % open new window for the image
        set(self.gh.imageGUI.Figure,'doublebuffer','on');
        set(gca,'Position',[0 0 1 1]);
        set(gcf,'MenuBar','none','NumberTitle','off');
        basename=get(self.gh.mainGUI.Figure,'name');                   % associate it with the correct Name  
        set(self.gh.imageGUI.Figure,'name',basename);
        setappdata(self.gh.imageGUI.Figure,'object',['objects([objects.index]==' num2str(self.ID) ').object']);           % associate the new window this this data structure
        
        % handle window closing gracefully
        set(self.gh.imageGUI.Figure,'closeRequestFcn','global objects;obj=eval(getappdata(gcf,''object''));obj=set(obj,''state.display.showstack'',0);showstack(obj);');
        
        % handle window resizing gracefully
        % set(self.gh.imageGUI.Figure,'ResizeFcn','global objects;obj=eval(getappdata(gcf,''object''));obj=recordWindowPos(obj); eval([getappdata(gcf,''object'') ''=obj;'']);');
        
        obj=set(obj,'struct',self);
        % move it to the correct position
        obj=updateWindowPosByObj(obj);
        obj=recordWindowPos(obj);
    end
else
    if (isfield(self.gh.imageGUI,'Figure'))
        obj=recordWindowPos(obj);
        delete(obj.gh.imageGUI.Figure)
    end
    obj.gh.imageGUI=[];    
end
ogh=self.gh;
updateGUIbyglobal('self.state.display.showstack');
objects([objects.index]==obj.ID).object=obj;
