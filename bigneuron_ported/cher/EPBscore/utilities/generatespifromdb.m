function generatespifromdb(targetdir,imagedir,db)
global objects;
for i=1:size(db.series,2)
    cd(targetdir);
    
    %generate a directory for this series    
    if ~exist(db.series(i).names{1},'dir')
        mkdir(db.series(i).names{1});
    end
    cd(db.series(i).names{1});
    % gather relevant image files
    for j=1:size(db.series(i).names,2)
        [path,bytes,name]=dirr(imagedir, [db.series(i).names{j} '\.tif\>'],'name');
        if length(name)==0            
            break;
        end
        %skip if already processed
        if ~exist([db.series(i).names{j} '.tif']) && ~exist([db.series(i).names{j} '.spi'])
            [suc,msg]=copyfile(name{1},'.');
            if ~suc
                display(['error while copying: ' name{1} ' : ' msg]);
            end
        end
    end
    
    % generate the spi files
    for j=1:size(db.series(i).names,2)
        path=pwd;
        name=db.series(i).names{j};
        ext='.tif';
        % skip if already processed or if image does not exist
        if ~exist([db.series(i).names{j} '.spi']) && exist([db.series(i).names{j} '.tif'])
            
            try
                self=[];
                ogh=[];
                image=spineanalysisimageGUI;
                str=getappdata(image,'object');
                imageobj=eval(str);
                imageobj=set(imageobj,'state.display.dualchannel',0);
                imageobj=openImageByNameGUI(imageobj,[path,'\',name,ext]);
                imageobj=set(imageobj,'state.display.highpixel',200);
                %imageobj=BatchProcess(imageobj,0);
                
                % set the correct dendrite, spine, and marks
                denids=db.series(i).dendrites(:,j);
                d=db.dendrites([db.series(i).dendrites(:,j)]);
                imageobj= set(imageobj,'data.dendrites',d);
                
                s=db.spines([db.dendrites([denids]).spineuids]);
                imageobj= set(imageobj,'data.spines',s);
                
                % regenerate marks
                
                % try to spread the marks over the whole range
                maxlen=0.7*(db.series(i).maxdis-db.series(i).mindis);
                maxstd=max([db.dendrites([db.series(i).dendrites(:,j)]).stdmarks]);
                ratio=maxlen/(2*maxstd);
                lastmark=1;
                marks=[];
                for k=1:length(denids)
                    dendis=db.dendrites(denids(k)).meanmarks+[-0.9 -0.5 0 0.5 0.9]*db.dendrites(denids(k)).stdmarks*ratio;
                    for l=1:length(dendis)
                        marks(lastmark).dendis=dendis(l);
                        marks(lastmark).den_ind=k;
                        thisdendis=find(db.dendrites(denids(k)).length>dendis(l));
                        marks(lastmark).voxel=db.dendrites(denids(k)).voxel(1:3,thisdendis(1));
                        marks(lastmark).voxel(:,2)=marks(lastmark).voxel(:,1);
                        marks(lastmark).voxel(:,3)=marks(lastmark).voxel(:,1);
                        marks(lastmark).voxels=3;
                        marks(lastmark).type=0;
                        marks(lastmark).color=[0.7 0.7 0.7];
                        marks(lastmark).edittype=0;
                        marks(lastmark).label=lastmark;
                        lastmark=lastmark+1;
                    end
                end
                imageobj= set(imageobj,'data.marks',marks);
                
                % set marks
                
                imageobj=set(imageobj,'state.display.displayspines',1);
                imageobj=set(imageobj,'state.display.displaymarks',1);
                displayProjectionImage(imageobj);
                
                imageobj=savebyname(imageobj,[path,'\',name,'.spi'],'');
                destroy(imageobj);
                ogh=[];
                self=[];
            catch
                disp(['error in ' name]);
                disp(lasterr);
            end
        end
    end
    
%     % delete all image files from directory, we are done
%     DELETE('*.tif');
end
