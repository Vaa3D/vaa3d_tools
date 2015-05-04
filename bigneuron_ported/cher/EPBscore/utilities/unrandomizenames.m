function unrandomizenames
currentpath=pwd;
path =  uigetdir;
cd(path);
% recurse two levels
d=dir(path);
d=d([d.isdir]);
%permutation key
key1perm=[2     1     4     7     5     6     3];
key2perm=[ 10     3     4     7     1     8    11     2     6     5     9];
key1=[];
key1(key1perm)=1:7;
key2=[];
key2(key2perm)=1:11;
for i=3:max(size(d))
    cd(d(i).name);
    f=dir('*_edited.spi');
    d(i).name
    for j=1:max(size(f))
        oldname=f(j).name;
%         if size(f,1)==7
%             newind=key1(j);
%         else 
%             if size(f,1)==11
%                 newind=key2(j);
%             else
%                 disp(['ERROR:NOT RIGHT NUMBER OF FILES' d(i).name]);
%                 newind=0;
%                 break;
%             end
%         end
%         if newind>0
              [name status]=strtok(oldname,'_');
        if ~exist([name 'ziptemp'],'dir')
            [s,w]=system(['unzip -u ' name status ' -d ' '.\' name 'ziptemp']);
            if (s~=0)
                disp('Could not unzip data');
                close(h);
                return;
            end
        end
        fs=dir([name 'ziptemp\*.tif']);
        [name ext]=strtok(fs(1).name,'.');
        newname=[name status];
            if (~movefile(oldname,newname))
                disp(['ERROR:CANNOT RENAME ' oldname]);  
            end
            %end
        
    end
    cd('..');
end