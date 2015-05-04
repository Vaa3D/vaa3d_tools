function reanalyzeallinfolder(targetdir)
global objects;
cd(targetdir);
dirs=dir('.');
for i=3:size(dirs,1)
    cd(targetdir);
    if dirs(i).isdir
        cd (dirs(i).name);
        spifiles=dir('*.spi');
        for j=1:size(spifiles,1)
            try
                self=[];
                ogh=[];
                image=spineanalysisimageGUI;
                str=getappdata(image,'object');
                imageobj=eval(str);
                imageobj=set(imageobj,'state.display.dualchannel',0);
                imageobj=loadImageByNameGUI(imageobj,[spifiles(j).name]);
                imageobj=set(imageobj,'state.display.highpixel',200);
                % 2 means do both median filtering and dendrite/bouton
                % finding
                % 1 means dendrite finding only
                % 0 means median filtering only
                imageobj=BatchProcess(imageobj,2);
                path=pwd;
                imageobj=savebyname(imageobj,[path '\' spifiles(j).name ],'');
                destroy(imageobj);
            catch ME
                disp(['error when processing:' spifiles(j).name]);
                disp(ME);
            end
        end
        
        tiffiles=dir('*.tif');
        for j=1:size(tiffiles,1)
            t=tiffiles(j).name
            if ~exist([t(1:end-4) '.spi'])
                try
                    self=[];
                    ogh=[];
                    image=spineanalysisimageGUI;
                    str=getappdata(image,'object');
                    imageobj=eval(str);
                    imageobj=set(imageobj,'state.display.dualchannel',0);
                    imageobj=openImageByNameGUI(imageobj,[pwd '\' t]);
                    imageobj=set(imageobj,'state.display.highpixel',200);
                    % 2 means do both median filtering and dendrite/bouton
                    % finding
                    % 1 means dendrite finding only
                    % 0 means median filtering only
                    imageobj=BatchProcess(imageobj,0);
                    path=pwd;
                    imageobj=savebyname(imageobj,[path '\' t(1:end-4) '.spi'],'');
                    destroy(imageobj);
                catch ME
                    disp(['error when processing:' t]);
                    disp(ME);
                end
            end
        end
    end
end
