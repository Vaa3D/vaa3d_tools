%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Generate plots
%
%%%%%%%%%%%%%%%%%%%%%%%%%%
figure;
load -mat d:\axons\newA3.db
IDs=[ 28 29 30 32 33 34 35];
summariescn=db.summaries(IDs);
generatebigboutonplot(summariescn,'r');

figure; % new boutons
title('all Type 3 big bouton');

load -mat d:\axons\newA3.db
IDs=[20 21 22 23 24 25 27 1 2 4 5 6 7 8 9 11 12 13 14];%3 a bit screwy big bouton disappear on day c but value is still a bit up from nearby bouton, so estimation of value is strange
%IDs=[15 16 17 18];
summariescn=db.summaries(IDs);
%generatebigboutonplot(summariescn,'r');
for j=1:size(summariescn,2)
    summariescn(j).type=0;
    summariescn(j).ratios2=[];
    summariescn(j).variationsbig=0;
    summariescn(j).ratiosbig=0;
end






%figure;
%title('all Type 3 big bouton');

load -mat d:\axons\control.db
IDs=[2 4 5 6 8 9 10 11 15 17 20 21];
summariesc=db.summaries(IDs);
for j=1:size(summariesc,2)
    summariesc(j).type=0;
    summariesc(j).ratios2=[];
    summariesc(j).variationsbig=0;
    summariesc(j).ratiosbig=0;
end
newsummariescn=[];
fields=fieldnames(summariesc(1));
for i=1:size(summariescn,2)
    for j=1:size(fields,1)
        newsummariescn(i).(char(fields(j)))=summariescn(i).(char(fields(j)));
    end
end
generatebigboutonplot([newsummariescn summariesc],'r');
global perboutona perboutonvara perboutonmeana bigida namea perROIa perROIvara namesa perROImeana;
perboutonc=perboutona;
perboutonvarc=perboutonvara;
perboutonmeanc=perboutonmeana;
bigidc=bigida;
namec=char(namea);

perROIc=perROIa;
perROIvarc=perROIvara;
perROImeanc=perROImeana;
nameROIc=char(namesa);


%figure;
%generateallboutonplot([newsummariescn summariesc],'r');

% 
% load -mat d:\axons\lwah016.db
% % Axon 10, outside of barrel cortex
% 
% IDs=[20 21 22 23 25 27];
% 
% summaries=db.summaries(IDs);
% generatebigboutonplot([summariesc summaries],'b');


% figure; % new boutons
% title('all Type 3 big bouton');
%figure;
load -mat d:\axons\newA3.db
%IDs=[1 2 3 4 5 6 7 8 9 11 12 13 14];
IDs=[28 29 30 31 32 33 34 35 15 16 17 18 19];
%IDs=[18 19 28 30 31 32 33 34 35]; %15 16 17 29 big difference in mean intensity
summariescn=db.summaries(IDs);
%generatebigboutonplot(summariescn,'r');
for j=1:size(summariescn,2)
    summariescn(j).type=0;
    summariescn(j).ratios2=[];
    summariescn(j).variationsbig=0;
    summariescn(j).ratiosbig=0;
end
%generatebigboutonplot([newsummariescn],'b');

load -mat d:\axons\lwah016.db


IDsold=[39 48 35 46 16 18];
IDsnew=[43 42 44 45 50 55];
IDsnotsure=[29 51 30 14 10 11];
IDs=[IDsnew IDsold IDsnotsure];
summaries=db.summaries(IDs);
newsummariescn=[];
fields=fieldnames(summaries(1));
for i=1:size(summariescn,2)
    for j=1:size(fields,1)
        newsummariescn(i).(char(fields(j)))=summariescn(i).(char(fields(j)));
    end
end
generatebigboutonplot([summaries newsummariescn],'b');
global perboutona perboutonvara perboutonmeana bigida namea perROIa perROIvara namesa perROImeana;

perboutond=perboutona;
perboutonvard=perboutonvara;
perboutonmeand=perboutonmeana;
bigidd=bigida;
named=char(namea);

perROId=perROIa;
perROIvard=perROIvara;
perROImeand=perROImeana;
nameROId=char(namesa);


IDs=[IDsnew IDsold];
summaries=db.summaries(IDs);
%generatebigboutonplot([newsummariescn summaries],'g');

%per animal control


%
subplot(4,2,2);
legend('blue-deprived','green-out of barrel','red-control');

