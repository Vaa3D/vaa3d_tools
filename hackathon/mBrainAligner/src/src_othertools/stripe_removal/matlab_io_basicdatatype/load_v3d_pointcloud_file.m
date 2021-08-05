function m_struct = load_v3d_pointcloud_file(filename)
%function m_struct = load_v3d_pointcloud_file(filename)
%
% Load the .apo point cloud format data file used in V3D
% 
% m_struct will consist of point cloud coordinates and other information
% (e.g. name/comments/types/shape)
%
% V3D website: see software page of http://penglab.janelia.org
%
% by Hanchuan Peng
% 20090724

m_struct = [];

L = loadfilelist(filename);

k = 0; % k is the real counter of markers
for i=1:length(L),
  curline = deblank(L{i});
  if isempty(curline),
    continue;
  end;

  if (curline(1)=='#') 
    continue;
  end;

  ipos = find(curline==',');
  cnt = length(ipos)+1;
  if (cnt<12),
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

  S.n = round(str2num(segs{1}));

  S.orderinfo = '';
  S.name = '';
  S.comment = '';
  S.z = -1;
  S.x = -1;
  S.y = -1;
  S.pixmax = 0;
  S.intensity = 0;
  S.sdev = 0;
  S.volsize = 0;
  S.mass = 0;
  rr = rand(3,1);
  rr = rr./sqrt(sum(rr.*rr)).*255;
  S.color.r = round(rr(1));
  S.color.g = round(rr(2));
  S.color.b = round(rr(3));

  if (cnt>=2), S.orderinfo = segs{2}; end;
  if (cnt>=3), S.name = segs{3}; end;
  if (cnt>=4), S.comment = segs{4}; end;
  if (cnt>=5), S.z = str2num(segs{5}); end;
  if (cnt>=6), S.x = str2num(segs{6}); end;
  if (cnt>=7), S.y = str2num(segs{7}); end; 
  if (cnt>=8), S.pixmax = str2num(segs{8}); end;
  if (cnt>=9), S.intensity = str2num(segs{9}); end;
  if (cnt>=10), S.sdev = str2num(segs{10}); end;
  if (cnt>=11), S.volsize = str2num(segs{11}); end;
  if (cnt>=12), S.mass = str2num(segs{12}); end; 
  if (cnt>=16), S.color.r = round(str2num(segs{16})); end;
  if (cnt>=17), S.color.g = round(str2num(segs{17})); end;
  if (cnt>=18), S.color.b = round(str2num(segs{18})); end;
  
  
  %% now assign m_struct
  k = k+1;
  m_struct{k} = S;

end;

return;


