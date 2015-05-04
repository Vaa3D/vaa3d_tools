function obj=showparameters(obj)
global self;
global ogh;
global objects;
obj=recordWindowPos(obj);
self=struct(obj);
%ogh=self.gh;
if (self.state.display.showparameters==1)
    if ~(isfield(self.gh.parametersGUI,'Figure'))
        parametersfig=openfig('spineanalysisparameters.fig','new');
        self.gh.parametersGUI=guihandles(parametersfig);
        self.gh.parametersGUI.Figure=parametersfig;
        setappdata(self.gh.parametersGUI.Figure,'object',['objects([objects.index]==' num2str(self.ID) ').object']);           % associate the new window this this data structure
        basename=get(self.gh.mainGUI.Figure,'name');                   % associate it with the correct Name  
        set(self.gh.parametersGUI.Figure,'name',['parameters' basename]);
        ogh=self.gh;
        backself=self;
        initGUIs('standardspineanalysisparameters.ini');
        self.parameters=backself.parameters;
        % update parameters
        names=fieldnames(self.parameters.threedma);
        for i=1:size(names,1)
            updateGUIbyglobal(['self.parameters.threedma.' names{i}]);
        end
        
        % handle window closing gracefully
        set(self.gh.parametersGUI.Figure,'closeRequestFcn','global objects;objstr=getappdata(gcf,''object'');obj=eval(objstr);obj=set(obj,''state.display.showparameters'',0);showparameters(obj);');
        
        % handle window resizing gracefully
        %    set(self.gh.parametersGUI.Figure,'ResizeFcn','global objects;obj=eval(getappdata(gcf,''object''));obj=recordWindowPos(obj); eval([getappdata(gcf,''object'') ''=obj;'']);');
        
        obj=set(obj,'struct',self);
        % move it to the correct position
        obj=updateWindowPosByObj(obj);
        obj=recordWindowPos(obj);
    end
else
    if (isfield(self.gh.parametersGUI,'Figure'))
        obj=recordWindowPos(obj);
        delete(obj.gh.parametersGUI.Figure)
    end
    obj.gh.parametersGUI=[];    
end
ogh=self.gh;
updateGUIbyglobal('self.state.display.showparameters');
objects([objects.index]==obj.ID).object=obj;
