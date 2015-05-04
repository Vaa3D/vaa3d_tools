function obj=loadImageByName(obj,filepath)
[path,name,ext] = fileparts(filepath);
global self;
try
    mkdir(path, 'ziptemp');
    unzip([pname '\' fname],[path]);
    datfile=dir([path 'ziptemp\*.dat']);
    load('-mat',[path 'ziptemp\' datfile(1).name]);
    % kludge for now
    
    if ~isfield(obj.data,'marks')
        obj.data.marks=[];
    end
    
        
    % end
    
    try
        self.data.imageArray=opentif([path '\ziptemp\' self.state.rawImageName '.tif']);  % get the image
    catch
        display('warning: no rawImage found');
    end
    try
        self.data.filteredArray=opentif([path '\ziptemp\' self.state.rawImageName 'fil.tif']);  % get the filtered image
    catch
        display('warning: no filterImage found');
    end
    
    %     if ~isempty(obj.data.binaryArray)   % needs work!
    %         arraytotiff(obj.data.binaryArray,[obj.state.rawImagePath '\ziptemp\' obj.state.rawImageName 'bin.tif'],'');
    %         self.data.binaryArray=[];
    %     end
    %     if ~isempty(obj.data.burnArray)
    %         arraytotiff(obj.data.burnArray,[obj.state.rawImagePath '\ziptemp\' obj.state.rawImageName 'burn.tif'],'');
    %         self.data.burnArray=[];
    %     end
    
    %rmdir([path 'ziptemp'],'s');
    
    
    % Initialize the application appropriately.
    
    % close all windows
    if (isfield(obj.gh.projectionGUI,'Figure'))
        close(obj.gh.projectionGUI.Figure)
    end
    obj.gh.projectionGUI=[];    
    if (isfield(obj.gh.imageGUI,'Figure'))
        close(obj.gh.imageGUI.Figure)
    end
    obj.gh.imageGUI=[];    
    if (isfield(obj.gh.parametersGUI,'Figure'))
        close(obj.gh.parametersGUI.Figure)
    end
    obj.gh.parametersGUI=[];   
    
    % reset the values to initial state
    
    global ogh;
    self.ID=obj.ID;
    ogh=obj.gh;
    % initialize the graphical handles correctly
    for field=fieldnames(ogh)'
        self.gh.(field{1})=ogh.(field{1});
    end
    self.state.rawImagePath='';
    self.state.rawImageName=name;
    set(self.gh.mainGUI.Figure,'name',[name '-' path '\' name]);
    obj=set(obj,'struct',self);    % generic object assigning part
    updateGUIbyglobalAll('self.state.display');
    zslider=getGuiOfGlobal('self.state.display.currentz');
    zslider=eval(zslider{1});
    set(zslider,'Min',self.state.ROI.minz,'Max',self.state.ROI.maxz,'SliderStep',[1/(self.state.ROI.maxz-self.state.ROI.minz) 1/(self.state.ROI.maxz-self.state.ROI.minz)]);
catch
    display('Could not open data');
    display(lasterr);
    return;
end
