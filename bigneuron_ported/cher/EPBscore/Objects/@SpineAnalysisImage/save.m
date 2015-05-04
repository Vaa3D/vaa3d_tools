function obj=save(obj)
if isfield(obj.state,'Path')
    [filename, pathname] = uiputfile([obj.state.Path,'\',obj.state.Name,'.spi'], 'Save Spine Analysis as');
else
    [filename, pathname] = uiputfile([obj.state.rawImagePath,'\',obj.state.rawImageName,'.spi'], 'Save Spine Analysis as');
end
%if (obj.data.
obj=savebyname(obj,[pathname,filename],'_edited');