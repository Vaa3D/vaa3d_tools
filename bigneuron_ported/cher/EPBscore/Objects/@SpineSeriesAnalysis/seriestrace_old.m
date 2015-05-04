function obj= seriestrace(obj,pathnames)
global self;
global objects;
global db;  
filenames=obj.state.display.filenames;
if (nargin==1)
    pathnames=obj.state.display.pathnames;
end
pathname=obj.state.pathname;

if isfield(obj.state,'dbpath')
    load('-mat',obj.state.dbpath);
else
    [dbname, dbpath] =  uigetfile({'*.db'}, 'Choose database file to load');
    load('-mat',[dbpath '\' dbname]);
end


try
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    %       This section get all the data and store them in the
    %       database
    %
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    if (size(db.series,2)==0)
        seriesuid=1;
    else
        seriesuid=max([db.series.uid])+1;
    end
    db.series(seriesuid).uid=seriesuid;
    db.series(seriesuid).experiments=[];
    db.series(seriesuid).dendrites=[];
    db.series(seriesuid).names={};
    db.series(seriesuid).exclusion={};
    
    % go through each file and get all the data
    for q=1:size(pathnames,2)
        [path name ext]=fileparts(char(pathnames(q)));
        [name,status]=strtok(name,'_');
        db.series(seriesuid).names{q}=name;
        if (strcmp(ext,'.spi'))
            warning off MATLAB:MKDIR:DirectoryExists;
            mkdir(path, [name 'ziptemp']);
            h = waitbar(0,'Unzipping Spi File...', 'Name', 'Unzipping Spi File', 'Pointer', 'watch');
            waitbar(0,h, ['Unzipping Spi File...']);
            if exist([path '\' name 'ziptemp'],'dir')
                tiffile=dir([path '\' name 'ziptemp\*.tif']);
                filtif=dir([path '\' name 'ziptemp\*.tif']);
                datfile=dir([path '\' name 'ziptemp\*.dat']);
                if ~((size(tiffile,1)>=2) & (size(filtif,1)>=1) & (size(datfile,1)>=1))
                    % infounzip([path '\' name ext],[path '\' name 'ziptemp']);
                    [s,w]=system(['unzip -u ' path '\' name status ext ' -d ' path '\' name 'ziptemp']);
                    if (s~=0)
                        disp('Could not unzip data');
                        close(h);
                        return;
                    end
                end
            else
                %  infounzip([path '\' name ext],[path '\' name 'ziptemp']);
                [s,w]=system(['unzip -u ' path '\' name status ext ' -d ' path '\' name 'ziptemp']);
                if (s~=0)
                    disp('Could not unzip data');
                    close(h);
                    return;
                end
            end
            try
                cd ([path '\' name 'ziptemp\ziptemp']);
                copyfile('*.*','..');
            catch
                
            end
            waitbar(1,h, 'Done');
            close(h);
            
            cd (path);               
            
            datfile=dir([path '\' name 'ziptemp\*.dat']);
            load('-mat',[path '\' name 'ziptemp\' datfile(1).name]);
            
            % Add experiment
            if (size(db.experiments,2)==0)
                experimentuid=1;
            else
                experimentuid=max([db.experiments.uid])+1;
            end
            db.experiments(experimentuid).uid=experimentuid;
            db.experiments(experimentuid).imagemedian=self.data.imagemedian;
            db.experiments(experimentuid).imagestd=self.data.imagestd;
            %db.experiments(experimentuid).imagestd=self.data.imagestd;
            db.experiments(experimentuid).m=self.data.m;
            db.experiments(experimentuid).s=self.data.s;
            db.experiments(experimentuid).rawImagePath=path;
            db.experiments(experimentuid).rawImageName=name;
            db.experiments(experimentuid).parameters=self.parameters;
            db.experiments(experimentuid).seriesuid=seriesuid;
            if isfield(self.data,'ROI')
                db.experiments(experimentuid).ROIintensity=self.data.ROI.meanintensity;
            end
            db.series(seriesuid).experiments=[db.series(seriesuid).experiments ' ' self.state.rawImageName];
            
            
            
            % Add dendrites
            for i=1:size(self.data.dendrites,2)
                if (size(db.dendrites,2)==0)
                    uid=1;
                else
                    uid=max([db.dendrites.uid])+1;
                end
                db.dendrites(uid).uid=uid;
                names=fieldnames(self.data.dendrites(i));
                self.data.dendrites(i).uid=uid;
                for j=1:size(names,1)
                    db.dendrites(uid).(char(names(j)))=self.data.dendrites(i).(char(names(j)));
                end
                db.dendrites(uid).uid=uid;
                db.dendrites(uid).seriesuid=seriesuid;
                db.dendrites(uid).experimentuid=experimentuid;
                db.dendrites(uid).totallength=max(db.dendrites(uid).length);
                db.dendrites(uid).spineuids=[];
                db.dendrites(uid).spineints=[];
                db.series(seriesuid).dendrites(i,q)=uid;
                
                if isfield(self.data,'marks')
                    % recalculate the dendis of all the marks to be used in
                    % the future for alignment
                    if (size(self.data.marks([self.data.marks.den_ind]==db.dendrites(uid).index),2)<2)
                        disp(['ERROR! missing marks from ' name]);
                        return;
                    end
                    db.dendrites(uid).meanmarks=mean(db.dendrites(uid).length([self.data.marks([self.data.marks.den_ind]==db.dendrites(uid).index).dendis]));
                    db.dendrites(uid).stdmarks=std(db.dendrites(uid).length([self.data.marks([self.data.marks.den_ind]==db.dendrites(uid).index).dendis]));
                    
                else
                    disp(['error in ' name ':no marks detected']);
                end
            end
            
            % Add spines
            for i=1:size(self.data.spines,2)
                uid=size(db.spines,2)+1;
                db.spines(uid).uid=uid;
                self.data.spines(i).uid=uid;
                den_uid=self.data.dendrites(self.data.spines(i).den_ind).uid;
                db.spines(uid).den_uid=den_uid;
                if (self.data.spines(i).len==0)
                    db.spines(uid).intensity=-1;
                else
                    db.spines(uid).intensity=-1;
                    %self.data.filtereddata(db.spines(uid).voxel(1,1),db.spines(uid).voxel(2,1),db.spines(uid).voxel(3,1));
                    %db.spines(uid).intensity=self.data.filtereddata(db.spines(uid).voxel(1,1),db.spines(uid).voxel(2,1),db.spines(uid).voxel(3,1))*db.dendrites(den_uid).meanback/db.dendrites(den_uid).maxint(db.spines(uid).den_ind);
                end
                names=fieldnames(self.data.spines(i));
                for j=1:size(names,1)
                    db.spines(uid).(char(names(j)))=self.data.spines(i).(char(names(j)));
                end
                
                db.spines(uid).uid=uid;
                db.dendrites(den_uid).spineuids=[db.dendrites(den_uid).spineuids uid];
                db.dendrites(den_uid).spineints=[db.dendrites(den_uid).spineints db.spines(uid).intensity];
                db.spines(uid).seriesuid=seriesuid;
                db.spines(uid).experimentuid=experimentuid;
            end
            for i=1:max(size(self.data.dendrites)) % consider only first dendrite for now
                uid=self.data.dendrites(i).uid;
                %terminal bouton
                length=[db.spines(db.dendrites(uid).spineuids).len];
                db.dendrites(uid).ibsterminal=db.dendrites(uid).totallength/sum(length>0);
                %non-terminal bouton
                db.dendrites(uid).ibs=db.dendrites(uid).totallength/sum(length==0);
                db.dendrites(uid).variability=std(db.dendrites(uid).medianfiltered);
                if isfield(db.experiments(experimentuid),'ROIintensity')
                    db.dendrites(uid).ROIintensity=db.experiments(experimentuid).ROIintensity;
                end
            end    
        end
    end
catch
    disp(['problem with getting data from sereis:' pathnames(q)]);
    disp(lasterr);
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%generate series overviews
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
try
    seriesuid=max([db.series.uid]);
    dendrites=db.dendrites([db.dendrites.seriesuid]==seriesuid);
    ids=unique([dendrites.index]);
    
    % align dendrites over days
    for i=1:size(ids,2) % iterate through all dendrites in this series
        overdayden=dendrites([dendrites.index]==ids(i));    
        %correct so first one has offset zero
        overdayden(1).offset=0;
        overdayden(1).stretch=1;
        for j=2:size(overdayden,2) %across day alignment
            overdayden(j).stretch=overdayden(j).stdmarks/overdayden(1).stdmarks;
            overdayden(j).offset=overdayden(j).meanmarks-overdayden(1).meanmarks*overdayden(j).stretch;
        end
        overdayden(1).overdaylen=overdayden(1).length;
        for j=2:size(overdayden,2)
            overdayden(j).overdaylen=(overdayden(j).length-overdayden(j).offset)/overdayden(j).stretch;
            if isempty(overdayden(j).overdaylen)
                disp(['ERROR: empty overdaylen for ' pathnames(j)]);
            end
        end
        for j=1:size(overdayden,2)
            for k=1:size([overdayden(j).spineuids],2)
                id=[db.spines.uid]==overdayden(j).spineuids(k);
                db.spines(id).overdaydis=overdayden(j).overdaylen(db.spines(id).dendis);
            end
        end
        
        for j=1:size(overdayden,2)
            db.dendrites(overdayden(j).uid).stretch=overdayden(j).stretch;
            db.dendrites(overdayden(j).uid).offset=overdayden(j).offset;
            db.dendrites(overdayden(j).uid).overdaylen=overdayden(j).overdaylen;
        end
        
   
        % stretch factors
        factor=[];
        
        dayfactor=[overdayden.stretch];
        factor=exp(abs(diff(log(dayfactor))))
        
        
        
        lastlabel=0;
        % Retrace all spines across days
        
        done=0    
        while ~done
            
            if max(size(db.series([db.series.uid]==seriesuid).exclusion))>=i
                exclusion=db.series([db.series.uid]==seriesuid).exclusion{i};
            else
                exclusion=[];
            end
            mindis=-10000000;
            maxdis=100000000;
            for j=1:size(overdayden,2)
                mindis=max(mindis,min(overdayden(j).overdaylen));
                maxdis=min(maxdis,max(overdayden(j).overdaylen));
            end
            db.series([db.series.uid]==seriesuid).mindis=mindis;
            db.series([db.series.uid]==seriesuid).maxdis=maxdis;
            %code to estimate noise level
            [scalea,scaleb,ratio1,ratio2]=estimatenoise(overdayden,exclusion);
            
            disp('std/mean');
            disp(ratio1);
            disp(ratio2);
            
            
            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
            %
            %   Cluster potential site positions
            %
            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
            
            overdaypos={} 
            for k=1:max(size(overdayden))        
                overdayden(k).maxint=reflectivesmooth(double(overdayden(k).maxintraw),7);
                overdaypos{k}=overdayden(k).overdaylen(findTerminals(double(overdayden(k).maxint)./median(double(overdayden(k).maxint)), 0.5, 1.5, 30,10));
            end   
            terminaloverdaypos={} 
            for k=1:max(size(overdayden))                    
                terminaloverdaypos{k}=overdayden(k).overdaylen([db.spines(([db.spines.den_uid]==overdayden(k).uid) & ([db.spines.len]>0) ).dendis]);
            end   
            
            [positions,daypositions,terminalpositions,dayterminalpositions]=clusterboutons(overdayden,overdaypos,terminaloverdaypos,1.5,5,exclusion);
            
            figure;
            [val,ind]=sort(positions);
            hold on;
            % plot the assignment of clusters
            for j=1:max(size(positions))
                plot(daypositions((daypositions(:,j)~=0),j),j,'.');
            end
            xlabel(path);
            title(['clustering' num2str(seriesuid) '_' num2str(i)]);
%            saveas(gcf,['clustering' num2str(seriesuid) '_' num2str(i)],'fig');
            
            
            acrossday=getoverdayintensities(overdayden,positions,daypositions);
            terminalacrossdaylength=[];
            
            if (0)  % 1 for assigning terminal boutons based on distance, 0 for based on labeling
                % Add in terminal boutons
                terminalacrossday=[];
                for k=1:size(overdayden,2)
                    for j=1:max(size(terminalpositions))
                        intensity=[db.spines(([db.spines.den_uid]==overdayden(k).uid) & ([db.spines.len]>0) ).intensity];
                        lengths=[db.spines(([db.spines.den_uid]==overdayden(k).uid) & ([db.spines.len]>0) ).len];
                        index=find(terminaloverdaypos{k}==dayterminalpositions(k,j)); 
                        % Was there a peak detected on this particular day
                        if (dayterminalpositions(k,j)>0)
                            terminalacrossday(k,j)=intensity(index(1));
                            terminalacrossdaylength(k,j)=lengths(index(1));
                        else
                            terminalacrossday(k,j)=0;
                            terminalacrossdaylength(k,j)=0;
                        end            
                    end
                end
            else         
                % Rely on the labels for the terminal boutons
                
                %             dayterminalpositions=[
                %                 terminalpositions=
                %                 
                terminalindicies=unique([db.spines((ismember([db.spines.den_uid],[overdayden.uid])) & ([db.spines.len]>0)).label]);
                terminalindicies=terminalindicies(terminalindicies>0);
                terminalacrossday=zeros(size(overdayden,2),max(size(terminalindicies)));
                dayterminalpositions=zeros(size(overdayden,2),max(size(terminalindicies)));
                for k=1:size(overdayden,2)
                    index=find(([db.spines.den_uid]==overdayden(k).uid) & ([db.spines.len]>0) );
                    for l=1:size(index,2)
                        if (db.spines(index(l)).label>0)
                            
                            dayterminalpositions(k,find(terminalindicies==db.spines(index(l)).label))=db.spines(index(l)).overdaydis;           
                            terminalacrossday(k,find(terminalindicies==db.spines(index(l)).label))=db.spines(index(l)).intensity;  
                            terminalacrossdaylength(k,find(terminalindicies==db.spines(index(l)).label))=db.spines(index(l)).len; 
                        end
                    end
                end
                
                terminalpositions=[];
                for k=1:max(size(terminalindicies))
                    terminalpositions=[terminalpositions mean(dayterminalpositions(terminalacrossday(:,k)>0,k))];
                end
            end
            
            mindis=-10000000;
            maxdis=100000000;
            for j=1:size(overdayden,2)
                mindis=max(mindis,min(overdayden(j).overdaylen));
                maxdis=min(maxdis,max(overdayden(j).overdaylen));
            end
            
            index=find((terminalpositions>mindis) & (terminalpositions<maxdis));
            terminalpositions=terminalpositions(index); % is the position in range for all days   
            [terminalpositions,ord]=sort(terminalpositions);
            index=index(ord);
            dayterminalpositions=dayterminalpositions(:,index);
            terminalacrossday=terminalacrossday(:,index);
            terminalacrossdaylength=terminalacrossdaylength(:,index);
            
            
            
            try
                [a,b,variations,variations1,variations2,ratios,ratios1,ratios2,turnover,turnoverputative,turnoverputative1,turnoverputative2,meanmeanback]=calculatechanges(acrossday,terminalacrossday,[overdayden.meanback],0,scalea,1.7,1.3);
                
%                 figure;
%                 subplot(3,2,2);
%                 meanvariations= mean(abs(variations'));
%                 bar(meanvariations);
%                 disp(variations);
%                 axis([0.5 0.5+size(meanvariations,2) 0 max(meanvariations(:))+0.05]);
%                 %set(gca,'xticklabel',{'corr'; 'raw'; 'rawmax'; 'rawsmooth';'corrgau';'gau';'gaumax';'gausmooth';'filtered';'filteredmax';'filteredsmooth'});
%                 title(['mean variations' num2str(seriesuid) '-' num2str(i)]);
%                 xlabel(path);
%                 
%                 subplot(3,2,1);
%                 bar(variations);
%                 axis([0.5 0.5+size(variations,1) min(variations(:))-0.05 max(variations(:))+0.05]);
%                 %set(gca,'xticklabel',{'corr'; 'raw'; 'rawmax'; 'rawsmooth';'corrgau';'gau';'gaumax';'gausmooth';'filtered';'filteredmax';'filteredsmooth'});
%                 title(['variations' num2str(seriesuid) '-' num2str(i)]);
%                 xlabel(path);
%                 
%                 subplot(3,2,3);
%                 meanvariations= mean(abs(variations2'));
%                 bar(meanvariations);
%                 disp(variations);
%                 axis([0.5 0.5+size(meanvariations,2) 0 max(meanvariations(:))+0.05]);
%                 %set(gca,'xticklabel',{'corr'; 'raw'; 'rawmax'; 'rawsmooth';'corrgau';'gau';'gaumax';'gausmooth';'filtered';'filteredmax';'filteredsmooth'});
%                 title(['backbone corrected' num2str(seriesuid) '-' num2str(i)]);
%                 xlabel(path);
%                 
%                 subplot(3,2,5);
%                 bar(ratios1);
%                 axis([0.5 size(ratios1,1)+0.5 min(ratios1(:))-0.05 max(ratios1(:))+0.05]);
%                 %set(gca,'xticklabel',{'corr'; 'raw'; 'rawmax'; 'rawsmooth';'corrgau';'gau';'gaumax';'gausmooth';'filtered';'filteredmax';'filteredsmooth'});
%                 xlabel(path);
%                 title(['ratios' num2str(seriesuid) '-' num2str(i)]);
%                 
%                 subplot(3,2,6);
%                 meanratios=mean(abs(ratios1'));
%                 bar(meanratios);
%                 axis([0.5 0.5+size(meanratios,2) 0 max(meanratios(:))+0.05]);
%                 %set(gca,'xticklabel',{'corr'; 'raw'; 'rawmax'; 'rawsmooth';'corrgau';'gau';'gaumax';'gausmooth';'filtered';'filteredmax';'filteredsmooth'});
%                 xlabel(path);
%                 title(['meanratios' num2str(seriesuid) '-' num2str(i)]);
%                 
%                 
%                 subplot(3,2,4);
%                 bar([sum(abs(turnover),2) sum(abs(turnoverputative2),2)]);
%                 axis([0.5 0.5+size(turnover,1) 0 max([sum(abs(turnover),2); sum(abs(turnoverputative2),2)])+0.05]);
%                 xlabel(path);
%                 title(['turnover rates' num2str(seriesuid) '-' num2str(i)]);
%                 
%                 
%                 saveas(gcf,['bars' num2str(seriesuid) '_' num2str(i)], 'fig');
            catch
                
                
            end
            
%            summarygraph(overdayden,positions,terminalpositions,daypositions,dayterminalpositions,acrossday,terminalacrossday,lastlabel,a,b,pathnames);        
%            saveas(gcf, ['summary' num2str(seriesuid) '_' num2str(i)], 'fig');
            
            summarygraphshort(overdayden,positions,terminalpositions,daypositions,dayterminalpositions,acrossday,terminalacrossday,lastlabel,a,b,pathnames); 
            
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
            
            h=msgbox('Please check whether you want to exclude any boutons from analysis now.');
            w=waitforbuttonpress;
            close(h);
            answer=inputdlg({'Enter boutons to exclude from analysis:'},'Do you want to exclude boutons from analysis?',1,{''});
            if isempty(answer)
                done=1;
            else
                answer=answer{1};
                exclude=[];
                while ~isempty(answer)
                    [tok,rem]= strtok(answer);
                    [first,second]=strtok(tok,'-');
                    firstnum=str2num(first);
                    secondnum=-str2num(second);
                    if (secondnum>firstnum) && (firstnum>mindis) && (secondnum<maxdis)
                        exclude=[exclude; firstnum secondnum];
                    end
                    answer=rem;
                end
                db.series([db.series.uid]==seriesuid).exclusion{i}=exclude;
                % reset spineacrossday
                done=0;
            end
        end
        % add summary data to database    
        
        if (size(db.summaries,2)==0)
            summaryuid=1;
        else
            summaryuid=max([db.summaries.uid])+1;
        end
        db.series([db.series.uid]==seriesuid).summariesuid(i)=summaryuid;
        db.summaries(summaryuid).uid=summaryuid;
        db.summaries(summaryuid).seriesuid=seriesuid;   
        db.summaries(summaryuid).dendriteindex=ids(i);
        db.summaries(summaryuid).mindis=mindis;
        db.summaries(summaryuid).maxdis=maxdis;
        
        db.summaries(summaryuid).meanmeanback=meanmeanback;
        db.summaries(summaryuid).positions=positions;
        db.summaries(summaryuid).terminalpositions=terminalpositions;
        db.summaries(summaryuid).daypositions=daypositions;
        db.summaries(summaryuid).dayterminalpositions=dayterminalpositions;
        db.summaries(summaryuid).acrossday=acrossday;
        db.summaries(summaryuid).terminalacrossday=terminalacrossday;
        db.summaries(summaryuid).meanback=[overdayden.meanback];
        db.summaries(summaryuid).scaledacrossday=acrossday./(db.summaries(summaryuid).meanback'*ones(1,size(acrossday,2)));
        if isempty(terminalacrossday)
            db.summaries(summaryuid).scaledterminalacrossday=[];
        else            
            db.summaries(summaryuid).scaledterminalacrossday=terminalacrossday./(db.summaries(summaryuid).meanback'*ones(1,size(terminalacrossday,2)));
        end
        db.summaries(summaryuid).terminalacrossdaylength=terminalacrossdaylength;
        db.summaries(summaryuid).acrossdayindex=lastlabel+(1:max(size(positions)));
        db.summaries(summaryuid).terminalacrossdayindex=lastlabel+max(size(positions))+(1:max(size(terminalpositions)));
        db.summaries(summaryuid).names=db.series([db.series.uid]==seriesuid).names;
        db.summaries(summaryuid).correctionfactora=a;
        db.summaries(summaryuid).correctionfactorb=b;
        db.summaries(summaryuid).variations=variations;
        db.summaries(summaryuid).ratios=ratios;
        db.summaries(summaryuid).ratios1=ratios1;
        db.summaries(summaryuid).ratios2=ratios2;
        db.summaries(summaryuid).turnover=turnover;
        db.summaries(summaryuid).turnoverputative=turnoverputative;
        db.summaries(summaryuid).turnoverputative1=turnoverputative1;
        db.summaries(summaryuid).turnoverputative2=turnoverputative2;
        db.summaries(summaryuid).variations1=variations1;
        db.summaries(summaryuid).variations2=variations2;
        db.summaries(summaryuid).scalea=scalea;
        db.summaries(summaryuid).ratio1=ratio1;
        db.summaries(summaryuid).ratio2=ratio2;
        db.summaries(summaryuid).ibs=ibs;
        db.summaries(summaryuid).icbs=icbs;
        db.summaries(summaryuid).itbs=itbs;
       
        lastlabel=lastlabel+max(size(positions))+max(size(terminalpositions));
        %        db.summaries(summaryuid).turnoverrates=rates;
    end
catch
    disp('problem processing series');
    disp(lasterr);
end
if 1
    % 
    try
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        
        % This section alters the spine labels in the analyzed spi files
        
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        
        % go through each file and get all the data
        for q=1:size(pathnames,2)
            [path name ext]=fileparts(char(pathnames(q)));
            [name,status]=strtok(name,'_');
            if (strcmp(ext,'.spi'))
                warning off MATLAB:MKDIR:DirectoryExists;
                mkdir(path, [name 'ziptemp']);
                h = waitbar(0,'Unzipping Spi File...', 'Name', 'Unzipping Spi File', 'Pointer', 'watch');
                waitbar(0,h, ['Unzipping Spi File...']);
                if exist([path '\' name 'ziptemp'],'dir')
                    tiffile=dir([path '\' name 'ziptemp\*.tif']);
                    filtif=dir([path '\' name 'ziptemp\*.tif']);
                    datfile=dir([path '\' name 'ziptemp\*.dat']);
                    if ~((size(tiffile,1)>=2) & (size(filtif,1)>=1) & (size(datfile,1)>=1))
                        % infounzip([path '\' name ext],[path '\' name 'ziptemp']);
                        [s,w]=system(['unzip -u ' path '\' name status ext ' -d ' path '\' name 'ziptemp']);
                        if (s~=0)
                            disp('Could not unzip data');
                            close(h);
                            return;
                        end
                    end
                else
                    %  infounzip([path '\' name ext],[path '\' name 'ziptemp']);
                    [s,w]=system(['unzip -u ' path '\' name status ext ' -d ' path '\' name 'ziptemp']);
                    if (s~=0)
                        disp('Could not unzip data');
                        close(h);
                        return;
                    end
                end
                try
                    cd ([path '\' name 'ziptemp\ziptemp']);
                    copyfile('*.*','..');
                catch
                    
                end
                waitbar(1,h, 'Done');
                close(h);
                
                cd (path);               
                
                datfile=dir([path '\' name 'ziptemp\*.dat']);
                load('-mat',[path '\' name 'ziptemp\' datfile(1).name]);
                denuid=db.series([db.series.uid]==seriesuid).dendrites;
                daypositions={};
                dayterminalpositions={};
                index={};
                terminalindex={};
                for i=1:size(denuid,1)
                    daypositions{i}=db.summaries([db.summaries.uid]==db.series([db.series.uid]==seriesuid).summariesuid(i)).daypositions;
                    dayterminalpositions{i}=db.summaries([db.summaries.uid]==db.series([db.series.uid]==seriesuid).summariesuid(i)).dayterminalpositions;
                    index{i}=db.summaries([db.summaries.uid]==db.series([db.series.uid]==seriesuid).summariesuid(i)).acrossdayindex;
                    terminalindex{i}=db.summaries([db.summaries.uid]==db.series([db.series.uid]==seriesuid).summariesuid(i)).terminalacrossdayindex;
                end
                % Add spines
                for i=1:size(self.data.spines,2)
                    len=db.dendrites(denuid(self.data.spines(i).den_ind,q)).overdaylen;
                    if (self.data.spines(i).len==0)
                        % Find the corresponding spine according to distance
                        daypos=daypositions{self.data.spines(i).den_ind};
                        ind=index{self.data.spines(i).den_ind};
                        dis=(abs(daypos(q,:)-len(self.data.spines(i).dendis)));                
                        [mindis,j]=min(dis);
                        if (mindis<1)
                            self.data.spines(i).label=ind(j);
                        else
                            self.data.spines(i).label=-1;
                        end
                    else
                        daypos=dayterminalpositions{self.data.spines(i).den_ind};
                        ind=terminalindex{self.data.spines(i).den_ind};
                        dis=(abs(daypos(q,:)-len(self.data.spines(i).dendis)));   
                        [mindis,j]=min(dis);
                        if (mindis<1)
                            self.data.spines(i).label=ind(j);
                        else
                            self.data.spines(i).label=-1;
                        end
                    end
                    
                end
                
                save('-mat',[path '\' name 'ziptemp\' datfile(1).name],'self');
                h = waitbar(0,'Zipping Spi File...', 'Name', 'Zipping Spi File', 'Pointer', 'watch');
                waitbar(0,h, ['Zipping Spi File...']);
                try
                    cd ([path '\' name 'ziptemp\']);
                    [s,w]=system(['zip -3u ' path '\' name status '.spi ' '*.*']);
                    if (s~=0)
                        disp('Could not zip data');
                    end
                catch
                    disp('Could not zip data');
                end     
                waitbar(1,h, 'Done');
                close(h); 
            end
        end
    catch
        disp(['problem with saving data to sereis:' path]);
        disp(lasterr);
    end
end