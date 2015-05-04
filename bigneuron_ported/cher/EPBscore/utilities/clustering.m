clustering

load -mat d:\axons\clustering.db
branchnum=[2  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  1  0  0  2  0  0  2  3  0  0  0  0  0  1  2  0  1  0  0  0  1  0  0  0  0  0  0  0  0  0  1  0  0  0  1  0  0  0  0  0  0  0  0  0  0  0  0  1  0  0  0  0  0  0  0  0  0  1  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  1  0  5  0  1  2  1  0  0  1  0  0  2  0  1  0  0  2  2  2  0  0  2  0  0  0  0  0  0  0  0  0  0  3  3  3  0  0  0  0  1  0  1  1  0  0  0  0  0  0  0  0  0  0  0  1  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  1  3  0  0  0  0  1  0  0  1  1  1  0  0  1  0  1  2  2  2  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  2  1  1  0  0  1  0  0  1  0  0  1  0  1  1  0  0  0  0  1  1  0  0  0  0  0  1  0  1  0];

for i=1:size(db.dendrites,2)
    if isempty(db.dendrites(i).maxintensities)
        db.dendrites(i).maxintensities=0;
       
    end
    if isempty(db.dendrites(i).maxcubeintensities)
        db.dendrites(i).maxcubeintensities=0;
    end
    if isempty(db.dendrites(i).maxterminalintensities)
        db.dendrites(i).maxterminalintensities=0;
    end
    if isnan(db.dendrites(i).meanintensities)
        db.dendrites(i).meanintensities=0;
    end
    [maxv,ind]=max(db.dendrites(i).cube(21,:,2));
    db.dendrites(i).medianprofile=median(db.dendrites(i).cube(:,:,2),2);
    db.dendrites(i).maxprofile=mean(db.dendrites(i).cube(:,max(1,ind-5):min(size(db.dendrites(i).cube,2),ind+5),2),2);
    db.dendrites(i).meanbackcube=median(mean(db.dendrites(i).cube([15:20 22:27],:,2)));
    db.dendrites(i).background=median(mean(db.dendrites(i).cube([1:10 31:41],:,2)));
    db.dendrites(i).caliber=db.dendrites(i).meanbackcube/(db.dendrites(i).background-1);
    db.dendrites(i).maxcombinedintensities=max([db.dendrites(i).maxintensities db.dendrites(i).maxterminalintensities]);
    db.dendrites(i).maxterminalboutonlen=max([db.spines(db.dendrites(i).spineuids).len]);
    db.dendrites(i).branchnum=branchnum(i);
    db.dendrites(i).ibd=1/db.dendrites(i).ibs;
    db.dendrites(i).itbd=1/db.dendrites(i).itbs;
    db.dendrites(i).boutonnum=size(db.dendrites(i).positions);
    db.dendrites(i).terminalboutonnum=size(db.dendrites(i).terminalpositions);
end

% link up axons which belong together
axonnumber=[1    2   -3    1    4    5   -3    6    5    7    8    9   10   11   12   13   14   15   15   15   16   17   17   17   17   17   17   17   17   17  -18  -19  -18  -19  -19   20   20  -18   20   20   22   21   23   23   23   23   23   23   24   24   24   24   24   24   24   25   26  141   27  142   27   27   27   27   28   29   28   30   31   29   28  -32   33   33  -34  -34  -34  -35  -35  -34  140   79   80   81   81   82   83   84   85   86   87   88   89   90  -36  -52  -38   39   40   41   42   43   44   45   41   44   43   41   44   45   43   46   47   48   49   50  -47   49   50   51  -52  -36  -36  -52   53  -54   55  -56  -54   53   57  -58  -59   60   60   60   61   61   61   61   61   61   61   61   61   61   61   61   61   61   61   62   63   62   64   65   66   67   68   69   69   69   70   71  -72 -144  -72   73  -74   75   76  -77  -78  -77  -77  -78   91   92   93   94   95   96   96   97   98   96   98   99   99  100   99  100  101  102  103  100  104   99  100  105  106  107  108  105  108  109  110  110  111  110  110  112  113  113  113  114  113  113  115  116  115  117  117  117  117  120  117  120  117  120  119  121  121  124  123  118  122  122  122  122  122  122  122  122  122  122  125  125  126  125  125  125  127  128  129  130  130  130  130  130  130  130  131  132  134  134  135  134  134  136  137  138  138  138  139  139  139  139];

number=unique(abs(axonnumber));
%difference ROIs

