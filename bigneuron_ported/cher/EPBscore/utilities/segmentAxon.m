function segmentation = segmentAxon(voxel,imageArray,segmentation)

%fix the neurite so it coincides with the maximal intensity in the perpendicular plane

maxx=size(imageArray,2);
maxy=size(imageArray,1);
maxz=size(imageArray,3);

arraysize=size(voxel,2);

%iterate this process

maxx=size(imageArray,2);
maxy=size(imageArray,1);
maxz=size(imageArray,3);

%find maxint as center of line
x=round(min(max(voxel(2,:)-1,0),maxx-1));
y=round(min(max(voxel(1,:)-1,0),maxy-1));
z=round(min(max(voxel(3,:)-1,0),maxz-1));

%fix the neurite so it coincides with the maximal intensity in the perpendicular plane

%calculate the instantaneous slope in X-Y plane use neighborhood of 5 pixels to each side
sizeofaxon=size(voxel,2);

%calculate the normal direction of the axon based on 3 pixels on
%each side
oldvoxel=voxel;
if (sizeofaxon>7)
    %        smoothing again to futher suppress jumps
    voxel(1,:)=smooth(voxel(1,:),7)';
    voxel(2,:)=smooth(voxel(2,:),7)';
    voxel(3,:)=smooth(voxel(3,:),7)';
end
neighborhood=14;
if (sizeofaxon>neighborhood)
    dy=([voxel(1,1)*ones(1,neighborhood) voxel(1,1:sizeofaxon-neighborhood)]-[voxel(1,neighborhood+1:sizeofaxon) voxel(1,sizeofaxon)*ones(1,neighborhood)]);
    dx=([voxel(2,1)*ones(1,neighborhood) voxel(2,1:sizeofaxon-neighborhood)]-[voxel(2,neighborhood+1:sizeofaxon) voxel(2,sizeofaxon)*ones(1,neighborhood)]);
else
    dx=ones(1,sizeofaxon)*(voxel(1,1:sizeofaxon)-voxel(1,1));
    dy=ones(1,sizeofaxon)*(voxel(2,1:sizeofaxon)-voxel(2,1));
end
dr=sqrt(dx.*dx+dy.*dy);
%    change length of 0 to 1 to avoid division by zero
dr(find(dr==0))=1;
%    normalize to unit vector length
dx=dx./dr;
dy=dy./dr;



plane=[];
line=[];
neighborhood=20;
height=5;
inds=[];
zs=[];
composite=[];
for j=-height:height   %five focal section up and three plane down
    plane=[];
    for k=1:neighborhood*2+1  %crosssection; neighborhood pixels away on each side
        crossdx=-(k-neighborhood-1).*dy;
        crossdy=(k-neighborhood-1).*dx;
        x=round(min(max(voxel(2,:)-1+crossdx,0),maxx-1));
        y=round(min(max(voxel(1,:)-1+crossdy,0),maxy-1));
        z=round(min(max(voxel(3,:)-1+j,0),maxz-1));
        plane=[plane; imageArray(y+x*maxy+z*maxy*maxx+1)];
    end
    composite=[composite;mean(plane')];
    
   
end
%create indexed composite profile
cutoff=composite>(max(max(composite))+min(min(composite)))/2;


seg1=zeros(size(segmentation));
seg2=zeros(size(segmentation));
for j=-height:height   %five focal section up and three plane down
    plane=[];
    for k=1:neighborhood*2+1  %crosssection; neighborhood pixels away on each side
        crossdx=-(k-neighborhood-1).*dy;
        crossdy=(k-neighborhood-1).*dx;
        xlag=round(min(max(voxel(2,:)-1+crossdx-dy,0),maxx-1));
        ylag=round(min(max(voxel(1,:)-1+crossdy-dx,0),maxy-1));
        zlag=round(min(max(voxel(3,:)-1+j,0),maxz-1));
        x=round(min(max(voxel(2,:)-1+crossdx,0),maxx-1));
        y=round(min(max(voxel(1,:)-1+crossdy,0),maxy-1));
        z=round(min(max(voxel(3,:)-1+j,0),maxz-1));
        xadv=round(min(max(voxel(2,:)-1+crossdx+dy,0),maxx-1));
        yadv=round(min(max(voxel(1,:)-1+crossdy+dx,0),maxy-1));
        zadv=round(min(max(voxel(3,:)-1+j,0),maxz-1));
        segmentation(ylag+xlag*maxy+zlag*maxy*maxx+1)=max(segmentation(ylag+xlag*maxy+zlag*maxy*maxx+1),composite(j+height+1,k));
        segmentation(y+x*maxy+z*maxy*maxx+1)=max(segmentation(y+x*maxy+z*maxy*maxx+1),composite(j+height+1,k));
        segmentation(yadv+xadv*maxy+zadv*maxy*maxx+1)=max(segmentation(yadv+xadv*maxy+zadv*maxy*maxx+1),composite(j+height+1,k));
    end
end


