function summarygraphshort(overdayden,positions,terminalpositions,daypositions,dayterminalpositions,acrossday,terminalacrossday,stablethres,lastlabel,a,b,pathnames,daystartpos,dayendpos)
figure;
maximize(gcf);
try
    ibs=[];
    itbs=[];
    icbs=[];
    for j=1:size(overdayden,2)
        if isfinite(overdayden(j).ibs)
            ibs=[ibs overdayden(j).ibs];
        end
        if isfinite(overdayden(j).ibsterminal)
            itbs=[itbs overdayden(j).ibsterminal];
        end
        icbs=[icbs overdayden(j).totallength/size([overdayden(j).spineuids],2)];        
    end
catch
    disp('error calculting ibs');
end

try
    meanbackbone=mean([overdayden.meanback]./a);
    minlen=min([overdayden.overdaylen]);
    maxlen=max([overdayden.overdaylen]);
    
    a=1./a;
    worldline=[];
    worldlen=[];
    warning off MATLAB:divideByZero;
    x=floor(minlen):0.25:ceil(maxlen);
    for j=x
        thisline=zeros(1,size(overdayden,2));
        thislen=zeros(1,size(overdayden,2));
        
        for k=1:size(overdayden,2)
            s=reflectivesmooth(double(overdayden(k).maxintraw),7);
            points=(overdayden(k).overdaylen<j+0.25)&(overdayden(k).overdaylen>=j);
            
            thisline(k)=(mean(s(points))/a(k))/meanbackbone;
            thislen(k)=mean(overdayden(k).length(points));
        end
        worldline=[worldline;thisline];
        worldlen=[worldlen;thislen];
    end
    
 
    
catch
    disp('error generating worldline');
    disp(k);    
end
    
