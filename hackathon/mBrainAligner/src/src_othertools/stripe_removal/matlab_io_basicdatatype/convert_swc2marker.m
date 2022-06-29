function convert_swc2marker(inswcfile, outmarkerfile)
%function convert_swc2marker(inswcfile, outmarkerfile)
% by Hanchuan Peng
% 2012-01-22

a = load_v3d_neuron_file(inswcfile);
for i=1:size(a,1),
  c.x = a(i,3);
  c.y = a(i,4);
  c.z = a(i,5);
  c.radius = a(i,6);
  c.shape = 1;
  c.name = '';
  c.comment = '';

  b{i} = c;
end;

save_v3d_marker_file(b, outmarkerfile);
