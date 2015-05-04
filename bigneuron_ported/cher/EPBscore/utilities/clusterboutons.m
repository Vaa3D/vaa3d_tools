function [positions,daypositions,terminalpositions,dayterminalpositions]=clusterboutons (overdayden,overdaypos,terminaloverdaypos,threshold,terminalthreshold,exclusion)
try
    % get all potential positions
    positions=[];
    spineintensity=[];
    allpos=[overdaypos{:}]';
    alldaypos=[];
    assignment=[];
    for k=1:max(size(overdayden))
        if (max(size(overdaypos{k}))>0)
            onedaypos=zeros(max(size(overdaypos{k})),max(size(overdayden)));
            onedaypos(:,k)=overdaypos{k}';
            alldaypos=[alldaypos;onedaypos];
        end
    end
    if (max(size(alldaypos))>0)
        if (size(alldaypos,1)>2) 
            Y=pdist(alldaypos,@distdendis);
            Z=linkage(Y,'complete');
            assignment=cluster(Z,'cutoff',threshold,'criterion','distance');
            positions=[];
            for k=1:max(assignment)
                positions=[positions mean(allpos(assignment==k))];
            end
        else
            if (size(alldaypos,1)==2)
                a=sum(alldaypos,1);
                a=a(abs(a)>0);
                if (abs(diff(a))<0.5) %different days same place
                    positions=[mean(a)];
                    assignment=[1;1];
                else 
                    positions=alldaypos(:);
                    positions=positions(abs(positions)>0)';
                    assignment=[1;2];
                end
            else     
                positions=[sum(allpos)];
                assignment=[1];
            end
            disp('less than 3 boutons!!!!!!!!!!!!! needs work');
        end
    end
    daypositions=zeros(max(size(overdayden)),max(assignment));
    for k=1:max(assignment)
        for j=1:max(size(overdayden))
            daypositions(j,k)=sum(alldaypos(:,j).*(assignment==k));
        end
    end
    % get all potential positions for terminal boutons
    
    terminalspineintensity=[];
    terminalpositions=[];
    
    terminalallpos=[terminaloverdaypos{:}]';
    terminalalldaypos=[];
    terminalassignment=[];
    for k=1:max(size(overdayden))
        if (max(size(terminaloverdaypos{k}))>0)
            onedaypos=zeros(max(size(terminaloverdaypos{k})),max(size(overdayden)));
            onedaypos(:,k)=terminaloverdaypos{k}';
            terminalalldaypos=[terminalalldaypos;onedaypos];
        end
    end
    terminalassignment=[];
    if (max(size(terminalalldaypos))>0)
        if (size(terminalalldaypos,1)>2) 
            Y=pdist(terminalalldaypos,@distdendis);
            Z=linkage(Y,'average');
            terminalassignment=cluster(Z,'cutoff',terminalthreshold,'criterion','distance');
            terminalpositions=[];
            for k=1:max(terminalassignment)
                terminalpositions=[terminalpositions mean(terminalallpos(terminalassignment==k))];
            end
        else
            if (size(terminalalldaypos,1)==2)
                a=sum(terminalalldaypos,1);
                a=a(abs(a)>0);
                if (abs(diff(a))<0.5) %different days same place
                    terminalpositions=[mean(a)];
                    terminalassignment=[1;1];
                else 
                    terminalpositions=terminalalldaypos(:);
                    terminalpositions=terminalpositions(abs(terminalpositions)>0)';
                    terminalassignment=[1;2];
                end
            else     
                terminalpositions=[sum(terminalallpos)];
                terminalassignment=[1];
            end
            disp('less than 3 terminal boutons!!!!!!!!!!!!! needs work');
        end
    end    
    
    dayterminalpositions=zeros(max(size(overdayden)),max(terminalassignment));    
    for k=1:max(terminalassignment)
        for j=1:max(size(overdayden))
            dayterminalpositions(j,k)=sum(terminalalldaypos(:,j).*(terminalassignment==k));
        end
    end    
    
    
    mindis=-10000000;
    maxdis=100000000;
    for j=1:size(overdayden,2)
        mindis=max(mindis,min(overdayden(j).overdaylen));
        maxdis=min(maxdis,max(overdayden(j).overdaylen));
    end
    
    
    index=(positions>mindis) & (positions<maxdis);
    for k=1:size(exclusion,1)
        index=index & ((positions<exclusion(k,1))|(positions>exclusion(k,2)));
    end
    index=find(index);
    positions=positions(index); % is the position in range for all days   
    [positions,ord]=sort(positions);
    index=index(ord);
    daypositions=daypositions(:,index);
    
    index=find((terminalpositions>mindis) & (terminalpositions<maxdis));
    terminalpositions=terminalpositions(index); % is the position in range for all days   
    [terminalpositions,ord]=sort(terminalpositions);
    index=index(ord);
    dayterminalpositions=dayterminalpositions(:,index);
    
catch
    disp('error clustering boutons');
    disp(path);
end