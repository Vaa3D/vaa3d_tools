function obj= saveseries(obj)
seriesdata= getappdata(gcbf,'seriesdata');
[filename, pathname] = uiputfile([seriesdata.pathname,'.ser'], 'Save Spine Series Analysis as');
try
save([pathname filename '.ser'],'seriesdata');
catch
    print('Could not save analysis');
    return;
end
