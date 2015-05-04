function generateterminalboutonplot(summaries,color)
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
IDs=1:size(summaries,2);
for i=1:size(IDs,2)
   % perROIturnover=[perROIturnover sum(diff(summaries(IDs(i)).putativestate),2)./sum(summaries(IDs(i)).putativestate(1:(size(summaries(IDs(i)).scaledacrossday,1)-1),:),2)];
    perboutonturnover=[perboutonturnover summaries(IDs(i)).terminalacrossday(1:7,:)>0];
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
    lastdayboutonstate=[lastdayboutonstate summaries(IDs(i)).terminalacrossday(7,:)>0];
    lastdayperboutonturnover=[lastdayperboutonturnover abs(summaries(IDs(i)).terminalacrossday(8,:))>0];
end

subplot(1,2,1);
hold on;
errorbar([sum(perboutonturnover(1:7,:),2)' sum(lastdayboutonstate)]  ...
            ,[sqrt(sum(perboutonturnover(1:7,:),2)') sqrt(sum(lastdayboutonstate))],color);

% hold on;
% errorbar([mean(perROIturnover') mean(lastdayperROIturnover')],[std(perROIturnover')/sqrt(size(perROIturnover,2)) std(lastdayperROIturnover')/sqrt(size(lastdayperROIturnover,2))],color);
% xlabel(['signrank--' num2str(signrank(mean(perROIturnover(1:2,:)),mean(perROIturnover(5:6,:)))) ' t-' num2str(ttest(mean(perROIturnover(1:2,:)),mean(perROIturnover(5:6,:))))]);
subplot(1,2,2);
hold on;
errorbar([sum(abs(diff(perboutonturnover)'))./(sum(perboutonturnover(1:6,:),2)'+sum(perboutonturnover(2:7,:),2)') sum(abs(lastdayperboutonturnover-lastdayboutonstate))/(sum(lastdayperboutonturnover)+sum(lastdayboutonstate))]  ...
            ,[sqrt(sum(abs(diff(perboutonturnover)')))./(sum(perboutonturnover(1:6,:),2)'+sum(perboutonturnover(2:7,:),2)') sqrt(sum(abs(lastdayperboutonturnover-lastdayboutonstate)))/(sum(lastdayperboutonturnover)+sum(lastdayboutonstate))],color);
%errorbar(sum(diff(perboutonturnover),2)./sum(perboutonturnover(1:8,:)',2),sqrt(sum(abs(diff(perboutonturnover)'))./sum(perboutonturnover(1:8,:),2))
%plot(sum(diff(perboutonturnover),2)./sum(perboutonturnover(1:6,:),2));
%errorbar([mean(perboutonturnover') mean(lastdayperboutonturnover')],[std(perboutonturnover')/sqrt(size(perboutonturnover,2)) std(lastdayperboutonturnover')/sqrt(size(lastdayperboutonturnover,2))],color);
%xlabel(['signrank--' num2str(signrank(mean(perboutonturnover(1:2,:)),mean(perboutonturnover(5:6,:)))) ' t-' num2str(ttest(mean(perboutonturnover(1:2,:)),mean(perboutonturnover(5:6,:))))]);
% 
% figure;
% hist([summaries.meanstrength],20);
% xlabel('mean bouton intensity');
% ylabel('counts');
% figure;plot([summaries.meanstrength],mean(perboutonvar),'.');
% xlabel('mean bouton intensity');
% ylabel('mean absolute change');
