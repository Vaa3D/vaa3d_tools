function cleandir
currentpath=pwd;
path =  uigetdir;
cd(path);
% recurse two levels
d=dir(path);
d=d([d.isdir]);
%permutation key
for i=3:max(size(d))
    cd(d(i).name);
    d1=dir(pwd);
    d1=d1([d1.isdir]);
    for j=3:max(size(d1))
        rmdir(d1(j).name,'s');
    end
    cd ('..');
end