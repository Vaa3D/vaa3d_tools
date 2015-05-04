function [uniquename, boutondensities, TORs, survivals, c1, c1den, c2, c2den, c3,c3den, c4,c4den,c5,c5den,c6,c6den,c7]=analyzeA2data(boutongainthres, boutonlossthres, boutonscorethres,switching,transientswitching)
% switching parameter: 1 by segment 2 by animal 3 by axon 4 by bouton
[fname, pname] =  uigetfile({'*.*'}, 'Choose analysis file to load');
[name,type,label,axonlength,b1,b2,b3,b4,b5,b6,b7,b8]=textread([pname fname],'%s%s%s%s%f%f%f%f%f%f%f%f','delimiter',',');

% assign axon names
axoncount=1;
axon=[];
for i=1:size(name,1)
    if strcmp(name(i),'break')
        axon=[axon; 0];
        axoncount=axoncount+1;
    else
        axon=[axon; axoncount];
    end
end



 boutonlength=[b1 b2 b3 b4 b5 b6 b7];
% this sets a cutoff on the length TBs need to have on the first t.p. to
%  be considered in all measurements
status=boutonlength(:,1)>boutonscorethres;
presence=[status];
new=[];
loss=[];
for j=1:size(boutonlength,2)-1
    new=[new status==0 & boutonlength(:,j+1)>boutongainthres];
    loss=[loss status==1 & boutonlength(:,j+1)<boutonlossthres];
    status=status+new(:,j)-loss(:,j);
    presence=[presence status];
end

switch transientswitching
    %this applies to all measurements. Useful for length change
    %measurements and to 
    case 1 % do nothing
        select = ones(size(new,1),1);
    case 2 % get rid of transient boutons
        select=~(sum(new,2)>0 & sum(loss,2)>0 );%& presence(:,1)==0);
    case 3 % keep only transient boutons
        select=(sum(new,2)>0 & sum(loss,2)>0);% & presence(:,1)==0;
    case 4 % keep only stable boutons. Useful for length change measurements
        select=(sum(presence,2)==7);
end
a=char(axonlength);
axonlengths=[];
for j=1:size(a,1)
    %%defines the lenght of ROIs if D is empty then Length=45 um
    if isempty(str2num(a(j,:)))
       axonlengths=[axonlengths; 45];
    else
        axonlengths=[axonlengths; str2num(a(j,:))];
    end
