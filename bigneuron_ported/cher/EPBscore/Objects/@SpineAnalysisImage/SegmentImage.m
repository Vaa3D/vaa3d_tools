function [totalpos, totalfalsepos, totalneg, totalfalseneg,singlearea,area]=test(startimage,endimage)
load filterbank.mat;
filterbank=reshape(filterbank,[size(filterbank,1)*size(filterbank,2)*size(filterbank,3)*size(filterbank,4) size(filterbank,5) size(filterbank,6) size(filterbank,7)]);
load classifier.mat;
totalfalsepos=zeros(size(-1:0.05:1));
totalpos=zeros(size(-1:0.05:1));
totalfalseneg=zeros(size(-1:0.05:1));
totalneg=zeros(size(-1:0.05:1));
singlearea=[];
area=[];

for im=startimage:endimage
    load(['training_' num2str(im) '.mat']);
    training.indeximage=uint8(zeros(size(training.image)));
    indexbins=[-9.00	    -3.00	    -2.00	    -2.00	    -1.00	    -1.00	    -1.00	    -1.00	    -1.00	    -1.00	    -1.00	    -1.00	    -1.00	    -1.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     0.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     1.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     2.00	     3.00	     3.00	     3.00	     3.00	     3.00	     3.00	     3.00	     3.00	     3.00	     3.00	     3.00	     3.00	     4.00	     4.00	     4.00	     4.00	     4.00	     5.00	     5.00	     5.00	     6.00	     6.00	     6.00	     7.00	     8.00	     8.00	     9.00	    10.00	    12.00	    13.00	    15.00	    18.00	    21.00	    26.00	    32.00	    44.00	    70.00	  3243.00	  5001.00];	

    for b=1:252
        training.indeximage=training.indeximage+uint8(training.image>indexbins(b));
    end
    training.indeximage=single(training.indeximage)-single(median(training.indeximage(:)));

    output=single(zeros(size(training.image)));
    output=output(:);
    for j=1:length(parameters)
        fea=find(feature(:,j));
        if (fea>1)
            if (fea>129)
                filteredtemp=[];
                for l=1:8
                    [i,j,k]=ind2sub([1 4 4],fea-129);
                   try 
                    f=reshape(filterbank(sub2ind([8 4 4 1],l,j,k,1),:,:,:),[121,121,5]);
                    filteredtemp(:,:,:,l)=conv3fft(single(training.indeximage),f);
                    catch
                       i
                       j
                       k
                       l
                   end
                end
                filtered=max(filteredtemp,[],4);
            else
            
            
                f=reshape(filterbank(fea-1,:,:,:),[121,121,5]);
                filtered=conv3fft(single(training.indeximage),f);
            end
        else
            filtered=training.image;
            
        end
        filtered=filtered(:);
        %bin it
        [his binned]=histc(filtered,bins(fea,:));
        binned(binned==0)=1;
        if fea==1
            outputsingle=(log(classhistpos(binned,j))-log(classhistmin(binned,j)));
        end
        output=output+parameters(j)*(log(classhistpos(binned,j))-log(classhistmin(binned,j)));
    end
    output=output/length(parameters);
    output=reshape(output,size(training.image));
    outputsingle=reshape(outputsingle,size(training.image));
    training.rawoutput=output;
    training.rawoutputsingle=outputsingle;
    
    outputsingle=(outputsingle>=0).*outputsingle/max(outputsingle(:))-(outputsingle<0).*outputsingle/min(outputsingle(:));
    
