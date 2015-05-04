function reanalyzespifromdb(targetdir,imagedir,db)
global objects;
for i=1:size(db.series,2)
    cd(targetdir);
    cd(db.series(i).names{1});    
    % generate the spi files
    for j=1:size(db.series(i).names,2)
        path=pwd;
        name=db.series(i).names{j};
        ext='.tif';
        
        % check to see if it exists
        if exist([db.series(i).names{j} '.spi'])
            try
                self=[];
                ogh=[];
                image=spineanalysisimageGUI;
                str=getappdata(image,'object');
                imageobj=eval(str);
                imageobj=set(imageobj,'state.display.dualchannel',0);
                imageobj=loadImageByNameGUI(imageobj,[db.series(i).names{j},'.spi']);
                imageobj=set(imageobj,'state.display.highpixel',200);
                % 2 means do both median filtering and dendrite/bouton
                % finding
                % 1 means dendrite finding only
                % 0 means median filtering only
                imageobj=BatchProcess(imageobj,2);
                imageobj=savebyname(imageobj,[path '\' db.series(i).names{j},'.spi'],'');        
                destroy(imageobj);   
            catch
                disp(['error when processing:' db.series(i).names{j}]);
                disp(lasterr);    
            end
        end
                
    end
    
%     % delete all image files from directory, we are done
%     DELETE('*.tif');
end
