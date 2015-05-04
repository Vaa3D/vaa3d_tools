function db=changethresholds(db,summariesuids,threshold,higherthreshold)
try
for i=1:size(summariesuids,2)
    seriesuid=db.summaries(summariesuids(i)).seriesuid;
    if max(size(db.series([db.series.uid]==seriesuid).exclusion))>=db.summaries(summariesuids(i)).dendriteindex
        exclusion=db.series([db.series.uid]==seriesuid).exclusion{db.summaries(summariesuids(i)).dendriteindex};
    else
        exclusion=[];
    end
    overdayden=[db.dendrites(db.series([db.series.uid]==seriesuid).dendrites(db.summaries(summariesuids(i)).dendriteindex,:))];
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
    
    overdaypos={};
    for k=1:max(size(overdayden))        
        overdayden(k).maxint=reflectivesmooth(double(overdayden(k).maxintraw),7);
        overdaypos{k}=overdayden(k).overdaylen(findTerminals(double(overdayden(k).maxint)./median(double(overdayden(k).maxint)), 0.5, threshold, 30 ,10));
        overdayposbig{k}=overdayden(k).overdaylen(findTerminals(double(overdayden(k).maxint)./median(double(overdayden(k).maxint)), 0.5, higherthreshold, 30 ,10));
    end   
    terminaloverdaypos={}; 
    for k=1:max(size(overdayden))                    
        terminaloverdaypos{k}=overdayden(k).overdaylen([db.spines(([db.spines.den_uid]==overdayden(k).uid) & ([db.spines.len]>0) ).dendis]);
    end   
    
    [positions,daypositions,terminalpositions,dayterminalpositions]=clusterboutons(overdayden,overdaypos,terminaloverdaypos,1.5,5,exclusion);

    isbig=[];
    for k=1:max(size(overdayden))
        isbig=[isbig; ismember(daypositions(k,:),overdayposbig{k})];
    end
        
        
        
    acrossday=getoverdayintensities(overdayden,positions,daypositions);
    % Add in terminal boutons
    terminalacrossday=[];
