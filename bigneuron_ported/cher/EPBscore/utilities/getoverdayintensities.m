function acrossday=getoverdayintensities(overdayden,positions,daypositions,tolerance,field);
% get the values at the sites from each time point
acrossday=[];
try
    for k=1:size(overdayden,2)
        for j=1:max(size(positions))
            % Was there a peak detected on this particular day
            if (daypositions(k,j)>0)
                left=[find(overdayden(k).overdaylen>daypositions(k,j)-tolerance/2) 1];
                right=find(overdayden(k).overdaylen<daypositions(k,j)+tolerance/2);
                if (right(end)<left(1))
                    right(end)=right(end)+1;
                    left(1)=left(1)-1;
                end
                
                
                
                if isfield(overdayden(k),'maxintdual')
                    acrossday(k,j)=max(overdayden(k).maxintdual(left(1):right(end)));  
                    % figure out code to normalize across the two channels
                   
                    
                else
                    t=getfield(overdayden(k),field);
                    [pks, pos] = findpeaks(t(left(1):right(end)));
                    if size(pos,2)==1 % peak is at the edge
                        for l=1:length(pos)
                            if (pos==1) || (pos==(right(end)-left(1)+1))
                                pks(l)=-100
                            end
                        end
                    end
                    
                    if max(pks)>0
                        maxint=max(pks);
                        acrossday(k,j)=maxint;
                    else
                        p=round(daypositions(k,j));
                        if p<1
                            p=1;
                        end
                        if p>length(t)
                            p=length(t);
                        end
                        acrossday(k,j)=t(round(daypositions(k,j)));
                    end
                end
            else
                %                    if 1%(db.series([db.series.uid]==seriesuid).spineacrossday(k,j+lastlabel)==0) && (db.series([db.series.uid]==seriesuid).spineacrossday(k,j+lastlabel)==0)
                left=[find(overdayden(k).overdaylen>positions(j)-tolerance/2) 1];
                right=find(overdayden(k).overdaylen<positions(j)+tolerance/2);
                if (right(end)<left(1))
                    right(end)=right(end)+1;
                    left(1)=left(1)-1;
                end
                if isfield(overdayden(k),'maxintdual')
                    acrossday(k,j)=max(overdayden(k).maxintdual(left(1):right(end)));  
                    % figure out code to normalize across the two channels
                    
                else
                    t=getfield(overdayden(k),field);
                    stretch=t(left(1):right(end));
                     [pks, pos] = findpeaks(stretch);
                    
                     % if no peaks are found
                     if isempty(pks)
                         pks=-100;
                     end
                     if size(pks,2)==1 % peak is at the edge
                         for l=1:length(pos)
                             if (pos(l)==1) || (pos(l)==(right(end)-left(1)+1))
                                 pks(l)=-100
                             end
                         end
                     end
                    
                    if max(pks)>0
                        maxint=max(pks);
                        acrossday(k,j)=maxint;
                    else
                        try
                        middle=[find(overdayden(k).overdaylen>positions(j))];
                        acrossday(k,j)=t(round(middle(1)));
                        catch
                        end
                    end
                end
            end                    
            %                 voxel=double(overdayden(k).voxel(:,pos+left(1)-1));
            %                 neighborhood=getneighborhoodpixels(imageArray,voxel(2,:),voxel(1,:),voxel(3,:),10,10,3);
            %                 db.series([db.series.uid]==seriesuid).spineacrossdayrawmax(k,j+lastlabel)=max(neighborhood(:));
            %                 [iy,ix]=find(neighborhood==max(neighborhood(:)));
            %                 db.series([db.series.uid]==seriesuid).spineacrossdayrawmax(k,j+lastlabel)=mean(mean(neighborhood(max(1,iy-0):min(size(neighborhood,1),iy+0),max(1,ix-0):min(size(neighborhood,2)*size(neighborhood,3),ix+0))));
            %                 
            %                 
            %                 
            %                 
            %                 s=reflectivesmooth(double(overdayden(k).maxintraw),7);
            %                 overdayden(k).maxintrawsmooth=s';
            %                 db.series([db.series.uid]==seriesuid).spineacrossdayrawsmooth(k,j+lastlabel)=max(s(left(1):right(end)));                
            %                 db.series([db.series.uid]==series
            %                 uid).spineacrossdayrawgaussian(k,j+lastlabel)=max(overdayden(k).maxintrawgaussian(left(1):right(end)));
        end
    end
catch ME
    disp('Error in getting overday intensities');
    j
    k
    disp(ME);
    disp(ME.message);
    disp(ME.stack(1));
end





