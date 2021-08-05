function save_v3d_marker_file(m_struct, filename)
%function save_v3d_marker_file(m_struct, filename)
%
% Save the .marker format data file used in V3D
% 
% m_struct will consist of marker coordinates and name/comments/types/shape
%
% V3D website: see software page of http://penglab.janelia.org
%
% by Hanchuan Peng
% 20090723

fp = fopen(filename, 'w');
if (fp<0),
    disp('Fail to open the file to save V3D .marker format data');
    return;
end;

fprintf(fp, '#x, y, z, radius, shape, name, comment\n');

for i=1:length(m_struct),
  S = m_struct{i};  
  fprintf(fp, '%5.3f, %5.3f, %5.3f, %d, %d, %s, %s\n', ...
      S.x, ...
      S.y, ...
      S.z, ...
      S.radius, ...
      S.shape, ...
      trimmed_str(S.name, 99), ...
      trimmed_str(S.comment, 99)); 
end;

fclose(fp);

return;


