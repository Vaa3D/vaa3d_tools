function fuse_markerCoordsToSwcNodes(inswcfile, inmarkerfile, outswcfile)
%function fuse_markerCoordsToSwcNodes(inswcfile, inmarkerfile, outswcfile)
% Merge the information of 3D point coordinate (from the marker file) and the radius/topology (from the inswcfile)
% and save to the output swc file.
%
% by Hanchuan Peng
% 2012-01-22

a = load_v3d_neuron_file(inswcfile);
b = load_v3d_marker_file(inmarkerfile);

if (size(a,1)~=size(b)),
  error('The marker file and the swc file do not have the same number of entries. Do nothing!');
  return;
end;
  
c = a;
for i=1:size(a,1),
  c(i,3) = b{i}.x;
  c(i,4) = b{i}.y;
  c(i,5) = b{i}.z;
end;

save_v3d_swc_file(c, outswcfile);
