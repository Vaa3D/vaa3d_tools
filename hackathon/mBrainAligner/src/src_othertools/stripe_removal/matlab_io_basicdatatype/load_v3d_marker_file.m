function m_struct = load_v3d_marker_file(filename)
%function m_struct = load_v3d_marker_file(filename)
%
% Load the .marker format data file used in V3D
% 
% m_struct will consist of marker coordinates and name/comments/types/shape
%
% V3D website: see software page of http://penglab.janelia.org
%
% by Hanchuan Peng
% 20090723

m_struct = [];

L = loadfilelist(filename);

k = 0; % k is the real counter of markers
for i=1:length(L),
  curline = deblank(L{i});
  if isempty(curline),
    continue;
  end;

  if (curline(1)=='#' | curline(1)=='x' | curline(1)=='X') 
    continue;
  end;

  ipos = find(curline==',');
  cnt = length(ipos)+1;
  if (cnt<3),
    continue;
  end;

  %% get all segments
  segs = [];
  segs{1} = curline(1:ipos(1)-1);
  for j=2:cnt-1,
    segs{j} = curline(ipos(j-1)+1:ipos(j)-1);
  end;
  segs{j+1} = curline(ipos(cnt-1)+1:end);

  %% now assign values
  S.x = str2num(segs{1});
  S.y = str2num(segs{2});
  S.z = str2num(segs{3});

  S.radius = 0;
  S.shape = 1;
  S.name = '';
  S.comment = '';

  if (cnt>=4), S.radius = str2num(segs{4}); end;
  if (cnt>=5), S.shape = str2num(segs{5}); end;
  if (cnt>=6), S.name = segs{6}; end;
  if (cnt>=7), S.comment = segs{7}; end;

  %% now assign m_struct
  k = k+1;
  m_struct{k} = S;

end;

return;


