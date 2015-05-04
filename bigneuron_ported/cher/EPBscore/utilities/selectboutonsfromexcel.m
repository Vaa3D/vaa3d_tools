% first read in the source file
function selectboutonsfromexcel

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% change parameters here!!!!
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

gainthres=1.3;
lossthres=1.3;
keepthres=2;
stableperiod=7;

[fname, pname] =  uigetfile({'*.xlsx','*.xls'}, 'Specify the file to analyze');
pane=input('Which pane?','s');

[NUMERIC,TXT,RAW]=xlsread([pname '/' fname],pane);

% find out the data columns
% h=find(strcmp(TXT(1,4:end),''));
% numdays=h(1)-1;


headerrow=3;
header=RAW(1:headerrow,:);

% first calculate bouton inclusion criterion
%include=max(data>1.7,[],2);


backbonecol=11;
startingcol=27;
global startingratiocol;
startingratiocol=17; % following startingcol
backbone=RAW(headerrow+1:end,backbonecol:startingcol-1);
left=RAW(headerrow+1:end,1:startingcol-1);
data=RAW(headerrow+1:end,startingcol:end);

% clean out all texts in the data section
for x=1:size(data,1)
    for y=1:size(data,2)
        if ~isnumeric(data{x,y})
            data(x,y)={NaN};
        end
    end
end

% clean out all texts in the backbone section
for x=1:size(backbone,1)
    for y=1:size(backbone,2)
        if ~isnumeric(backbone{x,y})
            backbone(x,y)={NaN};
        end
    end
end


animalidcol=2;
animalid=RAW(headerrow+1:end,2); % animal ID Column B
axonidcol=3;
axonid=RAW(headerrow+1:end,3); % axon ID column C
ROIidcol=10;
ROIid=left(1:end,10);   % ROI ID column J
axonlengths=RAW(headerrow+1:end,4);  % column D
axonclass=left(1:end,5); % Axcon class column E
sidebranches=left(1:end,6);     %Is it a side branch? column F
septalborder=left(1:end,7);     %Is it a side branch? column G
qualitycheck=left(1:end,end);   % column Z last column before startingcol

splittings=[];

% Restrict to a certain axon class

targetaxonclass=input('which axon class do you want to analyze? (0 for both, 1,2 for each type)');
if targetaxonclass==1 || targetaxonclass==2
    for i=1:size(data,1)
        if axonclass{i}~=targetaxonclass
            data(i,1:end)={NaN};
        end
    end
end
% get rid of quality check fail rows
for i=1:size(data,1)
    if ~sum(qualitycheck{i}<4)
        data(i,1:end)={NaN};
    end
end


% renormalizes the side branches by the backbone intensity of the
% main branch

sidebrancharray=zeros(size(sidebranches));
for i=1:size(sidebranches,1)
    if sidebranches{i}>0
        sidebrancharray(i)=1;
    end
end
ROIs=tabulateROI(ROIid);
for i=1:length(ROIs)
    % any side branches for this axon?
    axonsidebranch=sidebrancharray(ROIs{i});    
    if sum(axonsidebranch)>0
        %sidebranches
        axonsidebranch=ROIs{i}(find(axonsidebranch));
        firstbranch=ROIs{i}(1);
        for j=1:length(axonsidebranch)
              for k=1:12
                  data{axonsidebranch(j),k}=data{axonsidebranch(j),k}*backbone{axonsidebranch(j),k}/backbone{firstbranch,k};
              end
        end
    end
end

%calculate turnover days
newnumeric=[];
newtxt=[];
splittings=[];
for i=1:size(data,1)
    done=0;
    col=1;
    ratiocol=startingratiocol;
    %empty row?
    if data{i,col}>0
        current=data{i,col}>gainthres;
        startings=0;
        if current==0
            data{i,col}=0;
        else
            startings=1; % in the middle of a stretch
        end
        start=1;
        while ~done
            col=col+1;
            if isnan(data{i,col})
                done=1;
            else
                if current==0
                    data{i,ratiocol}=0;
                    if data{i,col}>gainthres
                        current=1;
                        if startings==0
                            startings=1;
                        else
                            % initiating new stretch
                            % is the last stretch over high thres?
                            if max([data{i,start:col-1}])<keepthres
                                data(i,start:col-1)={0};
                                % clear up ratio columns
                                data(i,ratiocol-col+1+(start:col-1))={0};
                            else % no need to split if last stretch is discarded
                                splittings=[splittings; i col];
                            end
                            start=col;
                        end
                    else
                        data{i,col}=0;
                    end
                else
                    if data{i,col}<lossthres
                        current=0;
                        data{i,col}=0;
                        data{i,ratiocol}=0;
                    end
                end
            end
            ratiocol=ratiocol+1;
        end
        % is the last stretch over high thres?
        if max([data{i,start:col-1}])<keepthres
            data(i,start:col-1)={0};
            % clear up ratio columns
            data(i,ratiocol-col+(start:col-1))={0}; % ratiocol already incremented
        end
    end
end


% recompute the ratio columns
for i=1:size(data,1)
    data(i,startingratiocol:end)={NaN};
end

