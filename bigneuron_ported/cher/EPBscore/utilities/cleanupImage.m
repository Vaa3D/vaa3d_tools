function copy1=cleanupImage(inputImage,row,col)
% Fixes distortions from Confoc.

copy1=inputImage;
copy2=copy1;
imgSize=size(inputImage);
totalRows=imgSize(1);
totalCols=imgSize(2);

%rows first
copy1(1:totalRows-row,:)=copy2(row+1:totalRows,:);
copy1(totalRows-row+1:totalRows,:)=copy2(1:row,:);

% now copy1 is corrected in Y direction so copy it again./
copy2=copy1;

%cols second
copy1(:,totalCols-col+1:totalCols)=copy2(:,1:col);
copy1(:,1:totalCols-col)=copy2(:,col+1:totalCols);



