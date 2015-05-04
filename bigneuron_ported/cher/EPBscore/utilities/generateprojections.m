function generateprojections

global objects;
global ogh;

for z=1:1 %get the list of directories
    [path,name,ext] = fileparts('d:\clustering\spi\1.spi');
    pathnames=dir([path '\*.spi']);     
    for i=1:max(size(pathnames))
        [p,name,ext] = fileparts(pathnames(i).name);
        [name,status]=strtok(name,'_');
        if (strcmp(ext,'.spi'))      
            try
                self=[];
                ogh=[];
                image=spineanalysisimageGUI;
                str=getappdata(image,'object');
                imageobj=eval(str);
                imageobj=loadImageByNameGUI(imageobj,[path,'\',name,status,ext]);
                imageobj=set(imageobj,'state.display.highpixel',200);
                try
                    maxproj=max(get(imageobj,'data.imageArray'),[],3);
                     arraytotiff(maxproj,[path '\' name 'max'],'');
                catch
                    disp(['error processing:' name]);
                    disp(lasterr);
                end
               
                destroy(imageobj);
                ogh=[];
                %set(obj.gh.mainGUI.listbox,'String',pathnames);
            catch
                disp(['Could not open file' name ':' lasterr]);
            end        
        end
    end;
   
end