for i=1:size(data,1)
   for j=2:12
        % intensity ratio
        data{i,j+startingratiocol-2}=abs(log(data{i,j-1})-log(data{i,j}));
        if data{i,j+startingratiocol-2}<0.0001
            data{i,j+startingratiocol-2}=0.0001;
        end
        % intensity difference
        data{i,j+startingratiocol*2-3}=abs(data{i,j}-data{i,j-1});
        if data{i,j+startingratiocol*2-3}<0.0001
            data{i,j+startingratiocol*2-3}=0.0001;
        end
        % going up
        data{i,j+startingratiocol*3-4}= data{i,j+startingratiocol*2-3}*(data{i,j}>=data{i,j-1});        
        % going down
        data{i,j+startingratiocol*4-5}= data{i,j+startingratiocol*2-3}*(data{i,j}<data{i,j-1});
        % raw ratio
        data{i,j+startingratiocol*5-6}=abs(log(data{i,j-1}*backbone{i,j-1})-log(data{i,j}*backbone{i,j}));
        if data{i,j+startingratiocol*5-6}<0.0001
            data{i,j+startingratiocol*5-6}=0.0001;
        end
        % flag suspicious columns
        data{i,j+startingratiocol*6-7}=data{i,j+startingratiocol*5-6}- data{i,j+startingratiocol-2};

        
        % intensity ratio
        if isinf( data{i,j+startingratiocol-2})
            data{i,j+startingratiocol-2}=NaN;
        else if  data{i,j+startingratiocol-2}==0
            data{i,j+startingratiocol-2}=NaN;
            end
        end
        
        if isnan( data{i,j+startingratiocol-2})
            data{i,j+startingratiocol*2-3}=NaN;
            data{i,j+startingratiocol*3-4}=NaN;
            data{i,j+startingratiocol*4-5}=NaN;
            data{i,j+startingratiocol*5-6}=NaN;
            data{i,j+startingratiocol*6-7}=NaN;  
        end
   end
   % calculate before and after deprivation scores 
   %intensity
   t=[data{i,1:3}];
   data{i,startingratiocol-3}=sum(t)/sum(t>0);
   t=[data{i,4:7}];
   t(isnan(t))=[];
   data{i,startingratiocol-2}=sum(t)/sum(t>0);
   
   % intensity ratio
   t=[data{i,startingratiocol:1+startingratiocol}];
   t(isnan(t))=[];
   data{i,startingratiocol*2-4}=mean(t);
   t=[data{i,startingratiocol+2:5+startingratiocol}];
   t(isnan(t))=[];
   data{i,startingratiocol*2-3}=mean(t);
   
   % intensity difference
   t=[data{i,startingratiocol*2-1:2*startingratiocol}];
   t(isnan(t))=[];
   data{i,startingratiocol*3-5}=mean(t);
   t=[data{i,startingratiocol*2+1:4+2*startingratiocol}];
   t(isnan(t))=[];
   data{i,startingratiocol*3-4}=mean(t);
   
   % going up
   t=[data{i,startingratiocol*3-2:3*startingratiocol-1}];
   t(isnan(t))=[];
   t(t==0)=[];
   data{i,startingratiocol*4-6}=mean(t);
   t=[data{i,startingratiocol*3:3+3*startingratiocol}];
   t(isnan(t))=[];
   t(t==0)=[];
   data{i,startingratiocol*4-5}=mean(t);
   
   % going down
   t=[data{i,startingratiocol*4-3:4*startingratiocol-2}];
   t(isnan(t))=[];
   t(t==0)=[];
   data{i,startingratiocol*5-7}=mean(t);
   t=[data{i,startingratiocol*4-1:2+4*startingratiocol}];
   t(isnan(t))=[];
   t(t==0)=[];
   data{i,startingratiocol*5-6}=mean(t);  
end

% Statistics of changes


singles=[];
for row=1:size(data,1)
    for col=1:8
        if data{row,col}>0
            singles=[singles; data{row,col}];
        end
    end
end

befores=[];
afters=[];
for row=1:size(data,1)
    for col=startingratiocol*2-1:startingratiocol*2
        if ~isnan(data{row,col})
            befores=[befores; data{row,col}];
        end
    end
    for col=startingratiocol*2+1:startingratiocol*2+2
        if ~isnan(data{row,col})
            afters=[afters; data{row,col}];
        end
    end
end
figure;
hist(log10(befores),20);
xlabel('log 10 (bouton change size)');
ylabel('frequency');
title('before deprivation');

figure;
hist(log10(afters),20);
xlabel('log 10 (bouton change size)');
ylabel('frequency');
title('after deprivation');


figure;
hist(singles,20);
xlabel('bouton size');
ylabel('frequency');
title('distriution of bouton size');


figure;
hist(log10(singles),20);
xlabel('log 10 bouton size');
ylabel('frequency');
title('log scale distriution of bouton size');


pairs=[];

% bouton needs to be present on all previous days
for row=1:size(data,1)
    for col=1:8
    if data{row,col+1}>0 && data{row,col}>0
        pairs=[pairs; data{row,col} data{row,col+1}];
    end
    end
end


% bin bouton sizes
[dum,i]=sort(pairs,1);
spairs=pairs(i(:,1),:);
spairs(:,3)=abs(spairs(:,2)-spairs(:,1));
binnedpairs=[];
step=round(size(spairs,1)/100);
for i=1:step:size(spairs,1)
    index=i:min(i+step-1,size(spairs,1));
    binnedpairs=[binnedpairs; mean(spairs(index,1)) mean(spairs(index,3)) std(spairs(index,3))];
end


figure;
plot(binnedpairs(:,1),binnedpairs(:,2),'.')
xlabel('mean bouton size')
ylabel('mean absolute change')
figure;
errorbar(binnedpairs(:,1),binnedpairs(:,2),binnedpairs(:,3))
xlabel('mean bouton size')
ylabel('mean absolute change')

% Align boutons based disappearance

boutonstrengths={};
select=[];
for back=1:6 % number of days to go back
    for endday=back:6
        backbs=[];
        % bouton needs to be present on all previous days
        for row=1:size(data,1)
            if data{row,endday+1}==0 && data{row,endday}>0
                p=1;
                backb=[data{row,endday}];
                for b=2:back
                    if data{row,endday-b+1}==0
                        p=0;
                        break;
                    else
                        backb=[backb data{row,endday-b+1}];
                    end
                end
                if p>0
                    select=[select; back endday row];
                    backbs=[backbs; backb];
                end
            end
        end
        boutonstrengths{back,endday}=fliplr(backbs);
    end
end


% going back three sessions
try
figure
hold on;
c=errorbar([-2 -1],mean([boutonstrengths{2,2}; boutonstrengths{2,3}; boutonstrengths{2,4}; boutonstrengths{2,5}; boutonstrengths{2,6}]),std([boutonstrengths{2,2}; boutonstrengths{2,3}; boutonstrengths{2,4}; boutonstrengths{2,5}; boutonstrengths{2,6}]))
errorbar([-3 -2 -1],mean([boutonstrengths{3,3}; boutonstrengths{3,4}; boutonstrengths{3,5}; boutonstrengths{3,6}]),std([boutonstrengths{3,3}; boutonstrengths{3,4}; boutonstrengths{3,5}; boutonstrengths{3,6}]),'r')
errorbar([-4 -3 -2 -1],mean([boutonstrengths{4,4}; boutonstrengths{4,5}; boutonstrengths{4,6}]),std([boutonstrengths{4,4}; boutonstrengths{4,5}; boutonstrengths{4,6}]),'g')
errorbar([-5 -4 -3 -2 -1],mean([boutonstrengths{5,5}; boutonstrengths{5,6}]),std([boutonstrengths{5,5}; boutonstrengths{5,6}]),'m')
errorbar([-6 -5 -4 -3 -2 -1],mean([boutonstrengths{6,6}],1),std([boutonstrengths{6,6}],0,1),'c')
xlabel('day')
ylabel('strength')
catch
end




% Align boutons based on appearance
% 
% boutonstrengths={};
% select=[];
% for back=1:6 % number of days to go back
%     for endday=back:6
%         backbs=[];
%         % bouton needs to be present on all previous days
%         for row=1:size(data,1)
%             if data{row,endday+1}==0 && data{row,endday}>0
%                 p=1;
%                 backb=[data{row,endday}];
%                 for b=2:back
%                     if data{row,endday-b+1}==0
%                         p=0;
%                         break;
%                     else
%                         backb=[backb data{row,endday-b+1}];
%                     end
%                 end
%                 if p>0
%                     select=[select; back endday row];
%                     backbs=[backbs; backb];
%                 end
%             end
%         end
%         boutonstrengths{back,endday}=backbs;
%     end
% end


    





