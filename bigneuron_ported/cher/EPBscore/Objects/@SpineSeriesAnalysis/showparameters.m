function obj=showparameters(obj)
global self;
global ogh;
self=struct(obj);
 if (self.state.display.showparameters==1)
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
    else
        if (isfield(self.gh.parametersGUI,'Figure'))
            close(self.gh.parametersGUI.Figure)
        end
        self.gh.parametersGUI=[];    
    end
    obj=set(obj,'struct',self);
