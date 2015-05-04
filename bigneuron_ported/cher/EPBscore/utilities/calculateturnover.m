function calculateturnover

% A1 axons

load -mat d:\axons\newA1.db 

global db
summariesuids=1:44;
threshold=1.3;
highthreshold=2;
db1=changethresholds(db,summariesuids,threshold,highthreshold);

pres1=[db1.summaries(summariesuids).daypositions]>0;
isbig1=[db1.summaries(summariesuids).isbig];

% assemble a list of which ROI it came from

names={};
segid=[];
count=1;
for i=1:size(summariesuids,2)
    for j=1:size(db1.summaries(summariesuids(i)).daypositions,2)
        names{count}=db1.summaries(summariesuids(i)).names{1};
        segid(count)=i;
        count=count+1;
    end
end


% old A1 axons

load -mat d:\axons\lwah016.db

global db names segid

load -mat d:\axons\lwah016.db
IDsold=[1 2 3 4 5 7 14 47 12 15 17 31 33 34 53 54 55];% only small boutons  72 73  12 is not sure but included, 13 got deleted, why? 40 duplicated in new boutons
IDsnotsure=[];
IDs=[IDsold IDsnotsure];
summariesuids=IDs;
db2=changethresholds(db,summariesuids,threshold,highthreshold);

pres2=[db2.summaries(summariesuids).daypositions]>0;
isbig2=[db2.summaries(summariesuids).isbig];

for i=1:size(summariesuids,2)
    for j=1:size(db2.summaries(summariesuids(i)).daypositions,2)
        names{count}=db2.summaries(summariesuids(i)).names{1};
        segid(count)=i;
        count=count+1;
    end
end

pres=[pres1(1:7,:) pres2(1:7,:)];
isbig=[isbig1 isbig2(1:7,:)];
twopres=isbig(1:6,:)+isbig(2:7,:);
%twopres=pres(1:6,:)+pres(2:7,:);

%sel=sum(twopres==2)>0;

%has to be big the first two sessions

sel=isbig(1,:) & isbig(2,:);

%calculate turnover ratio on these 
gain=sum(diff(pres(:,sel),1,1)>0,2);
loss=sum(diff(pres(:,sel),1,1)<0,2);
total=sum(pres(:,sel),2);
turnover=(gain+loss)./(2*total(1:6));
surviving=(1:7)'*ones(1,size(pres,2))<=cumsum(pres);
surviving=surviving';
survivalfraction=sum(surviving(sel,:));


% do survivalfraction per animal
limits=[1 103 433 459 590 614 661 672 706 720 730 752 770];
survivalfractionpa=[];
for i=1:12
    survivalfractionpa=[survivalfractionpa; sum(surviving(intersect(find(sel),limits(i):limits(i+1)-1),:),1)];
end












% A3 axons

load -mat d:\axons\A3deprived.db

global db
summariesuids=[1:13 16 18 20 46 49 50 26:29 32 35:36 41 52:83]; % removed and changed to type 1: 14 15 19 30 34 37 38 39 40 42:43
threshold=1.3;
highthreshold=2;
db1=changethresholds(db,summariesuids,threshold,highthreshold);


% find only boutons which were present for more than two consecutive days
pres1=[db1.summaries(summariesuids).daypositions]>0;
isbig1=[db1.summaries(summariesuids).isbig];

% assemble a list of which ROI it came from
names={};
segid=[];
count=1;
for i=1:size(summariesuids,2)
    for j=1:size(db1.summaries(summariesuids(i)).daypositions,2)
        names{count}=db1.summaries(summariesuids(i)).names{1};
        segid(count)=i;
        count=count+1;
    end
end


%old A3 axons from axon outside barrel cortex

load -mat d:\axons\lwah016.db
% Axon 10, outside of barrel cortex

IDs=[20 21 22 23 25 27];
global db
summariesuids=IDs;
db2=changethresholds(db,summariesuids,threshold,highthreshold);


