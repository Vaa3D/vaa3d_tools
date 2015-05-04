function renaming
currentpath=pwd;
path =  uigetdir;
cd(path);
% recurse two levels
d=dir(path);
d=d([d.isdir]);
%permutation key
key1=[2     1     4     7     5     6     3];
key2=[ 10     3     4     7     1     8    11     2     6     5     9];
for i=3:max(size(d))
    cd(d(i).name);
    d(i).name
    d1=dir;
    d1=d1([d1.isdir]);
    for k=3:max(size(d1))
        cd(d1(k).name);
        f=dir('*.spi');
        d1(k).name
        for j=1:size(f,1)
            oldname=f(j).name;
            newname=[oldname(1:end-4) '.zip'];
             if (~movefile(oldname,newname))
                disp(['ERROR:CANNOT RENAME ' oldname]);  
            end
        end
        cd('..');
           
    end
    cd('..');
end