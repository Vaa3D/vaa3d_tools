function collectfiles
currentpath=pwd;
path =  uigetdir;
% recurse two levels
d=dir(path);
d=d([d.isdir]);
cd (path);
for i=3:max(size(d))
    cd(d(i).name);
    % recurse again
    % first try to find *b0??.spi edited
    f=dir('*b0*edited.spi');
    if size(f,1)>0
        copyfile (f(1).name, currentpath);
    else
        f=dir('*b0*.spi');
        if size(f,1)>0
            copyfile(f(1).name, currentpath); 
        else
            % then try to find other .spi files
            f=dir('*0*.spi');
            if size(f,1)>0
                copyfile(f(1).name, currentpath);               
            else
                f=dir('*b0??.tif');
                if (size(f,1)>0)
                    copyfile(f(1).name, currentpath);        
                end
            end
        end
    end
    cd('..');
end