% keep only stable boutons
% has to be stable for 7 sessions
% however if data has less than 7 time points, they should be NaN, so only
% requiring all sessions to be stable CHECK TO MAKE SURE!

stabledata=data;
for i=1:size(data,1)
    d=[data{i,1:stableperiod}];
    if min(d(~isnan(d)))==0
        stabledata(i,1:end)={NaN};
    end
end


% keep only unstable boutons

unstabledata=data;
for i=1:size(data,1)
    d=[data{i,1:stableperiod}];
    if min(d(~isnan(d)))>0
        unstabledata(i,1:end)={NaN};
    end
end

dataintensity=data;


% calculate turnover ratios
newnumericturnover=[];
newtxtturnover=[];
% split the rows
newleft=[];
newdata=[];
for i=1:size(data,1)
    if ismember(i,splittings(:,1))
        pos=splittings(splittings(:,1)==i,2);
        row=data(i,1:startingratiocol*2-4);
        % length of data
        n=find(isnan([row{:}]));
        for j=1:length(pos)
            row1=row;
            row1(pos(j):n(1)-1)={0};
            newdata=[newdata;row1];
            row(1:pos(j)-1)={0};
            newleft=[newleft;left(i,:)];
        end
        newdata=[newdata;row];
        newleft=[newleft;left(i,:)];
    else
        newdata=[newdata;data(i,1:startingratiocol*2-4)];
        newleft=[newleft;left(i,:)];
    end
end

%binarize
for i=1:size(newdata,1)
    for j=1:size(newdata,2)
        if newdata{i,j}>0
            newdata{i,j}=1;
        end
    end
end

% change 0 to NaN for intensity data
% needs to be after the data is copied over to rawdata for computing
% turnover ratios
for i=1:size(data,1)
    for j=1:size(data,2)
        if data{i,j}==0
            data{i,j}=NaN;
        end
    end
end

for i=1:size(data,1)
    for j=1:size(data,2)
        if stabledata{i,j}==0
            stabledata{i,j}=NaN;
        end
    end
end


for i=1:size(data,1)
    for j=1:size(data,2)
        if unstabledata{i,j}==0
            unstabledata{i,j}=NaN;
        end
    end
end


% bouton presence change
for i=startingratiocol:startingratiocol*2-1
    for j=1:size(newdata,1)
        newdata{j,i}=newdata{j,i-startingratiocol+2}-newdata{j,i-startingratiocol+1};
    end
end


newraw=[header;left data];
newrawstable=[header;left stabledata];
newrawunstable=[header;left unstabledata];

paddedleft=left;


[paddedleft avgdata]=calculateavgperROI(ROIid,data,left);
% 
% % pad to be same size as avgdata
% for i=size(left,1)+1:size(avgdata,1)
%     for j=1:size(left,2)
%         paddedleft(i,j)={NaN};
%     end
% end

newavgraw=[header;paddedleft avgdata];

[paddedleft peranimalavg]=calculateavgperROI(animalid,data,left);
newperanimalraw=[header;paddedleft(1:size(peranimalavg,1),:) peranimalavg];

[paddedleft peraxonavg]=calculateavgperROI(axonid,data,left);
newperaxonraw=[header;paddedleft(1:size(peraxonavg,1),:) peraxonavg];

[paddedleft avgstabledata]=calculateavgperROI(ROIid,stabledata,left);
newavgstableraw=[header;paddedleft(1:size(avgstabledata,1),:) avgstabledata];

[paddedleft peranimalstableavg]=calculateavgperROI(animalid,stabledata,left);
newperanimalstableraw=[header;paddedleft(1:size(peranimalstableavg,1),:) peranimalstableavg];

[paddedleft peraxonstableavg]=calculateavgperROI(axonid,stabledata,left);
newperaxonstableraw=[header;paddedleft(1:size(peraxonstableavg,1),:) peraxonstableavg];

[paddedleft avgunstabledata]=calculateavgperROI(ROIid,unstabledata,left);
newavgunstableraw=[header;paddedleft(1:size(avgunstabledata,1),:) avgunstabledata];

[paddedleft peranimalunstableavg]=calculateavgperROI(animalid,unstabledata,left);
newperanimalunstableraw=[header;paddedleft(1:size(peranimalunstableavg,1),:) peranimalunstableavg];

[paddedleft peraxonunstableavg]=calculateavgperROI(axonid,unstabledata,left);
newperaxonunstableraw=[header;paddedleft(1:size(peraxonunstableavg,1),:) peraxonunstableavg];

newrawturnover=[header(:,1:size(newleft,2)+size(newdata,2));newleft newdata];
ROIs=tabulateROI(ROIid);
for i=1:length(ROIs)
    l=[axonlengths{ROIs{i}}];
    l(isnan(l))=[];
    axonlengthROIs(i)=sum(unique(l));
end
newperROIturnover=calculateturnoverperROI(newleft,newleft(:,ROIidcol),newdata,axonlengthROIs);

ROIs=tabulateROI(animalid);
for i=1:length(ROIs)
    l=[axonlengths{ROIs{i}}];
    l(isnan(l))=[];
    axonlengthROIs(i)=sum(unique(l));
end
newperanimalturnover=calculateturnoverperROI(newleft,newleft(:,animalidcol),newdata,axonlengthROIs);

ROIs=tabulateROI(axonid);
for i=1:length(ROIs)
    l=[axonlengths{ROIs{i}}];
    l(isnan(l))=[];
    axonlengthROIs(i)=sum(unique(l));
end
newperaxonturnover=calculateturnoverperROI(newleft,newleft(:,axonidcol),newdata,axonlengthROIs);

xlswrite([pname '/' fname],newraw,[pane '-intensity']);
xlswrite([pname '/' fname],newavgraw,[pane '-avgintensity']);
xlswrite([pname '/' fname],newperanimalraw,[pane '-peranimalintensity']);
xlswrite([pname '/' fname],newperaxonraw,[pane '-peraxonintensity']);

xlswrite([pname '/' fname],newrawstable,[pane '-stableintensity']);
xlswrite([pname '/' fname],newavgstableraw,[pane '-avgstableintensity']);
xlswrite([pname '/' fname],newperanimalstableraw,[pane '-peranimalstableintensity']);
xlswrite([pname '/' fname],newperaxonstableraw,[pane '-peraxonstableintensity']);

xlswrite([pname '/' fname],newrawunstable,[pane '-unstableint']);
xlswrite([pname '/' fname],newavgunstableraw,[pane '-avgunstableint']);
xlswrite([pname '/' fname],newperanimalunstableraw,[pane '-peranimalunstableint']);
xlswrite([pname '/' fname],newperaxonunstableraw,[pane '-peraxonunstableint']);