%     binstest=min(outputsingle(:)):0.1:max(outputsingle(:));
%    
%     histo=histc(outputsingle(:),binstest);    
%     a=cumsum(histo);
%     binstest=[];
%     for i=0:250
%       ind=find(a>=i*max(a)/250);
%       binstest=[binstest ind(1)];
%     end
%     binstest=[binstest length(a)]+min(outputsingle(:));
%     
%      % preprocessing
%     indeximage=uint8(zeros(size(outputsingle)));
%     for b=1:252
%        indeximage=indeximage+uint8(outputsingle>binstest(b));
%     end
%     outputsingle=(single(indeximage)+0.5)/127-1;
%     
     training.outputsingle=outputsingle;
    
    fp=[];
    fn=[];
    for i=-1:0.05:1
        p=outputsingle>=i;
        fp=[fp sum(p(training.target)==0)];
        fn=[fn sum(p(~training.target)==1)];
    end
    totalfalsepos=totalfalsepos+fp;
    totalpos=totalpos+sum(training.target(:));
    totalfalseneg=totalfalseneg+fn;
    totalneg=totalneg+sum(~training.target(:));
    fp=fp/sum(training.target(:));
    fn=fn/sum(~training.target(:));
    training.fpsingle=fp;
    training.fnsingle=fn;
    training.singlearea=sum((fn(1:end-1)+fn(2:end)).*diff(fp))/2;
    singlearea=[singlearea training.singlearea];
   

    
%       figure;
 %      imagesc(collapse(output,'XY'));
 
 
 
 
 
 % histogram equalize

    binstest=min(output(:)):0.1:0;
    histo=zeros(size(binstest,1),1);
    histo=histo+histc(output(:),binstest);    
    a=cumsum(histo);
    binstest=[];
    for i=0:125
      ind=find(a>=i*max(a)/125);
      binstest=[binstest ind(1)];
    end
    binstest1=min(output(:))*(1-binstest/length(a));

    binstest=0:0.1:max(output(:));
    histo=zeros(size(binstest,1),1);
    histo=histo+histc(output(:),binstest);    
    a=cumsum(histo);
    binstest=[];
    for i=0:125
      ind=find(a>=i*max(a)/125);
      binstest=[binstest ind(1)];
    end
    binstest=[binstest1 max(output(:))*(binstest/length(a))];
    
     % preprocessing
    indeximage=uint8(zeros(size(output)));
    for b=1:252
       indeximage=indeximage+uint8(output>binstest(b));
    end
    output=(single(indeximage)+0.5)/127-1;
    
    filteredoutput=zeros(size(output));
    for z=1:size(output,3)
%        filteredoutput(:,:,z)=imdilate(output(:,:,z),se);
        filteredoutput(:,:,z)=medfilt2(output(:,:,z),[7 7]);
    end
    training.output=filteredoutput;
    
    %generate ROC curve
    %load training_1.mat;
    fp=[];
    fn=[];
    for i=-1:0.05:1
        p=filteredoutput>i;
        fp=[fp sum(p(training.target)==0)];
        fn=[fn sum(p(~training.target)==1)];
    end
    totalfalsepos=totalfalsepos+fp;
    totalpos=totalpos+sum(training.target(:));
    totalfalseneg=totalfalseneg+fn;
    totalneg=totalneg+sum(~training.target(:));
    fp=fp/sum(training.target(:));
    fn=fn/sum(~training.target(:));
    training.classifier={};
    training.classifier.parameters=parameters;
    training.classifier.classhistpos=classhistpos;
    training.classifier.classhistmin=classhistmin;
    training.classifier.feature=feature;
    
    training.fp=fp;
    training.fn=fn;
    training.area=sum((training.fn(1:end-1)+training.fn(2:end)).*diff(training.fp))/2;
    area=[area training.area];
    
    

    figure;
    imagesc(collapse(training.output,'XY'));
    output=training.rawoutput;
    fnsingle=training.fnsingle;
    fpsingle=training.fpsingle;
    outputsingle=training.outputsingle;

    save(['training_' num2str(im) '.mat'], 'training');
    
    save(['testing_' num2str(im) '.mat'], 'fp','fn','fpsingle','fnsingle', 'output', 'filteredoutput','outputsingle');
end
figure;
plot(totalfalsepos./totalpos,totalfalseneg./totalneg);
figure;
plot(area);
hold on;
plot(singlearea,'r');
figure;
plot(singlearea./area);
% 
%  filteredoutput=zeros(size(output));
% for z=1:size(output,3)
%     filteredoutput(:,:,z)=medfilt2(output(:,:,z),[7 7]);
% end