%         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%        
%         %
%         % search for optimal thresholds
%         %
%         %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% 
%         % find positions with the lowerest threshold
%         
%         spineintensity=[];
%         overdaypos={} 
%         for k=1:max(size(overdayden))        
%             overdaypos{k}=overdayden(k).overdaylen(findTerminals(reflectivesmooth(double(overdayden(k).maxintrawgaussian),7), 0.1, 0.0, 30));
%         end   
%         allpos=[overdaypos{:}]';
%         alldaypos=[];
%         for k=1:max(size(overdayden))
%             onedaypos=zeros(max(size(overdaypos{k})),max(size(overdayden)));
%             onedaypos(:,k)=overdaypos{k}';
%             alldaypos=[alldaypos;onedaypos];
%         end
%         
%         Y=pdist(alldaypos,@distdendis);
%         Z=linkage(Y,'average');
%         assignment=cluster(Z,'cutoff',1,'criterion','distance');
%         positions=[];
%         figure;
%         hold on;
%         for k=1:max(assignment)
%             % plot(allpos(assignment==k),k,'.');
%             positions=[positions mean(allpos(assignment==k))];
%         end
%         [val,ind]=sort(positions);
%         for k=1:max(assignment)
%             plot(allpos(assignment==k),find(ind==k),'.');
%             %        positions=[positions mean(allpos(assignment==k))];
%         end
%         
%         TT=[];
%         TF=[];
%         FF=[];
%         intensity=[];
%         ind=1;
%         global parameters;
%         occupancy=[];
%         parameters=[];
%         for th=0.1:0.2:1.0
%             for ab=0.0:0.3:2.0
%                 for dis=10:3:30
%                     try
%                         spineintensity=zeros(max(size(overdayden)),max(size(positions)));
%                         try
%                             for k=1:max(size(overdayden))        
%                                 overdaypos{k}=overdayden(k).overdaylen(findTerminals(reflectivesmooth(double(overdayden(k).maxintrawgaussian)/median(double(overdayden(k).maxintrawgaussian)),7),th, ab, dis));
%                             end
%                             % catch 
%                             % disp('findterminals');
%                             % k
%                             %end
%                             try
%                                 for k=1:max(size(overdayden))
%                                     pos=overdaypos{k};
%                                     for l=1:max(size(overdaypos{k}))
%                                         spineintensity(k,min(assignment(allpos==pos(l))))=1;
%                                     end
%                                 end
%                             catch
%                                 
%                                 k
%                             end
%                             TT=[TT; sum(mean(spineintensity,1).*mean(spineintensity,1))];
%                             FF=[FF; sum((1-mean(spineintensity,1)).*(1-mean(spineintensity,1)))];
%                             TF=[TF; 2*sum(mean(spineintensity,1).*(1-mean(spineintensity,1)))];
%                             occupancy=[occupancy sum(sum(spineintensity,1)>0)];
%                             intensity(:,:,ind)=spineintensity;
%                             ind=ind+1;
%                             parameters=[parameters;th ab dis];
%                         catch
%                             disp(lasterr);
%                             disp(path);
%                             [th ab dis k]
%                         end
%                     end
%                 end
%             end
%         end
%         global globalTT;
%         global globalTF;
%         global totalpos;
%         global globaloccupancy;
%         try
%             globalTT=[globalTT TT];
%             globalTF=[globalTF TF];
%             globaloccupancy=[globaloccupancy occupancy'];
%             totalpos=totalpos+max(size(positions));
%         catch
%             disp('error adding an entry');
%             disp(path);
%             [th ab dis k];
%         end
%        All positions



%     
%     
%     % get all potential positions
%     mindis=-10000000;
%     maxdis=100000000;
%     for j=1:size(overdayden,2)
%         mindis=max(mindis,min(overdayden(j).overdaylen));
%         maxdis=min(maxdis,max(overdayden(j).overdaylen));
%     end
%     positions=[100000];
%     positions1=[];
%     for j=1:size(spines,2)
%         if ((spines(j).overdaydis)<mindis) || ((spines(j).overdaydis)>maxdis)   % is the spine not in range for all days?
%             spines(j).label=-1;
%             db.spines([db.spines.uid]==spines(j).uid).label=-1;
%         else
%             if (spines(j).len==0) && (spines(j).edittype==0) % only consider spines which pass the more strigent threshold 
%                 if (min(abs(positions - spines(j).overdaydis))>1)          
%                     % add a position
%                     positions=[positions; mean([spines(abs([spines.overdaydis]-spines(j).overdaydis)<1).overdaydis])];
%                     positions1=[positions1;spines(j).overdaydis];
%                 end
%             end
%         end 
%    end
%    positions=sort(positions); 
%    positions=positions(1:end-1); % get rid of the 100000 used to seed the array
%    
