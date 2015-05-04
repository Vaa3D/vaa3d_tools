function generatebigboutonplot(summaries,color)
bigid=[];
name=[];
names=[];
segid=[];
for i=1:size(summaries,2)
    % boutons that are always there, if it's bigger than 2 on all days and
    % terminal boutons which are always present
    
    summaries(i).scaledacrossday=summaries(i).acrossday./(summaries(i).meanback'*ones(1,size(summaries(i).acrossday,2)));
    if isempty(summaries(i).terminalacrossday)
        summaries(i).scaledterminalacrossday=[];
    else
        summaries(i).scaledterminalacrossday=summaries(i).terminalacrossday./(summaries(i).meanback'*ones(1,size(summaries(i).terminalacrossday,2)));
    end
    if isempty(summaries(i).scaledacrossday)
        bigboutons=[];
    else
        bigboutons=(sum(summaries(i).scaledacrossday>1.7)==size(summaries(i).scaledacrossday,1));
    end
    
    if isempty(summaries(i).scaledterminalacrossday)
        stableterminals=[];
    else
        stableterminals=[];
        stableterminals=(sum(summaries(i).scaledterminalacrossday>0.05)==size(summaries(i).scaledacrossday,1));
        %stableterminals=stableterminals(stableterminals>0);
    end
    
    if 1% stable boutons
    summaries(i).bigboutons=bigboutons;
    summaries(i).variationsbig=diff([summaries(i).scaledacrossday(:,bigboutons)]);% summaries(i).scaledterminalacrossday(:,stableterminals)]);
    summaries(i).ratiosbig=diff(log([summaries(i).scaledacrossday(:,bigboutons)]));% summaries(i).scaledterminalacrossday(:,stableterminals)]));
    summaries(i).variations3=diff([summaries(i).scaledacrossday]);
    summaries(i).meanstrength=mean([summaries(i).scaledacrossday(:,bigboutons)]);% summaries(i).scaledterminalacrossday(:,stableterminals)]);
    else %stable terminals
    bigboutons=stableterminals;
    summaries(i).bigboutons=bigboutons;
    summaries(i).variationsbig=diff([summaries(i).scaledterminalacrossday(:,bigboutons)]);% summaries(i).scaledterminalacrossday(:,stableterminals)]);
    summaries(i).ratiosbig=diff(log([summaries(i).scaledterminalacrossday(:,bigboutons)]));% summaries(i).scaledterminalacrossday(:,stableterminals)]));
    summaries(i).variations3=diff([summaries(i).scaledterminalacrossday]);
    summaries(i).meanstrength=mean([summaries(i).scaledterminalacrossday(:,bigboutons)]);% summaries(i).scaledterminalacrossday(:,stableterminals)]);    
    end
    summaries(i).days=size(summaries(i).ratiosbig,1);       
    summaries(i).sizebig=size(summaries(i).ratiosbig,2);
    bigid=[bigid find(bigboutons)];
    if (sum(bigboutons)>0)
        segid=[segid i*ones(1,sum(bigboutons))];
    end
    for j=1:summaries(i).sizebig
        name=[name;summaries(i).names(2)];
    end
    names=[names;summaries(i).names(2)];
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
IDs=1:size(summaries,2);
IDs=IDs([summaries(IDs).sizebig]>0);

for i=1:size(IDs,2)
    perROI=[perROI mean(exp(abs(summaries(IDs(i)).ratiosbig(1:6,:))),2)];
    perbouton=[perbouton exp(abs(summaries(IDs(i)).ratiosbig(1:6,:)))];
  %      perbouton=[perbouton abs(summaries(IDs(i)).ratiosbig(1:6,:))];
    perROIvar=[perROIvar mean(abs(summaries(IDs(i)).variationsbig(1:6,:)),2)];
    perboutonvar=[perboutonvar abs(summaries(IDs(i)).variationsbig(1:6,:))];
    perROImean=[perROImean mean(summaries(IDs(i)).scaledacrossday(1:7,summaries(IDs(i)).bigboutons),2)];
    perboutonmean=[perboutonmean summaries(IDs(i)).scaledacrossday(1:7,summaries(IDs(i)).bigboutons)];
  
end
lastdayperROIvar=[];
lastdayperROI=[];
lastdayperROIturnover=[];
lastdayperbouton=[];
lastdayperboutonvar=[];
lastdayperboutonturnover=[];
lastdayperboutonmean=[];
lastdayperROImean=[];
IDs=find([summaries.days]==7);
for i=1:size(IDs,2)
    lastdayperROI=[lastdayperROI mean(exp(abs(summaries(IDs(i)).ratiosbig(7,:))),2)];
    lastdayperbouton=[lastdayperbouton exp(abs(summaries(IDs(i)).ratiosbig(7,:)))];
    %   lastdayperbouton=[lastdayperbouton abs(summaries(IDs(i)).ratiosbig(7,:))];
    lastdayperROIvar=[lastdayperROIvar mean(abs(summaries(IDs(i)).variationsbig(7,:)),2)];
    lastdayperboutonvar=[lastdayperboutonvar abs(summaries(IDs(i)).variationsbig(7,:))];
    lastdayperROImean=[lastdayperROImean mean(summaries(IDs(i)).scaledacrossday(8,summaries(IDs(i)).bigboutons),2)];
    lastdayperboutonmean=[lastdayperROImean summaries(IDs(i)).scaledacrossday(8,summaries(IDs(i)).bigboutons)];
