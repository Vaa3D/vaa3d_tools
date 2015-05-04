function [a,b,differences,differences1,differences2,ratios,ratios1,ratios2,turnover,turnoverputative,turnoverputative1,turnoverputative2,meanmeanback]=calculatechanges(acrossday,terminalacrossday,meanback,noiselevel,scalea,higherthreshold,lowerthreshold)
a=[1];
b=[1];
differences=[];
ratios=[];
turnover=[];
if ~isempty(acrossday)
for j=2:size(acrossday,1)
    %scale each trace to give minimal difference   -- not true
%    scale=0.3:0.0001:3;
%    difference=sum(abs((ones(max(size(scale)),1)*acrossday(j-1,:)-scale'*acrossday(j,:))),2)';
%    [mindiff,minscale]=min(difference);    
%    a=[a a(j-1)*(0.3+(minscale-1)*0.0001)];
    a=1./mean(acrossday,2)';
    differences=[differences; -a(j-1)*acrossday(j-1,:)+a(j)*acrossday(j,:)];
    
  
    
    if (max(size(terminalacrossday))>0)
        turnover=[turnover; diff((terminalacrossday(j-1:j,:)~=0))];
    else
        turnover=zeros(size(acrossday,1)-1,0);
    end
    
    %scale each trace to give minimal difference normalized by bouton
    %intensities
    scale=exp(log(0.1):0.0001:log(10));
    difference=sum(abs(ones(max(size(scale)),1)*(acrossday(j-1,:)-noiselevel*meanback(j-1))-scale'*(acrossday(j,:)-noiselevel*meanback(j)))./(ones(max(size(scale)),1)*(acrossday(j-1,:)-noiselevel*meanback(j-1))+scale'*(acrossday(j,:)-noiselevel*meanback(j))),2)';
    [mindiff,minscale]=min(difference);    
    b=[b b(j-1)*exp(log(0.1)+minscale*0.0001)];
    ratios=[ratios; (-(acrossday(j-1,:)-noiselevel*meanback(j-1))+b(j)*(acrossday(j,:)-noiselevel*meanback(j))/b(j-1))./((acrossday(j-1,:)-noiselevel*meanback(j-1))+b(j)*(acrossday(j,:)-noiselevel*meanback(j))/b(j-1))];
end




meanmeanback=mean(meanback.*a);

a=a./meanmeanback;
scaledacrossday=acrossday.*(a'*ones(1,size(acrossday,2)));
turnoverputative=[];
putativestate=ones(1,size(acrossday,2));
differences=differences./meanmeanback;
for j=1:size(scaledacrossday,1)
    turnoverputative=[turnoverputative; putativestate-(putativestate & (scaledacrossday(j,:)<lowerthreshold))+(~putativestate & (scaledacrossday(j,:)>higherthreshold))];
end         
turnoverputative=diff(turnoverputative,1);
meanmeanback=mean(meanback./scalea)*scalea(1);
scaledacrossday=acrossday./(scalea'*ones(1,size(acrossday,2)));      % Scale by mean intensity
scaledacrossday=(scaledacrossday*scalea(1))./meanmeanback;

turnoverputative1=[];
putativestate=ones(1,size(acrossday,2));
differences1=diff(scaledacrossday,1);
for j=1:size(scaledacrossday,1)
    turnoverputative1=[turnoverputative1; putativestate-(putativestate & (scaledacrossday(j,:)<lowerthreshold))+(~putativestate & (scaledacrossday(j,:)>higherthreshold))];
end    
turnoverputative1=diff(turnoverputative1,1);


scaledacrossday=acrossday./(meanback'*ones(1,size(acrossday,2)));
turnoverputative2=[];
putativestate=ones(1,size(acrossday,2));
differences2=diff(scaledacrossday,1);
for j=1:size(scaledacrossday,1)
    turnoverputative2=[turnoverputative2; putativestate-(putativestate & (scaledacrossday(j,:)<lowerthreshold))+(~putativestate & (scaledacrossday(j,:)>higherthreshold))];
end    
turnoverputative2=diff(turnoverputative2,1);
ratios1=diff(log(acrossday)-(mean(log(acrossday),2))*ones(1,size(acrossday,2))); % Scale by mean log values
ratios2=diff(log(acrossday)-log(meanback)'*ones(1,size(acrossday,2))); % Scale by back backone
else
a=ones(1,8);
b=ones(1,8);
differences=[];
differences1=[];
differences2=[];
ratios=[];
ratios1=[];
ratios2=[];
turnoverputative=[];
turnoverputative1=[];
turnoverputative2=[];
meanmeanback=mean(meanback);
turnover=[];
% for j=2:size(acrossday,1)
% 
%     if (max(size(terminalacrossday))>0)
%         turnover=[turnover; diff((terminalacrossday(j-1:j,:)~=0))];
%     else
%         turnover=zeros(size(acrossday,1)-1,0);
%     end
% end
end

