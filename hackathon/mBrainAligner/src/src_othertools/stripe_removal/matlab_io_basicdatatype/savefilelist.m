function savefilelist(filelist, filename)
% savefilelist(filelist, filename)
% save a cell array of strings as a plain text file.
%
% By Hanchuan Peng
% June,2005

if ~isa(filelist, 'cell'),
  error('The first input is not a cell array'); 
end;

fid = fopen(filename, 'wt');
if fid==-1,
    disp(['Error to open the file : ' filename]);
    return;
else
    for i=1:length(filelist),
      if isa(filelist{i}, 'char'),
        fprintf(fid, '%s\n', filelist{i});
      end;
    end;
    fclose(fid);
end;
