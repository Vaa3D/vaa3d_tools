function obj=save(obj)
[filename, pathname] = uiputfile([obj.state.rawImagePath,'\',obj.state.rawImageName,'.spi'], 'Save Spine Analysis as');
[path,name,ext] = fileparts([pathname filename]);
self=struct(obj);
try
    mkdir(pathname, 'ziptemp');
    if ~isempty(obj.data.imageArray)
        arraytotiff(obj.data.imageArray,[pathname '\ziptemp\' name],'');
        self.data.imageArray=[];
    end
    if ~isempty(obj.data.filteredArray)
        arraytotiff(obj.data.filteredArray,[obj.state.rawImagePath '\ziptemp\' name 'fil'],'');
        self.data.filteredArray=[];
    end
%     if ~isempty(obj.data.binaryArray)   % needs work!
%         arraytotiff(obj.data.binaryArray,[obj.state.rawImagePath '\ziptemp\' obj.state.rawImageName 'bin.tif'],'');
%         self.data.binaryArray=[];
%     end
%     if ~isempty(obj.data.burnArray)
%         arraytotiff(obj.data.burnArray,[obj.state.rawImagePath '\ziptemp\' obj.state.rawImageName 'burn.tif'],'');
%         self.data.burnArray=[];
%     end
    save([obj.state.rawImagePath '\ziptemp\' obj.state.rawImageName '.dat'],'self');
    zip([pathname filename '.spi'],[obj.state.rawImagePath '\ziptemp']);
    rmdir([obj.state.rawImagePath '\ziptemp'],'s');
catch
    display('Could not save data');
    display(lasterr);
    return;
end