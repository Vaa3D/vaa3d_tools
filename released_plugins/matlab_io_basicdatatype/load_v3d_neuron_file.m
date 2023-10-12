function a = load_v3d_neuron_file(filename, b_minusFirst)
%function a = load_v3d_neuron_file(filename, b_minusFirst)
% Load the swc file as a neuron structure
% by Hanchuan Peng
% 070726
% 070809: I notice the majority of neuron swc files have their soma have
% the corredinates (0,0,0), but there are still exceptions. Thus I subtract
% the soma coordinate to assure any neurons have the same origin.
% 080513: add a b_minusFirst flag
% 0904: add a get first 7 columns check
% 090415: add a deblank check

if nargin<2,
    b_minusFirst=0;
end;

L = loadfilelist(filename);
a = zeros(length(L), 7);

k=0;
for i=1:length(L),
    if isempty(deblank(L{i})),
        continue;
    end;
    if (L{i}(1)=='#'),
        continue;
    end;
    
    k=k+1;
    tmp = str2num(L{i});
    a(k,:) = tmp(1:7);
end;

a = a(1:k,:); %%remove the non-used lines

%make sure all the origin (neuron soma) will be 0
if b_minusFirst,
    a(:,3:5) = a(:,3:5) - repmat(a(1,3:5), size(a,1), 1);
end;

return;