%     for k=1:size(overdayden,2)
%         for j=1:max(size(terminalpositions))
%             intensity=[db.spines(([db.spines.den_uid]==overdayden(k).uid) & ([db.spines.len]>0) ).intensity];
%             index=find(terminaloverdaypos{k}==dayterminalpositions(k,j)); 
%             % Was there a peak detected on this particular day
%             if (dayterminalpositions(k,j)>0)
%                 terminalacrossday(k,j)=intensity(index(1));
%             else
%                 terminalacrossday(k,j)=0;
%             end            
%         end
%     end
    [a,b,variations,variations1,variations2,ratios,ratios1,turnover,turnoverputative,turnoverputative1,turnoverputative2,meanmeanback]=calculatechanges(acrossday,terminalacrossday,[overdayden.meanback],0.3,scalea,threshold,threshold);
 
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
    
    
    % add summary data to database    
    
    
    summaryuid=summariesuids(i);
    
    lastlabel=0;
    db.summaries(summaryuid).meanmeanback=meanmeanback;
    db.summaries(summaryuid).positions=positions;
    db.summaries(summaryuid).terminalpositions=terminalpositions;
    db.summaries(summaryuid).daypositions=daypositions;
    db.summaries(summaryuid).isbig=isbig;
    db.summaries(summaryuid).dayterminalpositions=dayterminalpositions;
    db.summaries(summaryuid).acrossday=acrossday;
    db.summaries(summaryuid).terminalacrossday=terminalacrossday;
    if ~isempty(acrossday)
        
        db.summaries(summaryuid).scaledacrossday=acrossday.*(a'*ones(1,size(acrossday,2)));
    else
        db.summaries(summaryuid).scaledacrossday=[];
    end
    if ~isempty(acrossday)
        db.summaries(summaryuid).backbonescaledacrossday=acrossday./([overdayden.meanback]'*ones(1,size(acrossday,2)));;
    else
        db.summaries(summaryuid).backbonescaledacrossday=[];
    end
    % in the new version backbonescaledacrossday is just called
    % scaledacrossday
    db.summaries(summaryuid).scaledacrossday=db.summaries(summaryuid).backbonescaledacrossday;
    
    if isempty(terminalacrossday)
        db.summaries(summaryuid).scaledterminalacrossday=[];
    else            
        db.summaries(summaryuid).scaledterminalacrossday=terminalacrossday.*(a'*ones(1,size(terminalacrossday,2)));
    end
    db.summaries(summaryuid).acrossdayindex=lastlabel+(1:max(size(positions)));
    db.summaries(summaryuid).terminalacrossdayindex=lastlabel+max(size(positions))+(1:max(size(terminalpositions)));
    db.summaries(summaryuid).names=db.series([db.series.uid]==seriesuid).names;
    db.summaries(summaryuid).correctionfactora=a;
    db.summaries(summaryuid).correctionfactorb=b;
    db.summaries(summaryuid).variations=variations;
    db.summaries(summaryuid).ratios=ratios;
    db.summaries(summaryuid).ratios1=ratios1;
    db.summaries(summaryuid).ratios2=diff(log(db.summaries(summaryuid).backbonescaledacrossday),1);
    db.summaries(summaryuid).turnover=turnover;
    db.summaries(summaryuid).turnoverputative=diff(db.summaries(summaryuid).scaledacrossday>threshold);
    presence=sum((db.summaries(summaryuid).scaledacrossday>threshold)');

    db.summaries(summaryuid).turnoverputativeratio=sum(abs(db.summaries(summaryuid).turnoverputative'))./presence(1:end-1);

    db.summaries(summaryuid).turnoverputative1=diff(db.summaries(summaryuid).daypositions>0);
    presence=sum((db.summaries(summaryuid).daypositions>0)');

    db.summaries(summaryuid).turnoverputativeratio1=sum(abs(db.summaries(summaryuid).turnoverputative1'))./presence(1:end-1);

    
    
    
    %db.summaries(summaryuid).turnoverputative1=turnoverputative1;
    db.summaries(summaryuid).turnoverputative2=turnoverputative2;
    db.summaries(summaryuid).variations1=variations1;
    db.summaries(summaryuid).variations2=variations2;
    db.summaries(summaryuid).scalea=scalea;
    db.summaries(summaryuid).ratio1=ratio1;
    db.summaries(summaryuid).ratio2=ratio2;
    db.summaries(summaryuid).ibs=ibs;
    db.summaries(summaryuid).icbs=icbs;
    db.summaries(summaryuid).itbs=itbs;
    db.summaries(summaryuid).meanback=[overdayden.meanback];
end
catch
    summariesuids(i)
end
% db.summaries(summariesuids).turnoverputativeratio
% mean(mean([db.summaries(summariesuids).turnoverputativeratio]))
% 
% mean(sum(abs([db.summaries(summariesuids).turnoverputative1]),2))/size(abs([db.summaries(summariesuids).turnoverputative1]),2)
% 
% ind=sum([db.summaries(summariesuids).scaledacrossday]>4)>0;
% turnover=abs([db.summaries(summariesuids).turnoverputative1]);
% mean(sum(turnover(:,ind),2))/size(turnover,2)
% 
% hold on;
% subplot(2,2,1);
% hold on;
% errorbar(mean(abs([db.summaries(summariesuids).variations])'),std(abs([db.summaries(summariesuids).variations]'))/sqrt(size([db.summaries(summariesuids).variations]',1)),'c');
% subplot(2,2,2);
% hold on;
% errorbar(mean(abs([db.summaries(summariesuids).variations2])'),std(abs([db.summaries(summariesuids).variations2]'))/sqrt(size([db.summaries(summariesuids).variations2]',1)),'c');
% subplot(2,2,3);
% hold on;
% errorbar(mean(abs([db.summaries(summariesuids).ratios])'),std(abs([db.summaries(summariesuids).ratios]'))/sqrt(size([db.summaries(summariesuids).ratios]',1)),'c');
% subplot(2,2,4);
% hold on;
% errorbar(mean(abs([db.summaries(summariesuids).ratios1])'),std(abs([db.summaries(summariesuids).ratios1]'))/sqrt(size([db.summaries(summariesuids).ratios1]',1)),'c');