for i=1:size(number,2)
    ibd=size([db.dendrites(abs(axonnumber)==number(i)).positions],2)/sum([db.dendrites(abs(axonnumber)==number(i)).totallength]);
    itbd=size([db.dendrites(abs(axonnumber)==number(i)).terminalpositions],2)/sum([db.dendrites(abs(axonnumber)==number(i)).totallength]);
    maxcaliber=max([db.dendrites(abs(axonnumber)==number(i)).caliber]);
    maxbouton=max([db.dendrites(abs(axonnumber)==number(i)).maxcombinedintensities]);
    maxterminalbouton=max([db.dendrites(abs(axonnumber)==number(i)).maxterminalintensities]);
    maxterminalboutonlen=max([db.dendrites(abs(axonnumber)==number(i)).maxterminalboutonlen]);
    [db.dendrites(abs(axonnumber)==number(i)).combinedibd]=deal(ibd);
    [db.dendrites(abs(axonnumber)==number(i)).combineditbd]=deal(itbd);
    [db.dendrites(abs(axonnumber)==number(i)).combinedmaxcaliber]=deal(maxcaliber);
    [db.dendrites(abs(axonnumber)==number(i)).combinedmaxbouton]=deal(maxbouton);
    [db.dendrites(abs(axonnumber)==number(i)).combinedmaxterminalbouton]=deal(maxterminalbouton);
    [db.dendrites(abs(axonnumber)==number(i)).combinedmaxterminalboutonlen]=deal(maxterminalboutonlen);
    [db.dendrites(abs(axonnumber)==number(i)).totalbranch]=deal(sum([db.dendrites(abs(axonnumber)==number(i)).branchnum]));
end

type=[3 -1 -3  3  1  1 -3  1 -1 -3  1 -1  4  4 -1  1  1  2  2  2  2  3  3  3  3  3  3  3  3  3  1  1  1  1  1  3  3  1  3  3  1  2  3  3  3  3  3  3  3  3  3  3  3  3  3  1  2  2  1  1  1  1  1  1  1  1  1  4  1  1  1 -1  2  2  1  3  3  1  3  3  1  1  1  1  1  4  1  1  1  1  1  1  1  1  3  1  2  1  3  1  2 -1  1  2  1  1  1  1  1  2  1  1  1  2  1  1  1  1  1  1 -1 -3 -3 -3  2 -3  1  2 -3  2  4  3  1  3  3  3  3  3  3  3  3  3  3  3  3  3  3  3  3  3  3  2  4  2  1  1  1  1  1  2  2  2  1  1  3  4  3  1  1 -3  1  3  3  3  3  3  4  1  1  1  1  3  3  3  3  3  3  3  3  3  3  3  3  3  3  3  3  3  3  3  3  3  3  3  3  3  3  3  4  3  3  4  3  3  3  3  3  3  3  4  3  2  2  2  2  3  2  3  2  3  2  2  2  4 -2  4 -2  2  2  2  2  2  2  2  2  2  3  3  3  3  3  3  3  3  3  3  3  3  3  3  3  3  3  2  2  2  3  2  2  3  3  3  3  3  3  3  3  3];
type=abs(type);
figure;
title('Type 2 graph');
plot([db.dendrites(type==2).combineditbd],[db.dendrites(type==2).combinedibd],'g.');
hold on;
plot([db.dendrites(type==1).combineditbd],[db.dendrites(type==1).combinedibd],'b.');
hold on;
plot([db.dendrites(type==3).combineditbd],[db.dendrites(type==3).combinedibd],'r.');
legend('type 2','type 1','type 3');
xlabel('terminal bouton density (1/um)');
ylabel('enpassant bouton density (1/um)');

figure;
title('terminal bouton density');
hist([db.dendrites.combineditbd],20);
xlabel('terminal bouton density (1/um)');
ylabel('segments');



figure;
title('Type 3 graph');
totalbranch=[db.dendrites.totalbranch];
plot([db.dendrites((type==3) & (totalbranch==0)).maxcombinedintensities],[db.dendrites((type==3) & (totalbranch==0)).caliber],'r.');
hold on;
plot([db.dendrites((type==3) & (totalbranch>0)).maxcombinedintensities],[db.dendrites((type==3) & (totalbranch>0)).caliber],'g.');
plot([db.dendrites(type==1).maxcombinedintensities],[db.dendrites(type==1).caliber],'b.');
legend('type 3 no branch','type 3 with branch','type 1');
xlabel('maximal bouton intensity');
ylabel('maximal caliber');



