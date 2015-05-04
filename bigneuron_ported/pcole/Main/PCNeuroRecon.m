clear
clc
tic

% ---------------------------------------------------------------------------------------------
% ---------------------------------------------------------------------------------------------
% User Input:
% Directory search path containing data sets, with stack name and image format

fileBasePath = '/Users/petercole/Desktop/PCThesisBB/PCThesisBB/PCThesis';
stackID = 'OP_1';
imageFormat = '.tif';

% ---------------------------------------------------------------------------------------------
% ---------------------------------------------------------------------------------------------
%Load images of type imageFormat from directory

directory =  sprintf('%s/ImageStacks/OlfactoryProjectionFibers/Stacks/%s', fileBasePath, stackID);
directoryFormat = sprintf('/*%s*', imageFormat);
images = dir([directory directoryFormat]);

%Add each image to a cell within imgCell - type cell array data structure
for i = 1 : length(images) 
   filename = [directory '/' images(i).name];
   im = imread(filename);
    if imageFormat == '.jpg';
        im = rgb2gray(im);
    end
   imgCell{i} = im;
   nameCell{i} = images(i).name;
end

% ---------------------------------------------------------------------------------------------
%Pre-process 2D images within image stack - adaptiveThreshold and cannyEdge

for i = 1:length(imgCell) 
    selectedImage = imgCell{i};

    %Apply adaptivethreshold function to indexed image
    thresholdImage = adaptiveThreshold(selectedImage,13,0.001,0);

    %Muliply original by binary mask to keep only important information
    thresh = (selectedImage .* uint8(thresholdImage));
    
    %Apply canny threshoold algorithm for edge detection
    cannyThreshImage = edge(thresh,'canny',0.13);
    
    %write thresholded images to tif stack/*
    %imwrite(cannyThreshImage, sprintf('%s', nameCell{i}));
    
%     subplot(length(imgCell),2,i*2);
%     imshow(cannyThreshImage);
%     title(sprintf('cannyThresh%s', nameCell{i}));
   
end

% ---------------------------------------------------------------------------------------------
%Process stack of pre-processed tif files into volumetric voxel matrix "volumeMatrix"

fileBase = sprintf('%s/ImageStacks/OlfactoryProjectionFibers/Stacks/%s_CannyThreshStack/', fileBasePath,stackID);
startFrame = 1;
endFrame = length(images);

%read frames and build volume matrix
for i = startFrame : endFrame
    fileName = [fileBase, num2str(i,'%3d'),'.tif'];
    volumeMatrix(:,:,i)=imread(fileName);
end

voxelData = smooth3(volumeMatrix);

% ---------------------------------------------------------------------------------------------
%Calculate isosurface parameters and render surface
figure()
hold on

[faces,verticies] = isosurface(voxelData,0,'verbose');
p = patch(isosurface(voxelData,0,'verbose'));
set(p,'facecolor','w','edgecolor','none');

%scale in the z direcion to compensate for slice thickness
zscale = 3;                         
daspect([1,1,zscale])

%Properties of figure
box off
light('position',[1,1,1])
light('position',[-1,-1,-1])
alpha(0.46);
%alpha(0.00);

%Properties of axes
axis on
set(gca,'color',[0 0 0])
sizeSlice = size(voxelData);
set(gca,'xlim',[0 sizeSlice(2)], 'ylim',[0 sizeSlice(1)], 'color', [0 0 0])
set(gcf, 'Color', [1 1 1]);

%Properties of view
az = -45;
el = 37;
view(az,el)

% ---------------------------------------------------------------------------------------------
%Calculate and plot centerline

%Calculate faces, vertices structure by isosurface function
fvStruct = isosurface(voxelData,0,'verbose');

%Determine voxels of the volume, using polygon2voxel function, within the surface defined by isosurface
%faces and vertices for proper skeletonization
fvVolume = polygon2voxel(fvStruct, [512 512 endFrame]);

%Skeletonize voxel data using parallel medial axis thinning technique
skelVolume = Skeleton3D(fvVolume);

d=size(skelVolume,1);
e=size(skelVolume,2);
f=size(skelVolume,3);

[xSkel,ySkel,zSkel]=ind2sub([d,e,f],find(skelVolume(:)));
skelPoints = [xSkel,ySkel,zSkel];
plot3(ySkel,xSkel,zSkel,'square','Markersize',3,'MarkerFaceColor','r','Color','r');
set(gca,'color',[0 0 0])
set(gcf, 'Color', [1 1 1]);

% ---------------------------------------------------------------------------------------------
%Detect and plot branch points by 3D convolution
skelVolumeDouble = double(skelVolume);
kernel = ones([7 7 7]);
convImage = convn(skelVolumeDouble,kernel,'same');
convImageMult = convImage .* skelVolumeDouble;

a=size(convImageMult,1);
b=size(convImageMult,2);
c=size(convImageMult,3);

[xBranch,yBranch,zBranch]=ind2sub([a,b,c],find(convImageMult > 10));
plot3(yBranch,xBranch,zBranch,'square','Markersize',4,'MarkerFaceColor','g','Color','g');
set(gca,'color',[0 0 0])
set(gcf, 'Color', [1 1 1]);

hold off
toc