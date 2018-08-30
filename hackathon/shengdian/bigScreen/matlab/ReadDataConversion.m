function out=ReadDataConversion(filename)
%filename='seu_allen_dataconversion.xlsx';
Data=readtable(filename);
% brainstate='BrainState';
% readyonserver='ReadyOnServer';
% [brainNumbers,~]=size(Data);
% brainNumbers=brainNumbers-2;
FinshedNumbers=sum(strcmp(Data.BrainState,'Finished'));
NeedToConvertNumbers=sum(strcmp(Data.BrainState,'NeedToConvert'));
ConvertingNumbers=sum(strcmp(Data.BrainState,'Converting'));
StopNumbers=sum(strcmp(Data.BrainState,'Stop'));
ReadyOnServerNumbers=sum(strcmp(Data.ReadyOnServer,'yes'));
number=[FinshedNumbers,NeedToConvertNumbers,ConvertingNumbers,StopNumbers,ReadyOnServerNumbers];
% Flabel=['Finished',' : ',num2str(FinshedNumbers)];
% Nlabel=['NeedToConvert',' : ',num2str(NeedToConvertNumbers)]; 
% Clabel=['Converting',' : ',num2str(ConvertingNumbers)];
% Slabel=['Stop',' : ',num2str(StopNumbers)];
Flabel=['Finished'];
Nlabel=['NeedToConvert']; 
Clabel=['Converting'];
Slabel=['Stop'];
Rlabel=['ReadyOnServer'];
labels={Flabel,Nlabel,Clabel,Slabel,Rlabel};
figure
bar(number);
for i=1:size(labels,2)
    ypos=number(i);
    if i==4
        text(i-0.2,ypos+1,labels{i},'color','red','FontSize',12);   
    else
         text(i-0.5,ypos+1,labels{i},'color','red','FontSize',12);
    end    
end
axis([0 size(labels,2)+1 0 max(number)+5]);
%xlabel('','FontSize',16)
ylabel('Brains','FontSize',16)
title('Data Conversion','color','red','FontSize',16);
out=1;
end