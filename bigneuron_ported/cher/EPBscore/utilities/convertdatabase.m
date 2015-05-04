cd c:\axons\obselete\indatabase
%load -mat control.db
load -mat A3control.db  
myopen('localhost','song','amanda');
dbopen('axons');
IDs=1:length(db.summaries);
gExperimentType='Control';
gType='Tha';
global db;
try
    for i=1:length(IDs)
        % insert dendrites


        % insert boutons

        % name of animal?

        if ~isempty(db.summaries(i).uid)
            name=db.series(db.summaries(IDs(i)).seriesuid).names{1};
            if strcmp(name,'vd039')
               name= db.series(db.summaries(IDs(i)).seriesuid).names{2};
               name(6)='a';
               db.series(db.summaries(IDs(i)).seriesuid).names{1}=name;
            end
                    
            token='([a-zA-Z\-]{2,10}\d{1,3})[a-z]{1}[_]{0,1}\d{2,3}';
            a=regexp(name,token,'tokens');
            b=a{1};
            b=b{1};

            animalid=mym('SELECT id FROM animal WHERE Name="{S}"', b);
            if isempty(animalid)
                mym('INSERT INTO animal(Name) VALUES("{S}")',b);
                animalid=mym('SELECT LAST_INSERT_ID();');
            end

            Other=db.summaries(i);
            fields={'uid','seriesuid','dendriteindex','positions','terminalpositions'};
            Other=rmfield(Other,fields);

            % remove empty fields
            names=fieldnames(Other);
            for k=1:length(names)
                if isempty(Other.(names{k}))
                    Other.(names{k})=[];
                end
            end

            exclusion=db.series(db.summaries(i).seriesuid).exclusion;
            if length(exclusion)>1
                if db.summaries(i).dendriteindex<=length(db.series(db.summaries(i).seriesuid).exclusion)
                    exclusion=db.series(db.summaries(i).seriesuid).exclusion(db.summaries(i).dendriteindex);
                    if isempty(exclusion{1})
                        exclusion={};
                    end
                else
                    exclusion={};
                end
            end

            mym('INSERT INTO series(AnimalID, Exclusion, ExperimentType, Other) VALUES("{Si}","{M}","{S}","{M}")',animalid,exclusion,gExperimentType,Other);
            seriesid=mym('SELECT LAST_INSERT_ID();');

            % boutonclusters=[];
            % for k=1:length(db.summaries(i).position)

            series=db.series(db.summaries(IDs(i)).seriesuid);
            % insert dendrites
            for j=1:length(series.dendrites)
                dendrite=db.dendrites(series.dendrites(db.summaries(i).dendriteindex,j));
                Other=dendrite;
                fields={'uid','seriesuid','experimentuid','spineuids','spineints'};
                Other=rmfield(Other,fields);

                % remove empty fields
                names=fieldnames(Other);
                for k=1:length(names)
                    if isempty(Other.(names{k}))
                        Other.(names{k})=[];
                    end
                end
                mym('INSERT INTO segment(SeriesID, Session, ImageName, AnimalID, Length,  BackboneIntensity, Distance, Intensity, Coordinates, Type, Other,Datetime) VALUES("{Si}","{Si}","{S}","{Si}","{S}","{S}","{M}","{M}","{M}","{S}","{M}",NOW())',...
                seriesid,j,series.names{j},animalid,dendrite.totallength,dendrite.meanback,dendrite.overdaylen,dendrite.maxint,dendrite.voxel(1:3,:),gType,Other);
                segmentid=mym('SELECT LAST_INSERT_ID();');
            end
            % regenerate all the boutons

            %         % insert terminal boutons
            %         for k=1:length(dendrite.spineuids)
            %             spine=db.spines(dendrite.spineuids(k));
            %             if spine.len>0
            %             Other=spine;
            %             fields={'uid','den_uid','intensity','den_ind','dendis','type','len','label','seriesuid','experimentuid','overdaydis'};
            %             Other=rmfield(Other,fields);
            %             try
            %                 mym('INSERT INTO bouton(SegmentID, Distance, AlignedDistance, Length, ClusterID,  Type, Intensity, NormalizedIntensity, Other) VALUES("{Si}","{S}","{S}","{S}", "{Si}","{Si}","{S}","{S}","{M}")',segmentid,dendrite.length(spine.dendis),spine.overdaydis,spine.len,spine.label,spine.type,spine.intensity,spine.intensity/dendrite.meanback,Other);
            %             catch
            %                 display('error');
            %                 spine.label
            %             end
            %         end


            threshold=1.5;
            higherthreshold=1.8;
            changethresholdsdb(db,seriesid,threshold,higherthreshold,series.dendrites(db.summaries(i).dendriteindex,:),IDs(i));

        end
        %     end
    end

    % generate summary sheet

    %               session 1   session 2  session 3 session 4 session 5
    % clusterid

    % Build rows

    [clusterid, session, name, type, normalizedintensity]=mym('SELECT b.clusterid, s.session, s.imagename, b.normalizedintensity from bouton as b inner join segment as s on b.segmentid=s.id and b.length=0');
    u=unique(clusterid);
    indexing=sparse(u,1,(1:length(u))');
    rows=indexing(clusterid);
    S=full(sparse(rows,session,normalizedintensity))
    N=full(sparse(rows,session,1:length(name)))
    segmentname=name(full(N(:,1)));

    % insert boutons

    %db.summaries


catch
    lasterr
end