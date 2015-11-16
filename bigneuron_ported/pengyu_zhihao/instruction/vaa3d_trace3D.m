% vaa3d_trace3D: sample code for vaa3d plugin
% Zhihao Zheng 10/2015
% zhihaozh@brandeis.edu
function result = vaa3d_trace3D(raw,unit_bytes,x,y,z,t,para,fileDir)
p1 = para(1);
p2 = para(2);
p3 = para(3);
swcOutputDir = [fileDir , '_sample.swc']
raw = double(raw);
unit_bytes = double(unit_bytes);
x = double(x);
y = double(y);
z = double(z);
t = double(t);
result = raw;
end