end
subplot(4,2,1);
hold on;
errorbar([mean(perROI')-1 mean(lastdayperROI')-1],[std(perROI')/sqrt(size(perROI,2)) std(lastdayperROI')/sqrt(size(lastdayperROI,2))],color);
xlabel(['signrank--' num2str(signrank(mean(perROI(1:2,:)),mean(perROI(3:6,:)))) ' t-' num2str(ttest(mean(perROI(1:2,:)),mean(perROI(3:6,:))))]);
ylabel('ratio:(big-small)/small');
subplot(4,2,2);
hold on;
errorbar([mean(perbouton')-1 mean(lastdayperbouton')-1],[std(perbouton')/sqrt(size(perbouton,2)) std(lastdayperbouton')/sqrt(size(lastdayperbouton,2))],color);
%errorbar([exp(mean(perbouton'))-1 exp(mean(lastdayperbouton'))-1],[exp(std(perbouton'))/sqrt(size(perbouton,2)) exp(std(lastdayperbouton'))/sqrt(size(lastdayperbouton,2))],color);
xlabel(['signrank--' num2str(signrank(mean(perbouton(1:2,:)),mean(perbouton(3:6,:)))) ' t-' num2str(ttest(mean(perbouton(1:2,:)),mean(perbouton(3:6,:))))]);
ylabel('ratio:(big-small)/small');
subplot(4,2,3);
hold on;
errorbar([mean(perROIvar') mean(lastdayperROIvar')],[std(perROIvar')/sqrt(size(perROIvar,2)) std(lastdayperROIvar')/sqrt(size(lastdayperROIvar,2))],color);
xlabel(['signrank--' num2str(signrank(mean(perROIvar(1:2,:)),mean(perROIvar(3:6,:)))) ' t-' num2str(ttest(mean(perROIvar(1:2,:)),mean(perROIvar(3:6,:))))]);
ylabel('absolute:backbone units');
subplot(4,2,4);
hold on;
errorbar([mean(perboutonvar') mean(lastdayperboutonvar')],[std(perboutonvar')/sqrt(size(perboutonvar,2)) std(lastdayperboutonvar')/sqrt(size(lastdayperboutonvar,2))],color);
xlabel(['signrank--' num2str(signrank(mean(perboutonvar(1:2,:)),mean(perboutonvar(3:6,:)))) ' t-' num2str(ttest(mean(perboutonvar(1:2,:)),mean(perboutonvar(3:6,:))))]);
ylabel('absolute:backbone units');
subplot(4,2,5);
hold on;
errorbar([mean(perROImean') mean(lastdayperROImean')],[std(perROImean')/sqrt(size(perROImean,2)) std(lastdayperROImean')/sqrt(size(lastdayperROImean,2))],color);
xlabel(['signrank--' num2str(signrank(mean(perROImean(1:3,:)),mean(perROImean(4:7,:)))) ' t-' num2str(ttest(mean(perROImean(1:3,:)),mean(perROImean(4:7,:))))]);
ylabel('mean intensity:backbone units');
subplot(4,2,6);
hold on;
errorbar([mean(perboutonmean') mean(lastdayperboutonmean')],[std(perboutonmean')/sqrt(size(perboutonmean,2)) std(lastdayperboutonmean')/sqrt(size(lastdayperboutonmean,2))],color);
xlabel(['signrank--' num2str(signrank(mean(perboutonmean(1:3,:)),mean(perboutonmean(4:7,:)))) ' t-' num2str(ttest(mean(perboutonmean(1:3,:)),mean(perboutonmean(4:7,:))))]);
ylabel('mean intensity:backbone units');

disp('number of segments');
size(perROI,2)
disp('number of boutons');
size(perbouton,2)
% 
% figure;plot([summaries.meanstrength],mean(perboutonvar),'.');
% xlabel('mean bouton intensity');
% ylabel('mean absolute change');
global perboutona perboutonvara perboutonmeana bigida perROIvara perROIa namea namesa perROImeana segida;

perboutona=perbouton;
perboutonvara=perboutonvar;
perboutonmeana=perboutonmean;
perROIvara=perROIvar;
perROIa=perROI;
perROImeana=perROImean;

segida=segid;
bigida=bigid;
namea=name;
namesa=names;