xlswrite([pname '/' fname],newrawturnover,[pane '-turnover']);
xlswrite([pname '/' fname],newperROIturnover,[pane '-perROIturnover']);
xlswrite([pname '/' fname],newperanimalturnover,[pane '-peranimalturnover']);
xlswrite([pname '/' fname],newperaxonturnover,[pane '-peraxonturnover']);

function ROIs=tabulateROI(left)
ROIs={};
on=1;
currentROI=[];
ROIcount=1;
for i=1:size(left,1)
    if on
        if isnan(left{i})
            on=0;
            ROIs{ROIcount}=currentROI;
            ROIcount=ROIcount+1;
            currentROI=[];
        else
            currentROI=[currentROI i];
        end
    else
        if ~isnan(left{i})
            on=1;
            currentROI=[currentROI i];
        end
    end
end
if on
    on=0;
    ROIs{ROIcount}=currentROI;
    ROIcount=ROIcount+1;
    currentROI=[];
end

function [newleft newdata]=calculateavgperROI(left,data,oldleft)
global startingratiocol;
% tabulate ROIs
ROIs=tabulateROI(left);

newdata=data;
newleft=oldleft;
% add empty spaces    
for i=size(data,1):2046
    for j=1:size(data,2)
        newdata{i,j}=NaN;
    end
    for j=1:size(oldleft,2)
        newleft{i,j}=NaN;
    end
end

summaryrow=2046;
% calculate averages
for i=1:size(ROIs,2)
    rows=ROIs{i};
    matrix=[];
    for j=1:size(rows,2)
        matrix=[matrix; [data{rows(j),:}]];
    end
    
    newleft{summaryrow+i,25}=left{ROIs{i}(1)};
    newleft{summaryrow+i+100,25}=left{ROIs{i}(1)};
    for j=1:size(data,2)
        col=matrix(:,j);
        col1=col;
        col1(isnan(col1))=[];
        newdata{summaryrow+i,j}=sum(col1)/sum(~isnan(col1));
        newdata{summaryrow+100+i,j}=median(col1);
    end

    for k=1:5
        offset=(startingratiocol-1)*k+1;
        newdata{summaryrow+i,offset-1}=left{ROIs{i}(1)};
        newdata{summaryrow+i+100,offset-1}=left{ROIs{i}(1)};
    end
    
    for k=0:5
        %calculate counts
        cnt=sum(matrix(:,1:7)>0);
        cnt(cnt==0)=[];
        cnt=mean(cnt);
        newdata{summaryrow+i,offset-1}=cnt;     
        newdata{summaryrow+i+100,offset-1}=cnt;   
        
        % before 
        offset=(startingratiocol-1)*k+1+13;
        row=[newdata{summaryrow+i,offset-13:offset-12}];
        row1=row;
        row1(isnan(row1))=[];
        newdata{summaryrow+i,offset}=mean(row1,2);
       
        %after
        row=[newdata{summaryrow+i,offset-11:offset-8}];
        row1=row;
        row1(isnan(row1))=[];
        newdata{summaryrow+i,offset+1}=mean(row1,2);

        
        
        % median
        row=[newdata{summaryrow+i+100,offset-13:offset-12}];
        row1=row;
        row1(isnan(row1))=[];
        newdata{summaryrow+i+100,offset}=mean(row1,2);
        
        row=[newdata{summaryrow+i+100,offset-11:offset-8}];
        row1=row;
        row1(isnan(row1))=[];
        newdata{summaryrow+100+i,offset+1}=mean(row1,2);
    end
    
    
    % newdata{rows(end)+1,12}=mean(newdata{rows(end)+1,1:3});
   
%     newdata{rows(end)+1,15}=mean([newdata{rows(end)+1,1:3}]);
%     newdata{rows(end)+1,16}=mean([newdata{rows(end)+1,4:7}]);
% 
%     newdata{rows(end)+1,31}=mean([newdata{rows(end)+1,17:18}]);
%     newdata{rows(end)+1,32}=mean([newdata{rows(end)+1,19:22}]);
% 
%     newdata{rows(end)+1,47}=mean([newdata{rows(end)+1,33:34}]);
%     newdata{rows(end)+1,48}=mean([newdata{rows(end)+1,35:38}]);
end

function newdata=calculateturnoverperROI(left,ROIid,data,axonlengths)

% tabulate ROIs
ROIs=tabulateROI(ROIid);

newdata={};
numROIs=50;
for i=0:30
    for k=1:45
    for j=1:18
        newdata(i*numROIs+k,j)={NaN};
    end
    end
end

% calculate averages
%total number of EPB, gains, gains+losses, losses, stable, stable
%fraction, survival fraction, survival from day 0, TOR).

