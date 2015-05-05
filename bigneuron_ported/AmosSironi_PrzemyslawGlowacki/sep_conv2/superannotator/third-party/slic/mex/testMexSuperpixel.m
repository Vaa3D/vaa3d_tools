addpath /home/cjb/matlab/
addpath ../

img = readMultiPageTiff('/data/phd/twoexamples/00147.tif');


%% cluster
%[res, idxs] = mex_supervoxel(img, 10, 40);
[res, numlabels] = slic_supervoxel(img, 6, 10);

%% Test centroid search
if 1
    centroids = slic_findcentroids( res, numlabels );
    
    % IMPORTANT: convert to matlab array-indexing (add 1)
    centroids = centroids + 1;
end

%% Plot voxels for given idxs
if 1
    idxList = [5698+2];%, 5408];
    colorList{1} = 'r';
    colorList{2} = 'g';
    colorList{3} = 'b';
    colorList{4} = 'k';
    %idxList = 12;
    
    figure;
    hold on;
    
    alpha = 1;
    for I=1:length(idxList)
        idx = idxList(I);
        [x,y,z]=ind2sub( size(res), find( res(:) == idx ) );
        pts = [x,y,z];
        for J=1:size(pts,1);
            voxel( pts(J,:) - [1 1 1]/2, [1 1 1], colorList{I}, alpha );
        end
        
        % plot real centroid and medoid
        ctr = mean(pts);
        med = centroids(:, idx+1);
        
        plot3( ctr(1), ctr(2), ctr(3), 'b+');
        %plot3( med(1), med(2), med(3), 'g+');
        
        voxel( double(med)' - [1 1 1]/2, [1 1 1], 'g', alpha );
    end
    
    grid on;
    axis square;
    
end

%% try neighbor search
if 1
    neighbors = slic_findneighbors( res, numlabels );
    
    % find random one
    idx = 1000;
    
    nList = neighbors{idx+1};
    
    % find min and max z of current one
    [x,y,z] = ind2sub( size(res),  find( res(:) == idx ) );
    
    minZ = min(z) - 2;
    maxZ = max(z) + 2;
    
    for Z=minZ:maxZ
        clf;
    
        cutImg = img(:,:,Z);
        cutSeg = res(:,:,Z);
        
        toKeep = [];
        toKeep{1} = find(cutSeg(:) == idx);
        for I=1:length(nList)
            toKeep{I+1} = find(cutSeg(:) == nList(I));
        end
        
        cutSegOrig = cutSeg;
        cutSeg(:) = 0;
        for I=1:length(toKeep)
            cutSeg(toKeep{I}) = I;
        end
        
        % make sure that label2rgb generates the same color mappings every
        % time
        for I=1:length(toKeep)
            cutSeg(I+1) = I;
        end

        cutImg = squeeze(cutImg);
        cutSeg = squeeze(cutSeg);

        CMAP = label2rgb(cutSeg, 'lines', 'k');
        %CMAP = cut;        
        subplot(211);
        imshow(CMAP);
        %imshow(CMAP);

        subplot(212);
        z = imseg( cutImg, cutSeg );
        imshow( z/255.0 );

        pause;
    end
end

%%
%img = img * 255;
for I=60:size(img,3)
    clf;
    
    cutImg = img(:,:,I);
    cutSeg = res(:,:,I);
    
    cutImg = squeeze(cutImg);
    cutSeg = squeeze(cutSeg);
    
    CMAP = label2rgb(cutSeg, 'lines', 'c'); 
    %CMAP = cut;
    subplot(311);
    imshow(cutImg);
    subplot(312);
    imshow(CMAP);

    subplot(313);
    z = imseg( cutImg, cutSeg );
    imshow( z );
    
    pause;
end