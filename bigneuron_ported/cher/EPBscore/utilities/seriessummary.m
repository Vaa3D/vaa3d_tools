function seriesummary(db,seriesuid)
dendrites=db.dendrites([db.dendrites.seriesuid]==seriesuid);
ids=unique([dendrites.index]);
for i=1:size(ids,2)
    overdayden=dendrites([dendrites.index]==ids(i));    
   
    minlen=min([overdayden.overdaylen]);
    maxlen=max([overdayden.overdaylen]);
    x=floor(minlen):0.25:ceil(maxlen);
    worldline=[];
    for j=1:size(overdayden,2)
        worldline=[worldline;overdayden(j).worldline']; 
    end
    worldline=worldline';
    worldline(worldline<=-0.9)=-0.9;
   
    figure;
    correctedworldine=worldline;
    correctedworldline(worldline<=-0.9)=-0.9;
    imagesc(log(correctedworldline+1));
     
    for j=1:size(overdayden,2)
        for k=1:size([overdayden(j).spineuids],2)
            id=[db.spines.uid]==overdayden(j).spineuids(k);
            h=text(j,3+(db.spines(id).overdaydis-minlen)/0.25,num2str(db.spines(id).label));
            set(h,'Color',[ 1 1 1]);
        end
    end
    figure;
    subplot(2,1,2);
    hold on;
    ibs=[];
    itbs=[];
    for j=1:size(overdayden,2)
        h=plot(x,worldline(:,j)+1);
        c=hsv2rgb([j/size(overdayden,2) 1 1]);
        set(h,'Color',c);
        for k=1:size([overdayden(j).spineuids],2)
            id=[db.spines.uid]==overdayden(j).spineuids(k);
            h=text(db.spines(id).overdaydis,overdayden(j).maxintcorrected(db.spines(id).dendis)+1.5,num2str(db.spines(id).label));
            set(h,'Color',c);
        end
        ibs=[ibs overdayden(j).ibs];
        itbs=[itbs overdayden(j).ibsterminal];
    end
    xlabel(['ibs' strnum(mean(ibs)) ' itbs' strnum(itbs)]);
    subplot(2,1,1);
    [n,day]=min(ibs);
    try
        [path,name,ext] = fileparts([char(pathnames(day))]);
        if (strcmp(ext,'.spi'))
            mkdir(path, [name 'ziptemp']);
            h = waitbar(0,'Unzipping Spi File...', 'Name', 'Unzipping Spi File', 'Pointer', 'watch');
            waitbar(0,h, ['Unzipping Spi File...']);
            if exist([path '\' name 'ziptemp'],'dir')
                tiffile=dir([path '\' name 'ziptemp\*.tif']);
                filtif=dir([path '\' name 'ziptemp\*.tif']);
                datfile=dir([path '\' name 'ziptemp\*.dat']);
                if ~((size(tiffile,1)>=2) & (size(filtif,1)>=1) & (size(datfile,1)>=1))
                    infounzip([path '\' name ext],[path '\' name 'ziptemp']);
                end
            else
                infounzip([path '\' name ext],[path '\' name 'ziptemp']);
            end
            try
                cd ([path '\' name 'ziptemp\ziptemp']);
                copyfile('*.*','..');
            catch
                
            end
            waitbar(1,h, 'Done');
            close(h);
            
            cd (path);
            filtif=dir([path '\' name 'ziptemp\*.tif']);
            imageArray=opentif(filtif(1));       
            upperbound=max(overdayden(day).dendrites.meanmedianfiltered)*3;
            h=imagesc(collapse(imageArray,'XY'),[0 upperbound]);
        end
    catch
        disp('error generating preview image for series');
        disp(lasterr);
    end
end