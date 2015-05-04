function obj= seriestrace(obj,pathnames)
global self;
global objects;
global db;
filenames=obj.state.display.filenames;
if (nargin==1)
    pathnames=obj.state.display.pathnames;
end
pathname=obj.state.pathname;

[dbname, dbpath] =  uigetfile({'*.db'}, 'Choose database file to load');
if ~dbname
    [dbname, dbpath] =  uiputfile({'*.db'}, 'You canceled loading, choose database file to create');
    db.series=[];
    db.dendrites=[];
    db.spines=[];
    db.experiments=[];
    db.summaries=[];
    obj.state.dbpath=[dbpath '\' dbname];
else
    load('-mat',[dbpath '\' dbname]);
    obj.state.dbpath=[dbpath '\' dbname];
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
        db.series(seriesuid).names{q}=name;
        if (strcmp(ext,'.spi'))
            %  try
            load('-mat',[path '\' name ext]);
            self.gh=[];
            %  catch
            % see if it's a zip file
            %
            %
            %                 warning off MATLAB:MKDIR:DirectoryExists;
            %                 mkdir(path, [name 'ziptemp']);
            %                 h = waitbar(0,'Unzipping Spi File...', 'Name', 'Unzipping Spi File', 'Pointer', 'watch');
            %                 waitbar(0,h, ['Unzipping Spi File...']);
            %                 if exist([path '\' name 'ziptemp'],'dir')
            %                     datfile=dir([path '\' name 'ziptemp\*.dat']);
            %                     if ~(size(datfile,1)>=1)
            %                         % infounzip([path '\' name ext],[path'\' name 'ziptemp']);
            %                         try
            %                             unzip([path '\' name  ext], [path '\' name 'ziptemp\']);
            %                         catch
            %                             disp('Could not unzip data');
            %                             close(h);
            %                             return;
            %                         end
            %                     end
            %                 else
            %                     %  infounzip([path '\' name ext],[path '\' name 'ziptemp']);
            %                     try
            %                         unzip([path '\' name  ext], [path '\' name 'ziptemp\']);
            %                     catch
            %                         disp('Could not unzip data');
            %                         close(h);
            %                         return;
            %                     end
            %                 end
            %                 waitbar(1,h, 'Done');
            %                 close(h);
            %
            %                 cd (path);
            %
            %                 datfile=dir([path '\' name 'ziptemp\*.dat']);
            %                 load('-mat',[path '\' name 'ziptemp\' datfile(1).name]);
            % end
            
            % If there are multiple channels
            
            
            
            if isfield(self.data,'ch')
                data=self.data.ch(1);
            else
                data=self.data;
            end
            
            % check for image saturation
            maxint=max(data.imageArray(:));
            if maxint>4060
                disp(['Image Saturated: ' name ' max: ' num2str(maxint)]);                
            end
            
            
            % Add experiment
            if (size(db.experiments,2)==0)
                experimentuid=1;
            else
                experimentuid=max([db.experiments.uid])+1;
            end
            db.experiments(experimentuid).uid=experimentuid;
            try
                db.experiments(experimentuid).imagemedian=data.imagemedian;
            catch
                db.experiments(experimentuid).imagemedian=median(data.imageArray(:));
            end
            try
                db.experiments(experimentuid).imagestd=data.imagestd;
            catch
                db.experiments(experimentuid).imagestd=std(single(data.imageArray(:)))
            end
            %db.experiments(experimentuid).imagestd=self.data.imagestd;
            if isfield(data,'m')
                db.experiments(experimentuid).m=data.m;
                db.experiments(experimentuid).s=data.s;
            end
            db.experiments(experimentuid).rawImagePath=path;
            db.experiments(experimentuid).rawImageName=name;
            db.experiments(experimentuid).parameters=self.parameters;
            db.experiments(experimentuid).seriesuid=seriesuid;
            if isfield(data,'ROI')
                db.experiments(experimentuid).ROIintensity=data.ROI.meanintensity;
            end
            db.series(seriesuid).experiments=[db.series(seriesuid).experiments ' ' self.state.rawImageName];
        end
        
        
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
                db.dendrites(uid).marks=self.data.marks([self.data.marks.den_ind]==db.dendrites(uid).index);
                db.dendrites(uid).meanmarks=mean(db.dendrites(uid).length([self.data.marks([self.data.marks.den_ind]==db.dendrites(uid).index).dendis]));
                db.dendrites(uid).stdmarks=std(db.dendrites(uid).length([self.data.marks([self.data.marks.den_ind]==db.dendrites(uid).index).dendis]));
                
            else
                disp(['error in ' name ':no marks detected']);
            end
        end
        
        % Add spines
        for i=1:size(self.data.spines,2)
            if (size(db.spines,2)==0)
                uid=1;
            else
                uid=max([db.spines.uid])+1;
            end
            %xuesong added
            names=fieldnames(self.data.spines(i));
            for j=1:size(names,1)
                db.spines(uid).(char(names(j)))=self.data.spines(i).(char(names(j)));
            end
            %xuesong end
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
            %xuesong added
            %names=fieldnames(self.data.spines(i));
            %for j=1:size(names,1)
            %    db.spines(uid).(char(names(j)))=self.data.spines(i).(char(names(j)));
            %end
            %xuesong end
            
            db.spines(uid).uid=uid;
            db.dendrites(den_uid).spineuids=[db.dendrites(den_uid).spineuids uid];
            db.dendrites(den_uid).spineints=[db.dendrites(den_uid).spineints db.spines(uid).intensity];
            db.spines(uid).seriesuid=seriesuid;
            db.spines(uid).experimentuid=experimentuid;
        end
        for i=1:max(size(self.data.dendrites)) % consider only first dendrite for now
            uid=self.data.dendrites(i).uid;
            %terminal bouton
            tlength=[db.spines(db.dendrites(uid).spineuids).len];
            db.dendrites(uid).ibsterminal=db.dendrites(uid).totallength/sum(tlength>0);
            %non-terminal bouton
            db.dendrites(uid).ibs=db.dendrites(uid).totallength/sum(tlength==0);
            db.dendrites(uid).variability=std(db.dendrites(uid).medianfiltered);
            if isfield(db.experiments(experimentuid),'ROIintensity')
                db.dendrites(uid).ROIintensity=db.experiments(experimentuid).ROIintensity;
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
    clipboardheader=[];
    clipboardcontent=[];
    
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
        
        answer=inputdlg({'Enter thethreshld for stable bouton'},'Enter thethreshld for stable bouton',1,{'1.7'});
        stablethres=str2num(answer{1});
        
        lastlabel=0;
        % Retrace all spines across days
        %xuesong begin
        done=0
        repeating=0
        while ~done
            
            if max(size(db.series([db.series.uid]==seriesuid).exclusion))>=i
                exclusion=db.series([db.series.uid]==seriesuid).exclusion{i};
            else
                exclusion=[];
            end
            %xuesong end
            mindis=-10000000;
            maxdis=100000000;
            for j=1:size(overdayden,2)
                mindis=max(mindis,min(overdayden(j).overdaylen));
                maxdis=min(maxdis,max(overdayden(j).overdaylen));
            end
            db.series([db.series.uid]==seriesuid).mindis=mindis;
            db.series([db.series.uid]==seriesuid).maxdis=maxdis;
            %code to estimate noise level
            %
            
            %disp('std/mean');
            %         disp(ratio1);
            %         disp(ratio2);
            %
            
            % reanalyze for meanback
            try
                for ii=1:length(overdayden)
                    for jj=1:4
                        if jj<4 || isfield(overdayden(ii),'maxint3')  
                            switch jj
                                case 1
                                    maxint=overdayden(ii).maxint;
                                case 2
                                    maxint=overdayden(ii).maxint1;
                                case 3
                                    maxint=overdayden(ii).maxint2;
                                case 4
                                    maxint=overdayden(ii).maxint3;
                            end
                            m=double(median(maxint));
%                             step=round(m/100);
%                             if step<1
%                                 step=1;
%                             end
%                             
%                             [histogram]=histc(maxint(maxint<m*3),(0:step:m*3)');
%                             
%                             [t,maxind]=max(smooth(histogram,m*2/step,'rloess'));
%                             
%                             %interate to find self-consistent solution.
%                             for z=1:4
%                                 [t,maxind]=max(smooth(histogram,min(length(histogram)-1,maxind*2/step),'rloess'));
%                             end
                            
                         %   maxind=maxind*step;
%                             switch jj
%                                 case 1
%                                     overdayden(ii).meanback=maxind;
%                                 case 2
%                                     overdayden(ii).meanback1=maxind;
%                                 case 3
%                                     overdayden(ii).meanback2=maxind;
%                                 case 4
%                                     overdayden(ii).meanback3=maxind;
%                             end
                            switch jj
                                case 1
                                    overdayden(ii).meanback=m;
                                case 2
                                    overdayden(ii).meanback1=m;
                                case 3
                                    overdayden(ii).meanback2=m;
                                case 4
                                    overdayden(ii).meanback3=m;
                            end
                        end
                    end
                end
            catch ME
                ME
            end
            
            if ~isfield(db.series([db.series.uid]==seriesuid),'exclusion')
                db.series([db.series.uid]==seriesuid).exclusion=[];
            end
            %        done=0
            %        repeating=0
            %xuesong add
            %        while ~done
            %            if max(size(db.series([db.series.uid]==seriesuid).exclusion))>=i
            %                exclusion=db.series([db.series.uid]==seriesuid).exclusion{i};
            %            else
            %                exclusion=[];
            %            end
            %xuesong end
            % check for imaging abnormalities
            % check for big background change
       
            %
            
            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
            %
            %   Cluster potential site positions
            %
            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
            
            
            overdaypos={};
            answer=inputdlg({'Enter the min dip '},'Enter the min dip',1,{'0.5'});
            mindip=str2num(answer{1});
            
            answer=inputdlg({'Enter the min bouton width'},'Enter the min bouton width',1,{'10'});
            minwidth=str2num(answer{1});
            for k=1:max(size(overdayden))
                overdayden(k).maxint=reflectivesmooth(double(overdayden(k).maxintraw),7);
                % put in code here to deal with dual channel data
                
                
                % put in code to get threshold data
                if isfield(overdayden(k),'maxintdual')
                    overdaypos{k}=overdayden(k).overdaylen(findTerminals(double(overdayden(k).maxintdual)./median(double(overdayden(k).maxintdual)), mindip, stablethres, 30,minwidth));
                else
                    overdaypos{k}=overdayden(k).overdaylen(findTerminals(double(overdayden(k).maxint)./median(double(overdayden(k).maxint)), mindip, stablethres, 30,minwidth));
                end
            end
            terminaloverdaypos={}
            for k=1:max(size(overdayden))
                part1=[db.spines.den_uid]==overdayden(k).uid;
                part2=[db.spines.len]>0;
                part3=[db.spines(part1&part2).dendis];
                terminaloverdaypos{k}=overdayden(k).overdaylen(part3);
                %terminaloverdaypos{k}=overdayden(k).overdaylen([db.spines(([db.spines.den_uid]==overdayden(k).uid) & ([db.spines.len]>0) ).dendis]);
            end
            
            repeating=0;
            if repeating
                answer=inputdlg({'Enter dendrite range to exclude from analysis:'},'Do you want to exclude boutons from analysis? (e.g. 1-10,7-7.5)',1,{''});
                if isempty(answer)
                    exclude=[];
                else
                    answer=answer{1};
                    exclude=[];
                    while ~isempty(answer)
                        [tok,rem]= strtok(answer,',');
                        [first,second]=strtok(tok,'-');
                        firstnum=str2num(first);
                        secondnum=-str2num(second);
                        if (secondnum>firstnum)
                            exclude=[exclude; firstnum secondnum];
                        end
                        answer=rem;
                    end
                    db.series([db.series.uid]==seriesuid).exclusion{i}=exclude;
                    % reset spineacrossday
                    done=0;
                end
            else
                exclude=[];
            end
            
            
            answer=inputdlg({'Enter the distance tolerance for boutons to be considered the same:'},'Enter the distance tolerance for boutons to be considered the same',1,{'1.5'});
            tolerance=str2num(answer{1})
            
            [positions,daypositions,terminalpositions,dayterminalpositions]=clusterboutons(overdayden,overdaypos,terminaloverdaypos,tolerance,minwidth,exclude);
            
            figure;
            [val,ind]=sort(positions);
            hold on;
            % plot the assignment of clusters
            for j=1:max(size(positions))
                plot(daypositions((daypositions(:,j)~=0),j),j,'.');
            end
            %xlabel(path);
            title(['clustering' num2str(seriesuid) '_' num2str(i)]);
            %            saveas(gcf,['clustering' num2str(seriesuid) '_' num2str(i)],'fig');
            
            
            acrossday=getoverdayintensities(overdayden,positions,daypositions,tolerance,'maxint');
            
            acrossday1=getoverdayintensities(overdayden,positions,daypositions,tolerance,'maxint1');
            acrossday2=getoverdayintensities(overdayden,positions,daypositions,tolerance,'maxint2');
            if exist('maxint3')
            acrossday3=getoverdayintensities(overdayden,positions,daypositions,tolerance,'maxint3');
            end
            
            
            terminalacrossdaylength=[];
            
            if (1)  % 1 for assigning terminal boutons based on distance, 0 for based on labeling
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
            
            daystartpos=[];
            dayendpos=[];
            for j=1:size(overdayden,2)
                aa=find(overdayden(j).overdaylen>mindis);
                if size(aa,1)>0
                    daystartpos(j)=aa(1);
                else
                    daystartpos(j)=1;
                end
                aa=find(overdayden(j).overdaylen>=maxdis);
                if size(aa,1)>0
                    dayendpos(j)=aa(1);
                else
                    dayendpos(j)=length(overdayden(j).overdaylen);
                end
            end
            
            index=find((terminalpositions>mindis) & (terminalpositions<maxdis));
            terminalpositions=terminalpositions(index); % is the position in range for all days
            [terminalpositions,ord]=sort(terminalpositions);
            index=index(ord);
            dayterminalpositions=dayterminalpositions(:,index);
            terminalacrossday=terminalacrossday(:,index);
            terminalacrossdaylength=terminalacrossdaylength(:,index);
            
            [scalea,scaleb,ratio1,ratio2]=estimatenoise(overdayden,exclusion);
            
            % reestimate meanback using just the common stretch
            for j=1:size(overdayden,2)
                overdayden(j).meanback=median(overdayden(j).maxint(daystartpos(j):dayendpos(j)));
            end
            
            
            
            
            [a,b,variations,variations1,variations2,ratios,ratios1,ratios2,turnover,turnoverputative,turnoverputative1,turnoverputative2,meanmeanback]=calculatechanges(acrossday,terminalacrossday,[overdayden.meanback],0,scalea,stablethres,1.3);
            summarygraphshort(overdayden,positions,terminalpositions,daypositions,dayterminalpositions,acrossday,terminalacrossday,stablethres,lastlabel,a,b,pathnames,daystartpos,dayendpos);
            ButtonName = questdlg('Are you happy with the alignment?', ...
                'Happy Question', ...
                'Yes', 'No','No');
            if strcmp(ButtonName,'Yes')
                done=1
            end
            repeating=1;
        end
        try
            [a,b,variations,variations1,variations2,ratios,ratios1,ratios2,turnover,turnoverputative,turnoverputative1,turnoverputative2,meanmeanback]=calculatechanges(acrossday,terminalacrossday,[overdayden.meanback],0,scalea,stablethres,1.3);
            
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
        
        summarygraph(overdayden,positions,terminalpositions,daypositions,dayterminalpositions,acrossday,terminalacrossday,lastlabel,a,b,pathnames);
        saveas(gcf, ['summary' num2str(seriesuid) '_' num2str(i)], 'fig');
        
        % add summary data to database
        
        % also add summary data to clipboard
        
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
        
        %latest by Sen
        db.summaries(summaryuid).acrossday1=acrossday1;
        db.summaries(summaryuid).acrossday2=acrossday2;
        if exist('acrossday3')
            db.summaries(summaryuid).acrossday3=acrossday3;
        else
            db.summaries(summaryuid).acrossday3=[];
        end
        
        db.summaries(summaryuid).terminalacrossday=terminalacrossday;
        
        %latest by Sen
        db.summaries(summaryuid).meanback=[overdayden.meanback];
        db.summaries(summaryuid).meanback1=[overdayden.meanback1];
        db.summaries(summaryuid).meanback2=[overdayden.meanback2];
        if isfield(overdayden,'meanback3')
            db.summaries(summaryuid).meanback3=[overdayden.meanback3];
        else
            db.summaries(summaryuid).meanback3=[];
        end
        
        db.summaries(summaryuid).scaledacrossday=acrossday./(db.summaries(summaryuid).meanback'*ones(1,size(acrossday,2)));
        db.summaries(summaryuid).scaledacrossday1=acrossday1./(db.summaries(summaryuid).meanback1'*ones(1,size(acrossday,2)));
        db.summaries(summaryuid).scaledacrossday2=acrossday2./(db.summaries(summaryuid).meanback2'*ones(1,size(acrossday,2)));
        if exist('acrossday3')
            db.summaries(summaryuid).scaledacrossday3=acrossday3./(db.summaries(summaryuid).meanback3'*ones(1,size(acrossday,2)));
        else
            db.summaries(summaryuid).scaledacrossday3=[];
        end
        
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
        
        %latest by Sen
        l=ones(size(db.summaries(summaryuid).scaledacrossday,2),16-size(db.summaries(summaryuid).meanback,2))*99999;
        l1=ones(size(db.summaries(summaryuid).scaledacrossday,2),1);
        % we use big negative number to indicate a empty cell for now
        % later these are converted to NaN and eventually an empty cell for
        % excel sheet
        item=[l1*db.summaries(summaryuid).meanback -l db.summaries(summaryuid).scaledacrossday' -l exp(abs(diff(log(db.summaries(summaryuid).scaledacrossday'),1,2))) -l -l1*99999];
        item=[item l1*db.summaries(summaryuid).meanback1 -l db.summaries(summaryuid).scaledacrossday1' -l exp(abs(diff(log(db.summaries(summaryuid).scaledacrossday1'),1,2))) -l -l1*99999];
        item=[item l1*db.summaries(summaryuid).meanback2 -l db.summaries(summaryuid).scaledacrossday2' -l exp(abs(diff(log(db.summaries(summaryuid).scaledacrossday2'),1,2))) -l -l1*99999];
      
        
        
        % perform quality checking
        % print average s/n
             for ii=1:length([overdayden.ibs])-1
                if abs(log(overdayden(ii).meanback)-log(overdayden(ii+1).meanback))>0.2
                    disp(['meanback ' num2str(ii) '-' num2str(overdayden(ii).meanback) ' : ' num2str(overdayden(ii+1).meanback)]);
                end
             end
        
        
       disp('meanback:');
       disp(db.summaries(summaryuid).meanback);
       disp('meanback change ratio:');
       cm=exp(diff(log(db.summaries(summaryuid).meanback))); 
       disp(cm);
        disp('S/N is');
        a=db.summaries(summaryuid).scaledacrossday;
%        db.summaries(summaryuid).acrossdayindex
        t=find(prod(double(a>1.05))>0);
        m=mean(a(:,t),2)';
        disp(m);
        disp('S/N change ratio is');
        c=exp(diff(log(m)));
        disp(c);
        disp('problem spots');
        % first meanback should be larger than 60
        c1=db.summaries(summaryuid).meanback<60;
        % mean meanback should be larger than 80
        c2=ones(size(m))*(mean(db.summaries(summaryuid).meanback)<80);
        % meanback and S/N ratios make coordinated changes
        c3=[(cm>1.3 & c<0.75) | (cm<0.75 & c>1.3)];
        p=c1 | c2 | [0 c3] | [c3 0];
        disp(p);
        
        
        
        
        clipboardcontent=[clipboardcontent; item];
%          meanback..\;db.summaries(summaryuid).scaledacrossday;exp(abs(diff(log(db.summaries(summaryuid).scaledacrossday))));1 column of 0; meanback1..db.summaries(summaryuid).scaledacrossday1;db.summaries(summaryuid).scaledacrossday2;db.summaries(summaryuid).scaledacrossday3];
        for i=1:size(db.summaries(summaryuid).scaledacrossday,2)
            clipboardheader=[clipboardheader; {[db.summaries(summaryuid).names{1} '-' num2str(db.summaries(summaryuid).acrossdayindex(i))]}];
        end
    end
catch ME
    disp('problem processing series');
    disp(lasterr);
end

ButtonName = questdlg('Do you want to save to database?', ...
    'Happy Question', ...
    'Yes', 'No','No');
if strcmp(ButtonName,'Yes')
    save('-mat',obj.state.dbpath,'db');
end

clipboardcontent(isinf(clipboardcontent))=9999;
clipboardcontent(clipboardcontent<-9000)=NaN;


% place the tracing results on the clipboard
clipboard('copy',printmatrix(clipboardcontent,clipboardheader));
printmatrix(clipboardcontent,clipboardheader)

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
                try
                    load('-mat',[path '\' name ext]);
                    self.gh=[];
                    
                catch
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
                end
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
                save('-mat',[path '\' name ext],'self');
            end
        end
    catch
        disp(['problem with saving data to sereis:' path]);
        disp(lasterr);
    end
end