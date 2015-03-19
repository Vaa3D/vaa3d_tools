function [SeedMap,propsSeed,CCList] = get_frag_sta_3D(prob_img,CC)

% get the statistics of 3D filament fractions

%[SeedMap,propsSeed,h_seed,psz_seed]


% get the properties of each individual new added componnent

sz_I = size(prob_img);


s = regionprops(CC,prob_img,'Area','MeanIntensity','PixelList','Centroid');

SeedMap = zeros(sz_I);


for is = 1 : CC.NumObjects
    
    area_s(is) = length(CC.PixelIdxList{is});
    
end

% sort the region according to their area in a descend way

if(CC.NumObjects > 0)
    
    [nArea,order] = sort(area_s,'descend');
    
    propsSeed = zeros(CC.NumObjects,8);
        
    for is = 1 : length(s)
        
        sidx = order(is);
        
        pList = CC.PixelIdxList{sidx};
        
        CCList{is} = CC.PixelIdxList{sidx};
        
        if(nArea(is) > 2)
            
            cov_p = cov(s(sidx).PixelList);
            
            ellipsoid_r = eig(cov_p);
            
            ellipsoid_r = abs(ellipsoid_r);
            
            ellipsoid_r = sort(ellipsoid_r,'descend');
            
        else
            
            ellipsoid_r = [1 1 1]';
            
            
        end
        
        
        SeedMap(pList) = is;
        
        propsSeed(is,1) = s(sidx).Area;
        
        propsSeed(is,2) = ellipsoid_r(1);
        
        propsSeed(is,3) = ellipsoid_r(2);
        
        propsSeed(is,4) = ellipsoid_r(3);
        
        propsSeed(is,5) = s(sidx).MeanIntensity;
        
        propsSeed(is,6) = s(sidx).Centroid(2);
        
        propsSeed(is,7) = s(sidx).Centroid(1);
        
        propsSeed(is,8) = s(sidx).Centroid(3);
        
        
    end
    
else
    
    propsSeed = [];
    
    CCList = [];
    
end

