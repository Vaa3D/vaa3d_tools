function [maxint,cube] = getbackboneintensity(voxel,imageArray,dx,dy,neighborhood,height,average)
% fix the neurite so it coincides with the maximal intensity in the perpendicular plane

maxx=size(imageArray,2);
maxy=size(imageArray,1);
maxz=size(imageArray,3);


x=round(min(max(voxel(2,:)-1,0),maxx-1));
y=round(min(max(voxel(1,:)-1,0),maxy-1));
z=round(min(max(voxel(3,:)-1,0),maxz-1));
maxint=imageArray(y+x*maxy+z*maxy*maxx+1);



plane=[];
line=[];
cube=[];
% 
% inds=[];
% zs=[];
% dx=double(dx);
% dy=double(dy);
% for j=-height:height   %three focal section up and three plane down
%     plane=[];
%     for k=1:neighborhood*2+1  %crosssection; neighborhood pixels away on each side
%         crossdx=-(k-neighborhood-1).*dy;
%         crossdy=(k-neighborhood-1).*dx;
%         x=round(min(max(voxel(2,:)-1+crossdx,0),maxx-1));
%         y=round(min(max(voxel(1,:)-1+crossdy,0),maxy-1));
%         z=round(min(max(voxel(3,:)-1+j,0),maxz-1));
%         plane=[plane; imageArray(y+x*maxy+z*maxy*maxx+1)];
%     end
%     [maxinplane,ind]=max(plane,[],1);
%     meanplane=mean(plane,1);
%     cube(:,:,j+height+1)=plane;
%     
%     %    figure;
%     %    plot(mean(plane,2));
%     %    std(mean(plane,2))
%     if average
%         line=[line;meanplane];
%     else
%         line=[line; maxinplane]; %obj.data.filteredArray(y+x*maxy+z*maxy*maxx+1)];
%     end
%     inds=[inds; ind];
%     zs=[zs; z];
% end
% 
% 
% x=round(min(max(voxel(2,:)-1,0),maxx-1));
% y=round(min(max(voxel(1,:)-1,0),maxy-1));
% z=round(min(max(voxel(3,:)-1,0),maxz-1));
% maxint=imageArray(y+x*maxy+z*maxy*maxx+1);