% find only boutons which were present for more than two consecutive days
pres2=[db2.summaries(summariesuids).daypositions]>0;
isbig2=[db2.summaries(summariesuids).isbig];

% assemble a list of which ROI it came from
for i=1:size(summariesuids,2)
    for j=1:size(db2.summaries(summariesuids(i)).daypositions,2)
        names{count}=db2.summaries(summariesuids(i)).names{1};
        segid(count)=i;
        count=count+1;
    end
end

pres=[pres1 pres2(1:7,:)];
isbig=[isbig1 isbig2(1:7,:)];
twopres=isbig(1:6,:)+isbig(2:7,:);
%twopres=pres(1:6,:)+pres(2:7,:);
%sel=sum(twopres==2)>0;
sel=isbig(1,:) & isbig(2,:);

%calculate turnover ratio on these 
gain=sum(diff(pres(:,sel),1,1)>0,2);
loss=sum(diff(pres(:,sel),1,1)<0,2);
total=sum(pres(:,sel),2);
turnover=(gain+loss)./(2*total(1:6));
surviving=(1:7)'*ones(1,size(pres,2))<=cumsum(pres);
surviving=surviving';
survivalfraction=sum(surviving(sel,:));




% do survivalfraction per animal
limits=[1 116 181 207 231 256 280 300 310 375 421 569 592 670 701]; 
survivalfractionpa=[];
for i=1:14
    survivalfractionpa=[survivalfractionpa; sum(surviving(intersect(find(sel),limits(i):limits(i+1)-1),:),1)];
end
















%calculate turnover ratio on these 
gain=sum(diff(pres(:,sel),1,1)>0,2);
loss=sum(diff(pres(:,sel),1,1)<0,2);
total=sum(pres(:,sel),2);


% A3 axons

load -mat f:\deprivedA3.db

global db
summariesuids=[1:16 18:20 46 49 50 26:30 32 34:36 37 38 39:43 52:83];
threshold=2;
db1=changethresholds(db,summariesuids,threshold);


% find only boutons which were present for more than two consecutive days
pres=[db1.summaries(summariesuids).daypositions]>0;
twopres=pres(1:6,:)+pres(2:7,:);
sel=sum(twopres==2)>0;

%calculate turnover ratio on these 
gain=sum(diff(pres(:,sel),1,1)>0,2);
loss=sum(diff(pres(:,sel),1,1)<0,2);
total=sum(pres(:,sel),2);
turnover=(gain+loss)./(2*total(1:6));


% A3 control


load -mat d:\axons\lwah016.db
% Axon 10, outside of barrel cortex

IDs=[20 21 22 23 25 27];
global db
summariesuids=IDs;
threshold=2;
db1=changethresholds(db,summariesuids,threshold);


% find only boutons which were present for more than two consecutive days
pres=[db1.summaries(summariesuids).daypositions]>0;
twopres=pres(1:6,:)+pres(2:7,:);
sel=sum(twopres==2)>0;
% assemble a list of which ROI it came from
names={};
segid=[];
count=1;
for i=1:size(summariesuids,2)
    for j=1:size(db1.summaries(summariesuids(i)).daypositions,2)
        names{count}=db1.summaries(summariesuids(i)).names{1};
        segid(count)=i;
        count=count+1;
    end
end

%calculate turnover ratio on these 
gain=sum(diff(pres(:,sel),1,1)>0,2);
loss=sum(diff(pres(:,sel),1,1)<0,2);
total=sum(pres(:,sel),2);






% acrossday effect




db.summaries(summariesuids).turnoverputativeratio
mean(mean([db.summaries(summariesuids).turnoverputativeratio]))

mean(sum(abs([db.summaries(summariesuids).turnoverputative1]),2))/size(abs([db.summaries(summariesuids).turnoverputative1]),2)

ind=sum([db.summaries(summariesuids).scaledacrossday]>4)>0;
turnover=abs([db.summaries(summariesuids).turnoverputative1]);
mean(sum(turnover(:,ind),2))/size(turnover,2)