try
    a=([overdayden.meanback]);
    subplot(1,3,[2 3]);
    hold on;
    meanbackbone=mean([overdayden.meanback]./a);
    
    % find the big boutons
    
    scaledacrossday=acrossday./(a'*ones(1,size(acrossday,2)));
    bigboutons=(sum(scaledacrossday>stablethres)==size(scaledacrossday,1));
    
    for j=1:size(overdayden,2)
        h=plot(overdayden(j).overdaylen,overdayden(j).maxint/(a(j)*meanbackbone));
        c=hsv2rgb([(j+1)/(size(overdayden,2)+1) 1 1]);
        set(h,'Color',c);
        for k=1:max(size(positions)) 
            h=text(positions(k)-0.5, 0,num2str(k+lastlabel));
            if (bigboutons(k))
                set(h,'Color',[0 0 1]);
            else
                 set(h,'Color',[0 1 0]);
             end
        end
        for k=1:max(size(positions))
            if (daypositions(j,k)>0)
                h=plot(daypositions(j,k), acrossday(j,k+lastlabel)/(a(j)*meanbackbone),'+');
            else
                h=plot(positions(k), acrossday(j,k+lastlabel)/(a(j)*meanbackbone),'+');
            end
            set(h,'Color',c,'MarkerSize',3);
        end
        for k=1:max(size(terminalpositions))
            if (terminalacrossday(j,k)>0)
                h=plot(dayterminalpositions(j,k), terminalacrossday(j,k)/(a(j)*meanbackbone),'.');
                set(h,'Color',c);
            end
            h=text(terminalpositions(k)-0.5,-0.8,num2str(k+lastlabel+max(size(positions))));
            set(h,'Color',[1 0 0]);
        end
        %                for k=1:size([overdayden(j).spineuids],2)
        %                    id=[db.spines.uid]==overdayden(j).spineuids(k);
        %            [mdis,xind]=min(abs(x-db.spines(id).overdaydis));
        %                    if (db.spines(id).len>0)
        %                        h=plot(db.spines(id).overdaydis,db.spines(id).intensity/overdayden(j).meanbouton,'+');
        %                        set(h,'Color',c);
        %                    end
        %            h=text(conversion*db.spines(id).overdaydis,overdayden(j).maxintcorrected(db.spines(id).dendis)+1.5,num2str(db.spines(id).label));
        %                    if (db.spines(id).edittype<3)
        %                        h=text(db.spines(id).overdaydis,db.spines(id).intensity/overdayden(j).meanbouton+0.5,num2str(db.spines(id).label));
        %                        set(h,'Color',c);
        %                    end
        %                end
    end
    
    
    subplot(1,3,1);
    [n,day]=min(icbs);
    plotdailysummary(day,pathnames,overdayden,daypositions,positions,lastlabel);
    days=size(overdayden,2);
    
    % largest bouton size
    m=[];
    for day=1:length(overdayden)
        m=[m overdayden(day).maxint(daystartpos(day):dayendpos(day))/(a(day)*meanbackbone)];
    end
    maxbouton=max(m);
    
    
    figure;
    for i=1:4
        if i<=days
            x=rem(i-1,2);
            y=1-floor((i-1)/2);
            subplot('position',[x*0.5 y*0.5+0.03 0.49 0.45]);
            plotdailysummary(i,pathnames,overdayden,daypositions,positions,lastlabel);
            axis off;
        end
    end
    
    figure;
    for i=1:4
        if i<=days
            x=rem(i-1,2);
            y=1-floor((i-1)/2);
            
            
            subplot('position',[x*0.5+0.2 y*0.5+0.05 0.3 0.43]);
            plotdailybackbone(i,pathnames,overdayden,meanbackbone,a,daypositions,positions,bigboutons,acrossday,terminalpositions,dayterminalpositions,terminalacrossday,lastlabel,daystartpos,dayendpos);
            
            subplot('position',[x*0.5 y*0.5+0.05 0.18 0.43]);
            plotdailybackbonehist(i,overdayden,a,meanbackbone,daystartpos,dayendpos,maxbouton);
            
        end
    end
    
    if days>4
        figure;
        for i=5:8
            if i<=days
                x=rem(i-5,2);
                y=1-floor((i-5)/2);
                subplot('position',[x*0.5 y*0.5 0.49 0.47]);
                plotdailysummary(i,pathnames,overdayden,daypositions,positions,lastlabel);
                axis off;
            end
        end
        
        figure;
        for i=5:8
            if i<=days
                x=rem(i-5,2);
                y=1-floor((i-5)/2);
                subplot('position',[x*0.5+0.2 y*0.5+0.05 0.3 0.43]);
                plotdailybackbone(i,pathnames,overdayden,meanbackbone,a,daypositions,positions,bigboutons,acrossday,terminalpositions,dayterminalpositions,terminalacrossday,lastlabel,daystartpos,dayendpos);
                
                subplot('position',[x*0.5 y*0.5+0.05 0.18 0.43]);
                plotdailybackbonehist(i,overdayden,a,meanbackbone,daystartpos,dayendpos,maxbouton);
            end
        end
        
    end
    
    if days>8
        figure;
        for i=9:12
            if i<=days
                x=rem(i-9,2);
                y=1-floor((i-9)/2);
                subplot('position',[x*0.5 y*0.5 0.49 0.47]);
                plotdailysummary(i,pathnames,overdayden,daypositions,positions,lastlabel);
                axis off;
            end
        end
        
        figure;
        for i=9:12
            if i<=days
                x=rem(i-9,2);
                y=1-floor((i-9)/2);
                subplot('position',[x*0.5+0.2 y*0.5+0.05 0.3 0.43]);
                plotdailybackbone(i,pathnames,overdayden,meanbackbone,a,daypositions,positions,bigboutons,acrossday,terminalpositions,dayterminalpositions,terminalacrossday,lastlabel,daystartpos,dayendpos);
                
                subplot('position',[x*0.5 y*0.5+0.05 0.18 0.43]);
                plotdailybackbonehist(i,overdayden,a,meanbackbone,daystartpos,dayendpos,maxbouton);
            end
        end
    end
catch
    disp('problem generating summary figure---day');
    disp(j);
end

function plotdailybackbonehist(day,overdayden,a,meanbackbone,daystartpos,dayendpos,maxbouton)
        hold on;
        hist(overdayden(day).maxint(daystartpos(day):dayendpos(day))/(a(day)*meanbackbone),100);
        stem(1,70);
        xlabel(a(day)*meanbackbone);
        axis([0 maxbouton 0 75]);

function plotdailybackbone(day,pathnames,overdayden,meanbackbone,a,daypositions,positions,bigboutons,acrossday,terminalpositions,dayterminalpositions,terminalacrossday,lastlabel,daystartpos,dayendpos)
        j=day;
        hold on;
        h=plot(overdayden(j).overdaylen(daystartpos(day):dayendpos(day)),overdayden(j).maxint(daystartpos(day):dayendpos(day))/(a(j)*meanbackbone));
      
        for k=1:max(size(positions)) 
            h=text(positions(k)-0.5, 0,num2str(k+lastlabel));
            if (bigboutons(k))
                set(h,'Color',[0 0 1]);
            else
                 set(h,'Color',[0 1 0]);
             end
        end
        for k=1:max(size(positions))
            if (daypositions(j,k)>0)
                h=plot(daypositions(j,k), acrossday(j,k+lastlabel)/(a(j)*meanbackbone),'r+');
            else
                h=plot(positions(k), acrossday(j,k+lastlabel)/(a(j)*meanbackbone),'r+');
            end
            set(h,'MarkerSize',3);
        end
        for k=1:max(size(terminalpositions))
            if (terminalacrossday(j,k)>0)
                h=plot(dayterminalpositions(j,k), terminalacrossday(j,k)/(a(j)*meanbackbone),'r.');
            end
            h=text(terminalpositions(k)-0.5,-0.8,num2str(k+lastlabel+max(size(positions))));
            set(h,'Color',[1 0 0]);
        end
        plot([overdayden(j).overdaylen(daystartpos(day)) overdayden(j).overdaylen(dayendpos(day))],[1 1],'r');
        [path name ext]=fileparts(char(pathnames(day)));
        xlabel(name);

function plotdailysummary(day,pathnames,overdayden,daypositions,positions,lastlabel);
try
    
    [path name ext]=fileparts(char(pathnames(day)));
    title(name);
    if (strcmp(ext,'.spi'))
       try
        load('-mat',[path '\' name ext]);
        self.gh=[];
        imageArray=self.data.ch(1).imageArray;
    catch
        mkdir(path, [name 'ziptemp']);
        h = waitbar(0,'Unzipping Spi File...', 'Name', 'Unzipping Spi File', 'Pointer', 'watch');
        waitbar(0,h, ['Unzipping Spi File...']);
        if exist([path '\' name 'ziptemp'],'dir')
            tiffile=dir([path '\' name 'ziptemp\*.tif']);
            filtif=dir([path '\' name 'ziptemp\*fil.tif']);
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
        filtif=dir([path '\' name 'ziptemp\*fil.tif']);
        imageArray=opentif([path '\'  name 'ziptemp\' filtif(1).name]);
     end
       
        upperbound=max(overdayden(day).meanback)*8;
        colormap(makeColorMap('gray', 8));
        h=imagesc(collapse(imageArray,'XY'),[0 upperbound]);
        % put spines on image
        uids=[];
        hold on;
        for k=1:max(size(positions))
            if daypositions(day,k)>0
                p=daypositions(day,k);
            else
                p=positions(k);
            end
           
            [mind,ind]=min(abs(overdayden(day).overdaylen-p));
            h=plot(double(overdayden(day).voxel(2,ind)),double(overdayden(day).voxel(1,ind)),'r.');
            c=hsv2rgb([(day+1)/(size(overdayden,2)+1) 1 1]);
            set(h,'Color',c);
            h=text(double(overdayden(day).voxel(2,ind))+5,double(overdayden(day).voxel(1,ind)),num2str(k+lastlabel));
            %                        if (sum(([spines(abs([spines.overdaydis]-positions(k))<1).len]==0)&([spines(abs([spines.overdaydis]-positions(k))<1).edittype]==0))>0)
            set(h,'Color',[1 0 0]);
            %                       else
            %set(h,'Color',[0 0 1]);
            %                        end
        end
    end
    title(name);
    
catch
    disp('error generating preview image for series');
    disp(lasterr);
end













%         if 0   
%             
%             % get all potential positions
%             spines=db.spines(([db.spines.seriesuid]==seriesuid) & ([db.spines.den_ind]==i));
%             mindis=-10000000;
%             maxdis=100000000;
%             for j=1:size(overdayden,2)
%                 mindis=max(mindis,min(overdayden(j).overdaylen));
%                 maxdis=min(maxdis,max(overdayden(j).overdaylen));
%             end
%             positions=[100000];
%             positions1=[];
%             for j=1:size(spines,2)
%                 if ((spines(j).overdaydis)<mindis) || ((spines(j).overdaydis)>maxdis)   % is the spine not in range for all days?
%                     spines(j).label=-1;
%                     db.spines([db.spines.uid]==spines(j).uid).label=-1;
%                 else
%                     if (spines(j).len==0) && (spines(j).edittype==0) % only consider spines which pass the more strigent threshold 
%                         if (min(abs(positions - spines(j).overdaydis))>1)          
%                             % add a position
%                             positions=[positions; mean([spines(abs([spines.overdaydis]-spines(j).overdaydis)<1).overdaydis])];
%                             positions1=[positions1;spines(j).overdaydis];
%                         end
%                     end
%                 end 
%             end
%             positions=sort(positions); 
%             positions=positions(1:end-1); % get rid of the 100000 used to seed the array
%             terminals=[spines([spines.len]>0).label];
%             terminals=sort(terminals);
%             lastlabel=size(db.series([db.series.uid]==seriesuid).acrossday,2);
%             % generate of the matrix of intensities across days
%             db.series([db.series.uid]==seriesuid).acrossday=[db.series([db.series.uid]==seriesuid).acrossday zeros(size(overdayden,2),max(size(positions))+max(size(terminals)))];
%             db.series([db.series.uid]==seriesuid).acrossdayraw=[db.series([db.series.uid]==seriesuid).acrossdayraw zeros(size(overdayden,2),max(size(positions))+max(size(terminals)))];
%             db.series([db.series.uid]==seriesuid).acrossdaymaxgaussian=[db.series([db.series.uid]==seriesuid).acrossdaymaxgaussian zeros(size(overdayden,2),max(size(positions))+max(size(terminals)))];
%             db.series([db.series.uid]==seriesuid).acrossdaymax=[db.series([db.series.uid]==seriesuid).acrossdaymax zeros(size(overdayden,2),max(size(positions))+max(size(terminals)))];
%             db.series([db.series.uid]==seriesuid).acrossdayrawgaussian=[db.series([db.series.uid]==seriesuid).acrossdayrawgaussian zeros(size(overdayden,2),max(size(positions))+max(size(terminals)))];
%             db.series([db.series.uid]==seriesuid).acrossdayrawfiltered=[db.series([db.series.uid]==seriesuid).spineacrossdayrawfiltered zeros(size(overdayden,2),max(size(positions))+max(size(terminals)))];
%             db.series([db.series.uid]==seriesuid).spineacrossdaygaussian=[db.series([db.series.uid]==seriesuid).spineacrossdaygaussian zeros(size(overdayden,2),max(size(positions))+max(size(terminals)))];
%             
%             %   db.series([db.series.uid]==seriesuid).spineacrossdayraw=[db.series([db.series.uid]==seriesuid).spineacrossdayraw zeros(size(overdayden,2),max(size(positions))+max(size(terminals)))];
%             
%             
%             
%             for j=1:size(spines,2)
%                 if (spines(j).label>0) %&& (spines(j).edittype==0)
%                     if (spines(j).len==0)  % en passant bouton
%                         labels=find(abs(positions-spines(j).overdaydis)<1);
%                         if (labels)
%                             spines(j).label=lastlabel+labels(1);
%                             db.spines([db.spines.uid]==spines(j).uid).label=lastlabel+labels(1);
%                             
%                             db.series([db.series.uid]==seriesuid).spineacrossdaymax(find([overdayden.uid]==spines(j).den_uid),lastlabel+labels(1))=spines(j).intensity;
%                             db.series([db.series.uid]==seriesuid).spineacrossdaymaxgaussian(find([overdayden.uid]==spines(j).den_uid),lastlabel+labels(1))=spines(j).intensitygaussian;
%                         else
%                             db.spines([db.spines.uid]==spines(j).uid).edittype=3;
%                         end
%                         %               db.series([db.series.uid]==seriesuid).spineacrossday(find([overdayden.uid]==spines(j).den_uid),lastlabel+labels(1))=spines(j).intensity;
%                         %               db.series([db.series.uid]==seriesuid).spineacrossdayraw(find([overdayden.uid]==spines(j).den_uid),lastlabel+labels(1))=spines(j).intensityraw;
%                         
%                     else    % terminal bouton
%                         labels=find(unique(terminals)==spines(j).label);
%                         spines(j).label=lastlabel+size(positions)+labels(1);
%                         db.spines([db.spines.uid]==spines(j).uid).label=lastlabel+size(positions,1)+labels(1);
%                         db.series([db.series.uid]==seriesuid).spineacrossday(find([overdayden.uid]==spines(j).den_uid),lastlabel+size(positions)+labels(1))=spines(j).intensity; 
%                     end
%                 end 
%             end 
%             
%             % fill in spots where no en passant spines were detected on some days
%             for j=1:size(positions)
%                 for k=1:size(overdayden,2)
%                     if 1%(db.series([db.series.uid]==seriesuid).spineacrossday(k,j+lastlabel)==0) && (db.series([db.series.uid]==seriesuid).spineacrossday(k,j+lastlabel)==0)
%                         left=[find(overdayden(k).overdaylen>positions(j)-1) 1];
%                         right=find(overdayden(k).overdaylen<positions(j)+1);
%                         
%                         
%                         
%                         db.series([db.series.uid]==seriesuid).spineacrossday(k,j+lastlabel)=max(overdayden(k).maxintcorrected(left(1):right(end)));
%                         db.series([db.series.uid]==seriesuid).spineacrossdayraw(k,j+lastlabel)=max(overdayden(k).maxintraw(left(1):right(end)));
%                         db.series([db.series.uid]==seriesuid).spineacrossdayrawgaussian(k,j+lastlabel)=max(overdayden(k).maxintrawgaussian(left(1):right(end)));
%                         db.series([db.series.uid]==seriesuid).spineacrossdaygaussian(k,j+lastlabel)=max(overdayden(k).maxintcubegaussian(left(1):right(end)));
%                         db.series([db.series.uid]==seriesuid).spineacrossdayrawfiltered(k,j+lastlabel)=max(overdayden(k).maxintrawfiltered(left(1):right(end)));
%                         
%                     end 
%                 end
%             end 
%             
%         end




%             figure;
%             subplot(2,2,1);
%             hold on;
%             for j=1:size(overdayden,2)
%                 h=plot(overdayden(j).overdaylen,overdayden(j).maxintcorrected./overdayden(j).meanback);
%                 c=hsv2rgb([j/size(overdayden,2) 1 1]);
%                 set(h,'Color',c);  
%             end
%             xlabel('corrected by backbone');
%             
%             subplot(2,2,[3 4]);
%             hold on;
%             for j=1:size(overdayden,2)
%                 h=plot(overdayden(j).overdaylen,double(overdayden(j).maxintraw)./overdayden(j).meanboutonraw);
%                 c=hsv2rgb([j/size(overdayden,2) 1 1]);
%                 set(h,'Color',c);  
%             end
%             xlabel('corrected by backbone');
%             for k=1:size([overdayden(j).spineuids],2)
%                 id=[db.spines.uid]==overdayden(j).spineuids(k);
%                 %            [mdis,xind]=min(abs(x-db.spines(id).overdaydis));
% %                 if (db.spines(id).len>=0)
% %                     h=plot(db.spines(id).overdaydis,db.spines(id).intensitymax/overdayden(j).meanboutonraw,'.');
% %                     set(h,'Color',c);
% %                 end
%                 %            h=text(conversion*db.spines(id).overdaydis,overdayden(j).maxintcorrected(db.spines(id).dendis)+1.5,num2str(db.spines(id).label));
% %                 h=text(db.spines(id).overdaydis,db.spines(id).intensitymax/overdayden(j).meanboutonraw+0.5,num2str(db.spines(id).label));
% %                 set(h,'Color',c);
%             end
%             
%             subplot(2,2,2);
%             hold on;
%             for j=1:size(overdayden,2)
%                 if isfield(overdayden(j),'ROIintensity')
%                     h=plot(overdayden(j).overdaylen,double(overdayden(j).maxintcorrected)/overdayden(j).ROIintensity);
%                     c=hsv2rgb([j/size(overdayden,2) 1 1]);
%                     set(h,'Color',c);  
%                 end
%             end
%             xlabel('corrected by ROI');
%             
%    set(gcf,'Units','pixels','Position',);