leftmargin=4;
boutondensities=[];
for i=1:size(ROIs,2)
    cellblock=data([ROIs{i}]',:);
    block=reshape([cellblock{:}],size(cellblock));
    
    % - compute change in TB length before/after clipping
    % c7 [mean before mean after]
    
    %     c7=[mean(abs(diff(peraxonboutonlength(:,1:7),[],2)),1)];
    %
    
    % put in the ROI names
    for j=1:30
        newdata(numROIs*(j-1)+i+2,1)=ROIid(ROIs{i}(1));
    end
    
    % first NaN block
    temp=block;
    temp(~isnan(temp))=1;
    temp(isnan(temp))=0;
    temp1=find((sum(temp)==0));
    
    boutonpresence=block(sum(temp,2)>0,1:temp1(1)-1);
    thisaxonlength=axonlengths(i);
    
    peraxonid=boutonpresence;
    peraxonnew=diff(boutonpresence,1,2)>0;
    peraxonloss=diff(boutonpresence,1,2)<0;
    
    
    
    
    % skip empty blocks
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % Start of block in common with analyze TB script
    %
    %
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    boutondensity=sum(boutonpresence,1)/thisaxonlength;
    boutondensities=[boutondensities; {boutondensity}];
    if size(boutonpresence,1)>0
        
        %number of boutons
        numboutons=[sum(peraxonid(:,1),1) sum(peraxonid(:,1),1)+sum(cumsum(peraxonnew',1)'-cumsum(peraxonloss',1)',1)];
        
        if size(peraxonnew,2)>=6
            peraxonnew3=[sum(peraxonnew(:,1:2),2) sum(peraxonnew(:,3:4),2) sum(peraxonnew(:,5:6),2)];
            peraxonloss3=[sum(peraxonloss(:,1:2),2) sum(peraxonloss(:,3:4),2) sum(peraxonloss(:,5:6),2)];
            numboutons3=numboutons(:,[1 3 5 7]);
            
            
        else if size(peraxonnew,2)>=4
                peraxonnew3=[sum(peraxonnew(:,1:2),2) sum(peraxonnew(:,3:4),2)];
                peraxonloss3=[sum(peraxonloss(:,1:2),2) sum(peraxonloss(:,3:4),2)];
                numboutons3=numboutons(:,[1 3 5 ]);
            else if size(peraxonnew,2)>=2
                    peraxonnew3=[sum(peraxonnew(:,1:2),2)];
                    peraxonloss3=[sum(peraxonloss(:,1:2),2)];
                    numboutons3=numboutons(:,[1 3]);
                end
            end
        end
        
        
        %TOR day 0-4	day 4-8	 day 8-12	day 12-16	day 16-20	day20-24 	mean#
        TOR=[(sum(peraxonnew,1)+sum(peraxonloss,1))./(2*numboutons(1:end-1)) mean(numboutons)];
        
        %TOR3  day 0-8 (2n refers to 0d)	day 8-16 (2n refers to 8d)	day 16-24 (2n refers to 16d)
        TOR3=[(sum(peraxonnew3,1)+sum(peraxonloss3,1))./(2*numboutons3(1:end-1)) mean(numboutons3)];
        
        % survival fraction. Last column is the number of starting boutons which should be > boutonscorethreshold and >1 at least in one time point
        survival=[boutonpresence(boutonpresence(:,1)>0,1) -peraxonloss(boutonpresence(:,1)>0,:)];
        survival=cumsum(survival,2);
        survival(survival<0)=0;
        survival=sum(survival,1);
        survival=[survival/survival(1) survival(1)];
        
        if size(peraxonnew,2)>2
            % Add in survival from certain days
            survival3=[boutonpresence(boutonpresence(:,3)>0,3) -peraxonloss(boutonpresence(:,3)>0,3:end)];
            survival3=cumsum(survival3,2);
            survival3(survival3<0)=0;
            survival3=sum(survival3,1);
            survival3=[survival3/survival3(1) survival3(1)];
        else
            survival3=[];
        end
        
        
        
        % - % and density of new/lost before and after clipping
        % c1 [new before0-8 new after8-16 new after16-24 lost before0-8
        % lost after8-16 lost after16-24 TB#a-c TB#d-g] last two columns are total numbers
        %C1 the denominator is the sum of all TB present at each day (even if<boutonscorethres) before (a-c) or after clipping (c-g)
        
        if size(peraxonnew,2)>=6 % 7 sessions
            
            % c1 [new before0-8 new after8-16 new after16-24 lost before0-8
            % lost after8-16 lost after16-24 TB#a-c TB#d-g] last two columns are total numbers
            %C1 the denominator is the sum of all TB present at each day before (a-c) or after clipping (c-g)
            
            c1=[mean(sum(peraxonnew(:,1:2)))/mean(numboutons(:,1:3)) mean(sum(peraxonnew(:,3:6)))/mean(numboutons(:,3:7)) mean(sum(peraxonloss(:,1:2)))/mean(numboutons(:,1:3)) mean(sum(peraxonloss(:,3:6)))/mean(numboutons(:,3:7)) mean(numboutons(:,1:3)) mean(numboutons(:,3:7))  ];
             c1den=[mean(sum(peraxonnew(:,1:2)))/thisaxonlength mean(sum(peraxonnew(:,3:6))/thisaxonlength) mean(sum(peraxonloss(:,1:2))/thisaxonlength) mean(sum(peraxonloss(:,3:6))/thisaxonlength)];
           
            
            %c1a [new0-4 new4-8 new8-12 new12-16 new16-20 new20-24 lost0-4 lost4-8 lost8-12 lost12-16 lost16-20 lost20-24]
            c1a=[sum(sum(peraxonnew(:,1:1)))/sum(sum(numboutons(:,1:2))) sum(sum(peraxonnew(:,2:2)))/sum(sum(numboutons(:,2:3))) sum(sum(peraxonnew(:,3:3)))/sum(sum(numboutons(:,3:4))) sum(sum(peraxonnew(:,4:4)))/sum(sum(numboutons(:,4:5))) sum(sum(peraxonnew(:,5:5)))/sum(sum(numboutons(:,5:6))) sum(sum(peraxonnew(:,6:6)))/sum(sum(numboutons(:,6:7)))  sum(sum(peraxonloss(:,1:1)))/sum(sum(numboutons(:,1:2))) sum(sum(peraxonloss(:,2:2)))/sum(sum(numboutons(:,2:3))) sum(sum(peraxonloss(:,3:3)))/sum(sum(numboutons(:,3:4))) sum(sum(peraxonloss(:,4:4)))/sum(sum(numboutons(:,4:5))) sum(sum(peraxonloss(:,5:5)))/sum(sum(numboutons(:,5:6))) sum(sum(peraxonloss(:,6:6)))/sum(sum(numboutons(:,6:7)))];
            c1aden=[sum(sum(peraxonnew(:,1:1)))/thisaxonlength sum(sum(peraxonnew(:,2:2))/thisaxonlength) sum(sum(peraxonnew(:,3:3))/thisaxonlength) sum(sum(peraxonnew(:,4:4)))/thisaxonlength sum(sum(peraxonnew(:,5:5))/thisaxonlength) sum(sum(peraxonnew(:,6:6))/thisaxonlength)  sum(sum(peraxonloss(:,1:1))/thisaxonlength) sum(sum(peraxonloss(:,2:2))/thisaxonlength) sum(sum(peraxonloss(:,3:3))/thisaxonlength) sum(sum(peraxonloss(:,4:4))/thisaxonlength) sum(sum(peraxonloss(:,5:5))/thisaxonlength) sum(sum(peraxonloss(:,6:6))/thisaxonlength)];
            
           
            % c1b [new before0-8 new after8-16  new after16-24
            %     lost before0-8         8-16           16-24
            %     total before0-8         8-16           16-24
            %     TB#a-c c-e e-g] last three columns are total numbers
            
            c1b=[sum(sum(peraxonnew(:,1:2)))/sum(sum(numboutons(:,1:3))) sum(sum(peraxonnew(:,3:4)))/sum(sum(numboutons(:,3:5))) sum(sum(peraxonnew(:,5:6)))/sum(sum(numboutons(:,5:7))) ...
                sum(sum(peraxonloss(:,1:2)))/sum(sum(numboutons(:,1:3))) sum(sum(peraxonloss(:,3:4)))/sum(sum(numboutons(:,3:5))) sum(sum(peraxonloss(:,5:6)))/sum(sum(numboutons(:,5:7)))];
            c1b=[c1b c1b(1:3)+c1b(4:6)];
            c1b=[c1b sum(sum(numboutons(:,1:3))) sum(sum(numboutons(:,3:5)))  sum(sum(numboutons(:,5:7)))];
            
            
            c1bden=[sum(sum(peraxonnew(:,1:2)))/thisaxonlength sum(sum(peraxonnew(:,3:4)))/thisaxonlength sum(sum(peraxonnew(:,5:6)))/thisaxonlength ...
                sum(sum(peraxonloss(:,1:2)))/thisaxonlength sum(sum(peraxonloss(:,3:4)))/thisaxonlength sum(sum(peraxonloss(:,5:6)))/thisaxonlength];
            c1bden=[c1bden c1bden(1:3)+c1bden(4:6) thisaxonlength];
            
        else  if size(peraxonnew,2)>=4 % 5 sessions
                
                c1=[mean(sum(peraxonnew(:,1:2)))/mean(numboutons(:,1:3)) mean(sum(peraxonnew(:,3:4)))/mean(numboutons(:,3:5)) mean(sum(peraxonloss(:,1:2)))/mean(numboutons(:,1:3)) mean(sum(peraxonloss(:,3:4)))/mean(numboutons(:,3:5)) mean(numboutons(:,1:3)) mean(numboutons(:,3:5))  ];
                c1den=[mean(sum(peraxonnew(:,1:2)))/thisaxonlength mean(sum(peraxonnew(:,3:4))/thisaxonlength) mean(sum(peraxonloss(:,1:2))/thisaxonlength) mean(sum(peraxonloss(:,3:4))/thisaxonlength)];
           
                c1a=[sum(sum(peraxonnew(:,1:1)))/sum(sum(numboutons(:,1:2))) sum(sum(peraxonnew(:,2:2)))/sum(sum(numboutons(:,2:3))) sum(sum(peraxonnew(:,3:3)))/sum(sum(numboutons(:,3:4))) sum(sum(peraxonnew(:,4:4)))/sum(sum(numboutons(:,4:5)))  NaN NaN sum(sum(peraxonloss(:,1:1)))/sum(sum(numboutons(:,1:2))) sum(sum(peraxonloss(:,2:2)))/sum(sum(numboutons(:,2:3))) sum(sum(peraxonloss(:,3:3)))/sum(sum(numboutons(:,3:4))) sum(sum(peraxonloss(:,4:4)))/sum(sum(numboutons(:,4:5))) NaN NaN];                
                c1aden=[sum(sum(peraxonnew(:,1:1)))/thisaxonlength sum(sum(peraxonnew(:,2:2))/thisaxonlength) sum(sum(peraxonnew(:,3:3))/thisaxonlength) sum(sum(peraxonnew(:,4:4)))/thisaxonlength  NaN NaN sum(sum(peraxonloss(:,1:1))/thisaxonlength) sum(sum(peraxonloss(:,2:2))/thisaxonlength) sum(sum(peraxonloss(:,3:3))/thisaxonlength) sum(sum(peraxonloss(:,4:4))/thisaxonlength) NaN NaN];
                
                c1b=[sum(sum(peraxonnew(:,1:2)))/sum(sum(numboutons(:,1:3))) sum(sum(peraxonnew(:,3:4)))/sum(sum(numboutons(:,3:5))) NaN ...
                    sum(sum(peraxonloss(:,1:2)))/sum(sum(numboutons(:,1:3))) sum(sum(peraxonloss(:,3:4)))/sum(sum(numboutons(:,3:5))) NaN];
                c1b=[c1b c1b(1:3)+c1b(4:6)];
                c1b=[c1b sum(sum(numboutons(:,1:3))) sum(sum(numboutons(:,3:5))) NaN];
                
                
                c1bden=[sum(sum(peraxonnew(:,1:2)))/thisaxonlength sum(sum(peraxonnew(:,3:4)))/thisaxonlength NaN ...
                    sum(sum(peraxonloss(:,1:2)))/thisaxonlength sum(sum(peraxonloss(:,3:4)))/thisaxonlength NaN];
                c1bden=[c1bden c1bden(1:3)+c1bden(4:6) thisaxonlength];
                
                
            end
        end
        
        % trim long sessions
        
        if size(peraxonnew,2)>=6
            peraxonloss=peraxonloss(:,1:6);
            peraxonnew=peraxonnew(:,1:6);
        end
        
        if size(peraxonnew,2)>=3
            % - % and density of TBs that were stable for the first 8d and then were lost.
            %c2 [ fraction: stable then lost/total stable
            % notlost a-b+b-c]
            c2=[sum(sum(boutonpresence(:,1:3),2)==3 & sum(peraxonloss(:,3:end),2)>=1,1)/sum(sum(boutonpresence(:,1:3),2)==3) sum(sum(boutonpresence(:,1:3),2)==3)];
            c2den=[sum(sum(boutonpresence(:,1:3),2)==3 & sum(peraxonloss(:,3:end),2)>=1,1)/thisaxonlength];
            
            
            
            
            % - % and density of TBs that came up after clipping and then were lost
            % c5 fraction: came up then lost/total coming up
            c5=[sum((sum(peraxonnew(:,3:end),2)>0) & sum(peraxonloss(:,3:end),2)>0 & boutonpresence(:,end)<1,1)/sum(sum(peraxonnew(:,3:end),2)>0) sum(sum(peraxonnew(:,3:end),2)>0)];
            c5den=[sum((sum(peraxonnew(:,3:end),2)>0) & sum(peraxonloss(:,3:end),2)>0 & boutonpresence(:,end)<1,1)/thisaxonlength];
            
            
            
            
            % - % and density of TBs that came up after clipping and were
            % not lost after clipping
            % c3 fraction: stablized/total new boutons after clipping
            if size(peraxonnew,2)>5
                c3=[sum((sum(peraxonnew(:,3:5),2)==1) & sum(peraxonloss(:,3:end),2)==0,1)/sum(sum(peraxonnew(:,3:5),2)==1) sum(sum(peraxonnew(:,3:5),2)==1)];
                c3den=[sum((sum(peraxonnew(:,3:5),2)==1) & sum(peraxonloss(:,3:end),2)==0,1)/thisaxonlength];
            else
                c3=[NaN NaN];
                c3den=[NaN];
            end
            
            % - % and density of TB that came up after clipping and were stable for the last 8d
            % c6 fraction: stable/ total came up
            if size(peraxonnew,2)>=4
                c6=[sum((sum(peraxonnew(:,3:4),2)==1) & sum(peraxonloss(:,3:end),2)==0,1)/sum(sum(peraxonnew(:,3:4),2)==1) sum(sum(peraxonnew(:,3:4),2)==1)];
                c6den=[ sum((sum(peraxonnew(:,3:4),2)==1) & sum(peraxonloss(:,3:end),2)==0,1)/thisaxonlength];
            else
                c6=[NaN NaN];
                c6den=[NaN];
            end
            
            % - % and density of TB that came up after clipping and were stable for the last 12d
            % c8 fraction: stable/ total came up
            c8=[sum((sum(peraxonnew(:,3:3),2)==1) & sum(peraxonloss(:,3:end),2)==0,1)/sum(sum(peraxonnew(:,3:3),2)==1) sum(sum(peraxonnew(:,3:3),2)==1)];
            c8den=[sum((sum(peraxonnew(:,3:3),2)==1) & sum(peraxonloss(:,3:end),2)==0,1)/thisaxonlength];
            
            % - % and density of TBs that came up BEFORE clipping and then stbilized
            % c4 fraction: stablized/total new boutons  before clipping
            c4=[sum((sum(peraxonnew(:,1:2),2)==1) & sum(peraxonloss(:,1:end),2)==0,1)/sum(sum(peraxonnew(:,1:2),2)==1) sum(sum(peraxonnew(:,1:2),2)==1)];
            c4den=[sum((sum(peraxonnew(:,1:2),2)==1) & sum(peraxonloss(:,1:end),2)==0,1)/thisaxonlength];
        end
        
        
        %total number of EPB
        k=0;
        newdata(numROIs*k+2,2)={'total number of EPB a b c d e f g etc.'};
        for j=1:length(numboutons)
            newdata{numROIs*k+i+2,leftmargin+j}=numboutons(j);
        end
        
        %gains
        k=1;
        newdata(numROIs*k+2,2)={'gains a-b b-c etc'};
        g=sum(peraxonnew,1);
        for j=1:length(g)
            newdata{numROIs*k+i+2,leftmargin+j}=g(j);
        end
        
        %lossess
        k=2;
        newdata(numROIs*k+2,2)={'loss a-b b-c etc'};
        l=sum(peraxonloss,1);
        for j=1:length(l)
            newdata{numROIs*k+i+2,leftmargin+j}=l(j);
        end
        
        %gain+lossess
        k=3;
        newdata(numROIs*k+2,2)={'gain+loss'};
        for j=1:length(l)
            newdata{numROIs*k+i+2,leftmargin+j}=g(j)+l(j);
        end
        
        %stable
        k=4;
        newdata(numROIs*k+2,2)={'stable each session'};
        for j=1:length(l)
            newdata{numROIs*4+i+2,leftmargin+j}=numboutons(j)-l(j);
        end
        
        %stable ratio
        k=5;
        newdata(numROIs*k+2,2)={'stable fraction. stable/total'};
        for j=1:length(l)
            newdata{numROIs*5+i+2,leftmargin+j}=(numboutons(j)-l(j))/numboutons(j);
        end
        
        finalcol=12;
        
        % survival fraction Last column is the number of starting boutons which
        % should be > boutonscorethreshold and >1 at least in one time point
        k=6;
        newdata(numROIs*k+2,2)={'survival fraction from session 3'};
        if length(survival3)>1
            for j=1:length(survival3)-1
                newdata{numROIs*6+i+2,leftmargin+j}=survival3(j);
            end
            newdata{numROIs*6+i+2,leftmargin+finalcol}=survival3(end);
        end
        
        
        
        %survival from day 0
        k=7;
        newdata(numROIs*k+2,2)={'survival from session 1'};
        if length(survival)>1
            for j=1:length(survival)-1
                newdata{numROIs*k+i+2,leftmargin+j}=survival(j);
            end
            newdata{numROIs*k+i+2,leftmargin+finalcol}=survival(end);
        end
        
        %TOR
        k=8;
        newdata(numROIs*k+2,2)={'TOR (gain a-b+loss a-b)/(2*num a), etc. Last column: average number of boutons'};
        if length(TOR)>1
            for j=1:length(TOR)-1
                newdata{numROIs*k+i+2,leftmargin+j}=TOR(j);
            end
            newdata{numROIs*k+i+2,leftmargin+finalcol}=TOR(end);
        end
        
        %TOR3
        k=9;
        newdata(numROIs*k+2,2)={'TOR3. Turnover calculated every 2 sessions (gain a-c+loss a-c)/(2*#a), etc. Last column: average number of boutons'};
        if length(TOR3)>1
            for j=1:length(TOR3)-1
                newdata{numROIs*k+i+2,leftmargin+j}=TOR3(j);
            end
            newdata{numROIs*k+i+2,leftmargin+finalcol}=TOR3(end);
        end
        
        %     C1den
        % > density of new/lost before and after deprivation (8d interval)
        % > New 0-8 ??????new 8-16 ????new 16-24 lost 0-8 ??????lost 8-16 ??????lost
        % > 16-24
        k=10;
        newdata(numROIs*k+2,2)={'c1. MEAN gain and loss ratios before and after deprivation, new 0-8 (mean(new a-b+new b-c)/mean(#a+#b+#c), new 8-24, lost 0-8, lost 8-24, last two columns are mean numbers: mean TB# a-c  mean TB# c-g'};
        for j=1:length(c1)
            newdata{numROIs*k+i+2,leftmargin+j}=c1(j);
        end
        
        % > C1aden
        % > density of new/lost before and after deprivation (4d interval)
        % > New 0-4 ????new4-8 ????new8-12 new12-16 ????new16-20 ????new16-24
        % > lost0-4 ????lost4-8 lost8-12 ????lost12-16 ?????lost16-20 ????lost16-24
        k=11;
        newdata(numROIs*k+2,2)={'c1a.  gain and loss fractions over consecutive sessions'};
        for j=1:length(c1a)
            newdata{numROIs*k+i+2,leftmargin+j}=c1a(j);
        end
        
        % c1b [new before0-8 new after8-16  new after16-24
        %     lost before0-8         8-16           16-24
        %     total before0-8         8-16           16-24
        %     TB#a-c c-e e-g] last three columns are total numbers
        k=12;
        newdata(numROIs*k+2,2)={'c1b: gain and loss ratios every 3 sessions. ( 1-3 gains (gain a-c)/(#a+#b+#c) new 0-8, new 8-16, new 16-24;    4-6, lost 0-8, lost 0-16, lost 16-24;    7-9 gain+loss;   10-12 total TB# boutons a-c, c-e, e-g)'};
        for j=1:length(c1b)
            newdata{numROIs*k+i+2,leftmargin+j}=c1b(j);
        end
        
        % > C2 (destabilized EPBs)
        % > ?????????Fraction and density of TBs that were
        % > stable for the first 8d and then were lost.
        % > ???????????fraction: (stable then lost)/(not
        % > losta-b+notlostb-c) notlost a-b+b-c
        k=13;
        newdata(numROIs*k+2,2)={'c2:stable boutons which are then lost. (present a-c lost after c)/(total present a-c), total present a-c'};
        for j=1:length(c2)
            newdata{numROIs*k+i+2,leftmargin+j}=c2(j);
        end
        
        
        % > C5
        % > ?????????Fraction and density of EPBs that came
        % > up after deprivation and then were lost
        % > c5 fraction: (came up then lost)/(total Lost)
        k=14;
        newdata(numROIs*k+2,2)={'c5:come after deprivation and then lost (gain d-end and lost afterwards)/(total gain d-end), total gain d-end '};
        for j=1:length(c5)
            newdata{numROIs*k+i+2,leftmargin+j}=c5(j);
        end
        
        
        
        % > C3 (stabilized EPBs, last 4 days)
        % > ??????????Fraction and density of EPBs that came
        % > up after deprivation and were stable the last 4d.
        % > ???????????c3 fraction: (stablized last 4d)/(total new
        % > boutons after deprivation)
        k=15;
        newdata(numROIs*k+2,2)={'c3: fraction of new boutons stablized (gain d,e,f and stable after)/(gain d,e,f), gain d,e,f'};
        for j=1:length(c3)
            newdata{numROIs*k+i+2,leftmargin+j}=c3(j);
        end
        
        % > C6 (stabilized EPBs, last 8 days)
        % > % and density of EPBs that were stable for the last 8d came up after
        % > deprivation.
        % > c6 fraction: (stable last 8d)/(total came
        % > Up after session c)
        k=16;
        newdata(numROIs*k+2,2)={'c6: fraction of new boutons stablized (gain d,e and stable after)/(gain d,e), gain d,e'};
        for j=1:length(c6)
            newdata{numROIs*k+i+2,leftmargin+j}=c6(j);
        end
        
        % > C8 (stabilized EPBs, last 12 days)
        % > % and density of EPBs that were stable for the last 8d came up after
        % > deprivation.
        % > c8 fraction: (stable last 12d)/(total came
        % > Up after session c)
        k=17;
        newdata(numROIs*k+2,2)={'c8:fraction of new boutons stablized (gain d and stable after)/gain d, gain d'};
        for j=1:length(c8)
            newdata{numROIs*k+i+2,leftmargin+j}=c8(j);
        end
        
        % > C4 ????
        % > ?Fraction and density of EPBs that came up
        % > before deprivation and then stabilized
        k=18;
        newdata(numROIs*k+2,2)={'c4:fraction of new boutons stablized (gain b,c and stable afterwards)/gain b,c, gain b,c'};
        for j=1:length(c4)
            newdata{numROIs*k+i+2,leftmargin+j}=c4(j);
        end
        
        %     C1den
        % > density of new/lost before and after deprivation (8d interval)
        % > New 0-8 ??????new 8-16 ????new 16-24 lost 0-8 ??????lost 8-16 ??????lost
        % > 16-24
        k=19;
        newdata(numROIs*k+2,2)={'c1den: as C1 but den. '};
        for j=1:length(c1den)
            newdata{numROIs*k+i+2,leftmargin+j}=c1den(j);
        end
        
        % > C1aden
        % > density of new/lost before and after deprivation (4d interval)
        % > New 0-4 ????new4-8 ????new8-12 new12-16 ????new16-20 ????new16-24
        % > lost0-4 ????lost4-8 lost8-12 ????lost12-16 ?????lost16-20 ????lost16-24
        k=20;
        newdata(numROIs*k+2,2)={'c1aden:as c1a but density'};
        for j=1:length(c1aden)
            newdata{numROIs*k+i+2,leftmargin+j}=c1aden(j);
        end
        
        % > C1bden
        k=21;
        newdata(numROIs*k+2,2)={'c1bden:1-9 as c1b but density.  Last column is axon segment length'};
        for j=1:length(c1bden)
            newdata{numROIs*k+i+2,leftmargin+j}=c1bden(j);
        end
        
        
        % > C2 (destabilized EPBs)
        % > ?????????Fraction and density of TBs that were
        % > stable for the first 8d and then were lost.
        % > ???????????fraction: (stable then lost)/(not
        % > losta-b+notlostb-c) notlost a-b+b-c
        k=22;
        newdata(numROIs*k+2,2)={'c2den:as c2 but density'};
        for j=1:length(c2den)
            newdata{numROIs*k+i+2,leftmargin+j}=c2den(j);
        end
        
        % > C5den
        % > ?????????Fraction and density of EPBs that came
        % > up after deprivation and then were lost
        % > c5 fraction: (came up then lost)/(total Lost)
        k=23;
        newdata(numROIs*k+2,2)={'c5den:as c5 but density'};
        for j=1:length(c5den)
            newdata{numROIs*k+i+2,leftmargin+j}=c5den(j);
        end
        
        % > C3den (stabilized EPBs, last 4 days)
        % > ??????????Fraction and density of EPBs that came
        % > up after deprivation and were stable the last 4d.
        % > ???????????c3 fraction: (stablized last 4d)/(total new
        % > boutons after deprivation)
        k=24;
        newdata(numROIs*k+2,2)={'c3den:as c3 but density'};
        for j=1:length(c3den)
            newdata{numROIs*k+i+2,leftmargin+j}=c3den(j);
        end
        
        % > C6den (stabilized EPBs, last 8 days)
        % > % and density of EPBs that were stable for the last 8d came up after
        % > deprivation.
        % > c6 fraction: (stable last 8d)/(total came
        % > Up after session c)
        k=25;
        newdata(numROIs*k+2,2)={'c6den:as c6 but density'};
        for j=1:length(c6den)
            newdata{numROIs*k+i+2,leftmargin+j}=c6den(j);
        end
        
        %c8den
        k=26;
        newdata(numROIs*k+2,2)={'c8den:as c8 but density'};
        for j=1:length(c6den)
            newdata{numROIs*k+i+2,leftmargin+j}=c8den(j);
        end
        
        % > C4 den
        % > ?Fraction and density of EPBs that came up
        % > before deprivation and then stabilized
        k=27;
        newdata(numROIs*k+2,2)={'c4den:as c4 but density'};
        for j=1:length(c4den)
            newdata{numROIs*k+i+2,leftmargin+j}=c4den(j);
        end
        
        
        
        % > boutondensities each day
        k=28;
        newdata(numROIs*k+2,2)={'boutondensities'};
        for j=1:length(boutondensity)
            newdata{numROIs*k+i+2,leftmargin+j}=boutondensity(j);
        end
        
        % > C7
        try
            k=29;
            newdata(numROIs*k+2,2)={'c7:  change in TB length for each session a-b, b-c etc.'};
            for j=1:length(c7)
                newdata{numROIs*k+i+2,leftmargin+j}=c7(j);
            end
        catch
        end
    end
end
%
%
%
%
%
%     rows=ROIs{i};
%     matrix=[];
%     for j=1:size(rows,2)
%         matrix=[matrix; [data{rows(j),:}]];
%     end
%     for j=1:size(data,2)
%         col=matrix(:,j);
%         col1=col;
%         col1(isnan(col1))=0;
%         newdata{rows(end)+1,j}=sum(col1)/sum(~isnan(col));
%     end
% end


