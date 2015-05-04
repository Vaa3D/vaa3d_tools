function changethresholdsdb(db,seriesid,threshold,higherthreshold,dendriteuids,summariesuid)
try
    [exclusion other]=mym('Select Exclusion, Other from series where ID="{Si}"',seriesid);
    exclusion=exclusion{1};
    other=other{1};    
    [segmentid, session, overdayden]=mym('Select ID, Session, Other from segment where seriesid="{Si}" order by session',seriesid);
    for j=1:size(overdayden,1)
        overdayden{j}.uid=dendriteuids(j);
    end
    
    mindis=-10000000;
    maxdis=100000000;
    for j=1:size(overdayden,1)
        mindis=max(mindis,min(overdayden{j}.overdaylen));
        maxdis=min(maxdis,max(overdayden{j}.overdaylen));
    end
    
    
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %
    %   Cluster potential site positions
    %
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    overdaypos={};
    for k=1:length(overdayden)        
        overdayden{k}.maxint=reflectivesmooth(double(overdayden{k}.maxintraw),7);
        overdaypos{k}=overdayden{k}.overdaylen(findTerminals(double(overdayden{k}.maxint)./median(double(overdayden{k}.maxint)), 0.5, threshold, 30 ,10));
        overdayposbig{k}=overdayden{k}.overdaylen(findTerminals(double(overdayden{k}.maxint)./median(double(overdayden{k}.maxint)), 0.5, higherthreshold, 30 ,10));
    end   
    terminaloverdaypos={}; 
    for k=1:max(size(overdayden))                    
        terminaloverdaypos{k}=overdayden{k}.overdaylen([db.spines(([db.spines.den_uid]==overdayden{k}.uid) & ([db.spines.len]>0) ).dendis]);
    end   
    
    %[positions,daypositions,terminalpositions,dayterminalpositions]=clusterboutons(overdayden,overdaypos,terminaloverdaypos,1.5,5,exclusion);

    positions=db.summaries(summariesuid).positions;
    daypositions=db.summaries(summariesuid).daypositions;
    terminalpositions=db.summaries(summariesuid).terminalpositions;
    dayterminalpositions=db.summaries(summariesuid).dayterminalpositions;    
        
    acrossday=getoverdayintensities(overdayden,positions,daypositions);
    terminalacrossday=[];
    for k=1:size(overdayden,1)
        for j=1:max(size(terminalpositions))
            ids=find(([db.spines.den_uid]==overdayden{k}.uid) & ([db.spines.len]>0)) ;
%            index=find(terminalpositions(j)==dayterminalpositions(k,j)); 
            % Was there a peak detected on this particular day
            if (dayterminalpositions(k,j)>0)
                terminalacrossday(k,j)=ids(sum(dayterminalpositions(k,1:j)>0));
            else
                terminalacrossday(k,j)=0;
            end            
        end
    end
  %  [a,b,variations,variations1,variations2,ratios,ratios1,turnover,turnoverputative,turnoverputative1,turnoverputative2,meanmeanback]=calculatechanges(acrossday,terminalacrossday,[overdayden.meanback],0.3,scalea,threshold,threshold);
 
    meanback=[];
    for i=1:length(overdayden)
        meanback=[meanback overdayden{i}.meanback];
    end
    
    % add summary data to database    
    
    if ~isempty(acrossday)
        scaledacrossday=diag(1./meanback)*acrossday;
    else
        scaledacrossday=[];
    end
    
   consensuspos= ones(size(daypositions,1),1)*positions;
   daypositions(daypositions==0)=consensuspos(daypositions==0);
  
    % Add en passant boutons first
    
    for l=1:size(acrossday,2)
        mym('INSERT INTO boutoncluster(SeriesID, AlignedDistance, DateTime) VALUES("{Si}","{S}",NOW())', seriesid,positions(l));
        clusterid=mym('SELECT LAST_INSERT_ID();');
      
        for k=1:size(acrossday,1)            
            mym('INSERT INTO bouton(SegmentID, AlignedDistance, Length, ClusterID,  Type, Intensity, NormalizedIntensity,Datetime) VALUES("{Si}","{S}","{S}", "{Si}","{Si}","{S}","{S}",NOW())', ...
                                    segmentid(k), daypositions(k,l),0,clusterid,1,acrossday(k,l),scaledacrossday(k,l));
        end
    end
        
    % Add terminal boutons
     
        for l=1:size(terminalacrossday,2)
                       mym('INSERT INTO boutoncluster(SeriesID, AlignedDistance, DateTime) VALUES("{Si}","{S}",NOW())', seriesid,terminalpositions(l));
            clusterid=mym('SELECT LAST_INSERT_ID();');
 
               for k=1:size(terminalacrossday,1)
            if dayterminalpositions(k,l)>0
            spine=db.spines(terminalacrossday(k,l));
            Other=[];
            Other.voxel=spine.voxel;
            mym('INSERT INTO bouton(SegmentID, DistanceIndex, AlignedDistance, Length, ClusterID,  Type, Intensity, NormalizedIntensity, Other,Datetime) VALUES("{Si}","{S}","{S}","{S}", "{Si}","{Si}","{S}","{S}","{M}",NOW())', ...
            segmentid(k),spine.dendis, dayterminalpositions(k,l),spine.len,clusterid,2,spine.intensity,spine.intensity/overdayden{k}.meanback,Other);
            end
        end
    end
    
    
    
    
    
catch
    lasterr
end