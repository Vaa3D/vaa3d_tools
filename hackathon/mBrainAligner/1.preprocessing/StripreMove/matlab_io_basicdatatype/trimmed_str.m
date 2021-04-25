function out_str = trimmed_str(in_str, n)
% function out_str = trimmed_str(in_str, n)
% this function is to ensure there are at most n characterz in the str, so
% that later on the .apo and .marker file can be successfully erad in V3D (there is a
% max # character in a line limitation)
%
% by Hanchuan Peng
% 2009-07-25

if (n<0), n=0; end;
n0 = length(in_str);
if (n0>n), n0=n; end;
out_str = in_str(1:n0);
return;

