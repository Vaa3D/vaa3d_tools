function d=distdendis(XI,XJ)
for i=1:size(XJ,1)
    if sum((XI(1,:)>0).*(XJ(i,:)>0)) % same day
        d(i,1)=1000;
    else
        d(i,1)=abs(diff([sum(XI(1,:)) sum(XJ(i,:))]));
    end
end