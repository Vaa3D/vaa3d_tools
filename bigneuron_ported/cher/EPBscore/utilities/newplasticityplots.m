figure;
load -mat d:\axons\A3deprived.db
summaries=db.summaries([92:96]);%[1:16 18:20 46 49 50 26:30 32 34:36 37 38 39:43 52:83]); 

%summaries=db.summaries([37 38 39 40 41:43]);

%summaries=db.summaries([53:58]);

%summaries=db.summaries([59:66]);

%summaries=db.summaries([52:83]);

figure;generatebigboutonplot(summaries,'b');

load -mat d:\axons\lwah016.db
% % Axon 10, outside of barrel cortex

IDs=[20 21 22 23 25 27];

summaries=db.summaries(IDs);
generatebigboutonplot(summaries,'b');

% get the data for excel summary sheet
global perboutona perboutonvara perboutonmeana bigida namea perROIa perROIvara namesa perROImeana segida;
namea
segida
printmatrix(perboutona')
printmatrix(perboutonmeana')

% calculate the en passant boution density

totalenpassantcount=[];
totalterminalcount=[];
totallength=[];
names={};
for i=1:length(IDs)
    dendrites=db.series(db.summaries(IDs(i)).seriesuid).dendrites(db.summaries(IDs(i)).dendriteindex,:);
    names{i}=db.summaries(IDs(i)).names{1};
    totalenpassantcount=[totalenpassantcount mean([db.dendrites(dendrites).totallength]./[db.dendrites(dendrites).ibs])];
    totallength=[totallength mean([db.dendrites(dendrites).totallength])];
    totalterminalcount=[totalterminalcount mean([db.dendrites(dendrites).totallength]./[db.dendrites(dendrites).ibsterminal])];
end




















dbid=[1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  18  19  20  21  23  24  25  26  27  28  29  30  32  34  35  36];
animal=[1  1  1  1  1  1  1  1  2  2  2  2  2  3  3  3  3  3  3  4  4  4  4  5  5  5  6  6  6  6  7  7];
generatebigboutonplot(summaries,'b');
global perboutona perboutonvara perboutonmeana bigida namea perROIa perROIvara namesa perROImeana segida;

perROId=perROIa;
perROIvard=perROIvara;
perROImeand=perROImeana;

% redo name by bouton
namesbouton={};
for j=1:size(segida,2)
    namesbouton{j}=namesa{segida(j)};
end


names=char(namea);
anim=[];
for j=2:size(namea,1)
    anim=[anim ~strcmp(names(j,1:7),names(j-1,1:7))];
end


% bootstrap
difference=[];
for i=1:10000
    ind=ceil(rand(1,size(perboutona,2))*size(perboutona,2));
    difference=[difference mean(mean(perboutona(3:6,ind))-mean(perboutona(1:2,ind)))];
end

% do per animal from excel sheet
meanratio=[];
stderr=[];
for i=1:max(anim)
    meanratio=[meanratio; mean(ratios(anim==i,:))];        
end


%shuffle per animal
difference=[];
sizeanimal=[];
for i=1:10000
    ind=[];
    peranimal=floor(size(perboutona,2)/max(animal))+1;
    for j=1:max(animal)
        animalindex=find(ismember(segida,find(animal==j)));               
        ind=[ind animalindex(ceil(rand(1,peranimal)*size(animalindex,2)))];
    end
    difference=[difference mean(mean(perboutona(2:3,ind))-mean(perboutona(1:2,ind)))];
end
figure;
hist(difference);

% correlations
correlations=[];
for i=1:1000
    ind=ceil(rand(1,2)*size(perboutona,2));
    while ind(2)==ind(1)
         ind=ceil(rand(1,2)*size(perboutona,2));    
    end
    corr=corrcoef(perboutona(:,ind));
    correlations=[correlations corr(2,1)];
end

correlations=[];
for i=1:1000
    seg=floor(rand*size(dbid,2))+1;
    segids=find(segida==seg);
    while (size(segids,2)<2)
         seg=floor(rand*size(dbid,2))+1;
        segids=find(segida==seg);        
    end
    ind=ceil(rand(1,2)*size(segids,2));
    while ind(2)==ind(1)
         ind=ceil(rand(1,2)*size(segids,2));    
    end
    corr=corrcoef(perboutona(:,segids(ind)));
    correlations=[correlations corr(2,1)];
end

%per animal
figure;
peram=[];
peramvar=[];
perammean=[];
for i=1:max(animal)
peram=[peram;mean((ratio(animal==i,:)))];
perammean=[perammean;mean(meannum(animal==i,:))];
end

% do it per animal!
figure;
peram=[];
peramvar=[];
perammean=[];
for i=1:max(animal)
    peram=[peram;mean((perboutona(:,ismember(segida,find(animal==i)))'))];
   % peramvar=[peramvar;mean(perboutonvara(:,ismember(segida,find(animal==i)))')];
    perammean=[perammean;mean(perboutonmeana(:,ismember(segida,find(animal==i)))')];
end

subplot(3,1,1);
errorbar(mean(peram),std(peram)/sqrt(size(peram,1)));
xlabel(['signrank--' num2str(signrank(mean(peram(:,1:2)'),mean(peram(:,3:6)'))) ' t-' num2str(ttest(mean(peram(:,1:2)'),mean(peram(:,3:6)')))]);
subplot(3,1,2);
errorbar(mean(peramvar),std(peramvar)/sqrt(size(peramvar,1)));
subplot(3,1,3);
errorbar(mean(perammean),std(perammean)/sqrt(size(perammean,1)));

