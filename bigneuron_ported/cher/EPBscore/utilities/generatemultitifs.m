function generatemultitifs(directory)
cd(directory)
dirs=dir(directory);
for i=3:size(dirs,1)
cd(directory)
if dirs(i).isdir
    try
        cd (dirs(i).name);
        files=dir('*.tif');
        Aout=[];
        for j=1:size(files,1)
            Aout(:,:,j) = imread(files(j).name);
        end
        cd ..;
        Aout=uint16(Aout);
        imwrite(Aout(:,:,1),[dirs(i).name '.tif']);
        for j=2:size(Aout,3)
            imwrite(Aout(:,:,j),[dirs(i).name '.tif'],'WriteMode','append');
        end
    catch
    end 
end
end