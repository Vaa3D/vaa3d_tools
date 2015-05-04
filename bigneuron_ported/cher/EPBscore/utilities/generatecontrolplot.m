function generatecontrolplot()
color='k';
figure;

perROI=[];
perbouton=[];
meanperbouton=[];
errorperbouton=[];
perROIvar=[];
meanperboutonvar=[];
errorperboutonvar=[];
perboutonturnover=[];
 
load -mat d:\axons\fix.db
[a,b,c,d,e,f]=getavgvalues(db.summaries([1:2 4:5]));  
perROI=[perROI; a];
meanperbouton=[meanperbouton; mean(b)];
errorperbouton=[errorperbouton; std(b)/sqrt(size(b,2))];
perROIvar=[perROIvar; c];
meanperboutonvar=[meanperboutonvar; mean(d)];
errorperboutonvar=[errorperboutonvar; std(d)/sqrt(size(d,2))];
perboutonturnover=[perboutonturnover; mean(e)];  

load -mat d:\axons\samedayfast.db
[a,b,c,d,e,f]=getavgvalues(db.summaries(1:4));  
perROI=[perROI; a];
meanperbouton=[meanperbouton; mean(b)];
errorperbouton=[errorperbouton; std(b)/sqrt(size(b,2))];
perROIvar=[perROIvar; c];
meanperboutonvar=[meanperboutonvar; mean(d)];
errorperboutonvar=[errorperboutonvar; std(d)/sqrt(size(d,2))];
perboutonturnover=[perboutonturnover; mean(e)];       

load -mat d:\axons\sameday.db
[a,b,c,d,e,f]=getavgvalues(db.summaries(1:4));  
perROI=[perROI; a];
meanperbouton=[meanperbouton; mean(b)];
errorperbouton=[errorperbouton; std(b)/sqrt(size(b,2))];
perROIvar=[perROIvar; c];
meanperboutonvar=[meanperboutonvar; mean(d)];
errorperboutonvar=[errorperboutonvar; std(d)/sqrt(size(d,2))];
perboutonturnover=[perboutonturnover; mean(e)];       

load -mat d:\axons\overday.db
[a,b,c,d,e,f]=getavgvalues(db.summaries(1:4));  
perROI=[perROI; a];
meanperbouton=[meanperbouton; mean(b)];
errorperbouton=[errorperbouton; std(b)/sqrt(size(b,2))];
perROIvar=[perROIvar; c];
meanperboutonvar=[meanperboutonvar; mean(d)];
errorperboutonvar=[errorperboutonvar; std(d)/sqrt(size(d,2))];
perboutonturnover=[perboutonturnover; mean(e)];       

color='+k';
subplot(2,2,1);
hold on;
errorbar([mean(perROI')],[std(perROI')/sqrt(size(perROI,2))],color);
bar([mean(perROI')],color);
axis([0 5 1 1.7]);
ylabel('fractional intensity change');
subplot(2,2,2);
hold on;
errorbar([meanperbouton],[errorperbouton],color);
bar([meanperbouton],color);
axis([0 5 1 1.7]);
ylabel('fractional intensity change');
subplot(2,2,3);
hold on;
errorbar([mean(perROIvar')],[std(perROIvar')/sqrt(size(perROIvar,2))],color);
bar([mean(perROIvar')],color);
ylabel('absolute change (backbone units)');
subplot(2,2,4);
hold on;
errorbar([meanperboutonvar],[errorperboutonvar],color);
bar([meanperboutonvar],color);
ylabel('absolute change (backbone units)');

% 
% color='b';
% subplot(2,2,1);
% hold on;
% errorbar([2],[mean(perROI') ],[std(perROI')/sqrt(size(perROI,2))],color);
% subplot(2,2,2);
% hold on;
% errorbar([2],[mean(perbouton')],[std(perbouton')/sqrt(size(perbouton,2))],color);
% subplot(2,2,3);
% hold on;
% errorbar([2],[mean(perROIvar')],[std(perROIvar')/sqrt(size(perROIvar,2))],color);
% subplot(2,2,4);
% hold on;
% errorbar([2],[mean(perboutonvar')],[std(perboutonvar')/sqrt(size(perboutonvar,2))],color);

    
    
    
function [perROI,perbouton,perROIvar,perboutonvar,perboutonturnover,meanstrength]=getavgvalues(summaries)    
for i=1:size(summaries,2)
    % boutons that are always there, if it's bigger than 2 on all days and
    % terminal boutons which are always present
    if isempty(summaries(i).scaledacrossday)
        bigboutons=[];
    else
        bigboutons=(sum(summaries(i).scaledacrossday>2)==size(summaries(i).scaledacrossday,1));
    end
    if isempty(summaries(i).scaledterminalacrossday)
        stableterminals=[];
    else
        stableterminals=[];
        stableterminals=(sum(summaries(i).scaledterminalacrossday>0.05)==size(summaries(i).scaledacrossday,1));
    end
    summaries(i).scaledacrossday=summaries(i).acrossday./(summaries(i).meanback'*ones(1,size(summaries(i).acrossday,2)));
    summaries(i).scaledacrossdayterminal=summaries(i).acrossday./(summaries(i).meanback'*ones(1,size(summaries(i).acrossday,2)));
    summaries(i).variationsbig=[];
    summaries(i).ratiosbig=[];
    summaries(i).variationsall=[];
    summaries(i).ratiosall=[];
    try
    for j=1:100
        oneperm=randperm(size(summaries(i).scaledacrossday,1));
        summaries(i).variationsbig=[summaries(i).variationsbig;diff([summaries(i).scaledacrossday(oneperm(1:2),bigboutons)])];        
        summaries(i).ratiosbig=[summaries(i).ratiosbig;diff(log([summaries(i).scaledacrossday(oneperm(1:2),bigboutons)]))];        
        summaries(i).variationsall=[summaries(i).variationsall;diff([summaries(i).scaledacrossday(oneperm(1:2),:)])];        
        summaries(i).ratiosall=[summaries(i).ratiosall;diff(log([summaries(i).scaledacrossday(oneperm(1:2),:)]))];        
    end
catch
    i
end
end

hold on;
perROIvar=[];
perROI=[];
perROIturnover=[];
perbouton=[];
perboutonvar=[];
perboutonturnover=[];
perboutonmean=[];
perROImean=[];
meanstrength=[];
IDs=1:size(summaries,2);
for i=1:size(IDs,2)
    perROI=[perROI mean(mean(exp(abs(summaries(IDs(i)).ratiosbig(:,:))),1),2)];
    perbouton=[perbouton mean(exp(abs(summaries(IDs(i)).ratiosbig(:,:))),1)];
    perROIvar=[perROIvar mean(mean(abs(summaries(IDs(i)).variationsbig(:,:)),2),1)];
    perboutonvar=[perboutonvar mean(abs(summaries(IDs(i)).variationsbig(:,:)),1)];
    perboutonturnover=[perboutonturnover sum(abs(summaries(IDs(i)).turnoverputative2(:,:)))];
    meanstrength=[meanstrength mean(summaries(IDs(i)).scaledacrossday)];
end