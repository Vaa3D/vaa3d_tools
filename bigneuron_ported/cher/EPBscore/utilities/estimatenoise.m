function [a,b,ratio1,ratio2]=estimatenoise(overdayden,exclusion)
worldline=[];
worldlen=[];
warning off MATLAB:divideByZero;
 mindis=-10000000;
    maxdis=100000000;
    for j=1:size(overdayden,2)
        mindis=max(mindis,min(overdayden(j).overdaylen));
        maxdis=min(maxdis,max(overdayden(j).overdaylen));
    end
x=floor(mindis+2):0.25:ceil(maxdis-2);
for j=x
    thisline=zeros(1,size(overdayden,2));
    thislen=zeros(1,size(overdayden,2));
    
    for k=1:size(overdayden,2)
        s=reflectivesmooth(double(overdayden(k).maxintraw),7);
        points=(overdayden(k).overdaylen<j+0.25)&(overdayden(k).overdaylen>j-0.0001);
        if ~isempty(s(points))
            thisline(k)=mean(s(points));
        else
            if (k>1)
                thisline(k)=thisline(k-1);
            else
                thisline(k)=0;
            end
        end
        thislen(k)=mean(overdayden(k).length(points));
    end
    worldline=[worldline;thisline];
    worldlen=[worldlen;thislen];
end
% cut out excluded regions

index=[];
for j=1:size(exclusion,1)
    left=find(x>(exclusion(j,1)-0.25));
    left=max(1,left(1));
    right=find(x<(exclusion(j,2)+0.25));
    right=min(size(x,2),right(end));
    index=[index left:right];
end
worldline(index,:)=[];

y=worldline;
b=0;%mean(mean(y))-std(mean(y))*mean(std(y))/std(std(y));
a=mean((y-b));
% ycorr=(y-b)./(ones(size(y,1),1)*a);
% a=a.*mean(ycorr(ycorr(:,1)>1,:));
ycorr=(y-b)./(ones(size(y,1),1)*a);
% figure;
% plot(ycorr);
maxtios=[];
ratios=[];
ratio1=mean(std(ycorr(mean(ycorr,2)>max(mean(ycorr,2)*0.7),:)'))/mean(mean(ycorr(mean(ycorr,2)>max(mean(ycorr,2)*0.5),:)'));
ratio2=median(std(ycorr'))/median(mean(ycorr'));
%ratio3=std(max(ycorr))/mean(max(ycorr));
% figure;
% plot(mean(ycorr'),std(ycorr'),'.');
mean(std(ycorr'))/mean(mean(ycorr'));

% for i=1:8
% y=worldline(1+i*8:9+i*8,:);
% ycorr=(y-b)./(ones(size(y,1),1)*a);
% ratios=[ratios; mean(std(ycorr')) mean(mean(ycorr'))];
% maxtios=[maxtios; std(max(ycorr)) mean(max(ycorr))];
% end
% std(ratios(:,1))/std(ratios(:,2))
% mean(ratios(ratios(:,2)<0.3,1))/mean(ratios(ratios(:,2)<0.3,2))
% mean(ratios(ratios(:,2)>0.3,1))/mean(ratios(ratios(:,2)>0.3,2))