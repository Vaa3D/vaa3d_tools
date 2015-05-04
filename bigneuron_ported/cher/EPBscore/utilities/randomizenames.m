function randomizenames
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
    f=dir('*.spi');
    d(i).name
    for j=1:max(size(f))
        oldname=f(j).name;
        if size(f,1)==7
            newind=key1(j);
        else 
            if size(f,1)==11
                newind=key2(j);
            else
                disp(['ERROR:NOT RIGHT NUMBER OF FILES' d(i).name]);
                newind=0;
                break;
            end
        end
        if newind>0
            if strcmp(oldname(3),'0')
                newname=[oldname(1:5) 'rand' char(newind+96) oldname(7:end)];            
            else
                newname=[oldname(1:7) 'rand' char(newind+96) oldname(9:end)]; 
            end
            if (~movefile(oldname,newname))
                disp(['ERROR:CANNOT RENAME ' oldname]);  
            end
        end
        
    end
    cd('..');
end