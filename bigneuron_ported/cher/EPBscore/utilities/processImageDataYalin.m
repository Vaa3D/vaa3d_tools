function [xout,n,meanImg,stdImg,SEM]=processImageDataYalin(image,threshold,meanN,stdN)
% This is the function that will read in an image and dispaly a histogram of
% the pixels as well as the rescaled the histogram.
% Ti then looks for only thjose pixel intensities greater than 5 std above the mean and
% displays this image.

sizeImg=size(image);
bins=12;

if length(sizeImg) > 2
    error('processImageDataYalin: Only works on 2 D data');
end
pixels=sizeImg(1)*sizeImg(2);

if nargin < 2
    threshold = 5;
	meanN=[];
	stdN=[];
end
if ~strcmp(class(image),'double')    % Not a double...
    class=class(image);
    image=double(image);
else
    class='double';
end

rowOfImg=reshape(image,pixels,1);   % MAde a columns vector
rowOfImg(isnan(rowOfImg))=0;	%Remove NANs

maxImg=max(rowOfImg);
rowOfImg;
[n,xout]=hist(rowOfImg,bins);
if isempty(meanN)
	meanImg=mean(rowOfImg);
else
	meanImg=meanN;
end

if isempty(stdN)
	stdImg=std(rowOfImg);
else
	stdImg=stdN
end
meanImg=mean(rowOfImg);

SEM=stdImg/pixels;

% Look for pixels Greater than 5 SEM above the mean
r=find(rowOfImg < meanImg+threshold*stdImg);
rowOfImg(r) = 0;
rowOfImg = reshape(rowOfImg,sizeImg(1),sizeImg(2));
eval(['rowOfImg = ' class '(rowOfImg);']);
figure('DoubleBuffer','On','NumberTitle','off','Name','Pixel Histogram and Image with Noise Removed',...
    'Color','white','pos',[106   507   868   389]);
cmap = [ .9 0 .9;gray(255)]; 
colormap(cmap);
bar(xout,n);
hold on;
lims=ylim;
xlims=xlim;
line([meanImg+threshold*stdImg meanImg+threshold*stdImg],lims);
text(meanImg+threshold*stdImg+.05*(xlims(2)-xlims(1)),lims(2)-.05*(lims(2)-lims(1)),...
    ['Mean + ' num2str(threshold) '*SD = ' num2str(meanImg+threshold*stdImg)],'FontName','TimesNewRoman');
title('Pixel Histogram','FontName','TimesNewRoman');
set(get(gca,'YLabel'),'String','Number Of Pixels','FontName','TimesNewRoman');
set(get(gca,'XLabel'),'String','Intensity (a.u.)','FontName','TimesNewRoman');
cmap = [ .9 0 .9;gray(255)]; 
colormap(cmap);
figure('DoubleBuffer','On','NumberTitle','off','Name','Pixel Histogram and Image with Noise Removed',...
    'Color','white','pos',[106   507   868   389]);
cmap = [ .9 0 .9;gray(255)]; 
colormap(cmap);

% subplot(1,3,2);
imagesc(rowOfImg);
title(['Image with Intensities > ' num2str(threshold) '  SD above Mean'],'FontName','TimesNewRoman');
% subplot(1,3,3);
figure('DoubleBuffer','On','NumberTitle','off','Name','Pixel Histogram and Image with Noise Removed',...
    'Color','white','pos',[106   507   868   389]);
imagesc(medfilt2(rowOfImg,[3 3]));
cmap = [ .9 0 .9;gray(255)]; 
colormap(cmap);
title(['Image with Intensities > ' num2str(threshold) '  SD above Mean'],'FontName','TimesNewRoman');
figure('DoubleBuffer','On','NumberTitle','off','Name','Pixel Histogram and Image with Noise Removed',...
    'Color','white');
cmap = [ .9 0 .9;gray(255)]; 
colormap(cmap);

bar(xout,n);
cmap = [ .9 0 .9;gray(255)]; 
colormap(cmap);





