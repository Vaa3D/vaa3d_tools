function terminalturnover(summaries)
global db;
scaledterminalacrossdays=[];
terminallens=[];
terminalindex=[];
itbs=[];
denlen=[];
names=[];
for i=1:size(summaries,2)  
    if ~isempty(summaries(i).scaledterminalacrossday)
        % get the length of the terminal boutons
        den_uids=db.series([db.series.uid]==summaries(i).seriesuid).dendrites(summaries(i).dendriteindex,:);
        terminalacrossdaylen=zeros(size(summaries(i).scaledterminalacrossday,1),size(summaries(i).terminalacrossdayindex,2));
        for j=1:size(summaries(i).scaledterminalacrossday,1)        
%             index=find(([db.spines.den_uid]==den_uids(j)) & ([db.spines.len]>0) );            
%             
%             for l=1:size(index,2)
%                 if (db.spines(index(l)).label>0)
%                     terminalacrossdaylen(j,find(summaries(i).terminalacrossdayindex==db.spines(index(l)).label))=db.spines(index(l)).len;  
%                 end
%             end
                spines=[db.spines(([db.spines.den_uid]==den_uids(j)) & ([db.spines.len]>0) )];
                       
                for k=1:max(size(summaries(i).terminalpositions))
                       % Was there a peak detected on this particular day
                        if (summaries(i).dayterminalpositions(j,k)>0)
                              index=find([spines.overdaydis]==summaries(i).dayterminalpositions(j,k)); 
                       
                            terminalacrossdaylen(j,k)=spines(index(1)).len;
                        else
                            terminalacrossdaylen(j,k)=0;
                        end            
                end
        end
        terminallens=[terminallens terminalacrossdaylen];
        terminalindex=[terminalindex summaries(i).terminalacrossdayindex];
        denlen=[denlen ones(1,size(summaries(i).terminalacrossdayindex,2))*(summaries(i).maxdis-summaries(i).mindis)];       
        itbs=[itbs ones(1,size(summaries(i).terminalacrossdayindex,2))*mean(1./summaries(i).itbs)];
        
        
        scaledterminalacrossdays=[scaledterminalacrossdays summaries(i).scaledterminalacrossday];
        for j=1:size(summaries(i).scaledterminalacrossday,2)
            names=[names summaries(i).names(1)]; 
        end    
    end
end
terminallens

 