figure;
title('Type 3 graph');
totalbranch=  [db.dendrites.totalbranch];
plot([db.dendrites((type==3) & (totalbranch==0)).combinedmaxbouton],[db.dendrites((type==3) & (totalbranch==0)).combinedmaxcaliber],'r.');
hold on;
plot([db.dendrites((type==3) & (totalbranch>0)).combinedmaxbouton],[db.dendrites((type==3) & (totalbranch>0)).combinedmaxcaliber],'g.');
plot([db.dendrites(type==1).combinedmaxbouton],[db.dendrites(type==1).combinedmaxcaliber],'b.');
legend('type 3 no branch','type 3 with branch','type 1');
xlabel('maximal bouton intensity');
ylabel('maximal caliber');

figure;
title('terminal bouton density');
hist([db.dendrites.combineditbd],20);
xlabel('terminal bouton density (1/um)');
ylabel('segments');

figure;
title('maximal bouton intensity');
hist([db.dendrites.combinedmaxbouton],20);
xlabel('maximal bouton intensity (backbone units)');
ylabel('segments');

figure;
title('maximal caliber');
hist([db.dendrites.combinedmaxcaliber],20);
xlabel('maximal caliber (background units)');
ylabel('segments');

analyzed=[59   61   62   63   64   68   69   71   72    3    7    9   10   15   32   37   38   39   42   22   23   24   25   26   27   28   30   29   76   77   78  168  165  166  172  174  170  171  176  213  214  218  219    1    2  167  215  216  217    6  131  140  141  107  108  109   95  121  122  123  124  132  147  148   73   74  133  134  135  136  137  138  139];





1+([db.dendrites.combineditbd]>0.06)'*1 ...
+([db.dendrites.combineditbd]<0.06)'.*(([db.dendrites.totalbranch]>0)'|([db.dendrites.combinedmaxbouton]>10)'|([db.dendrites.combinedmaxcaliber]>5)')*2 ...
+([db.dendrites.combineditbd]<0.06)'.*([db.dendrites.totalbranch]==0)'.*([db.dendrites.combinedmaxbouton]<4)'.*([db.dendrites.combinedmaxcaliber]>5)'


% same ROI






maxintensities=[db.dendrites.maxintensities];
maxcubeintensities=[db.dendrites.maxcubeintensities];
backbone=[db.dendrites.meanback];
backbonecube=[db.dendrites.meanbackcube];
maxterminalintensities=[db.dendrites.maxterminalintensities];
meanintensities=[db.dendrites.meanintensities];
maxcombinedintensities=max([maxintensities';maxterminalintensities'],1);
background=[db.dendrites.background];

type=[1  1  1  1  1  1  1  1  1  4  1  4  1  1  2  1  3  3  3  3  3  3  3  3  3  1  1  1  1  3  1  1  1  2 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1  1  2  1  2  1  1  1  1  1  1  1  1  4  1  1  4  3  3  1  1  1  1  1  3  1  1  1  1  1  4  1  1  1  1  1  1  1  1  1  1  1  1  3  1  2  1  1  2  1  1  1  1  1  2  1  1  1  2  1  1  1  1  1  1  3  3  3  3  2  1  1  2  1  2  4  3  1  3  3  3  3  3  3  3  3  3  3  3  3  3  2  1  2  1  1  1  1  1  2  2  2  1  1  3  3  3  1  1  1  1  3  3  3  3  3  4  1  1  3  3  1  1  1  1  1  1  3  3  3  3  3  1  1  1  3  1  1  3  3  1  3  1  1  1  1  1  1  4  1  1  4  0  3  3  3  3  3  3  4  3  2  2  2  3  2  3  2  3  2  2  2  2  4  2  4  2  2  2  2  2  2  2  2  2  2  3  3  3  3  3  3  3  3 -3 -3 -3 -3 -3 -3 -3 -3 -3 -3 -3 -3 -3 -3 -3 -3 -3 -3 -3 -3 -3 -3 -3 -3];
number=[-1    0    0    2   -1    0    2    0    0    0    3    0    0   46  -47  -48    4    4    4    4    4    4    4    4    4    0   26   49    0  -43    0   44   45   46    0    0    0    0    0    0    0    0    0    0    0    0    0    0   50   51   50   52   51   51   51   51    0    0   53  -54   53   53  -55   56  -56  -57  -57  -57  -57  -57  -58   59   13   14   15   15    0   16   17   18   19   20    0   21   22   23   24  -25    0    0   24   28   27   29   30   24   29   27   24   29   30   27   31   32   28   33   34  -32   33   34   35  -36  -37  -37  -36   38  -39   40  -41  -39   38  -60  -61  -62   63   63   63   64   64   64   65   65   65   65   65   65   65   66   67   66   68   69   70   70   70   71   71   71    6    0   -7   -7   -7    8   -9   10    0  -11  -12  -11  -11  -12    0    0    0    0    0   72   72   73   74   72   75   76   76   77   77   77   78   79   80   77   81   82   83   84   85   86   85   87   85   88   89   89    0   89   89    0    0   90   90   91   90   90   92    0   92   93   93   93  -94  -95  -94  -95  -94  -96   97  -98  -98    0  -98    0  -98  -98  -98  -98  -98  -98  -99  -98  -98  -98  101  101  100  100  100  100  102  100    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0];