end
% axon length needs to be positive and bouton should be more than 1 um at some point
select=select' & (axonlengths>0)' & (max(boutonlength')>boutonscorethres); 
boutonlength=boutonlength(select,:) ;

status=boutonlength(:,1)>boutongainthres;    
new=[];
loss=[];
for j=1:size(boutonlength,2)-1
    new=[new status==0 & boutonlength(:,j+1)>boutongainthres];
    loss=[loss status==1 & boutonlength(:,j+1)<boutonlossthres];
    status=status+new(:,j)-loss(:,j);
end

id=boutonlength>boutongainthres;
% set id to 0 if bouton is always less than boutonscorethres
% enpassant=(sum(boutonlength>boutonscorethres,2)==0);
% 
% id(enpassant,:)=0;
% new(enpassant,:)=0;
% loss(enpassant,:)=0;
% status(enpassant,:)=0;


name=name(select);
type=type(select);
label=label(select);
axonlength=axonlengths(select);
switch switching
    case 1 % per segment
        name=strtok(name,'_');
        uniquename=unique(name);
        boutondensities=[];
        TORs=[];
        survivals=[];
        c1=[];
        c1a=[];
        c1den=[];
        c1aden=[];
        c2=[];
        c2den=[];
        c3=[];
        c3den=[];
        c4=[];
        c4den=[];
        c5=[];
        c5den=[];
        c6=[];
        c6den=[];
        c7=[];
        for i=1:size(uniquename,1)
            thisaxonlength=mean(axonlength(strcmp(name,uniquename(i,1)),:),1);
            peraxonid=id(strcmp(name,uniquename(i,1)),:);
            peraxonnew=new(strcmp(name,uniquename(i,1)),:);
            peraxonloss=loss(strcmp(name,uniquename(i,1)),:);
            peraxonboutonlength=boutonlength(strcmp(name,uniquename(i,1)),:);
            boutonpresence=cumsum([peraxonid(:,1) peraxonnew-peraxonloss],2);
            
            
            
            
            % Density follows the thresholds and the presence rules,i.e. 
            %present=scored if > scorethreshold on first day and >0 on other days, or if <scorethr on first day 
            %but >1 on other days, lost if < loss-thr, new if > new-thr.
            boutondensities=[boutondensities; sum(boutonpresence,1)/thisaxonlength];

            %number of boutons
            numboutons=[sum(peraxonid(:,1),1) sum(peraxonid(:,1),1)+sum(cumsum(peraxonnew',1)'-cumsum(peraxonloss',1)',1)];
            peraxonnew3=[sum(peraxonnew(:,1:2),2) sum(peraxonnew(:,3:4),2) sum(peraxonnew(:,5:6),2)];
            peraxonloss3=[sum(peraxonloss(:,1:2),2) sum(peraxonloss(:,3:4),2) sum(peraxonloss(:,5:6),2)];
            numboutons3=numboutons(:,[1 3 5 7]);
            %TOR day 0-4	day 4-8	 day 8-12	day 12-16	day 16-20	day20-24  day 0-8 (2n refers to 0d)	day 8-16 (2n refers to 8d)	day 16-24 (2n refers to 16d)	mean#
            TORs=[TORs; (sum(peraxonnew,1)+sum(peraxonloss,1))./(2*numboutons(1:end-1)) (sum(peraxonnew3,1)+sum(peraxonloss3,1))./(2*numboutons3(1:end-1)) mean(numboutons)];

            % survival fraction. Last column is the number of starting boutons which should be > boutonscorethreshold and >1 at least in one time point
            survival=[boutonpresence(boutonpresence(:,1)>0,1) -peraxonloss(boutonpresence(:,1)>0,:)];
            survival=cumsum(survival,2);
            survival(survival<0)=0;
            survival=sum(survival,1);
            survival=[survival/survival(1) survival(1)];
            survivals=[survivals; survival];



           % - % and density of new/lost before and after clipping
            % c1 [new before0-8 new after8-16 new after16-24 lost before0-8
            % lost after8-16 lost after16-24 TB#a-c TB#d-g] last two columns are total numbers
            %C1 the denominator is the sum of all TB present at each day (even if<boutonscorethres) before (a-c) or after clipping (c-g)
             %c1a [new0-4 new4-8 new8-12 new12-16 new16-20 new20-24 lost0-4 lost4-8 lost8-12 lost12-16 lost16-20 lost20-24]
            
            
            c1=[c1; sum(sum(peraxonnew(:,1:2)))/sum(sum(numboutons(:,1:3))) sum(sum(peraxonnew(:,3:6)))/sum(sum(numboutons(:,3:7))) sum(sum(peraxonloss(:,1:2)))/sum(sum(numboutons(:,1:3))) sum(sum(peraxonloss(:,3:6)))/sum(sum(numboutons(:,3:7))) sum(sum(numboutons(:,1:3))) sum(sum(numboutons(:,3:7)))  ];
            c1a=[c1a; sum(sum(peraxonnew(:,1:1)))/sum(sum(numboutons(:,1:2))) sum(sum(peraxonnew(:,2:2)))/sum(sum(numboutons(:,2:3))) sum(sum(peraxonnew(:,3:3)))/sum(sum(numboutons(:,3:4))) sum(sum(peraxonnew(:,4:4)))/sum(sum(numboutons(:,4:5))) sum(sum(peraxonnew(:,5:5)))/sum(sum(numboutons(:,5:6))) sum(sum(peraxonnew(:,6:6)))/sum(sum(numboutons(:,6:7)))  sum(sum(peraxonloss(:,1:1)))/sum(sum(numboutons(:,1:2))) sum(sum(peraxonloss(:,2:2)))/sum(sum(numboutons(:,2:3))) sum(sum(peraxonloss(:,3:3)))/sum(sum(numboutons(:,3:4))) sum(sum(peraxonloss(:,4:4)))/sum(sum(numboutons(:,4:5))) sum(sum(peraxonloss(:,5:5)))/sum(sum(numboutons(:,5:6))) sum(sum(peraxonloss(:,6:6)))/sum(sum(numboutons(:,6:7)))];
            c1den=[c1den; sum(sum(peraxonnew(:,1:2)))/thisaxonlength sum(sum(peraxonnew(:,3:6))/thisaxonlength) sum(sum(peraxonloss(:,1:2))/thisaxonlength) sum(sum(peraxonloss(:,3:6))/thisaxonlength)];
             c1aden=[c1aden; sum(sum(peraxonnew(:,1:1)))/thisaxonlength sum(sum(peraxonnew(:,2:2))/thisaxonlength) sum(sum(peraxonnew(:,3:3))/thisaxonlength) sum(sum(peraxonnew(:,4:4)))/thisaxonlength sum(sum(peraxonnew(:,5:5))/thisaxonlength) sum(sum(peraxonnew(:,6:6))/thisaxonlength)  sum(sum(peraxonloss(:,1:1))/thisaxonlength) sum(sum(peraxonloss(:,2:2))/thisaxonlength) sum(sum(peraxonloss(:,3:3))/thisaxonlength) sum(sum(peraxonloss(:,4:4))/thisaxonlength) sum(sum(peraxonloss(:,5:5))/thisaxonlength) sum(sum(peraxonloss(:,6:6))/thisaxonlength)];            
% - % and density of TBs that were stable for the first 8d and then were
            % lost. c2 [ fraction: stable then lost/not losta-b+notlostb-c
            % notlost a-b+b-c]
            c2=[c2; sum(sum(boutonpresence(:,1:3),2)==3 & sum(peraxonloss(:,3:6),2)>=1,1)/sum(sum(peraxonloss(:,1:2)==0)) sum(sum(peraxonloss(:,1:2)==0))];
            c2den=[c2den; sum(sum(boutonpresence(:,1:3),2)==3 & sum(peraxonloss(:,3:6),2)>=1,1)/thisaxonlength];

            % - % and density of TBs that came up after clipping and were stable the last 4d.
            % c3 fraction: stablized/total new boutons after clipping
            c3=[c3; sum((sum(peraxonnew(:,3:5),2)==1) & sum(peraxonloss(:,3:6),2)==0,1)/sum(sum(peraxonnew(:,3:6))) sum(sum(peraxonnew(:,3:6)))];
            c3den=[c3den; sum((sum(peraxonnew(:,3:5),2)==1) & sum(peraxonloss(:,3:6),2)==0,1)/thisaxonlength];


            % - % and density of TBs that came up before clipping and then stbilized
            % c4 fraction: stablized/total new boutons  before clipping
            c4=[c4; sum((sum(peraxonnew(:,1:2),2)==1) & sum(peraxonloss(:,1:6),2)==0,1)/sum(sum(peraxonnew(:,1:2))) sum(sum(peraxonnew(:,1:2)))];
            c4den=[c4den; sum((sum(peraxonnew(:,1:2),2)==1) & sum(peraxonloss(:,1:6),2)==0,1)/thisaxonlength];


            % - % and density of TBs that came up after clipping and then were lost
            % c5 fraction: came up then lost/total lost
            c5=[c5; sum((sum(peraxonnew(:,3:6),2)>0) & sum(peraxonloss(:,3:6),2)>0,1)/sum(sum(peraxonloss(:,3:6))) sum(sum(peraxonloss(:,3:6)))];
            c5den=[c5den; sum((sum(peraxonnew(:,3:6),2)>0) & sum(peraxonloss(:,3:6),2)>0,1)/thisaxonlength];


            % - % and density of TB that were stable for the last 8d came up after clipping.
            % c6 fraction: stable/ total came up
            c6=[c6; sum((sum(peraxonnew(:,3:4),2)==1) & sum(peraxonloss(:,3:6),2)==0,1)/sum(sum(peraxonnew(:,3:6))) sum(sum(peraxonnew(:,3:6)))];
            c6den=[c6den; sum((sum(peraxonnew(:,3:4),2)==1) & sum(peraxonloss(:,3:6),2)==0,1)/thisaxonlength];


            % - compute change in TB length before/after clipping
            % c7 [mean before mean after]

            c7=[c7; mean(abs(diff(peraxonboutonlength(:,1:7),[],2)),1)];
        end
        uniquename
        
    case 2 % per animal
        animalname=[];
        for k=1:size(name,1)
            token='([a-zA-Z]{2,5}\d{3})[a-h]{1}[_]{0,1}\d{2,3}';
            a=regexp(name(k),token,'tokens');
            b=a{1};
            %%Concatenate strings vertically
            animalname=strvcat(animalname,char(b{1}));
        end
        axonname=strtok(name,'_');
        name=cellstr(animalname);
        uniquename=unique(name);
        boutondensities=[];
        TORs=[];
        survivals=[];
        c1=[];
        c1a=[];
        c1den=[];
        c1aden=[];
        c2=[];
        c2den=[];
        c3=[];
        c3den=[];
        c4=[];
        c4den=[];
        c5=[];
        c5den=[];
        c6=[];
        c6den=[];
        c7=[];
        for i=1:size(uniquename,1)
            uniqueaxonname=unique(axonname(strcmp(name,uniquename(i,1))));
            totalaxonlength=0;
            for k=1:size(uniqueaxonname,1)
                % total length is given by the sum of the mean of the
                % different ROIs 
                totalaxonlength=totalaxonlength+mean(axonlength(strcmp(axonname,uniqueaxonname(k,1)),:),1);
            end
            peraxonid=id(strcmp(name,uniquename(i,1)),:);
            peraxonnew=new(strcmp(name,uniquename(i,1)),:);
            peraxonloss=loss(strcmp(name,uniquename(i,1)),:);
            peraxonboutonlength=boutonlength(strcmp(name,uniquename(i,1)),:);
            boutonpresence=cumsum([peraxonid(:,1) peraxonnew-peraxonloss],2);
            boutondensities=[boutondensities; sum(boutonpresence,1)/totalaxonlength];

            %number of boutons
            numboutons=[sum(peraxonid(:,1),1) sum(peraxonid(:,1),1)+sum(cumsum(peraxonnew',1)'-cumsum(peraxonloss',1)',1)];
            peraxonnew3=[sum(peraxonnew(:,1:2),2) sum(peraxonnew(:,3:4),2) sum(peraxonnew(:,5:6),2)];
            peraxonloss3=[sum(peraxonloss(:,1:2),2) sum(peraxonloss(:,3:4),2) sum(peraxonloss(:,5:6),2)];
            numboutons3=numboutons(:,[1 3 5 7]);
            TORs=[TORs; (sum(peraxonnew,1)+sum(peraxonloss,1))./(2*numboutons(1:end-1)) (sum(peraxonnew3,1)+sum(peraxonloss3,1))./(2*numboutons3(1:end-1)) mean(numboutons)];

           % survival fraction. Last column is the number of starting boutons which should be > boutonscorethreshold and >1 at least in one time point
            survival=[boutonpresence(boutonpresence(:,1)>0,1) -peraxonloss(boutonpresence(:,1)>0,:)];
            survival=cumsum(survival,2);
            survival(survival<0)=0;
            survival=sum(survival,1);
            survival=[survival/survival(1) survival(1)];
            survivals=[survivals; survival];

           % - % and density of new/lost before and after clipping
            % c1 [new0-8 new8-24 lost0-8 lost8-24 TB#a-c TB#c-g]% last two columns are total numbers]
            % %C1 the denominator is the sum of all TB present at each day (even if<boutonscorethres) before (a-c) or after clipping (c-g)
             %c1a [new0-4 new4-8 new8-12 new12-16 new16-20 new20-24 lost0-4 lost4-8 lost8-12 lost12-16 lost16-20 lost20-24]
            %c1den [new before0-8 new after8-16 new after16-24 lost before0-8 lost after8-16 lost after16-24]
            c1=[c1; sum(sum(peraxonnew(:,1:2)))/sum(sum(numboutons(:,1:3)))  sum(sum(peraxonnew(:,3:4)))/sum(sum(numboutons(:,3:5))) sum(sum(peraxonnew(:,5:6)))/sum(sum(numboutons(:,5:7))) sum(sum(peraxonloss(:,1:2)))/sum(sum(numboutons(:,1:3))) sum(sum(peraxonloss(:,3:4)))/sum(sum(numboutons(:,3:5))) sum(sum(peraxonloss(:,5:6)))/sum(sum(numboutons(:,5:7))) sum(sum(numboutons(:,1:3))) sum(sum(numboutons(:,3:5))) sum(sum(numboutons(:,5:7)))  ];
            c1a=[c1a; sum(sum(peraxonnew(:,1:1)))/sum(sum(numboutons(:,1:2))) sum(sum(peraxonnew(:,2:2)))/sum(sum(numboutons(:,2:3))) sum(sum(peraxonnew(:,3:3)))/sum(sum(numboutons(:,3:4))) sum(sum(peraxonnew(:,4:4)))/sum(sum(numboutons(:,4:5))) sum(sum(peraxonnew(:,5:5)))/sum(sum(numboutons(:,5:6))) sum(sum(peraxonnew(:,6:6)))/sum(sum(numboutons(:,6:7)))  sum(sum(peraxonloss(:,1:1)))/sum(sum(numboutons(:,1:2))) sum(sum(peraxonloss(:,2:2)))/sum(sum(numboutons(:,2:3))) sum(sum(peraxonloss(:,3:3)))/sum(sum(numboutons(:,3:4))) sum(sum(peraxonloss(:,4:4)))/sum(sum(numboutons(:,4:5))) sum(sum(peraxonloss(:,5:5)))/sum(sum(numboutons(:,5:6))) sum(sum(peraxonloss(:,6:6)))/sum(sum(numboutons(:,6:7)))];
            c1den=[c1den; sum(sum(peraxonnew(:,1:2)))/totalaxonlength sum(sum(peraxonnew(:,3:4)))/totalaxonlength sum(sum(peraxonnew(:,5:6)))/totalaxonlength sum(sum(peraxonloss(:,1:2)))/totalaxonlength sum(sum(peraxonloss(:,3:4)))/totalaxonlength sum(sum(peraxonloss(:,5:6)))/totalaxonlength];
            c1aden=[c1aden; sum(sum(peraxonnew(:,1:1)))/totalaxonlength sum(sum(peraxonnew(:,2:2))/totalaxonlength) sum(sum(peraxonnew(:,3:3))/totalaxonlength) sum(sum(peraxonnew(:,4:4)))/totalaxonlength sum(sum(peraxonnew(:,5:5))/totalaxonlength) sum(sum(peraxonnew(:,6:6))/totalaxonlength)  sum(sum(peraxonloss(:,1:1))/totalaxonlength) sum(sum(peraxonloss(:,2:2))/totalaxonlength) sum(sum(peraxonloss(:,3:3))/totalaxonlength) sum(sum(peraxonloss(:,4:4))/totalaxonlength) sum(sum(peraxonloss(:,5:5))/totalaxonlength) sum(sum(peraxonloss(:,6:6))/totalaxonlength)];
             % - % and density of TBs that were stable for the first 8d and then were
            % lost. c2 [ fraction: stable then lost/not losta-b+notlostb-c
            % notlost a-b+b-c]
            c2=[c2; sum(sum(boutonpresence(:,1:3),2)==3 &  sum(peraxonloss(:,3:6),2)>=1,1)/sum(sum(peraxonloss(:,1:2)==0))  sum(sum(peraxonloss(:,1:2)==0))];
            c2den=[c2den; sum(sum(boutonpresence(:,1:3),2)==3 &  sum(peraxonloss(:,3:6),2)>=1,1)/totalaxonlength];
            
             % - % and density of TBs that came up after clipping and were stable the last 4d.
            % c3 fraction: stablized/total new boutons after clipping
            c3=[c3; sum((sum(peraxonnew(:,3:5),2)==1) & sum(peraxonloss(:,3:6),2)==0,1)/sum(sum(peraxonnew(:,3:6))) sum(sum(peraxonnew(:,3:6)))];
            c3den=[c3den; sum((sum(peraxonnew(:,3:5),2)==1) & sum(peraxonloss(:,3:6),2)==0,1)/totalaxonlength];

            % - % and density of TBs that came up before clipping and then stbilized
            % c4 fraction: stablized/total new boutons  before clipping
            c4=[c4; sum((sum(peraxonnew(:,1:2),2)==1) & sum(peraxonloss(:,1:6),2)==0,1)/sum(sum(peraxonnew(:,1:2))) sum(sum(peraxonnew(:,1:2)))];
            c4den=[c4den; sum((sum(peraxonnew(:,1:2),2)==1) & sum(peraxonloss(:,1:6),2)==0,1)/totalaxonlength];


            % - % and density of TBs that came up after clipping and then were lost
            % c5 fraction: came up then lost/total lost
            c5=[c5; sum((sum(peraxonnew(:,3:6),2)>0) & sum(peraxonloss(:,3:6),2)>0,1)/sum(sum(peraxonloss(:,3:6))) sum(sum(peraxonloss(:,3:6)))];
            c5den=[c5den; sum((sum(peraxonnew(:,3:6),2)>0) & sum(peraxonloss(:,3:6),2)>0,1)/totalaxonlength];


            % - % and density of TB that were stable for the last 8d came up after clipping.
            % c6 fraction: stable/ total came up
             c6=[c6; sum((sum(peraxonnew(:,3:4),2)==1) & sum(peraxonloss(:,3:6),2)==0,1)/sum(sum(peraxonnew(:,3:6))) sum(sum(peraxonnew(:,3:6)))];
            c6den=[c6den; sum((sum(peraxonnew(:,3:4),2)==1) & sum(peraxonloss(:,3:6),2)==0,1)/totalaxonlength];

            % - compute change in TB length before/after clipping
            % c7 [mean before mean after]

            c7=[c7; mean(abs(diff(peraxonboutonlength(:,1:7),[],2)),1)];
        end
        uniquename
        
        
    case 3 % per axon
        axonname=strtok(name,'_');

        name=cellstr(num2str(axon(select)));
        name=strtok(name,'_');
        uniquename=unique(name);
        boutondensities=[];
        TORs=[];
        survivals=[];
        c1=[];
        c1a=[];
        c1den=[];
        c1aden=[];
        c2=[];
        c2den=[];
        c3=[];
        c3den=[];
        c4=[];
        c4den=[];
        c5=[];
        c5den=[];
        c6=[];
        c6den=[];
        c7=[];
        for i=1:size(uniquename,1)
            uniqueaxonname=unique(axonname(strcmp(name,uniquename(i,1))));
            totalaxonlength=0;
            for k=1:size(uniqueaxonname,1)
                % total length is given by the sum of the mean of the
                % lengths of the different ROIs 
                totalaxonlength=totalaxonlength+mean(axonlength(strcmp(axonname,uniqueaxonname(k,1)),:),1);
            end
            peraxonid=id(strcmp(name,uniquename(i,1)),:);
            peraxonnew=new(strcmp(name,uniquename(i,1)),:);
            peraxonloss=loss(strcmp(name,uniquename(i,1)),:);
            peraxonboutonlength=boutonlength(strcmp(name,uniquename(i,1)),:);
            boutonpresence=cumsum([peraxonid(:,1) peraxonnew-peraxonloss],2);
            boutondensities=[boutondensities; sum(boutonpresence,1)/totalaxonlength];


            %number of boutons
            numboutons=[sum(peraxonid(:,1),1) sum(peraxonid(:,1),1)+sum(cumsum(peraxonnew',1)'-cumsum(peraxonloss',1)',1)];
            peraxonnew3=[sum(peraxonnew(:,1:2),2) sum(peraxonnew(:,3:4),2) sum(peraxonnew(:,5:6),2)];
            peraxonloss3=[sum(peraxonloss(:,1:2),2) sum(peraxonloss(:,3:4),2) sum(peraxonloss(:,5:6),2)];
            numboutons3=numboutons(:,[1 3 5 7]);
            TORs=[TORs; (sum(peraxonnew,1)+sum(peraxonloss,1))./(2*numboutons(1:end-1)) (sum(peraxonnew3,1)+sum(peraxonloss3,1))./(2*numboutons3(1:end-1)) mean(numboutons)];

            % survival fraction. Last column is the number of starting boutons which should be > boutonscorethreshold and >1 at least in one time point
            survival=[boutonpresence(boutonpresence(:,1)>0,1) -peraxonloss(boutonpresence(:,1)>0,:)];
            survival=cumsum(survival,2);
            survival(survival<0)=0;
            survival=sum(survival,1);
            survival=[survival/survival(1) survival(1)];
            survivals=[survivals; survival];

            % - % and density of new/lost before and after clipping
            % c1 [new0-8 new8-24 lost0-8 lost8-24 TB#a-c TB#c-g]% last two columns are total numbers]
            % %C1 the denominator is the sum of all TB present at each day (even if<boutonscorethres) before (a-c) or after clipping (c-g)
             %c1a [new0-4 new4-8 new8-12 new12-16 new16-20 new20-24 lost0-4 lost4-8 lost8-12 lost12-16 lost16-20 lost20-24]
            %c1den [new before0-8 new after8-16 new after16-24 lost before0-8 lost after8-16 lost after16-24]
            c1=[c1; sum(sum(peraxonnew(:,1:2)))/sum(sum(numboutons(:,1:3))) sum(sum(peraxonnew(:,3:6)))/sum(sum(numboutons(:,3:7))) sum(sum(peraxonloss(:,1:2)))/sum(sum(numboutons(:,1:3))) sum(sum(peraxonloss(:,3:6)))/sum(sum(numboutons(:,3:7))) sum(sum(numboutons(:,1:3))) sum(sum(numboutons(:,3:7)))  ];
            c1a=[c1a; sum(sum(peraxonnew(:,1:1)))/sum(sum(numboutons(:,1:2))) sum(sum(peraxonnew(:,2:2)))/sum(sum(numboutons(:,2:3))) sum(sum(peraxonnew(:,3:3)))/sum(sum(numboutons(:,3:4))) sum(sum(peraxonnew(:,4:4)))/sum(sum(numboutons(:,4:5))) sum(sum(peraxonnew(:,5:5)))/sum(sum(numboutons(:,5:6))) sum(sum(peraxonnew(:,6:6)))/sum(sum(numboutons(:,6:7)))  sum(sum(peraxonloss(:,1:1)))/sum(sum(numboutons(:,1:2))) sum(sum(peraxonloss(:,2:2)))/sum(sum(numboutons(:,2:3))) sum(sum(peraxonloss(:,3:3)))/sum(sum(numboutons(:,3:4))) sum(sum(peraxonloss(:,4:4)))/sum(sum(numboutons(:,4:5))) sum(sum(peraxonloss(:,5:5)))/sum(sum(numboutons(:,5:6))) sum(sum(peraxonloss(:,6:6)))/sum(sum(numboutons(:,6:7)))];
            c1den=[c1den; sum(sum(peraxonnew(:,1:2)))/totalaxonlength sum(sum(peraxonnew(:,3:4)))/totalaxonlength sum(sum(peraxonnew(:,5:6)))/totalaxonlength sum(sum(peraxonloss(:,1:2)))/totalaxonlength sum(sum(peraxonloss(:,3:4)))/totalaxonlength sum(sum(peraxonloss(:,5:6)))/totalaxonlength];
            c1aden=[c1aden; sum(sum(peraxonnew(:,1:1)))/totalaxonlength sum(sum(peraxonnew(:,2:2))/totalaxonlength) sum(sum(peraxonnew(:,3:3))/totalaxonlength) sum(sum(peraxonnew(:,4:4)))/totalaxonlength sum(sum(peraxonnew(:,5:5))/totalaxonlength) sum(sum(peraxonnew(:,6:6))/totalaxonlength)  sum(sum(peraxonloss(:,1:1))/totalaxonlength) sum(sum(peraxonloss(:,2:2))/totalaxonlength) sum(sum(peraxonloss(:,3:3))/totalaxonlength) sum(sum(peraxonloss(:,4:4))/totalaxonlength) sum(sum(peraxonloss(:,5:5))/totalaxonlength) sum(sum(peraxonloss(:,6:6))/totalaxonlength)];
            % - % and density of TBs that were stable for the first 8d and then were lost. 
            %c2 [ fraction: stable then lost/not losta-b+notlostb-c% notlost a-b+b-c]
            c2=[c2; sum(sum(boutonpresence(:,1:3),2)==3 &  sum(peraxonloss(:,3:6),2)>=1,1)/sum(sum(peraxonloss(:,1:2)==0)) sum(sum(peraxonloss(:,1:2)==0))];
            c2den=[c2den; sum(sum(boutonpresence(:,1:3),2)==3 &  sum(peraxonloss(:,3:6),2)>=1,1)/totalaxonlength];

            % - % and density of TBs that came up after clipping and were stable the last 4d.
            % c3 fraction: stablized/total new boutons after clipping
            c3=[c3; sum((sum(peraxonnew(:,3:5),2)==1) & sum(peraxonloss(:,3:6),2)==0,1)/sum(sum(peraxonnew(:,3:6))) sum(sum(peraxonnew(:,3:6)))];
            c3den=[c3den; sum((sum(peraxonnew(:,3:5),2)==1) & sum(peraxonloss(:,3:6),2)==0,1)/totalaxonlength];


            % - % and density of TBs that came up before clipping and then stbilized
            % c4 fraction: stablized/total new boutons  before clipping
            c4=[c4; sum((sum(peraxonnew(:,1:2),2)==1) & sum(peraxonloss(:,1:6),2)==0,1)/sum(sum(peraxonnew(:,1:2))) sum(sum(peraxonnew(:,1:2)))];
            c4den=[c4den; sum((sum(peraxonnew(:,1:2),2)==1) & sum(peraxonloss(:,1:6),2)==0,1)/totalaxonlength];


            % - % and density of TBs that came up after clipping and then were lost
            % c5 fraction: came up then lost/total lost
            c5=[c5; sum((sum(peraxonnew(:,3:6),2)>0) & sum(peraxonloss(:,3:6),2)>0,1)/sum(sum(peraxonloss(:,3:6))) sum(sum(peraxonloss(:,3:6)))];
            c5den=[c5den; sum((sum(peraxonnew(:,3:6),2)>0) & sum(peraxonloss(:,3:6),2)>0,1)/totalaxonlength];


            % - % and density of TB that were stable for the last 8d came up after clipping.
            % c6 fraction: stable/ total came up
            c6=[c6; sum((sum(peraxonnew(:,3:4),2)==1) & sum(peraxonloss(:,3:6),2)==0,1)/sum(sum(peraxonnew(:,3:6))) sum(sum(peraxonnew(:,3:6)))];
            c6den=[c6den; sum((sum(peraxonnew(:,3:4),2)==1) & sum(peraxonloss(:,3:6),2)==0,1)/totalaxonlength];
            
            % - compute change in TB length before/after clipping
            % c7 [mean before mean after]

            c7=[c7; mean(abs(diff(peraxonboutonlength(:,1:7),[],2)),1)];
        end
        uniquename
        
        
    case 4 % all boutons
        axonname=strtok(name,'_');

        name=cellstr(num2str(ones(size(name,1),1)));
        uniquename=unique(name);
        boutondensities=[];
        TORs=[];
        survivals=[];
        c1=[];
        c1a=[];
        c1den=[];
        c1aden=[];
        c2=[];
        c2den=[];
        c3=[];
        c3den=[];
        c4=[];
        c4den=[];
        c5=[];
        c5den=[];
        c6=[];
        c6den=[];
        c7=[];
        for i=1:size(uniquename,1)
            uniqueaxonname=unique(axonname(strcmp(name,uniquename(i,1))));
            totalaxonlength=0;
            for k=1:size(uniqueaxonname,1)
                totalaxonlength=totalaxonlength+mean(axonlength(strcmp(axonname,uniqueaxonname(k,1)),:),1);
            end
            peraxonid=id(strcmp(name,uniquename(i,1)),:);
            peraxonnew=new(strcmp(name,uniquename(i,1)),:);
            peraxonloss=loss(strcmp(name,uniquename(i,1)),:);
            peraxonboutonlength=boutonlength(strcmp(name,uniquename(i,1)),:);
            boutonpresence=cumsum([peraxonid(:,1) peraxonnew-peraxonloss],2);
            boutondensities=[boutondensities; sum(boutonpresence,1)/totalaxonlength];

            %number of boutons
            numboutons=[sum(peraxonid(:,1),1) sum(peraxonid(:,1),1)+sum(cumsum(peraxonnew',1)'-cumsum(peraxonloss',1)',1)];
            peraxonnew3=[sum(peraxonnew(:,1:2),2) sum(peraxonnew(:,3:4),2) sum(peraxonnew(:,5:6),2)];
            peraxonloss3=[sum(peraxonloss(:,1:2),2) sum(peraxonloss(:,3:4),2) sum(peraxonloss(:,5:6),2)];
            numboutons3=numboutons(:,[1 3 5 7]);
            TORs=[TORs; (sum(peraxonnew,1)+sum(peraxonloss,1))./(2*numboutons(1:end-1)) (sum(peraxonnew3,1)+sum(peraxonloss3,1))./(2*numboutons3(1:end-1)) mean(numboutons)];

           % survival fraction. Last column is the number of starting boutons which should be > boutonscorethreshold and >1 at least in one time point
            survival=[boutonpresence(boutonpresence(:,1)>0,1) -peraxonloss(boutonpresence(:,1)>0,:)];
            survival=cumsum(survival,2);
            survival(survival<0)=0;
            survival=sum(survival,1);
            survival=[survival/survival(1) survival(1)];
            survivals=[survivals; survival];


          % - % and density of new/lost before and after clipping
            % c1 [new before0-8 new after8-16 new after16-24 lost before0-8 lost after8-16 lost after16-24 TB#a-c TB#d-g]
            % %C1 the denominator is the sum of all TB present at each day (even if<boutonscorethres) before (a-c) or after clipping (c-g)
             %c1a [new0-4 new4-8 new8-12 new12-16 new16-20 new20-24 lost0-4 lost4-8 lost8-12 lost12-16 lost16-20 lost20-24]
            % last two columns are total numbers
            c1=[c1; sum(sum(peraxonnew(:,1:2)))/sum(sum(numboutons(:,1:3))) sum(sum(peraxonnew(:,3:6)))/sum(sum(numboutons(:,3:7))) sum(sum(peraxonloss(:,1:2)))/sum(sum(numboutons(:,1:3))) sum(sum(peraxonloss(:,3:6)))/sum(sum(numboutons(:,3:7))) sum(sum(numboutons(:,1:3))) sum(sum(numboutons(:,3:7)))  ];
            c1a=[c1a; sum(sum(peraxonnew(:,1:1)))/sum(sum(numboutons(:,1:2))) sum(sum(peraxonnew(:,2:2)))/sum(sum(numboutons(:,2:3))) sum(sum(peraxonnew(:,3:3)))/sum(sum(numboutons(:,3:4))) sum(sum(peraxonnew(:,4:4)))/sum(sum(numboutons(:,4:5))) sum(sum(peraxonnew(:,5:5)))/sum(sum(numboutons(:,5:6))) sum(sum(peraxonnew(:,6:6)))/sum(sum(numboutons(:,6:7)))  sum(sum(peraxonloss(:,1:1)))/sum(sum(numboutons(:,1:2))) sum(sum(peraxonloss(:,2:2)))/sum(sum(numboutons(:,2:3))) sum(sum(peraxonloss(:,3:3)))/sum(sum(numboutons(:,3:4))) sum(sum(peraxonloss(:,4:4)))/sum(sum(numboutons(:,4:5))) sum(sum(peraxonloss(:,5:5)))/sum(sum(numboutons(:,5:6))) sum(sum(peraxonloss(:,6:6)))/sum(sum(numboutons(:,6:7)))];
            c1den=[c1den; sum(sum(peraxonnew(:,1:2)))/totalaxonlength sum(sum(peraxonnew(:,3:4)))/totalaxonlength sum(sum(peraxonnew(:,5:6)))/totalaxonlength sum(sum(peraxonloss(:,1:2)))/totalaxonlength sum(sum(peraxonloss(:,3:4)))/totalaxonlength sum(sum(peraxonloss(:,5:6)))/totalaxonlength];
            c1aden=[c1aden; sum(sum(peraxonnew(:,1:1)))/totalaxonlength sum(sum(peraxonnew(:,2:2))/totalaxonlength) sum(sum(peraxonnew(:,3:3))/totalaxonlength) sum(sum(peraxonnew(:,4:4)))/totalaxonlength sum(sum(peraxonnew(:,5:5))/totalaxonlength) sum(sum(peraxonnew(:,6:6))/totalaxonlength)  sum(sum(peraxonloss(:,1:1))/totalaxonlength) sum(sum(peraxonloss(:,2:2))/totalaxonlength) sum(sum(peraxonloss(:,3:3))/totalaxonlength) sum(sum(peraxonloss(:,4:4))/totalaxonlength) sum(sum(peraxonloss(:,5:5))/totalaxonlength) sum(sum(peraxonloss(:,6:6))/totalaxonlength)];
            % - % and density of TBs that were stable for the first 8d and then were
            % lost. c2 [ fraction: stable then lost/not losta-b+notlostb-c  notlost a-b+b-c]
            c2=[c2; sum(sum(boutonpresence(:,1:3),2)==3 &  sum(peraxonloss(:,3:6),2)>=1,1)/sum(sum(peraxonloss(:,1:2)==0)) sum(sum(peraxonloss(:,1:2)==0))];
            c2den=[c2den; sum(sum(boutonpresence(:,1:3),2)==3 &  sum(peraxonloss(:,3:6),2)>=1,1)/totalaxonlength];

            % - % and density of TBs that came up after clipping and were stable the last 4d.
            % c3 fraction: stablized/total new boutons after clipping
            c3=[c3; sum((sum(peraxonnew(:,3:5),2)==1) & sum(peraxonloss(:,3:6),2)==0,1)/sum(sum(peraxonnew(:,3:6))) sum(sum(peraxonnew(:,3:6)))];
            c3den=[c3den; sum((sum(peraxonnew(:,3:5),2)==1) & sum(peraxonloss(:,3:6),2)==0,1)/totalaxonlength];


            % - % and density of TBs that came up before clipping and then stbilized
            % c4 fraction: stablized/total new boutons  before clipping
            c4=[c4; sum((sum(peraxonnew(:,1:2),2)==1) & sum(peraxonloss(:,1:6),2)==0,1)/sum(sum(peraxonnew(:,1:2))) sum(sum(peraxonnew(:,1:2)))];
            c4den=[c4den; sum((sum(peraxonnew(:,1:2),2)==1) & sum(peraxonloss(:,1:6),2)==0,1)/totalaxonlength];


            % - % and density of TBs that came up after clipping and then were lost
            % c5 fraction: came up then lost/total lost
            c5=[c5; sum((sum(peraxonnew(:,3:6),2)>0) & sum(peraxonloss(:,3:6),2)>0,1)/sum(sum(peraxonloss(:,3:6))) sum(sum(peraxonloss(:,3:6)))];
            c5den=[c5den; sum((sum(peraxonnew(:,3:6),2)>0) & sum(peraxonloss(:,3:6),2)>0,1)/totalaxonlength];


            % - % and density of TB that were stable for the last 8d came up after clipping.
            % c6 fraction: stable/ total came up
             c6=[c6; sum((sum(peraxonnew(:,3:4),2)==1) & sum(peraxonloss(:,3:6),2)==0,1)/sum(sum(peraxonnew(:,3:6))) sum(sum(peraxonnew(:,3:6)))];
            c6den=[c6den; sum((sum(peraxonnew(:,3:4),2)==1) & sum(peraxonloss(:,3:6),2)==0,1)/totalaxonlength];
            
            % - compute change in TB length before/after clipping
            % c7 [mean before mean after]

            c7=[c7; mean(abs(diff(peraxonboutonlength(:,1:7),[],2)),1)];
        end
        uniquename
end
display('boutondensities');
printmatrix(boutondensities);
TORs(isnan(TORs))=0;
TORs(isinf(TORs))=0;
display('TORs');
printmatrix(TORs);
c1(isnan(c1))=0;
display('c1');
printmatrix(c1);
c1a(isnan(c1))=0;
display('c1a');
printmatrix(c1a);
display('c1den');
printmatrix(c1den);
display('c1aden');
printmatrix(c1aden);
c2(isnan(c2))=0;
display('c2');
printmatrix(c2);
c2den
c3(isnan(c3))=0;
display('c3');
printmatrix(c3);
c3den
c4(isnan(c4))=0;
display('c4');
printmatrix(c4);
c4den
c5(isnan(c5))=0;
display('c5');
printmatrix(c5);
c5den
c6(isnan(c6))=0;
display('c6');
printmatrix(c6);
c6den
display('c7');
printmatrix(c7);
display('survival');
printmatrix(survivals);
