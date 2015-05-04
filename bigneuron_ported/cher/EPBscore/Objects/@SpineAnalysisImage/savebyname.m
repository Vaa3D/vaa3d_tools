function obj=savebyname(obj,filepath,status)
if (filepath)
    [path,name,ext] = fileparts(filepath);
else
    path=obj.state.Path;
    name=obj.state.Name;
end
rawname=obj.state.rawImageName;
self=struct(obj);
try
     save([path '\'  name '.spi'],'self');
%     warning off MATLAB:MKDIR:DirectoryExists;
%     mkdir(path, [name 'ziptemp']);
%     if ~isempty(obj.data.imageArray) && ~exist([path '\' name 'ziptemp\' rawname '.tif'])
%         arraytotiff(obj.data.imageArray,[path '\' name 'ziptemp\' rawname],'');
%         self.data.imageArray=[];
%     end
%     if ~isempty(obj.data.imagedualArray) && ~exist([path '\' name 'ziptemp\' rawname 'dual.tif'])
%         arraytotiff(obj.data.imagedualArray,[path '\' name 'ziptemp\' rawname 'dual'],'');
%         self.data.imagedualArray=[];
%     end
% 
%     if ~isempty(obj.data.filteredArray) && ( ~exist([path '\' name 'ziptemp\' rawname '.tif']) || (isfield(obj.state,'refiltered') && obj.state.refiltered))
%         arraytotiff(obj.data.filteredArray,[path '\' name 'ziptemp\' rawname 'fil'],'');
%         self.data.filteredArray=[];
%     end
%     if ~isempty(obj.data.filtereddualArray) && ( ~exist([path '\' name 'ziptemp\' rawname '.tif']) || (isfield(obj.state,'refiltered') && obj.state.refiltered))
%         arraytotiff(obj.data.filtereddualArray,[path '\' name 'ziptemp\' rawname 'fildual'],'');
%         self.data.filteredArray=[];
%     end
% %     if isfield(obj.data,'gaussianArray') && ~isempty(obj.data.gaussianArray) && ( ~exist([path '\' name 'ziptemp\' rawname '.tif']) || (isfield(obj.state,'refiltered') && obj.state.refiltered))
% %         arraytotiff(obj.data.gaussianArray,[path '\' name 'ziptemp\' rawname 'gauss'],'');
% %         self.data.gaussianArray=[];
% %     end
% %     if ~isempty(obj.data.binaryArray)   % needs work!
% %         arraytotiff(obj.data.binaryArray,[obj.state.rawImagePath '\ziptemp\' obj.state.rawImageName 'bin.tif'],'');
% %         self.data.binaryArray=[];
% %     end
% %     if ~isempty(obj.data.burnArray)
% %         arraytotiff(obj.data.burnArray,[obj.state.rawImagePath '\ziptemp\' obj.state.rawImageName 'burn.tif'],'');
% %         self.data.burnArray=[];
% %     end
%     self.data.binaryArray=[];
%     self.data.burnArray=[];
%     self.data.gaussianArray=[];
%     self.data.imageArray=[];
%     self.data.correctedArray=[];
%     self.data.filteredArray=[];
%     save([path '\' name 'ziptemp\' rawname '.dat'],'self');
%     try
%         summaryfigure(obj);
%         saveas(gcf, [path '\' name 'ziptemp\' rawname 'summary.fig'], 'fig');
%         close(gcf);
%     catch
%         display(lasterr);
%     end
%     h = waitbar(0,'Zipping Spi File...', 'Name', 'Zipping Spi File', 'Pointer', 'watch');
%     waitbar(0,h, ['Zipping Spi File...']);
%     try
%         cd ([path '\' name 'ziptemp\']);
% %         [s,w]=system(['zip -3u ' path '\' name status '.spi ' '*.*']);
% %         if (s~=0)
% %              disp('Could not zip data');
% %          end
%         
%         zip([path '\' name status '.spi'],[path '\' name 'ziptemp\*.*']);
%     catch
%         disp('Could not zip data');
%     end     
%     waitbar(1,h, 'Done');
%     close(h);
%     %     try
%     %         rmdir([path '\' name 'ziptemp'],'s');
% %     catch
% %         display('Could not delete temporary files');
% %         display(lasterr);
% %     end
catch
    disp('Could not save data');
    disp(lasterr);
end