type=type.*(sign(number));

figure;
hold on;
plot(maxcubeintensities(type==1)./maxintensities(type==1),itbd(type==1),'g.');
plot(maxcubeintensities(type==2)./maxintensities(type==2),itbd(type==2),'r.');
plot(maxcubeintensities(type==3)./maxintensities(type==3),itbd(type==3),'b.');

figure;
hold on;
plot(backbone(type==1)./backbonecube(type==1),itbd(type==1),'g.');
plot(backbone(type==2)./backbonecube(type==2),itbd(type==2),'r.');
plot(backbone(type==3)./backbonecube(type==3),itbd(type==3),'b.');


figure;
hold on;
plot(backbonecube(type==1)./background(type==1),itbd(type==1),'g.');
plot(backbonecube(type==2)./background(type==2),itbd(type==2),'r.');
plot(backbonecube(type==3)./background(type==3),itbd(type==3),'b.');



figure;
hold on;
plot(backbone(type==1),itbd(type==1),'g.');
plot(backbone(type==2),itbd(type==2),'r.');
plot(backbone(type==3),itbd(type==3),'b.');

figure;
hold on;
plot(background(type==1),itbd(type==1),'g.');
plot(background(type==2),itbd(type==2),'r.');
plot(background(type==3),itbd(type==3),'b.');

figure;
hold on;
plot(backbone(type==1),ibd(type==1),'g.');
plot(backbone(type==2),ibd(type==2),'r.');
plot(backbone(type==3),ibd(type==3),'b.');


figure;
hold on;
plot(backbone(type==1).*maxintensities(type==1),itbd(type==1),'g.');
plot(backbone(type==2).*maxintensities(type==2),itbd(type==2),'r.');
plot(backbone(type==3).*maxintensities(type==3),itbd(type==3),'b.');


figure;
hold on;
plot(backbone(type==1),itbd(type==1),'g.');
plot(backbone(type==2),itbd(type==2),'r.');
plot(backbone(type==3),itbd(type==3),'b.');


figure;
hold on;
plot(meanintensities(type==1),itbd(type==1),'g.');
plot(meanintensities(type==2),itbd(type==2),'r.');
plot(meanintensities(type==3),itbd(type==3),'b.');

figure;
hold on;
plot(meanintensities(type==1),maxintensities(type==1),'g.');
plot(meanintensities(type==2),maxintensities(type==2),'r.');
plot(meanintensities(type==3),maxintensities(type==3),'b.');


% classifications
meas1=[[itbd(type==2) itbd(type==1) itbd(type==3)]' ];
%meas=[[backbonecube(type==2)./backbone(type==2) backbonecube(type==1)./backbone(type==1) backbonecube(type==3)./backbone(type==3)]'];
%meas=[[[db.dendrites(type==1).maxcombinedintensities] [db.dendrites(type==3).maxcombinedintensities]]' [[db.dendrites(type==1).axonthickness] [db.dendrites(type==3).axonthickness]]'];
%meas=[[[db.dendrites(type==1).maxcombinedintensities] [db.dendrites(type==3).maxcombinedintensities]]' ];
meas=[[db.dendrites(type==1).axonthickness] [db.dendrites(type==3).axonthickness]]';
%meas=[[db.dendrites(type==1).meanback] [db.dendrites(type==3).meanback]]';
%meas=[[[db.dendrites(type==1).maxcombinedintensities] [db.dendrites(type==3).maxcombinedintensities]]' ]./[[db.dendrites(type==1).axonthickness] [db.dendrites(type==3).axonthickness]]';
types=[char(ones(sum(type==1),1)*65)' char(ones(sum(type==3),1)*66)']';
t = treefit(meas, types);%, 'splitcriterion', 'twoing' );  % create decision tree 
t = TREEPRUNE(t,'level',4);
sfit = treeval(t,meas);      % find assigned class numbers
sfit = t.classname(sfit);    % get class names
mean(strcmp(sfit,types))   % compute proportion correctly classified

