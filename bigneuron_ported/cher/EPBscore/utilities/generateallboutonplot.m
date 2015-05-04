function generateallboutonplot(summaries,color)
for i=1:size(summaries,2)
    % boutons that are always there, if it's bigger than 2 on all days and
    % terminal boutons which are always present
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
    end
    summaries(i).scaledacrossday=summaries(i).acrossday./(summaries(i).meanback'*ones(1,size(summaries(i).acrossday,2)));
    summaries(i).scaledacrossdayterminal=summaries(i).acrossday./(summaries(i).meanback'*ones(1,size(summaries(i).acrossday,2)));
    summaries(i).variationsbig=diff([summaries(i).scaledacrossday(:,bigboutons) summaries(i).scaledterminalacrossday(:,stableterminals)]);
    summaries(i).ratiosbig=diff(log([summaries(i).scaledacrossday(:,bigboutons) summaries(i).scaledterminalacrossday(:,stableterminals)]));
    summaries(i).variationsall=diff([summaries(i).scaledacrossday]);
    summaries(i).ratiosall=diff(log([summaries(i).scaledacrossday]));
    summaries(i).variations3=diff([summaries(i).scaledacrossday]);
    summaries(i).meanstrength=mean([summaries(i).scaledacrossday]);
    turnoverputative=[];
    putativestate=summaries(i).scaledacrossday>1.7;
    summaries(i).putativestate=putativestate;
    summaries(i).days=size(summaries(i).ratiosbig,1);
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
ints=[];
index=[];
IDs=1:size(summaries,2);
for i=1:size(IDs,2)
    perROI=[perROI mean(exp(abs(summaries(IDs(i)).ratiosall(1:6,:))),2)];
    perbouton=[perbouton exp(abs(summaries(IDs(i)).ratiosall(1:6,:)))];
    perROIvar=[perROIvar mean(abs(summaries(IDs(i)).variationsall(1:6,:)),2)];
    perboutonvar=[perboutonvar abs(summaries(IDs(i)).variationsall(1:6,:))];
    perROImean=[perROImean mean(summaries(IDs(i)).variationsall(1:6,:),2)];
    perboutonmean=[perROImean mean(summaries(IDs(i)).variationsall(1:6,:),2)];
   % perROIturnover=[perROIturnover sum(diff(summaries(IDs(i)).putativestate),2)./sum(summaries(IDs(i)).putativestate(1:(size(summaries(IDs(i)).scaledacrossday,1)-1),:),2)];
    perboutonturnover=[perboutonturnover summaries(IDs(i)).putativestate(1:7,:)];
    ints=[ints summaries(IDs(i)).scaledacrossday(1:7,:)];
    index=[index IDs(i)*100+(1:size(summaries(IDs(i)).scaledacrossday(1:7,:),2))];
end
lastdayperROIvar=[];
lastdayperROI=[];
lastdayboutonstate=[];
lastdayperbouton=[];
lastdayperboutonvar=[];
lastdayperboutonturnover=[];
lastdayperboutonmean=[];
lastdayperROImean=[];
IDs=find([summaries.days]==7);
for i=1:size(IDs,2)
    lastdayperROI=[lastdayperROI mean(exp(abs(summaries(IDs(i)).ratiosall(7,:))),2)];
    lastdayperbouton=[lastdayperbouton exp(abs(summaries(IDs(i)).ratiosall(7,:)))];
    lastdayperROIvar=[lastdayperROIvar mean(abs(summaries(IDs(i)).variationsall(7,:)),2)];
    lastdayperboutonvar=[lastdayperboutonvar abs(summaries(IDs(i)).variationsall(7,:))];
    lastdayperROImean=[lastdayperROImean mean(summaries(IDs(i)).variationsall(7,:),2)];
    lastdayperboutonmean=[lastdayperROImean mean(summaries(IDs(i)).variationsall(7,:),2)];
    lastdayboutonstate=[lastdayboutonstate summaries(IDs(i)).putativestate(7,:)];
    lastdayperboutonturnover=[lastdayperboutonturnover abs(summaries(IDs(i)).putativestate(8,:))];
end
subplot(4,2,1);
hold on;
errorbar([mean(perROI')-1 mean(lastdayperROI')-1],[std(perROI')/sqrt(size(perROI,2)) std(lastdayperROI')/sqrt(size(lastdayperROI,2))],color);
xlabel(['signrank--' num2str(signrank(mean(perROI(1:2,:)),mean(perROI(5:6,:)))) ' t-' num2str(ttest(mean(perROI(1:2,:)),mean(perROI(5:6,:))))]);
subplot(4,2,2);
hold on;
errorbar([mean(perbouton')-1 mean(lastdayperbouton')-1],[std(perbouton')/sqrt(size(perbouton,2)) std(lastdayperbouton')/sqrt(size(lastdayperbouton,2))],color);
xlabel(['signrank--' num2str(signrank(mean(perbouton(1:2,:)),mean(perbouton(5:6,:)))) ' t-' num2str(ttest(mean(perbouton(1:2,:)),mean(perbouton(5:6,:))))]);
subplot(4,2,3);
hold on;
errorbar([mean(perROIvar') mean(lastdayperROIvar')],[std(perROIvar')/sqrt(size(perROIvar,2)) std(lastdayperROIvar')/sqrt(size(lastdayperROIvar,2))],color);
xlabel(['signrank--' num2str(signrank(mean(perROIvar(1:2,:)),mean(perROIvar(5:6,:)))) ' t-' num2str(ttest(mean(perROIvar(1:2,:)),mean(perROIvar(5:6,:))))]);
subplot(4,2,4);
hold on;
errorbar([mean(perboutonvar') mean(lastdayperboutonvar')],[std(perboutonvar')/sqrt(size(perboutonvar,2)) std(lastdayperboutonvar')/sqrt(size(lastdayperboutonvar,2))],color);
xlabel(['signrank--' num2str(signrank(mean(perboutonvar(1:2,:)),mean(perboutonvar(5:6,:)))) ' t-' num2str(ttest(mean(perboutonvar(1:2,:)),mean(perboutonvar(5:6,:))))]);
subplot(4,2,5);
hold on;
errorbar([mean(perROImean') mean(lastdayperROImean')],[std(perROImean')/sqrt(size(perROImean,2)) std(lastdayperROImean')/sqrt(size(lastdayperROImean,2))],color);
xlabel(['signrank--' num2str(signrank(mean(perROImean(1:2,:)),mean(perROImean(5:6,:)))) ' t-' num2str(ttest(mean(perROImean(1:2,:)),mean(perROImean(5:6,:))))]);
subplot(4,2,6);
hold on;
errorbar([mean(perboutonmean') mean(lastdayperboutonmean')],[std(perboutonmean')/sqrt(size(perboutonmean,2)) std(lastdayperboutonmean')/sqrt(size(lastdayperboutonmean,2))],color);
xlabel(['signrank--' num2str(signrank(mean(perboutonmean(1:2,:)),mean(perboutonmean(5:6,:)))) ' t-' num2str(ttest(mean(perboutonmean(1:2,:)),mean(perboutonmean(5:6,:))))]);
subplot(4,2,7);
hold on;
errorbar([sum(perboutonturnover(1:7,:),2)' sum(lastdayboutonstate)]  ...
            ,[sqrt(sum(perboutonturnover(1:7,:),2)') sqrt(sum(lastdayboutonstate))],color);

% hold on;
% errorbar([mean(perROIturnover') mean(lastdayperROIturnover')],[std(perROIturnover')/sqrt(size(perROIturnover,2)) std(lastdayperROIturnover')/sqrt(size(lastdayperROIturnover,2))],color);
% xlabel(['signrank--' num2str(signrank(mean(perROIturnover(1:2,:)),mean(perROIturnover(5:6,:)))) ' t-' num2str(ttest(mean(perROIturnover(1:2,:)),mean(perROIturnover(5:6,:))))]);
subplot(4,2,8);
hold on;
errorbar([sum(abs(diff(perboutonturnover)'))./(sum(perboutonturnover(1:6,:),2)'+sum(perboutonturnover(2:7,:),2)') sum(abs(lastdayperboutonturnover-lastdayboutonstate))/(sum(lastdayperboutonturnover)+sum(lastdayboutonstate))]  ...
            ,[sqrt(sum(abs(diff(perboutonturnover)')))./(sum(perboutonturnover(1:6,:),2)'+sum(perboutonturnover(2:7,:),2)') sqrt(sum(abs(lastdayperboutonturnover-lastdayboutonstate)))/(sum(lastdayperboutonturnover)+sum(lastdayboutonstate))],color);
%errorbar(sum(diff(perboutonturnover),2)./sum(perboutonturnover(1:8,:)',2),sqrt(sum(abs(diff(perboutonturnover)'))./sum(perboutonturnover(1:8,:),2))
%plot(sum(diff(perboutonturnover),2)./sum(perboutonturnover(1:6,:),2));
%errorbar([mean(perboutonturnover') mean(lastdayperboutonturnover')],[std(perboutonturnover')/sqrt(size(perboutonturnover,2)) std(lastdayperboutonturnover')/sqrt(size(lastdayperboutonturnover,2))],color);
%xlabel(['signrank--' num2str(signrank(mean(perboutonturnover(1:2,:)),mean(perboutonturnover(5:6,:)))) ' t-' num2str(ttest(mean(perboutonturnover(1:2,:)),mean(perboutonturnover(5:6,:))))]);
% 
% % figure;
% hist([summaries.meanstrength],20);
% xlabel('mean bouton intensity');
% ylabel('counts');
% plot([summaries.meanstrength],mean(perboutonvar),'.k');
% xlabel('mean bouton intensity (backbone intensity units)');
% ylabel('mean intensity change over sessions');