summaryd=[bigidd' perboutond' perboutonvard' perboutonmeand'];
summaryROId=[perROId' perROIvard' perROImeand'];
summaryc=[bigidc' perboutonc' perboutonvarc' perboutonmeanc'];
summaryROIc=[perROIc' perROIvarc' perROImeanc'];

save -ascii -tabs controlname.mat namec;
save -ascii -tabs summarycontrol.mat summaryc;

save -ascii -tabs controlROIname.mat nameROIc;
save -ascii -tabs summaryROIcontrol.mat summaryROIc;

save -ascii -tabs deprivedname.mat named;
save -ascii -tabs summarydeprived.mat summaryd;

save -ascii -tabs deprivedROIname.mat nameROId;
save -ascii -tabs summaryROIdeprived.mat summaryROId;


% stretch factor overdays

factor=[];
names=[];
seriesuids=unique([summaries.seriesuid]);
for i=1:size(seriesuids,2)
    dendrites=db.series([db.series.uid]==seriesuids(i)).dendrites;
    for j=1:size(dendrites,1)
        dayfactor=[db.dendrites(dendrites(j,:)).stretch];
        factor=[factor;exp(abs(diff(log(dayfactor))))];
        names=[names;summaries(i).names(1)];
    end
end
        
















% plots of individual segments
figure;
for i=1:8
    h=plot(mean(perboutonmeand(:,seg==i)'));
    c=hsv2rgb([(i+1)/(8+1) 1 1]);
    hold on;
    set(h,'Color',c);
end
figure;
for i=1:8
    h=plot(mean(perboutond(:,seg==i)'));
    c=hsv2rgb([(i+1)/(8+1) 1 1]);
    hold on;
    set(h,'Color',c);
end

return;
figure;
title('all Type 3');

load -mat d:\axons\control.db
IDs=[2 4 5 6 8 9 10 11 15 17 20 21];
summariesc=db.summaries(IDs);
%generateallboutonplot(summaries,'r');
for j=1:size(summariesc,2)
    summariesc(j).type=0;
    summariesc(j).ratios2=0;
    summariesc(j).variationsbig=0;
    summariesc(j).ratiosbig=0;
end

load -mat d:\axons\lwah016.db
% Axon 10, outside of barrel cortex

IDs=[20 21 22 23 25 27];

summaries=db.summaries(IDs);
generateallboutonplot([summariesc summaries],'b');

IDsold=[39 48 35]; %16 18 46
IDsnew=[43 42 44 45 50 65];
IDsnotsure=[10 11 29 30 51 14];
IDs=[IDsold IDsnew];
summaries=db.summaries(IDs);
generateallboutonplot(summaries,'r');
subplot(4,2,2);
legend('blue-deprived','green-out of barrel','red-control');








IDsold=[46 16 18 39 48 35];
IDsnew=[43 42 44 45 50 ];
IDsnotsure=[29 30];
IDs=[IDsnew];
summaries=db.summaries(IDs);
generatebigboutonplot(summaries,'g');







IDsold=[46 16 18 39 48 35];
IDsnew=[43 42 44 45 50 ];
IDsnotsure=[10 11 29 30 51 14];
IDs=[IDsnotsure];
summaries=db.summaries(IDs);
generatebigboutonplot(summaries,'r');

title('Partial Type 3');
IDsold=[46 16 18 39 48 35];
IDsnew=[43 42 44 45 50 ];
IDsnotsure=[29 30 51 10 11 14];
IDs=[IDsold];
summaries=db.summaries(IDs);
generatebigboutonplot(summaries,'b');


subplot(4,2,2);
legend('blue-old animals','green-new mouse','red-not sure');

figure;
IDsold=[46 16 18 39 48 35];
IDsnew=[43 42 44 45 50 ];
IDsnotsure=[29 30 51 10 11 14];
IDs=[IDsold IDsnew];
summaries=db.summaries(IDs);
generatebigboutonplot(summaries,'b');

%%%%%%%%%%%%%%%%%%%%%%%%%%
% type 1
%%%%%%%%%%%%%%%%%%%%%%%%%%



figure;
title('all Type 1 big bouton');

load -mat d:\axons\control.db
IDs=[13 16 3]; %12 14 only small boutons

summaries=db.summaries(IDs);
generatebigboutonplot(summaries,'r');

load -mat d:\axons\lwah016.db




IDsold=[1  2 4 5 7 13 14 15 17 31 40 53 54 55]; % 8 33 34 47 3 72 73 only small boutons
IDsnotsure=[12 10 11];
IDs=[IDsold IDsnotsure];
summaries=db.summaries(IDs);
generatebigboutonplot(summaries,'b');
subplot(4,2,2);
legend('blue-deprived','green-out of barrel','red-control');


figure;
title('all Type 1');

load -mat d:\axons\control.db
IDs=[13 14 16 3 12 ];% only small boutons

summaries=db.summaries(IDs);
generateallboutonplot(summaries,'r');

load -mat d:\axons\lwah016.db
IDsold=[1 2 3 4 5 7 13 14 15 17 31 40 47 8 33 34 53 54 55];% only small boutons  72 73 
IDsnotsure=[12];
IDs=[IDsold IDsnotsure];
summaries=db.summaries(IDs);
generateallboutonplot(summaries,'b');
subplot(4,2,2);
legend('blue-deprived','green-out of barrel','red-control');

figure;
title('terminal Type 1');

load -mat d:\axons\control.db
IDs=[14 16];% only small boutons

summaries=db.summaries(IDs);
generateterminalboutonplot(summaries,'r');

load -mat d:\axons\lwah016.db
IDsold=[1 2 3 4 5 7 13 14 15 17 31 40 47 8 33 34 72 73 53 54 55];% only small boutons
IDsnotsure=[12];
IDs=[IDsold IDsnotsure];
summaries=db.summaries(IDs);
generateterminalboutonplot(summaries,'b');
subplot(4,2,2);
legend('blue-deprived','green-out of barrel','red-control');

figure;
title('all Type 2');

load -mat d:\axons\control.db
IDs=[23 24];% only small boutons

summaries=db.summaries(IDs);
generateallboutonplot(summaries,'r');

load -mat d:\axons\lwah016.db
IDsold=[56 58 61 62 63 64 66 68 69 70 71];% only small boutons
IDsnotsure=[9];
IDs=[IDsold IDsnotsure];
summaries=db.summaries(IDs);
generateallboutonplot(summaries,'b');
subplot(4,2,2);
legend('blue-deprived','green-out of barrel','red-control');

figure;
title('terminal Type 2 ');
load -mat d:\axons\control.db
IDs=[23 24];% only small boutons

summaries=db.summaries(IDs);
generateterminalboutonplot(summaries,'r');

load -mat d:\axons\lwah016.db
IDsold=[56 58 61 62 63 64 66 68 69 70 71];% only small boutons
IDsnotsure=[9];
IDs=[IDsold IDsnotsure];
summaries=db.summaries(IDs);
generateterminalboutonplot(summaries,'b');
subplot(1,2,2);
legend('blue-deprived','green-out of barrel','red-control');








%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
